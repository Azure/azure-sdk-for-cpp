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
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <stdexcept>

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
    {
      std::unique_lock<std::mutex> lock(m_propertiesClientLock);
      if (m_propertiesClient)
      {
        m_propertiesClient->Close(context);
        m_propertiesClient.reset();
      }
    }
    Log::Stream(Logger::Level::Verbose) << "Closing message senders.";
    for (auto& sender : m_senders)
    {
      sender.second.Close(context);
    }
    m_senders.clear();

#if ENABLE_RUST_AMQP
    Log::Stream(Logger::Level::Verbose) << "Closing sessions.";
    for (auto& session : m_sessions)
    {
      session.second.End(context);
    }
    Log::Stream(Logger::Level::Verbose) << "Closing connections.";
    for (auto& connection : m_connections)
    {
      connection.second.Close(context);
    }
#endif
    // Remove all the sessions and connections after they've been closed.
    m_sessions.clear();
    m_connections.clear();
  }

  EventDataBatch ProducerClient::CreateBatch(
      EventDataBatchOptions const& options,
      Core::Context const& context)
  {
    // No retry loop wraps this call, so a caller that keeps a poisoned stack sees the same
    // failure for the life of the client. The Send(EventData) overloads come through here
    // first, which makes this path as important as the retry loop in Send.
    EnsureSenderOrInvalidate(options.PartitionId, context);

    auto messageSender = GetSender(options.PartitionId);
    EventDataBatchOptions optionsToUse{options};
    if (!options.MaxBytes.HasValue())
    {
      optionsToUse.MaxBytes = messageSender.GetMaxMessageSize();
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
              try
              {
                auto result = GetSender(partitionId).Send(message, context);
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
                  InvalidateSender(partitionId, context);
                }
                throw;
              }
              catch (std::exception const&)
              {
                // The Rust transport reports a dead sender with a std::runtime_error, so
                // this handler is the one that starts its rebuild.
                if (!context.IsCancelled())
                {
                  InvalidateSender(partitionId, context);
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
    }
  }
  void ProducerClient::EnsureSenderOrInvalidate(
      std::string const& partitionId,
      Azure::Core::Context const& context)
  {
    try
    {
      EnsureSender(partitionId, context);
    }
    catch (Azure::Core::OperationCancelledException const&)
    {
      // The caller stopped the attach. The cached stack is not at fault, so keep it.
      throw;
    }
    catch (Azure::Core::Credentials::AuthenticationException const&)
    {
      // The credential failed, or the service refused the token. The connection carried
      // that answer, so the connection works. A new connection presents the same claim and
      // gets the same refusal, so keep this one. A dead connection reports itself as a
      // std::runtime_error from the CBS open instead, and the handler below discards it.
      throw;
    }
    catch (std::exception const&)
    {
      // A discard of the sender alone changes nothing here, because a failed attach never
      // cached one. The session and the connection are the objects that carry the fault to
      // the next attempt, so discard them.
      if (!context.IsCancelled())
      {
        InvalidateSender(partitionId, context);
      }
      throw;
    }
  }

  Azure::Core::Amqp::_internal::MessageSender ProducerClient::GetSender(
      std::string const& partitionId)
  {
    // InvalidateSender erases from this map, so this read takes the lock that protects it.
    std::unique_lock<std::mutex> lock(m_sendersLock);
    return m_senders.at(partitionId);
  }

  void ProducerClient::InvalidateSender(
      std::string const& partitionId,
      Azure::Core::Context const& context)
  {
    Log::Stream(Logger::Level::Informational)
        << "Discard the sender stack for partition '"
        << (partitionId.empty() ? std::string("<gateway>") : partitionId) << "'." << std::endl;

    // Take each lock in turn, and never hold two at one time. A close on a link that the
    // service already detached throws the detach error, and that is the usual case here, so
    // each close runs in its own try block and the teardown continues.
    {
      std::unique_lock<std::mutex> lock(m_sendersLock);
      auto sender = m_senders.find(partitionId);
      if (sender != m_senders.end())
      {
        try
        {
          sender->second.Close(context);
        }
        catch (std::exception const& ex)
        {
          Log::Stream(Logger::Level::Warning)
              << "Exception while closing a faulted message sender: " << ex.what() << std::endl;
        }
        m_senders.erase(sender);
      }
    }

    {
      std::unique_lock<std::recursive_mutex> lock(m_sessionsLock);
      auto session = m_sessions.find(partitionId);
      if (session != m_sessions.end())
      {
#if ENABLE_RUST_AMQP
        try
        {
          session->second.End(context);
        }
        catch (std::exception const& ex)
        {
          Log::Stream(Logger::Level::Warning)
              << "Exception while ending a faulted session: " << ex.what() << std::endl;
        }
#endif
        m_sessions.erase(session);
      }

      auto connection = m_connections.find(partitionId);
      if (connection != m_connections.end())
      {
#if ENABLE_RUST_AMQP
        try
        {
          connection->second.Close(context);
        }
        catch (std::exception const& ex)
        {
          Log::Stream(Logger::Level::Warning)
              << "Exception while closing a faulted connection: " << ex.what() << std::endl;
        }
#endif
        m_connections.erase(connection);
      }
    }

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
