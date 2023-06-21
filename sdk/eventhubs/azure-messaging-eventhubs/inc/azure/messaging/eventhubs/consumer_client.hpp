// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "models/consumer_client_models.hpp"
#include "models/management_models.hpp"
#include "models/partition_client_models.hpp"
#include "partition_client.hpp"

#include <azure/core/amqp.hpp>
#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
namespace Azure { namespace Messaging { namespace EventHubs {

  class ConsumerClient {

  protected:
    const uint32_t defaultMaxSize = 5000;
    const std::string defaultConsumerGroup = "$Default";
    std::map<std::string, Azure::Core::Amqp::_internal::MessageReceiver> m_receivers{};
    std::map<std::string, Azure::Core::Amqp::_internal::Session> m_sessions{};
    Models::ConsumerClientCreds m_credentials;
    Models::ConsumerClientOptions m_consumerClientOptions;

  public:
    ConsumerClient(ConsumerClient const& other)
        : m_receivers(other.m_receivers), m_sessions(other.m_sessions),
          m_credentials(other.m_credentials), m_consumerClientOptions(other.m_consumerClientOptions)
    {
    }

    ConsumerClient& operator=(ConsumerClient const& other)
    {
      if (&other == this)
      {
        return *this;
      }

      m_receivers = other.m_receivers;
      m_sessions = other.m_sessions;
      m_credentials = other.m_credentials;
      m_consumerClientOptions = other.m_consumerClientOptions;
      return *this;
    }
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
    std::string const& GetFullyQualifiedNamespace()
    {
      return m_credentials.FullyQualifiedNamespace;
    }

    /** @brief Getter for client id
     *
     * @returns Clientid for client
     */
    std::string const& GetClientId() { return m_consumerClientOptions.ApplicationID; }

    Models::ConsumerClientDetails GetDetails()
    {
      Models::ConsumerClientDetails details;
      details.ClientID = GetClientId();
      details.ConsumerGroup = GetConsumerGroup();
      details.EventHubName = GetEventHubName();
      details.FullyQualifiedNamespace = GetFullyQualifiedNamespace();
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
     *  Endpoint=sb://<your-namespace>.servicebus.windows.net/;SharedAccessKeyName=<key-name>;SharedAccessKey=<key>
     *  When the connection string DOES have an entity path, as shown below, the eventHub parameter
     *  will be ignored.
     *  Endpoint=sb://<your-namespace>.servicebus.windows.net/;
     *  SharedAccessKeyName=<key-name>;SharedAccessKey=<key>;EntityPath=<entitypath>;
     */
    ConsumerClient(
        std::string const& connectionsString,
        std::string const& eventHub = "",
        std::string const& consumerGroup = "$Default",
        Models::ConsumerClientOptions const& optionsv = {});
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
        std::string const& consumerGroup = "$Default",
        Models::ConsumerClientOptions const& options = {});

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
