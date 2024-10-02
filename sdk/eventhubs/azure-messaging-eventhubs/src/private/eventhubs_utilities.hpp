// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Useful utilities for the Event Hubs Clients.
#pragma once

#include "azure/messaging/eventhubs/event_data_batch.hpp"
#include "azure/messaging/eventhubs/eventhubs_exception.hpp"
#include "azure/messaging/eventhubs/models/management_models.hpp"
#include "azure/messaging/eventhubs/partition_client.hpp"
#include "package_version.hpp"

#include <azure/core/amqp/internal/connection.hpp>
#include <azure/core/amqp/internal/management.hpp>
#include <azure/core/amqp/internal/session.hpp>
#include <azure/core/context.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/internal/http/user_agent.hpp>

#include <chrono>

using namespace Azure::Core::Amqp::Models;

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {

  constexpr bool EnableAmqpTrace = true;

  class EventHubsExceptionFactory {
  public:
    /**
     * @brief Constructs a #EventHubsException with a message, an error condition, and an HTTP
     * status code.
     *
     * This constructor is primarily intended for use by the EventHubs Properties events, which
     * report their status using HTTP status codes.
     *
     * @param error The AMQP error indicating the error.
     * @param statusCode The HTTP status code associated with the error.
     */
    static EventHubsException CreateEventHubsException(
        Azure::Core::Amqp::Models::_internal::AmqpError const& error,
        std::uint32_t statusCode)
    {
      EventHubsException rv(error.Description);
      rv.ErrorCondition = error.Condition.ToString();
      rv.ErrorDescription = error.Description;
      rv.StatusCode = statusCode;
      rv.IsTransient = IsErrorTransient(error.Condition);
      return rv;
    }
    /**
     * @brief Constructs a #EventHubsException with an error condition.
     *
     * @param error The AMQP Error indicating the error.
     */
    static EventHubsException CreateEventHubsException(
        Azure::Core::Amqp::Models::_internal::AmqpError const& error)
    {
      EventHubsException rv(error.Description);
      Azure::Core::Diagnostics::_internal::Log::Stream(
          Azure::Core::Diagnostics::Logger::Level::Error)
          << "Creating EventHubsException with error condition: " << error;
      ;
      rv.ErrorCondition = error.Condition.ToString();
      rv.ErrorDescription = error.Description;
      rv.IsTransient = IsErrorTransient(error.Condition);
      return rv;
    }

  private:
    static bool IsErrorTransient(
        Azure::Core::Amqp::Models::_internal::AmqpErrorCondition const& condition);
  };

  class EventDataBatchFactory final {
  public:
    static EventDataBatch CreateEventDataBatch(EventDataBatchOptions const& options);
    EventDataBatchFactory() = delete;
  };

  class PartitionClientFactory final {
  public:
    static PartitionClient CreatePartitionClient(
        Azure::Core::Amqp::_internal::Session const& session,
        std::string const& partitionUrl,
        std::string const& receiverName,
        PartitionClientOptions options,
        Azure::Core::Http::Policies::RetryOptions retryOptions,
        Azure::Core::Context const& context);
    PartitionClientFactory() = delete;
  };

  class EventHubsPropertiesClient {
  public:
    EventHubsPropertiesClient(
        const Azure::Core::Amqp::_internal::Connection& connection,
        std::string eventHubName)
        : m_session{connection.CreateSession()}, m_eventHub{eventHubName} {};

    ~EventHubsPropertiesClient()
    {
      if (m_managementClientIsOpen)
      {
        m_managementClient->Close();
      }
    }

    Models::EventHubProperties GetEventHubsProperties(
        std::string const& eventHubName,
        Core::Context const& context)
    {
      EnsureManagementClient(context);

      // Send a message to the management endpoint to retrieve the properties of the eventhub.
      Azure::Core::Amqp::Models::AmqpMessage message;
      message.ApplicationProperties["name"]
          = static_cast<Azure::Core::Amqp::Models::AmqpValue>(eventHubName);
      message.SetBody(Azure::Core::Amqp::Models::AmqpValue{});
      auto result = m_managementClient->ExecuteOperation(
          "READ" /* operation */,
          "com.microsoft:eventhub" /* type of operation */,
          "" /* locales */,
          message,
          context);

      Models::EventHubProperties properties;
      if (result.Status != Azure::Core::Amqp::_internal::ManagementOperationStatus::Ok)
      {
        Azure::Core::Diagnostics::_internal::Log::Stream(
            Azure::Core::Diagnostics::Logger::Level::Warning)
            << "Management operation failed. StatusCode: " << result.StatusCode
            << " Error: " << result.Error;
        throw _detail::EventHubsExceptionFactory::CreateEventHubsException(
            result.Error, result.StatusCode);
      }
      else
      {
        if (result.Message->BodyType != Azure::Core::Amqp::Models::MessageBodyType::Value)
        {
          throw std::runtime_error("Unexpected body type");
        }

        auto const& body = result.Message->GetBodyAsAmqpValue();
        if (body.GetType() != Azure::Core::Amqp::Models::AmqpValueType::Map)
        {
          throw std::runtime_error("Unexpected body type");
        }
        auto bodyMap = body.AsMap();
        properties.Name = static_cast<std::string>(bodyMap["name"]);
        properties.CreatedOn = Azure::DateTime(std::chrono::system_clock::from_time_t(
            std::chrono::duration_cast<std::chrono::seconds>(
                static_cast<std::chrono::milliseconds>(bodyMap["created_at"].AsTimestamp()))
                .count()));
        auto partitions = bodyMap["partition_ids"].AsArray();
        for (const auto& partition : partitions)
        {
          properties.PartitionIds.push_back(static_cast<std::string>(partition));
        }
      }
      return properties;
    }

    Models::EventHubPartitionProperties GetEventHubsPartitionProperties(
        std::string const& eventHubName,
        std::string const& partitionId,
        Core::Context const& context)
    {
      EnsureManagementClient(context);

      // Send a message to the management endpoint to retrieve the properties of the eventhub.
      Azure::Core::Amqp::Models::AmqpMessage message;
      message.ApplicationProperties["name"]
          = static_cast<Azure::Core::Amqp::Models::AmqpValue>(eventHubName);
      message.ApplicationProperties["partition"]
          = Azure::Core::Amqp::Models::AmqpValue{partitionId};
      message.SetBody(Azure::Core::Amqp::Models::AmqpValue{});
      auto result = m_managementClient->ExecuteOperation(
          "READ" /* operation */,
          "com.microsoft:partition" /* type of operation */,
          "" /* locales */,
          message,
          context);

      Azure::Core::Diagnostics::_internal::Log::Stream(
          Azure::Core::Diagnostics::Logger::Level::Informational)
          << "Received partition properties: " << result.Message;

      Models::EventHubPartitionProperties properties;
      if (result.Status != Azure::Core::Amqp::_internal::ManagementOperationStatus::Ok)
      {
        throw _detail::EventHubsExceptionFactory::CreateEventHubsException(
            result.Error, result.StatusCode);
      }
      else
      {
        if (result.Message->BodyType != Azure::Core::Amqp::Models::MessageBodyType::Value)
        {
          throw std::runtime_error("Unexpected body type");
        }

        auto const& body = result.Message->GetBodyAsAmqpValue();
        if (body.GetType() != Azure::Core::Amqp::Models::AmqpValueType::Map)
        {
          throw std::runtime_error("Unexpected body type");
        }
        auto bodyMap = body.AsMap();
        properties.Name = static_cast<std::string>(bodyMap["name"]);
        properties.PartitionId = static_cast<std::string>(bodyMap["partition"]);
        properties.BeginningSequenceNumber = bodyMap["begin_sequence_number"];
        properties.LastEnqueuedSequenceNumber = bodyMap["last_enqueued_sequence_number"];
        // For <reasons> the last enqueued offset is returned as a string. Convert to an int64.
        properties.LastEnqueuedOffset = std::strtoull(
            static_cast<std::string>(bodyMap["last_enqueued_offset"]).c_str(), nullptr, 10);

        properties.LastEnqueuedTimeUtc = Azure::DateTime(std::chrono::system_clock::from_time_t(
            std::chrono::duration_cast<std::chrono::seconds>(
                static_cast<std::chrono::milliseconds>(
                    bodyMap["last_enqueued_time_utc"].AsTimestamp()))
                .count()));
        properties.IsEmpty = bodyMap["is_partition_empty"];
      }
      return properties;
    }

  private:
    void EnsureManagementClient(Azure::Core::Context const& context)
    {

      std::unique_lock<std::mutex> lock(m_managementClientMutex);
      if (!m_managementClient)
      {
        // Create a management client off the session.
        // Eventhubs management APIs return a status code in the "status-code" application
        // properties.
        Azure::Core::Amqp::_internal::ManagementClientOptions managementClientOptions;
        managementClientOptions.EnableTrace = _detail::EnableAmqpTrace;
        managementClientOptions.ExpectedStatusCodeKeyName = "status-code";
        m_managementClient = std::make_unique<Azure::Core::Amqp::_internal::ManagementClient>(
            m_session.CreateManagementClient(m_eventHub, managementClientOptions));

        auto openResult{m_managementClient->Open(context)};
        if (openResult != Azure::Core::Amqp::_internal::ManagementOpenStatus::Ok)
        {
          throw std::runtime_error("Could not open Management client");
        }
        m_managementClientIsOpen = true;
      }
    }

    std::unique_ptr<Azure::Core::Amqp::_internal::ManagementClient> const& GetManagementClient()
    {
      std::unique_lock<std::mutex> lock(m_managementClientMutex);
      return m_managementClient;
    }

    std::mutex m_managementClientMutex;
    Azure::Core::Amqp::_internal::Session m_session;
    std::unique_ptr<Azure::Core::Amqp::_internal::ManagementClient> m_managementClient;
    bool m_managementClientIsOpen{false};
    std::string m_eventHub;
  };

  class EventHubsUtilities {

  public:
    template <typename T>
    static void SetUserAgent(T& options, std::string const& applicationId, long cplusplusValue)
    {
      constexpr const char* packageName = "azure-messaging-eventhubs-cpp";

      options.Properties.emplace(AmqpSymbol{"product"}, packageName);
      options.Properties.emplace(AmqpSymbol{"version"}, PackageVersion::ToString());
#if defined(AZ_PLATFORM_WINDOWS)
      options.Properties.emplace(AmqpSymbol{"platform"}, "Windows");
#elif defined(AZ_PLATFORM_LINUX)
      options.Properties.emplace(AmqpSymbol{"platform"}, "Linux");
#elif defined(AZ_PLATFORM_MAC)
      options.Properties.emplace(AmqpSymbol{"platform"}, "Mac");
#endif
      options.Properties.emplace(
          AmqpSymbol{"user-agent"},
          Azure::Core::Http::_detail::UserAgentGenerator::GenerateUserAgent(
              packageName, PackageVersion::ToString(), applicationId, cplusplusValue));
    }

    static void LogRawBuffer(std::ostream& os, std::vector<uint8_t> const& buffer);
    ~EventHubsUtilities() = delete;
  };

}}}} // namespace Azure::Messaging::EventHubs::_detail
