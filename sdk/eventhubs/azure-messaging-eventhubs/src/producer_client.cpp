// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs/producer_client.hpp"

#include "azure/messaging/eventhubs/event_data_batch.hpp"
#include "azure/messaging/eventhubs/eventhubs_exception.hpp"
#include "private/eventhubs_constants.hpp"
#include "private/eventhubs_utilities.hpp"
#include "private/retry_operation.hpp"

#include <azure/core/amqp.hpp>
#include <azure/core/amqp/internal/claims_based_security.hpp>
#include <azure/core/amqp/internal/message_sender.hpp>
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;
namespace {
const std::string DefaultAuthScope = "https://eventhubs.azure.net/.default";
}

namespace Azure { namespace Messaging { namespace EventHubs {

  ProducerClient::ProducerClient(
      std::string const& connectionString,
      std::string const& eventHub,
      Azure::Messaging::EventHubs::ProducerClientOptions options)
      : m_connectionString{connectionString}, m_eventHub{eventHub}, m_producerClientOptions(options)
  {
    auto details
        = _detail::EventHubsUtilities::CreateConnectionStringDetails(connectionString, eventHub);
    m_credential = std::move(details.Credential);
    m_eventHub = std::move(details.EventHub);
    m_fullyQualifiedNamespace = std::move(details.FullyQualifiedNamespace);
    m_targetPort = details.Port;
    m_targetUrl = details.ServiceScheme + m_fullyQualifiedNamespace + ":"
        + std::to_string(m_targetPort) + "/" + m_eventHub;
  }

  ProducerClient::ProducerClient(
      std::string const& fullyQualifiedNamespace,
      std::string const& eventHub,
      std::shared_ptr<const Azure::Core::Credentials::TokenCredential> credential,
      Azure::Messaging::EventHubs::ProducerClientOptions options)
      : m_fullyQualifiedNamespace{fullyQualifiedNamespace}, m_eventHub{eventHub},
        m_targetUrl{_detail::EventHubsServiceScheme + m_fullyQualifiedNamespace + "/" + m_eventHub},
        m_credential{credential}, m_producerClientOptions(options)
  {
  }

  ProducerClient::~ProducerClient()
  {
    try
    {
      Close();
    }
    catch (std::exception const& ex)
    {
      Log::Stream(Logger::Level::Warning)
          << "Exception in ProducerClient::~ProducerClient(): " << ex.what();
    }
  }

  void ProducerClient::Close(Azure::Core::Context const& context)
  {
    Log::Stream(Logger::Level::Verbose) << "Close producer client.";
    // Do not close the properties client here directly. It rides the gateway connection
    // for the empty partition id, so the loop below reaches it through
    // InvalidateSender(""). Closing it first threw when the gateway link was already
    // dead from an idle detach, and left every other partition stack open.
    std::vector<std::string> partitionIds;
    {
      std::lock_guard<std::mutex> lock(m_sendersLock);
      for (auto const& sender : m_senders)
      {
        partitionIds.push_back(sender.first);
      }
    }
    {
      std::lock_guard<std::recursive_mutex> lock(m_sessionsLock);
      for (auto const& session : m_sessions)
      {
        partitionIds.push_back(session.first);
      }
      for (auto const& connection : m_connections)
      {
        partitionIds.push_back(connection.first);
      }
    }
    std::sort(partitionIds.begin(), partitionIds.end());
    partitionIds.erase(std::unique(partitionIds.begin(), partitionIds.end()), partitionIds.end());
    for (auto const& partitionId : partitionIds)
    {
      InvalidateSender(partitionId, {}, context);
    }
  }

