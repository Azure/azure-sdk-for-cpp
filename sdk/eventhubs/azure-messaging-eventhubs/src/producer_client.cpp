// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "azure/messaging/eventhubs/producer_client.hpp"

#include "private/eventhub_utilities.hpp"
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
    auto result = GetSender(eventDataBatch.GetPartitionID()).Send(message, context);
    return std::get<0>(result) == Azure::Core::Amqp::_internal::MessageSendStatus::Ok;
  });
}

Azure::Messaging::EventHubs::Models::EventHubProperties
Azure::Messaging::EventHubs::ProducerClient::GetEventHubProperties(Core::Context const& context)
{
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
      m_sessions.at("").CreateManagementClient(m_eventHub, managementClientOptions)};

  managementClient.Open();

  // Send a message to the management endpoint to retrieve the properties of the eventhub.
  Azure::Core::Amqp::Models::AmqpMessage message;
  message.ApplicationProperties["name"]
      = static_cast<Azure::Core::Amqp::Models::AmqpValue>(m_eventHub);
  message.SetBody(Azure::Core::Amqp::Models::AmqpValue{});
  auto result = managementClient.ExecuteOperation(
      "READ" /* operation */,
      "com.microsoft:eventhub" /* type of operation */,
      "" /* locales */,
      message,
      context);

  Models::EventHubProperties properties;
  if (result.Status == Azure::Core::Amqp::_internal::ManagementOperationStatus::Error)
  {
    std::cerr << "Error: " << result.Message.ApplicationProperties["status-description"];
  }
  else
  {
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
Azure::Messaging::EventHubs::ProducerClient::GetPartitionProperties(
    std::string const& partitionID,
    Core::Context const& context)
{
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
      m_sessions.at(partitionID).CreateManagementClient(m_eventHub, managementClientOptions)};

  managementClient.Open();

  // Send a message to the management endpoint to retrieve the properties of the eventhub.
  Azure::Core::Amqp::Models::AmqpMessage message;
  message.ApplicationProperties["name"]
      = static_cast<Azure::Core::Amqp::Models::AmqpValue>(m_eventHub);
  message.ApplicationProperties["partition"] = Azure::Core::Amqp::Models::AmqpValue{partitionID};
  message.SetBody(Azure::Core::Amqp::Models::AmqpValue{});
  auto result = managementClient.ExecuteOperation(
      "READ" /* operation */,
      "com.microsoft:partition" /* type of operation */,
      "" /* locales */,
      message,
      context);

  Models::EventHubPartitionProperties properties;
  if (result.Status != Azure::Core::Amqp::_internal::ManagementOperationStatus::Ok)
  {
    throw std::runtime_error(
        "Could not receive partition properties: "
        + static_cast<std::string>(result.Message.ApplicationProperties["status-description"]));
  }
  else
  {
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
