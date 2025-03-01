// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: word myeventhub

#pragma once
#include "models/consumer_client_models.hpp"
#include "models/management_models.hpp"
#include "partition_client.hpp"

#include <azure/core/amqp.hpp>
#include <azure/core/amqp/internal/connection.hpp>
#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
namespace Azure { namespace Messaging { namespace EventHubs {
  namespace _detail {
    class EventHubsPropertiesClient;
  }

  class ConsumerClient;

  /// @brief The default consumer group name.
  constexpr const char* DefaultConsumerGroup = "$Default";

  /**@brief Contains options for the ConsumerClient creation
   */
  struct ConsumerClientOptions final
  {
    /**@brief ApplicationID is used as the identifier when setting the User-Agent property.
     */
    std::string ApplicationID;

    /**@brief  RetryOptions controls how often operations are retried from this client and any
     * Receivers and Senders created from this client.
     */
    Azure::Core::Http::Policies::RetryOptions RetryOptions{};

    /** @brief Name of the consumer client. */
    std::string Name{};

  private:
    // The friend declaration is needed so that ConsumerClient could access CppStandardVersion,
    // and it is not a struct's public field like the ones above to be set non-programmatically.
    // When building the SDK, tests, or samples, the value of __cplusplus is ultimately controlled
    // by the cmake files in this repo (i.e. C++14), therefore we set distinct values of 0, -1, etc
    // when it is the case.
    friend class ConsumerClient;
    long CppStandardVersion =
#if defined(_azure_BUILDING_SDK)
        -2L
#elif defined(_azure_BUILDING_TESTS)
        -1L
#elif defined(_azure_BUILDING_SAMPLES)
        0L
#else
    // https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
#if defined(_MSVC_LANG) && __cplusplus == 199711L
        _MSVC_LANG
#else
        __cplusplus
#endif
#endif
        ;
  };

  /**
   * @brief The ConsumerClient class is a high level class used to consume events from an Event Hub.
   *
   * @details The ConsumerClient class uses a #Azure::Messaging::EventHubs::PartitionClient to
   * receive events from a specific partition of an Event Hub. The
   * #Azure::Messaging::EventHubs::PartitionClient is created by the #ConsumerClient and is
   * available via the NewPartitionClient method. The ConsumerClient is also responsible for
   * managing the connection to the Event Hub and will reconnect as necessary.
   */
  class ConsumerClient final {
  public:
    /** @brief creates a ConsumerClient from a token credential.
     *
     * @param fullyQualifiedNamespace fully qualified namespace name (e.g.
     * myeventhub.servicebus.windows.net)
     * @param eventHub event hub name
     * @param consumerGroup consumer group name
     * @param credential Token credential
     * @param options client options
     *
     */
    ConsumerClient(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHub,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
        std::string const& consumerGroup = DefaultConsumerGroup,
        ConsumerClientOptions const& options = {});

    /** Copy a new ConsumerClient from an existing one. */
    ConsumerClient(ConsumerClient const& other) = delete;

    /** Move a consumer client */
    ConsumerClient(ConsumerClient&& other) = delete;

    /** Assign a ConsumerClient to an existing one. */
    ConsumerClient& operator=(ConsumerClient const& other) = delete;

    /** Move a consumer client */
    ConsumerClient& operator=(ConsumerClient&& other) = delete;

    ~ConsumerClient();

    /** @brief Getter for event hub name
     *
     * @returns Event hub name for client
     */
    std::string const& GetEventHubName() const { return m_eventHub; }

    /** @brief Getter for consumer group name
     *
     * @returns Consumer group name for client
     */
    std::string const& GetConsumerGroup() const { return m_consumerGroup; }

