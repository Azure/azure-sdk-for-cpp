// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs/producer_client.hpp"

#include "azure/messaging/eventhubs/event_data_batch.hpp"
#include "azure/messaging/eventhubs/eventhubs_exception.hpp"
#include "private/eventhubs_utilities.hpp"
#include "private/retry_operation.hpp"

#include <azure/core/amqp.hpp>

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
            connectionString);

    m_credential = sasCredential;

    m_eventHub
        = (sasCredential->GetEntityPath().empty() ? eventHub : sasCredential->GetEntityPath());
    m_fullyQualifiedNamespace = sasCredential->GetHostName();
  }

  ProducerClient::ProducerClient(
      std::string const& fullyQualifiedNamespace,
      std::string const& eventHub,
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
      Azure::Messaging::EventHubs::ProducerClientOptions options)
      : m_fullyQualifiedNamespace{fullyQualifiedNamespace}, m_eventHub{eventHub},
        m_credential{credential}, m_producerClientOptions(options)
  {
  }

  void ProducerClient::EnsureSession(std::string const& partitionId = {})
  {
    if (m_sessions.find(partitionId) == m_sessions.end())
    {
      Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
      connectOptions.ContainerId = m_producerClientOptions.ApplicationID;
      connectOptions.EnableTrace = true;
      connectOptions.AuthenticationScopes = {"https://eventhubs.azure.net/.default"};

      // Set the UserAgent related properties on this message sender.
      _detail::EventHubsUtilities::SetUserAgent(
          connectOptions, m_producerClientOptions.ApplicationID);

      std::string fullyQualifiedNamespace{m_fullyQualifiedNamespace};

      Azure::Core::Amqp::_internal::Connection connection(
          fullyQualifiedNamespace, m_credential, connectOptions);

      Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
      sessionOptions.InitialIncomingWindowSize = std::numeric_limits<int32_t>::max();
      sessionOptions.InitialOutgoingWindowSize = std::numeric_limits<uint16_t>::max();

      Azure::Core::Amqp::_internal::Session session{connection.CreateSession(sessionOptions)};
      m_sessions.emplace(partitionId, session);
    }
  }

  Azure::Core::Amqp::_internal::Session ProducerClient::GetSession(std::string const& partitionId)
  {
    return m_sessions.at(partitionId);
  }

  void ProducerClient::EnsureSender(
      std::string const& partitionId,
      Azure::Core::Context const& context)
  {
    if (m_senders.find(partitionId) == m_senders.end())
    {
      m_targetUrl = "amqps://" + m_fullyQualifiedNamespace + "/" + m_eventHub;

      EnsureSession(partitionId);

      std::string targetUrl = m_targetUrl;

      if (!partitionId.empty())
      {
        targetUrl += "/Partitions/" + partitionId;
      }

      Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;
      senderOptions.Name = m_producerClientOptions.Name;
      senderOptions.EnableTrace = true;
      senderOptions.MaxMessageSize = m_producerClientOptions.MaxMessageSize;

      Azure::Core::Amqp::_internal::MessageSender sender
          = GetSession(partitionId).CreateMessageSender(targetUrl, senderOptions, nullptr);
      sender.Open(context);
      m_senders.emplace(partitionId, sender);
    }
  }
  Azure::Core::Amqp::_internal::MessageSender ProducerClient::GetSender(
      std::string const& partitionId)
  {
    return m_senders.at(partitionId);
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
    if (!batch.TryAddMessage(eventData))
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
      if (!batch.TryAddMessage(data))
      {
        throw std::runtime_error("Could not add message to batch.");
      }
    }
    Send(batch, context);
  }

  Models::EventHubProperties ProducerClient::GetEventHubProperties(Core::Context const& context)
  {
    // EventHub properties are not associated with a particular partition, so create a message
    // sender on the empty partition.
    EnsureSession();

    return _detail::EventHubsUtilities::GetEventHubsProperties(GetSession(), m_eventHub, context);
  }

  Models::EventHubPartitionProperties ProducerClient::GetPartitionProperties(
      std::string const& partitionId,
      Core::Context const& context)
  {
    EnsureSession(partitionId);

    return _detail::EventHubsUtilities::GetEventHubsPartitionProperties(
        GetSession(partitionId), m_eventHub, partitionId, context);
  }
}}} // namespace Azure::Messaging::EventHubs
