// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include <azure/core/amqp.hpp>
#include <azure/messaging/eventhubs.hpp>

Azure::Messaging::EventHubs::ConsumerClient::ConsumerClient(
    std::string const& connectionString,
    std::string const& eventHub,
    std::string const& consumerGroup,
    Azure::Messaging::EventHubs::Models::ConsumerClientOptions const& options)
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
    Azure::Messaging::EventHubs::Models::ConsumerClientOptions const& options)
    : m_credentials{"", fullyQualifiedNamespace, eventHub, consumerGroup, credential},
      m_consumerClientOptions(options)
{
  m_credentials.HostUrl = "amqps://" + m_credentials.FullyQualifiedNamespace + "/"
      + m_credentials.EventHub + "/ConsumerGroups/" + m_credentials.ConsumerGroup;
}

Azure::Messaging::EventHubs::PartitionClient
Azure::Messaging::EventHubs::ConsumerClient::NewPartitionClient(
    std::string partitionId,
    Azure::Messaging::EventHubs::Models::PartitionClientOptions const& options)
{
  Azure::Messaging::EventHubs::PartitionClient partitionClient(
      options, m_consumerClientOptions.RetryOptions);

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
  // TODO WHAT ABOUT TOKEN CREDENTIALS

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
  m_sessions.emplace(partitionId, session);
  partitionClient.PushBackReceiver(receiver);
  return partitionClient;
}

Azure::Messaging::EventHubs::Models::EventHubProperties
Azure::Messaging::EventHubs::ConsumerClient::GetEventHubProperties(
    Azure::Messaging::EventHubs::Models::GetEventHubPropertiesOptions options)
{
  (void)options;
  std::shared_ptr<PartitionClient> client;
  if (m_sessions.size()==0 && m_sessions.find("0") == m_sessions.end())
  {
    client  =std::make_shared<PartitionClient>(NewPartitionClient("0"));
  }

  // Create a management client off the session.
  // Eventhubs management APIs return a status code in the "status-code" application properties.
  Azure::Core::Amqp::_internal::ManagementOptions managementClientOptions;
  managementClientOptions.EnableTrace = false;
  managementClientOptions.ExpectedStatusCodeKeyName = "status-code";
  Azure::Core::Amqp::_internal::Management managementClient(
      m_sessions.at("0"), m_credentials.EventHub, managementClientOptions);

  managementClient.Open();

  // Send a message to the management endpoint to retrieve the properties of the eventhub.
  Azure::Core::Amqp::Models::AmqpMessage message;
  message.ApplicationProperties["name"]
      = Azure::Core::Amqp::Models::AmqpValue{m_credentials.EventHub};
  message.SetBody(Azure::Core::Amqp::Models::AmqpValue{});
  auto result = managementClient.ExecuteOperation(
      "READ" /* operation */,
      "com.microsoft:eventhub" /* type of operation */,
      "" /* locales */,
      message);

  Models::EventHubProperties properties;
  if (result.Status == Azure::Core::Amqp::_internal::ManagementOperationStatus::Error)
  {
    std::cerr << "Error: " << result.Message.ApplicationProperties["status-description"];
  }
  else
  {
    std::cout << "Management endpoint properties message: " << result.Message;
    if (result.Message.BodyType != Azure::Core::Amqp::Models::MessageBodyType::Value)
    {
      throw std::runtime_error("Unexpected body type");
    }

    auto body = result.Message.GetBodyAsAmqpValue();
    if (body.GetType() != Azure::Core::Amqp::Models::AmqpValueType::Map)
    {
      throw std::runtime_error("Unexpected body type");
    }
    auto bodyMap = body.AsMap();
    properties.Name = static_cast<std::string>(bodyMap["name"]);
    properties.CreatedOn = Azure::DateTime(std::chrono::system_clock::from_time_t(
        static_cast<std::chrono::milliseconds>(bodyMap["created_at"].AsTimestamp()).count()));
    auto partitions = bodyMap["partition_ids"].AsArray();
    for (const auto& partition : partitions)
    {
      properties.PartitionIDs.push_back(static_cast<std::string>(partition));
    }
  }
  managementClient.Close();

  return properties;
}

Azure::Messaging::EventHubs::Models::EventHubPartitionProperties
Azure::Messaging::EventHubs::ConsumerClient::GetPartitionProperties(
    std::string const& partitionID,
    Azure::Messaging::EventHubs::Models::GetPartitionPropertiesOptions options)
{
  (void)options;
  if (m_sessions.find(partitionID) == m_sessions.end())
  {
    NewPartitionClient(partitionID);
  }

  // Create a management client off the session.
  // Eventhubs management APIs return a status code in the "status-code" application properties.
  Azure::Core::Amqp::_internal::ManagementOptions managementClientOptions;
  managementClientOptions.EnableTrace = false;
  managementClientOptions.ExpectedStatusCodeKeyName = "status-code";
  Azure::Core::Amqp::_internal::Management managementClient(
      m_sessions[partitionID], m_credentials.EventHub, managementClientOptions);

  managementClient.Open();

  // Send a message to the management endpoint to retrieve the properties of the eventhub.
  Azure::Core::Amqp::Models::AmqpMessage message;
  message.ApplicationProperties["name"]
      = Azure::Core::Amqp::Models::AmqpValue{m_credentials.EventHub};
  message.ApplicationProperties["partition"] = Azure::Core::Amqp::Models::AmqpValue{partitionID};
  message.SetBody(Azure::Core::Amqp::Models::AmqpValue{});
  auto result = managementClient.ExecuteOperation(
      "READ" /* operation */,
      "com.microsoft:partition" /* type of operation */,
      "" /* locales */,
      message);

  Models::EventHubPartitionProperties properties;
  if (result.Status == Azure::Core::Amqp::_internal::ManagementOperationStatus::Error)
  {
    std::cerr << "Error: " << result.Message.ApplicationProperties["status-description"];
  }
  else
  {
    std::cout << "Partition properties message: " << result.Message;
    if (result.Message.BodyType != Azure::Core::Amqp::Models::MessageBodyType::Value)
    {
      throw std::runtime_error("Unexpected body type");
    }

    auto body = result.Message.GetBodyAsAmqpValue();
    if (body.GetType() != Azure::Core::Amqp::Models::AmqpValueType::Map)
    {
      throw std::runtime_error("Unexpected body type");
    }
    auto bodyMap = body.AsMap();
    properties.Name = static_cast<std::string>(bodyMap["name"]);
    properties.PartitionId = static_cast<std::string>(bodyMap["partition"]);
    properties.BeginningSequenceNumber = bodyMap["begin_sequence_number"];
    properties.LastEnqueuedSequenceNumber = bodyMap["last_enqueued_sequence_number"];
    properties.LastEnqueuedOffset = static_cast<std::string>(bodyMap["last_enqueued_offset"]);
    properties.LastEnqueuedTimeUtc = Azure::DateTime(std::chrono::system_clock::from_time_t(
        std::chrono::duration_cast<std::chrono::seconds>(
            static_cast<std::chrono::milliseconds>(bodyMap["last_enqueued_time_utc"].AsTimestamp()))
            .count()));
    properties.IsEmpty = bodyMap["is_partition_empty"];
  }
  managementClient.Close();

  return properties;
}
