// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include <azure/core/amqp.hpp>
#include <azure/messaging/eventhubs.hpp>
using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;
using namespace Azure::Messaging::EventHubs::Models;

Azure::Messaging::EventHubs::ConsumerClient::ConsumerClient(
    std::string const& connectionString,
    std::string const& eventHub,
    std::string const& consumerGroup,
    Azure::Messaging::EventHubs::ConsumerClientOptions const& options)
    : m_credentials{connectionString, "", eventHub, consumerGroup}, m_consumerClientOptions(options)
{
  auto sasCredential
      = std::make_shared<Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>(
          m_credentials.ConnectionString);

  m_credentials.Credential = sasCredential;
  if (!sasCredential->GetEntityPath().empty())
  {
    m_credentials.EventHub = sasCredential->GetEntityPath();
  }
  m_credentials.HostName = sasCredential->GetHostName();
  m_credentials.HostUrl = "amqps://" + m_credentials.HostName + "/" + m_credentials.EventHub
      + "/ConsumerGroups/" + m_credentials.ConsumerGroup;
}

Azure::Messaging::EventHubs::ConsumerClient::ConsumerClient(
    std::string const& hostName,
    std::string const& eventHub,
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
    std::string const& consumerGroup,
    Azure::Messaging::EventHubs::ConsumerClientOptions const& options)
    : m_credentials{"", hostName, eventHub, consumerGroup, credential},
      m_consumerClientOptions(options)
{
  m_credentials.HostUrl = "amqps://" + m_credentials.HostName + "/" + m_credentials.EventHub
      + "/ConsumerGroups/" + m_credentials.ConsumerGroup;
}

Azure::Messaging::EventHubs::PartitionClient
Azure::Messaging::EventHubs::ConsumerClient::CreatePartitionClient(
    std::string partitionId,
    Azure::Messaging::EventHubs::PartitionClientOptions const& options)
{
  Azure::Messaging::EventHubs::PartitionClient partitionClient(
      options, m_consumerClientOptions.RetryOptions);

  std::string suffix = !partitionId.empty() ? "/Partitions/" + partitionId : "";
  std::string hostUrl = m_credentials.HostUrl + suffix;

  Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
  connectOptions.ContainerId = m_consumerClientOptions.ApplicationID;
  connectOptions.EnableTrace = m_consumerClientOptions.ReceiverOptions.EnableTrace;

  Azure::Core::Amqp::_internal::Connection connection(
      m_credentials.HostName, m_credentials.Credential, connectOptions);
  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  sessionOptions.InitialIncomingWindowSize
      = static_cast<uint32_t>(m_consumerClientOptions.ReceiverOptions.MaxMessageSize.ValueOr(
          std::numeric_limits<int32_t>::max()));

  Azure::Core::Amqp::_internal::Session session{connection.CreateSession(sessionOptions)};

  Azure::Core::Amqp::_internal::MessageReceiver receiver
      = session.CreateMessageReceiver(hostUrl, m_consumerClientOptions.ReceiverOptions);

  // Open the connection to the remote.
  receiver.Open();
  m_sessions.emplace(partitionId, session);
  partitionClient.PushBackReceiver(receiver);
  Log::Write(Logger::Level::Informational, "Created new partition client");
  return partitionClient;
}

Azure::Messaging::EventHubs::Models::EventHubProperties
Azure::Messaging::EventHubs::ConsumerClient::GetEventHubProperties(
    Azure::Messaging::EventHubs::GetEventHubPropertiesOptions options)
{
  (void)options;
  std::shared_ptr<PartitionClient> client;
  if (m_sessions.size() == 0 && m_sessions.find("0") == m_sessions.end())
  {
    client = std::make_shared<PartitionClient>(CreatePartitionClient("0"));
  }

  // Create a management client off the session.
  // Eventhubs management APIs return a status code in the "status-code" application properties.
  Azure::Core::Amqp::_internal::ManagementClientOptions managementClientOptions;
  managementClientOptions.EnableTrace = false;
  managementClientOptions.ExpectedStatusCodeKeyName = "status-code";
  Azure::Core::Amqp::_internal::ManagementClient managementClient(
      m_sessions.at("0").CreateManagementClient(m_credentials.EventHub, managementClientOptions));

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
    std::string ss = "Error: "
        + static_cast<std::string>(result.Message.ApplicationProperties["status-description"]);
    Log::Write(Logger::Level::Error, ss);
  }
  else
  {
    std::cout << "Management endpoint properties message: " << result.Message;
    if (result.Message.BodyType != Azure::Core::Amqp::Models::MessageBodyType::Value)
    {
      throw std::runtime_error("Unexpected body type");
    }

    auto const& body = result.Message.GetBodyAsAmqpValue();
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
    Azure::Messaging::EventHubs::GetPartitionPropertiesOptions options)
{
  (void)options;
  if (m_sessions.find(partitionID) == m_sessions.end())
  {
    CreatePartitionClient(partitionID);
  }

  // Create a management client off the session.
  // Eventhubs management APIs return a status code in the "status-code" application properties.
  Azure::Core::Amqp::_internal::ManagementClientOptions managementClientOptions;
  managementClientOptions.EnableTrace = false;
  managementClientOptions.ExpectedStatusCodeKeyName = "status-code";
  Azure::Core::Amqp::_internal::ManagementClient managementClient{
      m_sessions.at(partitionID)
          .CreateManagementClient(m_credentials.EventHub, managementClientOptions)};

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

    auto const& body = result.Message.GetBodyAsAmqpValue();
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
