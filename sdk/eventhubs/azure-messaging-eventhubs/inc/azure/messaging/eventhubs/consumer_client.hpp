// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "partition_client.hpp"
#include <azure/core/amqp.hpp>
#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/messaging/eventhubs.hpp>

#ifdef TESTING_BUILD_AMQP
namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  class ConsumerClientTest_ConnectionStringEntityPath_Test;
}}}} // namespace Azure::Messaging::EventHubs::Test
#endif

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
#ifdef TESTING_BUILD_AMQP
  friend class Azure::Messaging::EventHubs::Test::
      ConsumerClientTest_ConnectionStringEntityPath_Test;
#endif // TESTING_BUILD_AMQP

protected:
  std::map<std::string, Azure::Core::Amqp::_internal::MessageReceiver> m_receivers{};
  ConsumerClientCreds m_credentials;
  ConsumerClientOptions m_consumerClientOptions;
  //std::map<std::string, std::function<void(Azure::Core::Amqp::Models::AmqpMessage)>> m_processors;

public:
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
      ConsumerClientOptions const& optionsv = ConsumerClientOptions());

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
      ConsumerClientOptions const& options = ConsumerClientOptions());

  /** @brief Create new Partition client o
   *
   * @param partitionId targeted partition
   * @param options client options
   */
  PartitionClient NewPartitionClient(
      std::string partitionId,
      PartitionClientOptions const& options);
};
}}} // namespace Azure::Messaging::EventHubs