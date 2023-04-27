// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "azure/messaging/eventhubs/consumer_client.hpp"
#include <azure/core/amqp.hpp>

Azure::Messaging::EventHubs::ConsumerClient::ConsumerClient(
    std::string const& connectionString,
    std::string const& eventHub,
    std::string const& consumerGroup,
    ConsumerClientOptions const& options)
    : m_credentials{connectionString, "", eventHub, consumerGroup},
      m_consumerClientOptions(std::move(options))
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

Azure::Messaging::EventHubs::ConsumerClient::ConsumerClient(
    std::string const& fullyQualifiedNamespace,
    std::string const& eventHub,
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
    std::string const& consumerGroup,
    ConsumerClientOptions const& options)
    : m_credentials{"", fullyQualifiedNamespace, eventHub, consumerGroup, credential},
      m_consumerClientOptions(options)
{
}

void Azure::Messaging::EventHubs::ConsumerClient::CreateConsumer(std::string const& partitionId)
{
  std::string suffix = !partitionId.empty() ? "/Partitions/" + partitionId : "";
  m_credentials.HostUrl = "amqps://" + m_credentials.FullyQualifiedNamespace + "/"
      + m_credentials.EventHub + "/ConsumerGroups/" + m_credentials.ConsumerGroup + suffix;

  Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
  connectOptions.ContainerId = m_consumerClientOptions.ApplicationID;
  connectOptions.EnableTrace = m_consumerClientOptions.ReceiverOptions.EnableTrace;
  connectOptions.HostName = m_credentials.FullyQualifiedNamespace;
  Azure::Core::Amqp::_internal::Connection connection(m_credentials.HostUrl, connectOptions);

  Azure::Core::Amqp::_internal::Session session(connection, nullptr);
  session.SetIncomingWindow(
      (uint32_t)m_consumerClientOptions.ReceiverOptions.MaxMessageSize.ValueOr(
          std::numeric_limits<int32_t>::max()));

  Azure::Core::Amqp::_internal::MessageReceiver receiver;
  if (m_credentials.SasCredential == nullptr)
  {
    receiver = Azure::Core::Amqp::_internal::MessageReceiver(
        session,
        m_credentials.Credential,
        m_credentials.HostUrl,
        m_consumerClientOptions.ReceiverOptions);
  }
  else
  {
    receiver = Azure::Core::Amqp::_internal::MessageReceiver(
        session,
        m_credentials.SasCredential,
        m_credentials.HostUrl,
        m_consumerClientOptions.ReceiverOptions);
  }

  // Open the connection to the remote.
  receiver.Open();
  m_receivers.insert_or_assign(partitionId, receiver);
}

Azure::Core::Amqp::_internal::MessageReceiver
Azure::Messaging::EventHubs::ConsumerClient::GetConsumer(std::string const& partitionId)
{
  if (m_receivers.find(partitionId) == m_receivers.end())
  {
    CreateConsumer(partitionId);
  }
  auto consumer = m_receivers.at(partitionId);
  return consumer;
}

void Azure::Messaging::EventHubs::ConsumerClient::StartConsuming(
    int const& maxMessages,
    std::string const& partitionId,
    Azure::Core::Context ctx)
{
  int messageReceived = 0;
  while (messageReceived < maxMessages && !ctx.IsCancelled())
  {
    auto receiver = GetConsumer(partitionId);
    auto message = receiver.WaitForIncomingMessage(ctx);
    if (message)
    {
      messageReceived++;

      auto processor
          = m_processors.at(partitionId.empty() ? m_credentials.ConsumerGroup : partitionId);
      if (processor != nullptr)
      {
        processor(message);
      }
    }
  }
}