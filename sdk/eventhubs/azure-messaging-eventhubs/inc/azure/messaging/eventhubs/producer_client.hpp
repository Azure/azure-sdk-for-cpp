// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "event_data_batch.hpp"
#include "retry_operation.hpp"
#include <azure/core/amqp.hpp>
#include <azure/core/amqp/management.hpp>
#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>

namespace Azure { namespace Messaging { namespace EventHubs {

  /**@brief Credentials data bag used internally by the producer
   */
  struct ProducerClientCreds
  {
    std::string ConnectionString;

    // the Event Hubs namespace name (ex: myservicebus.servicebus.windows.net)
    std::string FullyQualifiedNamespace;

    std::string EventHub;

    std::string TargetUrl;

    std::shared_ptr<Core::Credentials::TokenCredential> Credential;
    std::shared_ptr<Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>
        SasCredential;
  };

  /**@brief Contains options for the ProducerClient creation
   */
  struct ProducerClientOptions
  {
    /**@brief  Application ID that will be passed to the namespace.
     */
    std::string ApplicationID = "";

    /**@brief  RetryOptions controls how often operations are retried from this client and any
     * Receivers and Senders created from this client.
     */
    Azure::Core::Http::Policies::RetryOptions RetryOptions{};

    /**@brief  Message sender options.
     */
    Azure::Core::Amqp::_internal::MessageSenderOptions SenderOptions{};
  };

  /**@brief EventHubProperties represents properties of the Event Hub, like the number of
   * partitions.
   */
  struct EventHubProperties
  {
    Azure::DateTime CreatedOn;
    std::string Name;
    std::vector<std::string> PartitionIDs;
  };

  /**@brief GetEventHubPropertiesOptions contains optional parameters for the GetEventHubProperties
   * function
   */
  struct GetEventHubPropertiesOptions
  {
    // For future expansion
  };

  /**@brief  ProducerClient can be used to send events to an Event Hub.
   */
  class ProducerClient {

    const std::string m_defaultAuthScope = "https://eventhubs.azure.net/.default";
    ProducerClientCreds m_credentials{};
    ProducerClientOptions m_producerClientOptions{};
    std::map<std::string, Azure::Core::Amqp::_internal::MessageSender> m_senders{};
    std::map<std::string, Azure::Core::Amqp::_internal::Management> m_management{};

  public:
    std::string const& GetEventHubName() { return m_credentials.EventHub; }

    Azure::Core::Http::Policies::RetryOptions const& GetRetryOptions()
    {
      return m_producerClientOptions.RetryOptions;
    }

    /**@brief Constructs a new ProducerClient instance
     *
     * @param connectionString Event hubs connection string
     * @param eventHub Event hub name
     * @param options Additional options for creating the client
     */
    ProducerClient(
        std::string const& connectionString,
        std::string const& eventHub,
        ProducerClientOptions options = ProducerClientOptions());

    /**@brief Constructs a new ProducerClient instance
     *
     * @param fullyQualifiedNamespace Fully qualified namespace name
     * @param eventHub Event hub name
     * @param options Additional options for creating the client
     */
    ProducerClient(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHub,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
        ProducerClientOptions options = ProducerClientOptions());

    ~ProducerClient()
    {
      for (auto sender : m_senders)
      {
        sender.second.Close();
      }
      m_senders.clear();
    }

    /**@brief Proceeds to send and EventDataBatch
     *
     * @param eventDataBatch Batch to send
     * @param ctx Request context
     */
    bool const SendEventDataBatch(
        EventDataBatch& eventDataBatch,
        Azure::Core::Context ctx = Azure::Core::Context());

    EventHubProperties GetEventHubProperties(
        GetEventHubPropertiesOptions options = GetEventHubPropertiesOptions(),
        Azure::Core::Context ctx = Azure::Core::Context())
    {
      (void)options;
      (void)ctx;
      EventHubProperties ehp;
      if (m_management.find(m_credentials.EventHub) == m_management.end())
      {
        //auto management = CreateManagement(m_credentials.EventHub, true);
        //m_management[m_credentials.EventHub] = management;
      }

      auto management = m_management[m_credentials.EventHub];
      std::string token;
      if (m_credentials.SasCredential != nullptr)
      {

        Azure::Core::Credentials::TokenRequestContext tokenRequestContext;
        tokenRequestContext.MinimumExpiration = std::chrono::minutes(15);
        tokenRequestContext.Scopes = {m_defaultAuthScope};
        token = m_credentials.SasCredential->GetToken(tokenRequestContext, ctx).Token;
      }
      else
      {
        Azure::Core::Credentials::TokenRequestContext tokenRequestContext;
        tokenRequestContext.MinimumExpiration = std::chrono::minutes(15);
        tokenRequestContext.Scopes = {m_defaultAuthScope};
        token = m_credentials.Credential->GetToken(tokenRequestContext, ctx).Token;
      }
      Azure::Core::Amqp::Models::AmqpMessage message;
      message.ApplicationProperties.emplace("name", m_credentials.EventHub);
      message.ApplicationProperties.emplace("type", "com.microsoft:eventhub");
      message.ApplicationProperties.emplace("security_token", token);
      message.ApplicationProperties.emplace("operation", "READ");
      Azure::Core::Amqp::_internal::ManagementOpenStatus openStatus = management.Open(ctx);
      
      if (openStatus != Azure::Core::Amqp::_internal::ManagementOpenStatus::Ok)
      {
          throw std::runtime_error("Failed to open management link");
      }

      auto response
          = management.ExecuteOperation("READ", "com.microsoft:eventhub", "EN-US", message, ctx);
      management.Close();
      if (response.Status == Azure::Core::Amqp::_internal::ManagementOperationStatus::Ok)
      {
          
      }
      return ehp;
    }

  private:
    Azure::Core::Amqp::_internal::MessageSender GetSender(std::string const& partitionId = "");
    void CreateSender(std::string const& partitionId = "");
    /* Azure::Core::Amqp::_internal::Management CreateManagement(
        std::string name,
        bool eventHub = false);*/
  };
}}} // namespace Azure::Messaging::EventHubs