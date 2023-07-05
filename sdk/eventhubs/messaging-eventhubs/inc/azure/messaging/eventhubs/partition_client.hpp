// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "models/event_data.hpp"
#include "models/partition_client_models.hpp"

#include <azure/core/amqp.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/nullable.hpp>
namespace Azure { namespace Messaging { namespace EventHubs {

  /** PartitionClient is used to receive events from an Event Hub partition.
   *
   * This type is instantiated from the [ConsumerClient] type, using
   * [ConsumerClient.NewPartitionClient].
   */
  class PartitionClient {

  protected:
    std::vector<Azure::Core::Amqp::_internal::MessageReceiver> m_receivers{};
    std::string m_offsetExpression;
    uint64_t m_ownerLevel;
    int32_t m_prefetchCount;
    Models::PartitionClientOptions m_partitionOptions;
    std::string m_partitionId;
    Azure::Core::Http::Policies::RetryOptions RetryOptions{};

    /*@brief DefaultConsumerGroup is the name of the default consumer group in the Event Hubs
     * service.
     */
    const uint32_t defaultPrefetchSize = 300;

    /**@brief defaultLinkRxBuffer is the maximum number of transfer frames we can handle
     * on the Receiver. This matches the current default window size that go-amqp
     * uses for sessions.
     */
    const uint32_t defaultMaxCreditSize = 5000;

  public:
    PartitionClient(PartitionClient const& other)
        : m_receivers(other.m_receivers), m_offsetExpression(other.m_offsetExpression),
          m_ownerLevel(other.m_ownerLevel), m_prefetchCount(other.m_prefetchCount),
          m_partitionOptions(other.m_partitionOptions), m_partitionId(other.m_partitionId),
          RetryOptions(other.RetryOptions)
    {
    }

    PartitionClient& operator=(PartitionClient const& other)
    {
      if (this != &other)
      {
        m_receivers = other.m_receivers;
        m_offsetExpression = other.m_offsetExpression;
        m_ownerLevel = other.m_ownerLevel;
        m_prefetchCount = other.m_prefetchCount;
        m_partitionOptions = other.m_partitionOptions;
        m_partitionId = other.m_partitionId;
        RetryOptions = other.RetryOptions;
      }
      return *this;
    }
    std::vector<Models::ReceivedEventData> ReceiveEvents(
        uint32_t const& maxMessages,
        Azure::Core::Context ctx = Azure::Core::Context(),
        Models::ReceiveEventsOptions options = {})
    {
      (void)options;
      std::vector<Models::ReceivedEventData> messages;
      // bool prefetchDisabled = m_prefetchCount < 0;

      while (messages.size() < maxMessages && !ctx.IsCancelled())
      {
        auto message = m_receivers[0].WaitForIncomingMessage(ctx);
        if (message.first.HasValue())
        {
          messages.push_back(Models::ReceivedEventData{message.first.Value()});
        }
      }
      return messages;
    }

    void Close()
    {
      for (size_t i = 0; i < m_receivers.size(); i++)
      {
        m_receivers[i].Close();
      }
    }
    PartitionClient(
        Models::PartitionClientOptions options,
        Azure::Core::Http::Policies::RetryOptions retryOptions)
    {
      m_partitionOptions = options;
      RetryOptions = retryOptions;
    }

    void PushBackReceiver(Azure::Core::Amqp::_internal::MessageReceiver& receiver)
    {
      m_receivers.push_back(std::move(receiver));
    }
  };
}}} // namespace Azure::Messaging::EventHubs
