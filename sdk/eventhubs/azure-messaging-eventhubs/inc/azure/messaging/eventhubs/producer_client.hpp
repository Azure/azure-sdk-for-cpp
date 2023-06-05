// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "event_data_batch.hpp"
#include "retry_operation.hpp"
#include "models/management_models.hpp"

#include <azure/core/amqp.hpp>
#include <azure/core/amqp/management.hpp>
#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>

#include <iostream>

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

  /**@brief  ProducerClient can be used to send events to an Event Hub.
   */
  class ProducerClient {

    const std::string m_defaultAuthScope = "https://eventhubs.azure.net/.default";
    ProducerClientCreds m_credentials{};
    ProducerClientOptions m_producerClientOptions{};
    std::map<std::string, Azure::Core::Amqp::_internal::MessageSender> m_senders{};
    std::map<std::string, Azure::Core::Amqp::_internal::Session> m_sessions{};

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
        ProducerClientOptions options = {});

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
        ProducerClientOptions options = {});

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
    /**@brief  GetEventHubProperties gets properties of an eventHub. This includes data
     * like name, and partitions.
     *
     * @param options Additional options for getting partition properties
     */
    Models::EventHubProperties GetEventHubProperties(Models::GetEventHubPropertiesOptions options = {});
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

  private:
    Azure::Core::Amqp::_internal::MessageSender GetSender(std::string const& partitionId = "");
    void CreateSender(std::string const& partitionId = "");
  };
}}} // namespace Azure::Messaging::EventHubs