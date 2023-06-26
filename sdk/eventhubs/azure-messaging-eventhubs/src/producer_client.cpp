// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "azure/messaging/eventhubs/producer_client.hpp"

#include <azure/core/amqp.hpp>

Azure::Messaging::EventHubs::ProducerClient::ProducerClient(
    std::string const& connectionString,
    std::string const& eventHub,
    Azure::Messaging::EventHubs::Models::ProducerClientOptions options)
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
    Azure::Messaging::EventHubs::Models::ProducerClientOptions options)
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

  Azure::Core::Amqp::_internal::Connection connection(
      hostName,
      (m_credentials.SasCredential ? m_credentials.SasCredential : m_credentials.Credential),
      connectOptions);

  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  sessionOptions.InitialIncomingWindowSize = std::numeric_limits<int32_t>::max();
  sessionOptions.InitialOutgoingWindowSize = std::numeric_limits<uint16_t>::max();

  Azure::Core::Amqp::_internal::Session session{connection.CreateSession(sessionOptions)};
  m_sessions.emplace(partitionId, session);
  Azure::Core::Amqp::_internal::MessageSender sender
      = session.CreateMessageSender(targetUrl, m_producerClientOptions.SenderOptions, nullptr);
  sender.Open();
  m_senders.emplace(partitionId, sender);
}

bool Azure::Messaging::EventHubs::ProducerClient::SendEventDataBatch(
    EventDataBatch& eventDataBatch,
    Azure::Core::Context ctx)
{
  auto message = eventDataBatch.ToAmqpMessage();

  Azure::Messaging::EventHubs::_internal::RetryOperation retryOp(
      m_producerClientOptions.RetryOptions);
  return retryOp.Execute([&]() -> bool {
    auto result = GetSender(eventDataBatch.GetPartitionID()).Send(message, ctx);
    return std::get<0>(result) == Azure::Core::Amqp::_internal::MessageSendStatus::Ok;
  });
}

Azure::Messaging::EventHubs::Models::EventHubProperties
Azure::Messaging::EventHubs::ProducerClient::GetEventHubProperties(
    Models::GetEventHubPropertiesOptions options)
{
  (void)options;
  if (m_senders.find("") == m_senders.end())
  {
    CreateSender("");
  }

  // Create a management client off the session.
  // Eventhubs management APIs return a status code in the "status-code" application properties.
  Azure::Core::Amqp::_internal::ManagementClientOptions managementClientOptions;
  managementClientOptions.EnableTrace = false;
  managementClientOptions.ExpectedStatusCodeKeyName = "status-code";
  Azure::Core::Amqp::_internal::ManagementClient managementClient{
      m_sessions.at("").CreateManagementClient(m_credentials.EventHub, managementClientOptions)};

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
Azure::Messaging::EventHubs::ProducerClient::GetPartitionProperties(
    std::string const& partitionID,
    Models::GetPartitionPropertiesOptions options)
{
  (void)options;
  if (m_senders.find(partitionID) == m_senders.end())
  {
    CreateSender(partitionID);
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