  EventDataBatch ProducerClient::CreateBatch(
      EventDataBatchOptions const& options,
      Core::Context const& context)
  {
    // Establishing the stack resolves the host, opens the socket, negotiates TLS and runs the CBS
    // handshake, so it is the step most exposed to a transient transport failure. `Send` runs under
    // `RetryOperation`; this call did not, so a burst after an idle period lost every event whose
    // stack failed to build.
    //
    // Retry once, only for CbsOpenResult::Error - see CbsOpenFailedException. The bound is one
    // attempt because uAMQP logs the transport reason but returns no value carrying it, so `Error`
    // cannot separate a transient failure from a permanent one; do not make this a loop.
    // `EnsureSenderOrInvalidate` invalidates before it rethrows, so the retry builds a new
    // connection rather than reusing a socket a failed open may have left non-closed.
    try
    {
      EnsureSenderOrInvalidate(options.PartitionId, context);
    }
    catch (Azure::Core::Amqp::_detail::CbsOpenFailedException const& cbsFailure)
    {
      // Anything that is not a retryable CBS open failure is never caught here, so it propagates
      // untouched.
      if (cbsFailure.Result != Azure::Core::Amqp::_detail::CbsOpenResult::Error
          || context.IsCancelled())
      {
        throw;
      }

      Log::Stream(Logger::Level::Warning)
          << "Could not establish the message sender for partition '"
          << (options.PartitionId.empty() ? std::string("<gateway>") : options.PartitionId)
          << "'. Building the stack again for one final attempt: " << cbsFailure.what()
          << std::endl;
      EnsureSenderOrInvalidate(options.PartitionId, context);
    }

    EventDataBatchOptions optionsToUse{options};
    if (!options.MaxBytes.HasValue())
    {
      // EnsureSender only checks whether the map holds a sender; a link the service
      // detached during an idle period stays cached. Reading the peer max message size
      // is the first call that touches that dead link, and a live test that waited past
      // the 30 minute idle detach failed exactly here. Rebuild once and read again.
      auto readMaxMessageSize = [&](std::uint64_t& observedGeneration) -> std::uint64_t {
        auto& guard = GetPartitionGuard(options.PartitionId);
        std::shared_lock<std::shared_timed_mutex> stackLock(guard.stackLock);
        observedGeneration = guard.generation.load();
        return GetSender(options.PartitionId).GetMaxMessageSize();
      };

      std::uint64_t observedGeneration = 0;
      try
      {
        optionsToUse.MaxBytes = readMaxMessageSize(observedGeneration);
      }
      catch (Azure::Core::OperationCancelledException const&)
      {
        throw;
      }
      catch (std::exception const& ex)
      {
        if (context.IsCancelled())
        {
          throw;
        }
        Log::Stream(Logger::Level::Warning)
            << "Could not read the maximum message size from the cached sender for partition '"
            << (options.PartitionId.empty() ? std::string("<gateway>") : options.PartitionId)
            << "'. Discard the stack and build it again: " << ex.what() << std::endl;
        InvalidateSender(options.PartitionId, observedGeneration, context);
        EnsureSenderOrInvalidate(options.PartitionId, context);
        std::uint64_t rebuiltGeneration = 0;
        optionsToUse.MaxBytes = readMaxMessageSize(rebuiltGeneration);
      }
    }

    return _detail::EventDataBatchFactory::CreateEventDataBatch(optionsToUse);
  }

  void ProducerClient::Send(EventDataBatch const& eventDataBatch, Core::Context const& context)
  {
    auto message = eventDataBatch.ToAmqpMessage();

    Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(
        m_producerClientOptions.RetryOptions);
    // Defense in depth: RetryOperation::Execute rethrows the last exception when retries
    // are exhausted, but if the lambda ever returns false directly the batch must not be
    // silently dropped. See issue #7130.
    auto const& partitionId = eventDataBatch.GetPartitionId();
    if (!retryOp.Execute(
            [&]() -> bool {
              EnsureSenderOrInvalidate(partitionId, context);
              std::uint64_t observedGeneration = 0;
              auto& guard = GetPartitionGuard(partitionId);
              try
              {
                // Keeps a teardown off the sender copy; sends still run together.
                std::shared_lock<std::shared_timed_mutex> stackLock(guard.stackLock);
                auto sender = GetSender(partitionId);
                observedGeneration = guard.generation.load();
                auto result = sender.Send(message, context);
#if ENABLE_UAMQP
                auto sendStatus = std::get<0>(result);
                if (sendStatus == Azure::Core::Amqp::_internal::MessageSendStatus::Ok)
                {
                  return true;
                }
                // Throw an exception about the error we just received.
                throw Azure::Messaging::EventHubs::_detail::EventHubsExceptionFactory::
                    CreateEventHubsException(std::get<1>(result));
#elif ENABLE_RUST_AMQP
                if (result)
                {
                  throw Azure::Messaging::EventHubs::_detail::EventHubsExceptionFactory::
                      CreateEventHubsException(result);
                }
                return true;
#endif
              }
              catch (Azure::Core::OperationCancelledException const&)
              {
                throw;
              }
              catch (Azure::Messaging::EventHubs::EventHubsException const& ex)
              {
                if (!context.IsCancelled() && _detail::ShouldInvalidateSender(ex))
                {
                  InvalidateSender(partitionId, observedGeneration, context);
                }
                throw;
              }
              catch (std::exception const&)
              {
                if (!context.IsCancelled())
                {
                  InvalidateSender(partitionId, observedGeneration, context);
                }
                throw;
              }
            },
            context))
    {
      std::string failureDetail = "ProducerClient::Send failed after exhausting "
          + std::to_string(m_producerClientOptions.RetryOptions.MaxRetries)
          + " retry attempts (partition='"
          + (partitionId.empty() ? std::string("<gateway>") : partitionId)
          + "'). The underlying send returned false without throwing; no further "
            "diagnostic detail is available from this layer.";
      Azure::Messaging::EventHubs::EventHubsException ex(failureDetail);
      ex.ErrorCondition = "eventhubs:client:retries-exhausted";
      ex.IsTransient = true;
      throw ex;
    }
  }

