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
    auto sasCredential
        = std::make_shared<Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>(
            connectionString, eventHub);

    m_credential = sasCredential;

    m_eventHub
        = (sasCredential->GetEntityPath().empty() ? eventHub : sasCredential->GetEntityPath());
    m_fullyQualifiedNamespace = sasCredential->GetHostName();
    std::string serviceScheme = _detail::EventHubsServiceScheme;
    if (sasCredential->UseDevelopmentEmulator())
    {
      serviceScheme = _detail::EventHubsServiceScheme_Emulator;
      m_targetPort = Azure::Core::Amqp::_internal::AmqpPort; // When using the emulator, use the
                                                             // non-TLS endpoint by default.
    }
    m_targetUrl = serviceScheme + m_fullyQualifiedNamespace + ":"
        + std::to_string(sasCredential->GetPort()) + "/" + m_eventHub;
  }

  ProducerClient::ProducerClient(
      std::string const& fullyQualifiedNamespace,
      std::string const& eventHub,
      std::shared_ptr<Azure::Core::Credentials::TokenCredential const> credential,
      Azure::Messaging::EventHubs::ProducerClientOptions options)
      : m_fullyQualifiedNamespace{fullyQualifiedNamespace}, m_eventHub{eventHub},
        m_targetUrl{_detail::EventHubsServiceScheme + m_fullyQualifiedNamespace + "/" + m_eventHub},
        m_credential{credential}, m_producerClientOptions(options)
  {
  }

  EventDataBatch ProducerClient::CreateBatch(
      EventDataBatchOptions const& options,
      Core::Context const& context)
  {
    EnsureSender(options.PartitionId, context);

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
    retryOp.Execute([&]() -> bool {
      auto result = GetSender(eventDataBatch.GetPartitionId()).Send(message, context);
      auto sendStatus = std::get<0>(result);
      if (sendStatus == Azure::Core::Amqp::_internal::MessageSendStatus::Ok)
      {
        return true;
      }
      // Throw an exception about the error we just received.
      throw Azure::Messaging::EventHubs::_detail::EventHubsExceptionFactory::
          CreateEventHubsException(std::get<1>(result));
    });
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

  Azure::Core::Amqp::_internal::Connection ProducerClient::CreateConnection() const
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

    return Azure::Core::Amqp::_internal::Connection{
        m_fullyQualifiedNamespace, m_credential, connectOptions};
  }
  void ProducerClient::EnsureConnection(std::string const& partitionId)
  {
    std::unique_lock<std::recursive_mutex> lock(m_sessionsLock);
    if (m_connections.find(partitionId) == m_connections.end())
    {
      m_connections.emplace(partitionId, CreateConnection());
    }
  }

  void ProducerClient::EnsureSession(std::string const& partitionId)
  {
    // Ensure that a connection has been created for this producer.
    EnsureConnection(partitionId);

    // Ensure that a session has been created for this partition.
    std::unique_lock<std::recursive_mutex> lock(m_sessionsLock);
    if (m_sessions.find(partitionId) == m_sessions.end())
    {
      m_sessions.emplace(partitionId, CreateSession(partitionId));
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
      EnsureSession(partitionId);

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
          = GetSession(partitionId).CreateMessageSender(targetUrl, senderOptions, nullptr);
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
  Azure::Core::Amqp::_internal::MessageSender ProducerClient::GetSender(
      std::string const& partitionId)
  {
    return m_senders.at(partitionId);
  }

  Azure::Core::Amqp::_internal::Session ProducerClient::CreateSession(
      std::string const& partitionId)
  {
    Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
    sessionOptions.InitialIncomingWindowSize = (std::numeric_limits<int32_t>::max)();
    sessionOptions.InitialOutgoingWindowSize = (std::numeric_limits<uint16_t>::max)();
    return m_connections.at(partitionId).CreateSession(sessionOptions);
  }
  std::shared_ptr<_detail::EventHubsPropertiesClient> ProducerClient::GetPropertiesClient()
  {
    std::lock_guard<std::mutex> lock(m_propertiesClientLock);
    EnsureConnection({});
    if (!m_propertiesClient)
    {
      m_propertiesClient
          = std::make_shared<_detail::EventHubsPropertiesClient>(m_connections.at(""), m_eventHub);
    }
    return m_propertiesClient;
  }

  Models::EventHubProperties ProducerClient::GetEventHubProperties(Core::Context const& context)
  {
    return GetPropertiesClient()->GetEventHubsProperties(m_eventHub, context);
  }

  Models::EventHubPartitionProperties ProducerClient::GetPartitionProperties(
      std::string const& partitionId,
      Core::Context const& context)
  {
    return GetPropertiesClient()->GetEventHubsPartitionProperties(m_eventHub, partitionId, context);
  }
}}} // namespace Azure::Messaging::EventHubs
