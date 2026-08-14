// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs/producer_client.hpp"

#include "azure/messaging/eventhubs/event_data_batch.hpp"
#include "azure/messaging/eventhubs/eventhubs_exception.hpp"
#include "private/eventhubs_constants.hpp"
#include "private/eventhubs_utilities.hpp"
#include "private/retry_operation.hpp"

#include <azure/core/amqp.hpp>
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
    // The properties client is not closed here. It rides the gateway connection, which is
    // the entry for the empty partition id, so the loop below reaches it through
    // InvalidateSender(""). GetPropertiesClient calls EnsureConnection({}) before it builds
    // the client, so a cached properties client always has that connection behind it, and
    // the loop always covers it. Closing it here instead threw out of the first step of
    // this method and left every partition stack open, which is the exact failure that this
    // change is about: after an idle detach the gateway link is dead, so that close throws.
    //
    // Collect the partition ids under the map locks, and then tear each stack down
    // through the same guarded path that a failed send uses. Each teardown waits for
    // the sends in flight on its partition, so this close cannot free a sender that a
    // send still uses. A null generation makes each teardown unconditional.
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
    // No retry loop wraps this call, so a caller that keeps a poisoned stack sees the same
    // failure for the life of the client. The Send(EventData) overloads come through here
    // first, which makes this path as important as the retry loop in Send.
    EnsureSenderOrInvalidate(options.PartitionId, context);

    EventDataBatchOptions optionsToUse{options};
    if (!options.MaxBytes.HasValue())
    {
      // The shared lock keeps a teardown away while this call uses the cached sender.
      // Without it, InvalidateSender can free the sender under this call on the Rust
      // transport. The sender copy stays inside the lock scope.
      auto& guard = GetPartitionGuard(options.PartitionId);
      std::shared_lock<std::shared_timed_mutex> stackLock(guard.stackLock);
      optionsToUse.MaxBytes = GetSender(options.PartitionId).GetMaxMessageSize();
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
              // Build the sender at the start of every attempt. A previous attempt that
              // failed discarded the old one, so this call rebuilds the link, and it also
              // rebuilds the session and the connection when they are gone. A cached sender
              // that the service detached can never succeed, so an attempt against it wastes
              // the retry budget.
              //
              // This call does its own invalidation, so it stays outside the try below. An
              // attach that failed left no sender for the handlers below to discard, and it
              // needs the session and the connection discarded instead.
              EnsureSenderOrInvalidate(partitionId, context);
              // The generation of the stack that this attempt uses. It feeds the
              // InvalidateSender calls below, which skip the teardown when the stack
              // changed after this attempt captured the value. Zero never matches a
              // partition that ever cached a sender, so a failure before the capture
              // tears nothing down.
              std::uint64_t observedGeneration = 0;
              auto& guard = GetPartitionGuard(partitionId);
              try
              {
                // The shared lock keeps a teardown away while this attempt uses the
                // cached sender. Sends that share the partition also hold the lock
                // shared, so they run at the same time. The sender copy stays inside
                // the lock scope, so no use of it escapes the guard.
                std::shared_lock<std::shared_timed_mutex> stackLock(guard.stackLock);
                auto sender = GetSender(partitionId);
                // Read the generation after the sender lookup. A removal cannot run
                // while this thread holds the shared lock, so this value belongs to
                // the stack that the lookup returned.
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
                // The caller cancelled the context. The link is still good, so keep it.
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
                // The Rust transport reports a dead sender with a std::runtime_error, so
                // this handler is the one that starts its rebuild.
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
      // A new stack is now current. The bump separates it from the stack before it, so
      // a teardown for a failure on the old stack skips this one. The bump runs under
      // m_sendersLock, so a lookup that finds this sender also sees the new generation.
      GetPartitionGuard(partitionId).generation.fetch_add(1);
    }
  }
  void ProducerClient::EnsureSenderOrInvalidate(
      std::string const& partitionId,
      Azure::Core::Context const& context)
  {
    // Capture the generation before the attach. When the attach fails, the handler
    // below removes the stack that the attach ran on. When another thread replaced
    // that stack between this capture and the attach, the generation differs, the
    // teardown skips, and the retry pays one more attach. That direction is safe; a
    // capture after the throw could remove a stack that another thread just rebuilt.
    auto const observedGeneration = GetPartitionGuard(partitionId).generation.load();
    try
    {
      EnsureSender(partitionId, context);
    }
    catch (Azure::Core::OperationCancelledException const&)
    {
      // The caller stopped the attach. The cached stack is not at fault, so keep it.
      throw;
    }
    catch (std::exception const&)
    {
      // A discard of the sender alone changes nothing here, because a failed attach never
      // cached one. The session and the connection are the objects that carry the fault to
      // the next attempt, so discard them.
      //
      // An AuthenticationException gets no exemption, although a healthy connection raises
      // it for a credential that failed. The type is not specific enough to trust. On
      // uAMQP, PutTokenForAudience raises it for every non-Ok CBS result, and
      // claim_based_security.cpp maps a transport fault during the put-token round trip to
      // Error or to InstanceClosed. So the same type covers a service refusal on a healthy
      // connection and a $cbs link that died mid-operation. An exemption would keep the
      // stack for the second case, and every later call would then report a misleading
      // authentication failure. The cost of this choice is one connection rebuild after a
      // credential failure, which the next call pays.
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
    // InvalidateSender erases from this map, so this read takes the lock that protects it.
    std::unique_lock<std::mutex> lock(m_sendersLock);
    auto sender = m_senders.find(partitionId);
    if (sender == m_senders.end())
    {
      // A teardown on another thread removed this stack between the call that cached the
      // sender and this lookup. std::map::at would throw std::out_of_range here, which
      // derives from std::logic_error, and RetryOperation::Execute catches only
      // EventHubsException, OperationCancelledException, and std::runtime_error. So the
      // container error would pass the retry loop with the budget unspent, and it would
      // reach the caller as a map error for a race that one more attempt corrects. A
      // transient exception makes the next attempt build the stack again.
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
      // The exclusive lock waits for the sends in flight on this partition and blocks
      // new ones. Only map operations run under it, so the block is short. The map
      // locks nest in the order that EnsureSender uses, and holding both makes the
      // removal one step: a rebuild cannot attach a new sender to the old session
      // between the sender removal and the session removal.
      auto& guard = GetPartitionGuard(partitionId);
      std::unique_lock<std::shared_timed_mutex> stackLock(guard.stackLock);
      if (observedGeneration.HasValue() && guard.generation.load() != observedGeneration.Value())
      {
        // Another thread already removed the stack that the caller saw fail, and a new
        // stack can be in place. That stack is not at fault, so keep it.
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
        // The cached stack changed, so the old generation is no longer current.
        guard.generation.fetch_add(1);
      }
    }

    // The network closes run outside every lock, so no send waits on them. No other
    // thread can reach these objects: the maps no longer hold them, and the exclusive
    // lock above waited for every send in flight. A close on a link that the service
    // already detached throws the detach error, and that is the usual case here, so
    // each close runs in its own try block and the teardown continues.
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
    // Hold the sessions lock from the ensure call through the lookup below. A teardown of
    // the gateway stack erases m_connections[""] under this lock, so a lookup outside it
    // races the erase, and a lookup between the two steps can miss and throw
    // std::out_of_range. The lock is recursive, so the ensure call can take it again. The
    // teardown path releases this lock before it takes m_propertiesClientLock, so the two
    // locks never form a cycle.
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