  void ProducerClient::Send(Models::EventData const& eventData, Core::Context const& context)
  {
    auto batch = CreateBatch(EventDataBatchOptions{}, context);
    if (!batch.TryAdd(eventData))
    {
      throw std::runtime_error("Could not add message to batch.");
    }
    Send(batch, context);
  }

  void ProducerClient::Send(
      std::vector<Models::EventData> const& eventData,
      Core::Context const& context)
  {
    auto batch = CreateBatch(EventDataBatchOptions{}, context);
    for (const auto& data : eventData)
    {
      if (!batch.TryAdd(data))
      {
        throw std::runtime_error("Could not add message to batch.");
      }
    }
    Send(batch, context);
  }

  Azure::Core::Amqp::_internal::Connection ProducerClient::CreateConnection(
      Azure::Core::Context const& context) const
  {
    Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
    connectOptions.ContainerId = m_producerClientOptions.ApplicationID;
    connectOptions.EnableTrace = _detail::EnableAmqpTrace;
    connectOptions.AuthenticationScopes = {"https://eventhubs.azure.net/.default"};
    connectOptions.Port = m_targetPort;

    // Set the UserAgent related properties on this message sender.
    _detail::EventHubsUtilities::SetUserAgent(
        connectOptions,
        m_producerClientOptions.ApplicationID,
        m_producerClientOptions.CppStandardVersion);

    auto connection{Azure::Core::Amqp::_internal::Connection{
        m_fullyQualifiedNamespace, m_credential, connectOptions}};

#if ENABLE_RUST_AMQP
    connection.Open(context);
#endif
    return connection;
    (void)context;
  }
  void ProducerClient::EnsureConnection(
      std::string const& partitionId,
      Azure::Core::Context const& context)
  {
    std::unique_lock<std::recursive_mutex> lock(m_sessionsLock);
    if (m_connections.find(partitionId) == m_connections.end())
    {
      m_connections.emplace(partitionId, CreateConnection(context));
    }
  }

  void ProducerClient::EnsureSession(
      std::string const& partitionId,
      Azure::Core::Context const& context)
  {
    // Ensure that a connection has been created for this producer.
    EnsureConnection(partitionId, context);

    // Ensure that a session has been created for this partition.
    std::unique_lock<std::recursive_mutex> lock(m_sessionsLock);
    if (m_sessions.find(partitionId) == m_sessions.end())
    {
      m_sessions.emplace(partitionId, CreateSession(partitionId, context));
    }
  }

  Azure::Core::Amqp::_internal::Session ProducerClient::GetSession(std::string const& partitionId)
  {
    std::unique_lock<std::recursive_mutex> lock(m_sessionsLock);
    return m_sessions.at(partitionId);
  }

