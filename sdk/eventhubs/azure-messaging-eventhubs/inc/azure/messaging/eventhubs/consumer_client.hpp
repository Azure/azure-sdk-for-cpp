// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

// cspell: word myeventhub

#pragma once
#include "eventhub_constants.hpp"
#include "models/consumer_client_models.hpp"
#include "models/management_models.hpp"
#include "partition_client.hpp"

#include <azure/core/amqp.hpp>
#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
namespace Azure { namespace Messaging { namespace EventHubs {
  /**@brief Contains options for the ConsumerClient creation
   */
  struct ConsumerClientOptions
  {
    /**@brief ApplicationID is used as the identifier when setting the User-Agent property.
     */
    std::string ApplicationID = "";

    /**@brief  RetryOptions controls how often operations are retried from this client and any
     * Receivers and Senders created from this client.
     */
    Azure::Core::Http::Policies::RetryOptions RetryOptions{};

    /**@brief  Message sender options.
     */
    Azure::Core::Amqp::_internal::MessageReceiverOptions ReceiverOptions{};
  };

  /**@brief Contains credentials for the ConsumerClient creation
   */
  struct ConsumerClientCreds
  {
    /// The connection string for the Event Hubs namespace
    std::string ConnectionString;

    /// the Event Hubs namespace name (ex: myservicebus.servicebus.windows.net)
    std::string HostName;

    /// The name of the Event Hub
    std::string EventHub;

    /// The name of the consumer group
    std::string ConsumerGroup;

    /// Credentials to be used to authenticate the client.
    std::shared_ptr<Core::Credentials::TokenCredential> Credential{};

    /// The URL to the Event Hubs namespace
    std::string HostUrl{};
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
  class ConsumerClient {

  protected:
    /// @brief The message receivers used to receive messages for a given partition.
    std::map<std::string, Azure::Core::Amqp::_internal::MessageReceiver> m_receivers{};
    /// @brief The AMQP Sessions used to receive messages for a given partition.
    std::map<std::string, Azure::Core::Amqp::_internal::Session> m_sessions{};

    /// @brief the credentials used to connect to the event hub.
    ConsumerClientCreds m_credentials;

    /// @brief The options used to configure the consumer client.
    ConsumerClientOptions m_consumerClientOptions;

  public:
    /** Create a new ConsumerClient from an existing one. */
    ConsumerClient(ConsumerClient const& other) = default;

    /** Assign a ConsumerClient to an existing one. */
    ConsumerClient& operator=(ConsumerClient const& other) = default;

    /** @brief Getter for event hub name
     *
     * @returns Event hub name for client
     */
    std::string const& GetEventHubName() { return m_credentials.EventHub; }

    /** @brief Getter for consumer group name
     *
     * @returns Consumer group name for client
     */
    std::string const& GetConsumerGroup() { return m_credentials.ConsumerGroup; }

    /** @brief Getter FQDN
     *
     * @returns FQDN client
     */
    std::string const& GetHostName() { return m_credentials.HostName; }

    /** @brief Getter for client id
     *
     * @returns Clientid for client
     */
    std::string const& GetClientId() { return m_consumerClientOptions.ApplicationID; }

    /** @brief Getter for client details
     *
     * @returns Client details for client
     */
    Models::ConsumerClientDetails GetDetails()
    {
      Models::ConsumerClientDetails details;
      details.ClientID = GetClientId();
      details.ConsumerGroup = GetConsumerGroup();
      details.EventHubName = GetEventHubName();
      details.HostName = GetHostName();
      return details;
    }
    /** @brief Getter for retry options
     *
     * @returns Retry options for client
     */
    Azure::Core::Http::Policies::RetryOptions const& GetRetryOptions()
    {
      return m_consumerClientOptions.RetryOptions;
    }

    /** @brief creates a ConsumerClient from a connection string.
     *
     * @param connectionString connection string to resource
     * @param eventHub event hub name
     * @param consumerGroup consumer group name
     * @param options client options
     *
     * @remark connectionString can be one of two formats - with or without an EntityPath key.
     *  When the connection string does not have an entity path, as shown below, the eventHub
     *  parameter cannot be empty and should contain the name of your event hub.
     *  Endpoint=sb://\<your-namespace\>.servicebus.windows.net/;SharedAccessKeyName=\<key-name\>;SharedAccessKey=\<key\>
     *  When the connection string DOES have an entity path, as shown below, the eventHub parameter
     *  will be ignored.
     *  Endpoint=sb://\<your-namespace\>.servicebus.windows.net/;
     *  SharedAccessKeyName=\<key-name\>;SharedAccessKey=\<key\>;EntityPath=\<entitypath\>;
     */
    ConsumerClient(
        std::string const& connectionString,
        std::string const& eventHub = {},
        std::string const& consumerGroup = _detail::DefaultConsumerGroup,
        ConsumerClientOptions const& options = {});
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
        std::string const& consumerGroup = _detail::DefaultConsumerGroup,
        ConsumerClientOptions const& options = {});

    /** @brief Create new Partition client
     *
     * @param partitionId targeted partition
     * @param options client options
     */
    PartitionClient NewPartitionClient(
        std::string partitionId,
        Models::PartitionClientOptions const& options = {});

    /**@brief  GetEventHubProperties gets properties of an eventHub. This includes data
     * like name, and partitions.
     *
     * @param options Additional options for getting partition properties
     */
    Models::EventHubProperties GetEventHubProperties(
        Models::GetEventHubPropertiesOptions options = {});

    /**@brief  GetPartitionProperties gets properties for a specific partition. This includes data
     * like the last enqueued sequence number, the first sequence number and when an event was last
     * enqueued to the partition.
     *
     * @param partitionID partition ID to detail.
     * @param options Additional options for getting partition properties
     */
    Models::EventHubPartitionProperties GetPartitionProperties(
        std::string const& partitionID,
        Models::GetPartitionPropertiesOptions options = {});
  };
}}} // namespace Azure::Messaging::EventHubs
