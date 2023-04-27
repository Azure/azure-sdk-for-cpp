// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include <azure/core/amqp.hpp>
#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/messaging/eventhubs.hpp>
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

  struct ConsumerClientCreds
  {
    std::string ConnectionString;

    // the Event Hubs namespace name (ex: myservicebus.servicebus.windows.net)
    std::string FullyQualifiedNamespace;

    std::string EventHub;

    std::string ConsumerGroup;

    std::shared_ptr<Core::Credentials::TokenCredential> Credential;
    std::shared_ptr<Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>
        SasCredential;

    std::string HostUrl;
  };

  class ConsumerClient {
  private:
    std::string m_consumerGroup;
    std::string m_eventHub;
    std::map<std::string, Azure::Core::Amqp::_internal::MessageReceiver> m_receivers{};
    ConsumerClientCreds m_credentials;
    ConsumerClientOptions m_consumerClientOptions;
    std::map<std::string, std::function<void(Azure::Core::Amqp::Models::AmqpMessage)>> m_processors;
  public:
    std::string const& GetEventHubName() { return m_credentials.EventHub; }

    Azure::Core::Http::Policies::RetryOptions const& GetRetryOptions() { return m_consumerClientOptions.RetryOptions; }

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
     *  must be empty.
     *  Endpoint=sb://<your-namespace>.servicebus.windows.net/;
     *  SharedAccessKeyName=<key-name>;SharedAccessKey=<key>;EntityPath=<entitypath>;
     */
    ConsumerClient(
        std::string const& connectionsString,
        std::string const& eventHub,
        std::string const& consumerGroup = "$Default",
        ConsumerClientOptions const& optionsv = ConsumerClientOptions());

    /** @brief creates a ConsumerClient from a token credential.
     *
     * @param fullyQualifiedNamespace fully qualified namespace name (e.g. myeventhub.servicebus.windows.net)
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
        ConsumerClientOptions const& options = ConsumerClientOptions());

    void SetConsumer(
        std::function<void(Azure::Core::Amqp::Models::AmqpMessage)> consumerMethod,
        std::string partitionId = "")
    {
      m_processors.insert_or_assign(
          partitionId.empty() ? m_credentials.ConsumerGroup : partitionId, consumerMethod);
    }

    void StartConsuming(
        int const& maxMessages,
        std::string const& partitionId = "",
        Azure::Core::Context ctx = Azure::Core::Context());

    private:
    void CreateConsumer(std::string const& partitionId = "");
      Azure::Core::Amqp::_internal::MessageReceiver GetConsumer(
          std::string const& partitionId = "");
  };
}}} // namespace Azure::Messaging::EventHubs