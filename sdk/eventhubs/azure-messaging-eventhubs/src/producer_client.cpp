// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "azure/messaging/eventhubs/producer_client.hpp"

#include <azure/core/amqp.hpp>

Azure::Messaging::EventHubs::ProducerClient::ProducerClient(
    std::string const& connectionString,
    std::string const& eventHub,
    ProducerClientOptions options)
    : m_credentials{connectionString, "", eventHub}, m_producerClientOptions(options)
{
  m_credentials.SasCredential
      = std::make_shared<Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>(
          m_credentials.ConnectionString);
  m_credentials.EventHub
      = (m_credentials.SasCredential->GetEntityPath().empty()
             ? m_credentials.EventHub
             : m_credentials.SasCredential->GetEntityPath());
  m_credentials.FullyQualifiedNamespace = m_credentials.SasCredential->GetHostName();
}

Azure::Messaging::EventHubs::ProducerClient::ProducerClient(
    std::string const& fullyQualifiedNamespace,
    std::string const& eventHub,
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
    ProducerClientOptions options)
    : m_credentials{"", fullyQualifiedNamespace, eventHub, "", credential},
      m_producerClientOptions(options)
{
}

Azure::Core::Amqp::_internal::MessageSender Azure::Messaging::EventHubs::ProducerClient::GetSender(
    std::string const& partitionId)
{
  if (m_senders.find(partitionId) == m_senders.end())
  {
    CreateSender(partitionId);
  }

  auto sender = m_senders.at(partitionId);
  return sender;
}

void Azure::Messaging::EventHubs::ProducerClient::CreateSender(std::string const& partitionId)
{
  m_credentials.TargetUrl
      = "amqps://" + m_credentials.FullyQualifiedNamespace + "/" + m_credentials.EventHub;

  Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
  connectOptions.ContainerId = m_producerClientOptions.ApplicationID;
  connectOptions.EnableTrace = m_producerClientOptions.SenderOptions.EnableTrace;
  std::string hostName;
  if (m_credentials.SasCredential != nullptr)
  {
    connectOptions.Port = m_credentials.SasCredential->GetPort();
    hostName = m_credentials.SasCredential->GetHostName();
  }

  auto targetUrl = m_credentials.TargetUrl;

  if (!partitionId.empty())
  {
    targetUrl += "/Partitions/" + partitionId;
  }

  Azure::Core::Amqp::_internal::Connection connection(hostName, connectOptions);
  Azure::Core::Amqp::_internal::Session session;

  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  sessionOptions.InitialIncomingWindowSize = std::numeric_limits<int32_t>::max();
  sessionOptions.InitialOutgoingWindowSize = std::numeric_limits<uint16_t>::max();

  if (m_credentials.SasCredential == nullptr)
  {
    session = Azure::Core::Amqp::_internal::Session(connection, m_credentials.Credential);
    auto senderOptions = m_producerClientOptions.SenderOptions;
    Azure::Core::Amqp::_internal::MessageSender sender
        = Azure::Core::Amqp::_internal::MessageSender(session, targetUrl, senderOptions, nullptr);
    sender.Open();

    m_senders.insert_or_assign(partitionId, sender);
  }
  else
  {
    session = Azure::Core::Amqp::_internal::Session(connection, m_credentials.SasCredential);

    Azure::Core::Amqp::_internal::MessageSender sender
        = Azure::Core::Amqp::_internal::MessageSender(
            session, targetUrl, m_producerClientOptions.SenderOptions, nullptr);
    sender.Open();
    m_senders.insert_or_assign(partitionId, sender);
  }
}

bool const Azure::Messaging::EventHubs::ProducerClient::SendEventDataBatch(
    EventDataBatch& eventDataBatch,
    Azure::Core::Context ctx)
{
  auto messages = eventDataBatch.GetMessages();
  Azure::Core::Amqp::Models::AmqpMessage message;
  message.SetBody(Azure::Core::Amqp::Models::AmqpBinaryData{'H', 'e', 'l', 'l', 'o', '7'});

  Azure::Messaging::EventHubs::_internal::RetryOperation retryOp(
      m_producerClientOptions.RetryOptions);
  return retryOp.Execute([&]() -> bool {
    auto result = GetSender(eventDataBatch.GetPartitionID()).Send(messages[0], ctx);
    return std::get<0>(result) == Azure::Core::Amqp::_internal::MessageSendStatus::Ok;
  });
}

/* Azure::Core::Amqp::_internal::Management
Azure::Messaging::EventHubs::ProducerClient::CreateManagement(std::string name, bool eventHub)
{
  (void)eventHub;

  std::string manageUrl = "amqps://"+m_credentials.FullyQualifiedNamespace+"$management";
  Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
  connectOptions.EnableTrace = true;
  Azure::Core::Amqp::_internal::Connection connection(manageUrl, connectOptions);
  //Azure::Core::Amqp::_internal::Session session(connection);
  //Azure::Core::Amqp::_internal::Management managementClient(session, name, {});
  return nullptr;
  //managementClient;
}*/