  void ProducerClient::EnsureSender(
      std::string const& partitionId,
      Azure::Core::Context const& context)
  {
    std::unique_lock<std::mutex> lock(m_sendersLock);
    if (m_senders.find(partitionId) == m_senders.end())
    {
      EnsureSession(partitionId, context);

      std::string targetUrl{m_targetUrl};
      if (!partitionId.empty())
      {
        targetUrl += "/Partitions/" + partitionId;
      }

      Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;
      senderOptions.Name = m_producerClientOptions.Name;
      senderOptions.EnableTrace = _detail::EnableAmqpTrace;
      senderOptions.MaxMessageSize = m_producerClientOptions.MaxMessageSize;

      Azure::Core::Amqp::_internal::MessageSender sender
          = GetSession(partitionId).CreateMessageSender(targetUrl, senderOptions);
      auto openResult{sender.Open(context)};
      if (openResult)
      {
        Azure::Core::Diagnostics::_internal::Log::Stream(
            Azure::Core::Diagnostics::Logger::Level::Error)
            << "Failed to create message sender: " << openResult;
        throw Azure::Messaging::EventHubs::_detail::EventHubsExceptionFactory::
            CreateEventHubsException(openResult);
      }
      m_senders.emplace(partitionId, std::move(sender));
      GetPartitionGuard(partitionId).generation.fetch_add(1);
    }
  }
  void ProducerClient::EnsureSenderOrInvalidate(
      std::string const& partitionId,
      Azure::Core::Context const& context)
  {
    // A capture after the throw could remove a stack another thread rebuilt.
    auto const observedGeneration = GetPartitionGuard(partitionId).generation.load();
    try
    {
      EnsureSender(partitionId, context);
    }
    catch (Azure::Core::OperationCancelledException const&)
    {
      throw;
    }
    catch (std::exception const&)
    {
      // No exemption for AuthenticationException: on uAMQP it can mean a dead $cbs link (#7330).
      if (!context.IsCancelled())
      {
        InvalidateSender(partitionId, observedGeneration, context);
      }
      throw;
    }
  }

  Azure::Core::Amqp::_internal::MessageSender ProducerClient::GetSender(
      std::string const& partitionId)
  {
    std::unique_lock<std::mutex> lock(m_sendersLock);
    auto sender = m_senders.find(partitionId);
    if (sender == m_senders.end())
    {
      // A teardown on another thread removed this stack between the call that cached the
      // sender and this lookup. std::map::at would throw std::out_of_range here, and
      // RetryOperation::Execute does not catch that type, so the error would reach the
      // caller unspent. Throw a transient EventHubsException instead, so the next
      // attempt builds the stack again.
      EventHubsException missingSender{
          "The cached message sender for partition '" + partitionId
          + "' was removed by a teardown on another thread."};
      missingSender.IsTransient = true;
      throw missingSender;
    }
    return sender->second;
  }

  ProducerClient::PartitionGuard& ProducerClient::GetPartitionGuard(std::string const& partitionId)
  {
    std::lock_guard<std::mutex> lock(m_partitionGuardsLock);
    return m_partitionGuards[partitionId];
  }

