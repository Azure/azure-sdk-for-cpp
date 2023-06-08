// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "models/partition_client_models.hpp"
#include <azure/core/amqp.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/nullable.hpp>
namespace Azure { namespace Messaging { namespace EventHubs {
  class ConsumerClient;

  /** PartitionClient is used to receive events from an Event Hub partition.
   *
   * This type is instantiated from the [ConsumerClient] type, using
   * [ConsumerClient.NewPartitionClient].
   */
  class PartitionClient {
    friend class Azure::Messaging::EventHubs::ConsumerClient;

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
    std::vector<Azure::Core::Amqp::Models::AmqpMessage> ReceiveEvents(
        uint32_t const& maxMessages,
        Azure::Core::Context ctx = Azure::Core::Context(),
        Models::ReceiveEventsOptions options = {})
    {
      (void)options;
      std::vector<Azure::Core::Amqp::Models::AmqpMessage> messages;
      //bool prefetchDisabled = m_prefetchCount < 0;

      while (messages.size() < maxMessages && !ctx.IsCancelled())
      {
        auto message = m_receivers[0].WaitForIncomingMessage(ctx);
        if (message)
        {
          messages.push_back(message);
        }
      }

      return messages;
    }

    void Close()
    {
      for (int i = 0; i < m_receivers.size(); i++)
      {
        m_receivers[i].Close();
      }
    }
  };
}}} // namespace Azure::Messaging::EventHubs
