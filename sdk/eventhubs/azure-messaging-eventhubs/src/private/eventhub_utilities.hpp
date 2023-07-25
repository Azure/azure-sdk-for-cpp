// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Useful utilities for the Event Hubs Clients.
#pragma once

#include "azure/messaging/eventhubs/eventhubs_exception.hpp"
#include "azure/messaging/eventhubs/models/management_models.hpp"
#include "package_version.hpp"

#include <azure/core/amqp/management.hpp>
#include <azure/core/amqp/session.hpp>
#include <azure/core/context.hpp>
#include <azure/core/internal/http/user_agent.hpp>

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {

  class EventHubUtilities {

  public:
    template <typename T> static void SetUserAgent(T& options, std::string const& applicationId)
    {
      constexpr const char* packageName = "cpp-azure-messaging-eventhubs-cpp";

      options.Properties.emplace("Product", +packageName);
      options.Properties.emplace("Version", PackageVersion::ToString());
#if defined(AZ_PLATFORM_WINDOWS)
      options.Properties.emplace("Platform", "Windows");
#elif defined(AZ_PLATFORM_LINUX)
      options.Properties.emplace("Platform", "Linux");
#elif defined(AZ_PLATFORM_MAC)
      options.Properties.emplace("Platform", "Mac");
#endif
      options.Properties.emplace(
          "User-Agent",
          Azure::Core::Http::_detail::UserAgentGenerator::GenerateUserAgent(
              packageName, PackageVersion::ToString(), applicationId));
    }

    static Models::EventHubProperties GetEventHubsProperties(
        Azure::Core::Amqp::_internal::Session const& session,
        std::string const& eventHubName,
        Core::Context const& context)
    {

      // Create a management client off the session.
      // Eventhubs management APIs return a status code in the "status-code" application properties.
      Azure::Core::Amqp::_internal::ManagementClientOptions managementClientOptions;
      managementClientOptions.EnableTrace = false;
      managementClientOptions.ExpectedStatusCodeKeyName = "status-code";
      Azure::Core::Amqp::_internal::ManagementClient managementClient{
          session.CreateManagementClient(eventHubName, managementClientOptions)};

      managementClient.Open();

      // Send a message to the management endpoint to retrieve the properties of the eventhub.
      Azure::Core::Amqp::Models::AmqpMessage message;
      message.ApplicationProperties["name"]
          = static_cast<Azure::Core::Amqp::Models::AmqpValue>(eventHubName);
      message.SetBody(Azure::Core::Amqp::Models::AmqpValue{});
      auto result = managementClient.ExecuteOperation(
          "READ" /* operation */,
          "com.microsoft:eventhub" /* type of operation */,
          "" /* locales */,
          message,
          context);

      Models::EventHubProperties properties;
      if (result.Status != Azure::Core::Amqp::_internal::ManagementOperationStatus::Ok)
      {
        throw EventHubsException(result.Error);
        //        std::cerr << "Error: " <<
        //        result.Message.ApplicationProperties["status-description"];
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
          properties.PartitionIds.push_back(static_cast<std::string>(partition));
        }
      }
      managementClient.Close();

      return properties;
    }

    static Models::EventHubPartitionProperties GetEventHubsPartitionProperties(
        Azure::Core::Amqp::_internal::Session const& session,
        std::string const& eventHubName,
        std::string const& partitionId,
        Core::Context const& context)
    {
      // Create a management client off the session.
      // Eventhubs management APIs return a status code in the "status-code" application properties.
      Azure::Core::Amqp::_internal::ManagementClientOptions managementClientOptions;
      managementClientOptions.EnableTrace = false;
      managementClientOptions.ExpectedStatusCodeKeyName = "status-code";
      Azure::Core::Amqp::_internal::ManagementClient managementClient{
          session.CreateManagementClient(eventHubName, managementClientOptions)};

      managementClient.Open();

      // Send a message to the management endpoint to retrieve the properties of the eventhub.
      Azure::Core::Amqp::Models::AmqpMessage message;
      message.ApplicationProperties["name"]
          = static_cast<Azure::Core::Amqp::Models::AmqpValue>(eventHubName);
      message.ApplicationProperties["partition"]
          = Azure::Core::Amqp::Models::AmqpValue{partitionId};
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
        throw EventHubsException(result.Error);
        //        std::cerr << "Error: " <<
        //        result.Message.ApplicationProperties["status-description"];
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
                static_cast<std::chrono::milliseconds>(
                    bodyMap["last_enqueued_time_utc"].AsTimestamp()))
                .count()));
        properties.IsEmpty = bodyMap["is_partition_empty"];
      }
      managementClient.Close();

      return properties;
    }
    ~EventHubUtilities() = delete;
  };
}}}} // namespace Azure::Messaging::EventHubs::_detail
