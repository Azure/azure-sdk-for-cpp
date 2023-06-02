// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/connection_string_credential.hpp>
#include <azure/core/amqp/management.hpp>
#include <azure/core/amqp/message_sender.hpp>
#include <azure/identity/client_secret_credential.hpp>

#include <chrono>
#include <iostream>
#include <limits>
#include <string>

constexpr const char* EH_AUTHENTICATION_SCOPE = "https://eventhubs.azure.net/.default";

struct EventHubProperties final
{
  std::string Name;
  std::vector<std::string> PartitionIds;
  Azure::DateTime CreatedAt;
};

EventHubProperties GetEventHubProperties(
    Azure::Core::Amqp::_internal::Session const& session,
    std::string const& eventHubName)
{

  // Create a management client off the session.
  // Eventhubs management APIs return a status code in the "status-code" application properties.
  Azure::Core::Amqp::_internal::ManagementClientOptions managementClientOptions;
  managementClientOptions.EnableTrace = false;
  managementClientOptions.ExpectedStatusCodeKeyName = "status-code";
  Azure::Core::Amqp::_internal::ManagementClient managementClient(
      session.CreateManagementClient(eventHubName, managementClientOptions));

  managementClient.Open();

  // Send a message to the management endpoint to retrieve the properties of the eventhub.
  Azure::Core::Amqp::Models::AmqpMessage message;
  message.ApplicationProperties["name"] = Azure::Core::Amqp::Models::AmqpValue{eventHubName};
  message.SetBody(Azure::Core::Amqp::Models::AmqpValue{});
  auto result = managementClient.ExecuteOperation(
      "READ" /* operation */,
      "com.microsoft:eventhub" /* type of operation */,
      "" /* locales */,
      message);

  EventHubProperties properties;
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
    properties.CreatedAt = Azure::DateTime(std::chrono::system_clock::from_time_t(
        static_cast<std::chrono::milliseconds>(bodyMap["created_at"].AsTimestamp()).count()));
    auto partitions = bodyMap["partition_ids"].AsArray();
    for (const auto& partition : partitions)
    {
      properties.PartitionIds.push_back(static_cast<std::string>(partition));
    }
  }
  managementClient.Close();

  return properties;
}

struct EventHubPartitionProperties final
{
  std::string Name;
  std::string PartitionId;
  int64_t BeginningSequenceNumber{};
  int64_t LastEnqueuedSequenceNumber{};
  std::string LastEnqueuedOffset;
  Azure::DateTime LastEnqueuedTimeUtc;
  bool IsEmpty{};
};

EventHubPartitionProperties GetPartitionProperties(
    Azure::Core::Amqp::_internal::Session const& session,
    std::string const& eventHubName,
    std::string const& partitionId)
{

  // Create a management client off the session.
  // Eventhubs management APIs return a status code in the "status-code" application properties.
  Azure::Core::Amqp::_internal::ManagementClientOptions managementClientOptions;
  managementClientOptions.EnableTrace = false;
  managementClientOptions.ExpectedStatusCodeKeyName = "status-code";
  Azure::Core::Amqp::_internal::ManagementClient managementClient(
      session.CreateManagementClient(eventHubName, managementClientOptions));

  managementClient.Open();

  // Send a message to the management endpoint to retrieve the properties of the eventhub.
  Azure::Core::Amqp::Models::AmqpMessage message;
  message.ApplicationProperties["name"] = Azure::Core::Amqp::Models::AmqpValue{eventHubName};
  message.ApplicationProperties["partition"] = Azure::Core::Amqp::Models::AmqpValue{partitionId};
  message.SetBody(Azure::Core::Amqp::Models::AmqpValue{});
  auto result = managementClient.ExecuteOperation(
      "READ" /* operation */,
      "com.microsoft:partition" /* type of operation */,
      "" /* locales */,
      message);

  EventHubPartitionProperties properties;
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

int main()
{
  // Retrieve the eventhub connection string so we can extract the host name and entity name. We
  // are NOT using the connection string to authenticate with the eventhub, only to retrieve the
  // host name and entity (if present).
  std::string eventhubConnectionString = std::getenv("EVENTHUB_CONNECTION_STRING");

  Azure::Core::Amqp::_internal::ConnectionStringParser connectionParser(eventhubConnectionString);
  std::string eventhubsHost = connectionParser.GetHostName();
  std::string eventhubsEntity = connectionParser.GetEntityPath();
  if (eventhubsEntity.empty())
  {
    eventhubsEntity = std::getenv("EVENTHUB_NAME");
  }

  // Establish the connection to the eventhub.

  auto credential{std::make_shared<Azure::Identity::ClientSecretCredential>(
      std::getenv("SAMPLES_TENANT_ID"),
      std::getenv("SAMPLES_CLIENT_ID"),
      std::getenv("SAMPLES_CLIENT_SECRET"))};

  Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
  connectionOptions.ContainerId = "some";
  connectionOptions.EnableTrace = false;
  connectionOptions.Port = connectionParser.GetPort();
  Azure::Core::Amqp::_internal::Connection connection(
      connectionParser.GetHostName(), credential, connectionOptions);

  // Establish a session to the eventhub.
  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  sessionOptions.InitialIncomingWindowSize = std::numeric_limits<int32_t>::max();
  sessionOptions.InitialOutgoingWindowSize = std::numeric_limits<uint16_t>::max();
  sessionOptions.AuthenticationScopes = {EH_AUTHENTICATION_SCOPE};
  Azure::Core::Amqp::_internal::Session session(connection.CreateSession(sessionOptions));

  auto eventHubProperties = GetEventHubProperties(session, eventhubsEntity);
  for (const auto& partition : eventHubProperties.PartitionIds)
  {
    std::cout << "Partition: " << partition << std::endl;
    auto partitionProperties = GetPartitionProperties(session, eventhubsEntity, partition);
    std::cout << "Partition properties: " << std::endl;
    std::cout << "  Name: " << partitionProperties.Name << std::endl;
    std::cout << "  PartitionId: " << partitionProperties.PartitionId << std::endl;
    std::cout << "  BeginningSequenceNumber: " << partitionProperties.BeginningSequenceNumber
              << std::endl;
    std::cout << "  LastEnqueuedSequenceNumber: " << partitionProperties.LastEnqueuedSequenceNumber
              << std::endl;
    std::cout << "  LastEnqueuedOffset: " << partitionProperties.LastEnqueuedOffset << std::endl;
    std::cout << "  LastEnqueuedTimeUtc: " << partitionProperties.LastEnqueuedTimeUtc.ToString()
              << std::endl;
    std::cout << "  IsEmpty: " << std::boolalpha << partitionProperties.IsEmpty << std::endl;
  }
}
