// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs/producer_client.hpp"

#include "azure/messaging/eventhubs/eventhubs_exception.hpp"
#include "private/eventhubs_utilities.hpp"
#include "private/retry_operation.hpp"

#include <azure/core/amqp.hpp>

namespace {
const std::string DefaultAuthScope = "https://eventhubs.azure.net/.default";
}

Azure::Messaging::EventHubs::ProducerClient::ProducerClient(
    std::string const& connectionString,
    std::string const& eventHub,
    Azure::Messaging::EventHubs::ProducerClientOptions options)
    : m_connectionString{connectionString}, m_eventHub{eventHub}, m_producerClientOptions(options)
{
  auto sasCredential
      = std::make_shared<Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>(
          connectionString);

  m_credential = sasCredential;

  m_eventHub = (sasCredential->GetEntityPath().empty() ? eventHub : sasCredential->GetEntityPath());
  m_fullyQualifiedNamespace = sasCredential->GetHostName();
}

Azure::Messaging::EventHubs::ProducerClient::ProducerClient(
    std::string const& fullyQualifiedNamespace,
    std::string const& eventHub,
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
    Azure::Messaging::EventHubs::ProducerClientOptions options)
    : m_fullyQualifiedNamespace{fullyQualifiedNamespace}, m_eventHub{eventHub},
      m_credential{credential}, m_producerClientOptions(options)
{
}

Azure::Core::Amqp::_internal::MessageSender Azure::Messaging::EventHubs::ProducerClient::GetSender(
    std::string const& partitionId)
{
  if (m_senders.find(partitionId) == m_senders.end())
  {
    CreateSender(partitionId);
  }

  auto& sender = m_senders.at(partitionId);
  return sender;
}

void Azure::Messaging::EventHubs::ProducerClient::CreateSender(std::string const& partitionId)
{
  m_targetUrl = "amqps://" + m_fullyQualifiedNamespace + "/" + m_eventHub;

  Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
  connectOptions.ContainerId = m_producerClientOptions.ApplicationID;
  connectOptions.EnableTrace = m_producerClientOptions.VerboseLogging;
  connectOptions.AuthenticationScopes = {"https://eventhubs.azure.net/.default"};

  // Set the UserAgent related properties on this message sender.
  _detail::EventHubUtilities::SetUserAgent(connectOptions, m_producerClientOptions.ApplicationID);

  std::string hostName{m_fullyQualifiedNamespace};
  std::string targetUrl = m_targetUrl;

  if (!partitionId.empty())
  {
    targetUrl += "/Partitions/" + partitionId;
  }

  Azure::Core::Amqp::_internal::Connection connection(hostName, m_credential, connectOptions);

  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  sessionOptions.InitialIncomingWindowSize = std::numeric_limits<int32_t>::max();
  sessionOptions.InitialOutgoingWindowSize = std::numeric_limits<uint16_t>::max();

  Azure::Core::Amqp::_internal::Session session{connection.CreateSession(sessionOptions)};
  m_sessions.emplace(partitionId, session);
  Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;

  senderOptions.Name = m_producerClientOptions.Name;
  senderOptions.EnableTrace = m_producerClientOptions.VerboseLogging;
  senderOptions.MaxMessageSize = m_producerClientOptions.MaxMessageSize;
  senderOptions.SettleMode = m_producerClientOptions.SettleMode;

  Azure::Core::Amqp::_internal::MessageSender sender
      = session.CreateMessageSender(targetUrl, senderOptions, nullptr);
  sender.Open();
  m_senders.emplace(partitionId, sender);
}

bool Azure::Messaging::EventHubs::ProducerClient::SendEventDataBatch(
    EventDataBatch const& eventDataBatch,
    Core::Context const& context)
{
  auto message = eventDataBatch.ToAmqpMessage();

  Azure::Messaging::EventHubs::_detail::RetryOperation retryOp(
      m_producerClientOptions.RetryOptions);
  return retryOp.Execute([&]() -> bool {
    auto result = GetSender(eventDataBatch.GetPartitionId()).Send(message, context);
    auto sendStatus = std::get<0>(result);
    if (sendStatus == Azure::Core::Amqp::_internal::MessageSendStatus::Ok)
    {
      return true;
    }
    // Throw an exception about the error we just received.
    throw Azure::Messaging::EventHubs::EventHubsException(std::get<1>(result));
  });
}

Azure::Messaging::EventHubs::Models::EventHubProperties
Azure::Messaging::EventHubs::ProducerClient::GetEventHubProperties(Core::Context const& context)
{
  if (m_senders.find("") == m_senders.end())
  {
    CreateSender("");
  }

  return _detail::EventHubUtilities::GetEventHubsProperties(m_sessions.at(""), m_eventHub, context);
}

Azure::Messaging::EventHubs::Models::EventHubPartitionProperties
Azure::Messaging::EventHubs::ProducerClient::GetPartitionProperties(
    std::string const& partitionId,
    Core::Context const& context)
{
  if (m_senders.find(partitionId) == m_senders.end())
  {
    CreateSender(partitionId);
  }
  return _detail::EventHubUtilities::GetEventHubsPartitionProperties(
      m_sessions.at(partitionId), m_eventHub, partitionId, context);
}