  void ProducerClient::InvalidateSender(
      std::string const& partitionId,
      Azure::Nullable<std::uint64_t> observedGeneration,
      Azure::Core::Context const& context)
  {
    std::unique_ptr<Azure::Core::Amqp::_internal::MessageSender> sender;
    std::unique_ptr<Azure::Core::Amqp::_internal::Session> session;
    std::unique_ptr<Azure::Core::Amqp::_internal::Connection> connection;

    {
      auto& guard = GetPartitionGuard(partitionId);
      std::unique_lock<std::shared_timed_mutex> stackLock(guard.stackLock);
      if (observedGeneration.HasValue() && guard.generation.load() != observedGeneration.Value())
      {
        Log::Stream(Logger::Level::Informational)
            << "Skip the teardown for partition '"
            << (partitionId.empty() ? std::string("<gateway>") : partitionId)
            << "': the cached stack changed." << std::endl;
        return;
      }

      Log::Stream(Logger::Level::Informational)
          << "Discard the sender stack for partition '"
          << (partitionId.empty() ? std::string("<gateway>") : partitionId) << "'." << std::endl;

      std::lock_guard<std::mutex> sendersLock(m_sendersLock);
      std::lock_guard<std::recursive_mutex> sessionsLock(m_sessionsLock);
      // Test again under the map lock. EnsureSender bumps the generation under that
      // lock, not under stackLock, so a rebuild can finish between the check above and
      // here and cache a fresh stack. This second check catches that window and keeps
      // the fresh stack instead of tearing it down.
      if (observedGeneration.HasValue() && guard.generation.load() != observedGeneration.Value())
      {
        Log::Stream(Logger::Level::Informational)
            << "Skip the teardown for partition '"
            << (partitionId.empty() ? std::string("<gateway>") : partitionId)
            << "': another thread cached a new stack." << std::endl;
        return;
      }
      auto senderIterator = m_senders.find(partitionId);
      if (senderIterator != m_senders.end())
      {
        sender.reset(
            new Azure::Core::Amqp::_internal::MessageSender(std::move(senderIterator->second)));
        m_senders.erase(senderIterator);
      }
      auto sessionIterator = m_sessions.find(partitionId);
      if (sessionIterator != m_sessions.end())
      {
        session.reset(
            new Azure::Core::Amqp::_internal::Session(std::move(sessionIterator->second)));
        m_sessions.erase(sessionIterator);
      }
      auto connectionIterator = m_connections.find(partitionId);
      if (connectionIterator != m_connections.end())
      {
        connection.reset(
            new Azure::Core::Amqp::_internal::Connection(std::move(connectionIterator->second)));
        m_connections.erase(connectionIterator);
      }
      if (sender || session || connection)
      {
        guard.generation.fetch_add(1);
      }
    }

    // The network closes run outside every lock, so no send waits on them.
    if (sender)
    {
      try
      {
        sender->Close(context);
      }
      catch (std::exception const& ex)
      {
        Log::Stream(Logger::Level::Warning)
            << "Exception while closing a faulted message sender: " << ex.what() << std::endl;
      }
    }
#if ENABLE_RUST_AMQP
    if (session)
    {
      try
      {
        session->End(context);
      }
      catch (std::exception const& ex)
      {
        Log::Stream(Logger::Level::Warning)
            << "Exception while ending a faulted session: " << ex.what() << std::endl;
      }
    }
    if (connection)
    {
      try
      {
        connection->Close(context);
      }
      catch (std::exception const& ex)
      {
        Log::Stream(Logger::Level::Warning)
            << "Exception while closing a faulted connection: " << ex.what() << std::endl;
      }
    }
#endif

    // The properties client shares the gateway connection, so it dies with that connection.
    if (partitionId.empty())
    {
      std::unique_lock<std::mutex> lock(m_propertiesClientLock);
      if (m_propertiesClient)
      {
        try
        {
          m_propertiesClient->Close(context);
        }
        catch (std::exception const& ex)
        {
          Log::Stream(Logger::Level::Warning)
              << "Exception while closing the properties client: " << ex.what() << std::endl;
        }
        m_propertiesClient.reset();
      }
    }
  }

  Azure::Core::Amqp::_internal::Session ProducerClient::CreateSession(
      std::string const& partitionId,
      Azure::Core::Context const& context)
  {
    Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
    sessionOptions.InitialIncomingWindowSize = (std::numeric_limits<int32_t>::max)();
    sessionOptions.InitialOutgoingWindowSize = (std::numeric_limits<uint16_t>::max)();
    auto session{m_connections.at(partitionId).CreateSession(sessionOptions)};
#if ENABLE_RUST_AMQP
    session.Begin(context);
#endif
    return session;
    (void)context;
  }
  std::shared_ptr<_detail::EventHubsPropertiesClient> ProducerClient::GetPropertiesClient(
      Azure::Core::Context const& context)
  {
    std::lock_guard<std::mutex> lock(m_propertiesClientLock);
    // Hold this across both steps so a teardown cannot erase m_connections[""] here.
    std::lock_guard<std::recursive_mutex> sessionsLock(m_sessionsLock);
    EnsureConnection({}, context);
    if (!m_propertiesClient)
    {
      m_propertiesClient
          = std::make_shared<_detail::EventHubsPropertiesClient>(m_connections.at(""), m_eventHub);
    }
    return m_propertiesClient;
  }

  Models::EventHubProperties ProducerClient::GetEventHubProperties(Core::Context const& context)
  {
    return GetPropertiesClient(context)->GetEventHubsProperties(m_eventHub, context);
  }

  Models::EventHubPartitionProperties ProducerClient::GetPartitionProperties(
      std::string const& partitionId,
      Core::Context const& context)
  {
    return GetPropertiesClient(context)->GetEventHubsPartitionProperties(
        m_eventHub, partitionId, context);
  }
}}} // namespace Azure::Messaging::EventHubs
