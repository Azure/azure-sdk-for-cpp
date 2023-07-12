// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

// cspell: words myeventhub

#pragma once
#include "event_data_batch.hpp"
#include "models/management_models.hpp"

#include <azure/core/amqp.hpp>
#include <azure/core/amqp/management.hpp>
#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>

#include <iostream>

namespace Azure { namespace Messaging { namespace EventHubs {

  /**@brief Contains options for the ProducerClient creation
   */
  struct ProducerClientOptions final
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
  class ProducerClient final {
    /// The connection string for the Event Hubs namespace
    std::string m_connectionString;

    /// the Event Hubs namespace name (ex: myeventhub.servicebus.windows.net)
    std::string m_fullyQualifiedNamespace;

    /// The name of the Event Hub
    std::string m_eventHub{};

    /// The URL to the Event Hubs namespace
    std::string m_targetUrl{};

    /// Credentials to be used to authenticate the client.
    std::shared_ptr<Core::Credentials::TokenCredential> m_credential{};

    ProducerClientOptions m_producerClientOptions{};
    std::map<std::string, Azure::Core::Amqp::_internal::MessageSender> m_senders{};
    std::map<std::string, Azure::Core::Amqp::_internal::Session> m_sessions{};

  public:
    /** Get the fully qualified namespace from the connection string */
    std::string const& GetEventHubName() { return m_eventHub; }

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
        ProducerClientOptions options = {});

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
        ProducerClientOptions options = {});

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
     * @param context Request context
     */
    bool SendEventDataBatch(
        EventDataBatch const& eventDataBatch,
        Core::Context const& context = {});

    /**@brief  GetEventHubProperties gets properties of an eventHub. This includes data
     * like name, and partitions.
     *
     * @param context Context for the operation can be used for request cancellation.
     */
    Models::EventHubProperties GetEventHubProperties(Core::Context const& context = {});

    /**@brief  GetPartitionProperties gets properties for a specific partition. This includes data
     * like the last enqueued sequence number, the first sequence number and when an event was last
     * enqueued to the partition.
     *
     * @param partitionID partition ID to detail.
     * @param context Context for the operation can be used for request cancellation.
     */
    Models::EventHubPartitionProperties GetPartitionProperties(
        std::string const& partitionID,
        Core::Context const& context = {});

  private:
    Azure::Core::Amqp::_internal::MessageSender GetSender(std::string const& partitionId = "");
    void CreateSender(std::string const& partitionId = "");
  };
}}} // namespace Azure::Messaging::EventHubs