    /** @brief Getter for client details
     *
     * @returns Client details for client
     */
    Models::ConsumerClientDetails GetDetails() const
    {
      Models::ConsumerClientDetails details;
      details.ClientId = m_consumerClientOptions.ApplicationID;
      details.ConsumerGroup = m_consumerGroup;
      details.EventHubName = m_eventHub;
      details.FullyQualifiedNamespace = m_fullyQualifiedNamespace;
      return details;
    }
    /** @brief Getter for retry options
     *
     * @returns Retry options for client
     */
    Azure::Core::Http::Policies::RetryOptions const& GetRetryOptions() const
    {
      return m_consumerClientOptions.RetryOptions;
    }

    /** @brief Create new Partition client
     *
     * @param partitionId targeted partition
     * @param options client options
     * @param context The context for the operation can be used for request cancellation.
     */
    PartitionClient CreatePartitionClient(
        std::string const& partitionId,
        PartitionClientOptions const& options = {},
        Azure::Core::Context const& context = {});

    /**
     * @brief Closes the consumer client canceling any operations outstanding on any of the existing
     * partition clients.
     *
     * @param context The context for the operation can be used for request cancellation.
     */
    void Close(Azure::Core::Context const& context);

    /**@brief  GetEventHubProperties gets properties of an eventHub. This includes data
     * like name, and partitions.
     *
     */
    Models::EventHubProperties GetEventHubProperties(Core::Context const& context = {});

    /**@brief  GetPartitionProperties gets properties for a specific partition. This includes data
     * like the last enqueued sequence number, the first sequence number and when an event was last
     * enqueued to the partition.
     *
     * @param partitionID partition ID to detail.
     * @param context The context for the operation can be used for request cancellation.
     */
    Models::EventHubPartitionProperties GetPartitionProperties(
        std::string const& partitionID,
        Core::Context const& context = {});

  private:
    /// The connection string for the Event Hubs namespace
    std::string m_connectionString;

    /// the Event Hubs namespace name (ex: myeventhub.servicebus.windows.net)
    std::string m_fullyQualifiedNamespace;

    /// The name of the Event Hub
    std::string m_eventHub;

    /// The name of the consumer group
    std::string m_consumerGroup;

    /// Credentials to be used to authenticate the client.
    std::shared_ptr<Core::Credentials::TokenCredential> m_credential;

    /// The URL to the Event Hubs namespace
    std::string m_hostUrl;

    /// <summary>
    /// The expected port to be used. TLS by default.
    /// </summary>
    std::uint16_t m_targetPort = Azure::Core::Amqp::_internal::AmqpTlsPort;

    /// @brief The message receivers used to receive messages for a given partition.
    std::mutex m_receiversLock;
    std::map<std::string, Azure::Core::Amqp::_internal::MessageReceiver> m_receivers;

    /// @brief The AMQP Sessions used to receive messages for a given partition.
    std::recursive_mutex m_sessionsLock;
    std::map<std::string, Azure::Core::Amqp::_internal::Session> m_sessions;
    std::map<std::string, Azure::Core::Amqp::_internal::Connection> m_connections;

    // Client used for GetProperty operations.
    std::mutex m_propertiesClientLock;
    std::shared_ptr<_detail::EventHubsPropertiesClient> m_propertiesClient;

    /// @brief The options used to configure the consumer client.
    ConsumerClientOptions m_consumerClientOptions;

    void EnsureConnection(std::string const& partitionId, Azure::Core::Context const& context);
    void EnsureSession(std::string const& partitionId, Azure::Core::Context const& context);
    Azure::Core::Amqp::_internal::Connection CreateConnection(
        std::string const& partitionId,
        Azure::Core::Context const& context) const;
    Azure::Core::Amqp::_internal::Session CreateSession(
        std::string const& partitionId,
        Azure::Core::Context const& context) const;
    Azure::Core::Amqp::_internal::Session GetSession(std::string const& partitionId);
    std::shared_ptr<_detail::EventHubsPropertiesClient> GetPropertiesClient(
        Azure::Core::Context const& context);
  };
}}} // namespace Azure::Messaging::EventHubs
