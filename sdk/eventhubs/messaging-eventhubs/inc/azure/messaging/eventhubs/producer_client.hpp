// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "event_data_batch.hpp"
#include "models/management_models.hpp"
#include "models/producer_client_models.hpp"
#include "retry_operation.hpp"

#include <azure/core/amqp.hpp>
#include <azure/core/amqp/management.hpp>
#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>

#include <iostream>

namespace Azure { namespace Messaging { namespace EventHubs {
  /**@brief  ProducerClient can be used to send events to an Event Hub.
   */
  class ProducerClient {

    Models::ProducerClientCreds m_credentials{};
    Models::ProducerClientOptions m_producerClientOptions{};
    std::map<std::string, Azure::Core::Amqp::_internal::MessageSender> m_senders{};
    std::map<std::string, Azure::Core::Amqp::_internal::Session> m_sessions{};

  public:
    /** Get the fully qualified namespace from the connection string */
    std::string const& GetEventHubName() { return m_credentials.EventHub; }

    /** Get Retry options for this ProducerClient */
    Azure::Core::Http::Policies::RetryOptions const& GetRetryOptions()
    {
      return m_producerClientOptions.RetryOptions;
    }

    /** Create a ProducerClient from another ProducerClient. */
    ProducerClient(ProducerClient const& other) = default;

    /** Assign a ProducerClient another ProducerClient. */
    ProducerClient& operator=(ProducerClient const& other) = default;

    /** Default Constructor for a ProducerClient */
    ProducerClient() = default;

    /**@brief Constructs a new ProducerClient instance
     *
     * @param connectionString Event hubs connection string
     * @param eventHub Event hub name
     * @param options Additional options for creating the client
     */
    ProducerClient(
        std::string const& connectionString,
        std::string const& eventHub,
        Models::ProducerClientOptions options = {});

    /**@brief Constructs a new ProducerClient instance
     *
     * @param fullyQualifiedNamespace Fully qualified namespace name
     * @param eventHub Event hub name
     * @param credential Credential to use for authentication
     * @param options Additional options for creating the client
     */
    ProducerClient(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHub,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
        Models::ProducerClientOptions options = {});

    ~ProducerClient()
    {
      for (auto& sender : m_senders)
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
    bool SendEventDataBatch(
        EventDataBatch const& eventDataBatch,
        Azure::Core::Context ctx = Azure::Core::Context());
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

  private:
    Azure::Core::Amqp::_internal::MessageSender GetSender(std::string const& partitionId = "");
    void CreateSender(std::string const& partitionId = "");
  };
}}} // namespace Azure::Messaging::EventHubs
