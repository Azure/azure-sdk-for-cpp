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

  m_credentials.HostUrl = "amqps://" + m_credentials.FullyQualifiedNamespace + "/"
      + m_credentials.EventHub + "/ConsumerGroups/" + m_credentials.ConsumerGroup;
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
  m_credentials.HostUrl = "amqps://" + m_credentials.FullyQualifiedNamespace + "/"
      + m_credentials.EventHub + "/ConsumerGroups/" + m_credentials.ConsumerGroup;
}

Azure::Messaging::EventHubs::PartitionClient
Azure::Messaging::EventHubs::ConsumerClient::NewPartitionClient(
    std::string partitionId,
    Azure::Messaging::EventHubs::PartitionClientOptions const& options)
{
  Azure::Messaging::EventHubs::PartitionClient partitionClient;
  partitionClient.m_partitionOptions = options;
  partitionClient.RetryOptions = m_consumerClientOptions.RetryOptions;

  std::string suffix = !partitionId.empty() ? "/Partitions/" + partitionId : "";
  std::string hostUrl = m_credentials.HostUrl + suffix;
  std::string hostName;
  Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
  connectOptions.ContainerId = m_consumerClientOptions.ApplicationID;
  connectOptions.EnableTrace = m_consumerClientOptions.ReceiverOptions.EnableTrace;
  if (m_credentials.SasCredential != nullptr)
  {
	connectOptions.Port = m_credentials.SasCredential->GetPort();
    hostName = m_credentials.SasCredential->GetHostName();
  }
  //TODO WHAT ABOUT TOKEN CREDENTIALS
  
  Azure::Core::Amqp::_internal::Connection connection(hostName, connectOptions);
  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  sessionOptions.InitialIncomingWindowSize
      = (uint32_t)m_consumerClientOptions.ReceiverOptions.MaxMessageSize.ValueOr(
          std::numeric_limits<int32_t>::max());
  
  Azure::Core::Amqp::_internal::Session session;
  if (m_credentials.SasCredential == nullptr)
  {
      session = Azure::Core::Amqp::_internal::Session(
		connection, m_credentials.Credential, sessionOptions);
  }
  else
  {
      session = Azure::Core::Amqp::_internal::Session(
		connection, m_credentials.SasCredential, sessionOptions);
  }
  Azure::Core::Amqp::_internal::MessageReceiver receiver;
    receiver = Azure::Core::Amqp::_internal::MessageReceiver(
        session, hostUrl, m_consumerClientOptions.ReceiverOptions);

  // Open the connection to the remote.
  receiver.Open();
  partitionClient.m_receivers.push_back(std::move(receiver));
  return partitionClient;
}
