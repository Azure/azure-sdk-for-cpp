// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "azure/messaging/eventhubs/producer_client.hpp"
#include <azure/core/amqp.hpp>

Azure::Messaging::EventHubs::ProducerClient::ProducerClient(
    std::string const& connectionString,
    std::string const& eventHub,
    ProducerClientOptions options)
    : m_credentials{connectionString, "",eventHub},
      m_producerClientOptions(options)
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
    : m_credentials{"", fullyQualifiedNamespace, eventHub, "",credential},
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
  connectOptions.HostName = m_credentials.FullyQualifiedNamespace;

  auto targetUrl = m_credentials.TargetUrl;

  if (!partitionId.empty())
  {
    targetUrl += "/Partitions/" + partitionId;
  }

  Azure::Core::Amqp::_internal::Connection connection(
      targetUrl, connectOptions);
  Azure::Core::Amqp::_internal::Session session(connection);
  session.SetIncomingWindow(std::numeric_limits<int32_t>::max());
  session.SetOutgoingWindow(std::numeric_limits<uint16_t>::max());
    
  if (m_credentials.SasCredential == nullptr)
  {
    auto senderOptions = m_producerClientOptions.SenderOptions;
    if (senderOptions.AuthenticationScopes.empty())
    {
      senderOptions.AuthenticationScopes = {m_defaultAuthScope};
    }
    Azure::Core::Amqp::_internal::MessageSender sender
        = Azure::Core::Amqp::_internal::MessageSender(
        session, m_credentials.Credential, targetUrl, senderOptions, nullptr);
    sender.Open();
    m_senders.insert_or_assign(partitionId, sender);
  }
  else
  {
    Azure::Core::Amqp::_internal::MessageSender sender
        = Azure::Core::Amqp::_internal::MessageSender(
        session, m_credentials.SasCredential, targetUrl, m_producerClientOptions.SenderOptions, nullptr); 
    sender.Open();
    m_senders.insert_or_assign(partitionId, sender);
  }
}

bool const Azure::Messaging::EventHubs::ProducerClient::
    SendEventDataBatch(EventDataBatch& eventDataBatch, Azure::Core::Context ctx)
{
  
  auto messages = eventDataBatch.GetMessages();
  Azure::Core::Amqp::Models::AmqpMessage message;
  message.SetBody(Azure::Core::Amqp::Models::AmqpBinaryData{'H', 'e', 'l', 'l', 'o','7'});

  Azure::Messaging::EventHubs::_internal::RetryOperation retryOp(m_producerClientOptions.RetryOptions);
  return retryOp.Execute([&]() -> bool {
    auto result = GetSender(eventDataBatch.GetPartitionID()).Send(messages[0], ctx);
    return std::get<0>(result) == Azure::Core::Amqp::_internal::MessageSendResult::Ok;
  });
}

void Azure::Messaging::EventHubs::ProducerClient::CreateManagement(std::string name, bool eventHub)
{
  (void)eventHub;
  std::string manageUrl = 
       "amqps://" + m_credentials.FullyQualifiedNamespace + "/" + m_credentials.EventHub;

  Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
  connectOptions.ContainerId = m_producerClientOptions.ApplicationID;
  connectOptions.EnableTrace = m_producerClientOptions.SenderOptions.EnableTrace;
  connectOptions.HostName = m_credentials.FullyQualifiedNamespace;
   

  Azure::Core::Amqp::_internal::Connection connection(manageUrl, connectOptions);
  Azure::Core::Amqp::_internal::Session session(connection);
  session.SetIncomingWindow(std::numeric_limits<int32_t>::max());
  session.SetOutgoingWindow(std::numeric_limits<uint16_t>::max());

  Azure::Core::Amqp::_internal::ManagementOptions manageOptions;
  manageOptions.EnableTrace = m_producerClientOptions.SenderOptions.EnableTrace;

  Azure::Core::Amqp::_internal::Management managementClient(session, name, manageOptions);
 // m_management = managementClient; 
}