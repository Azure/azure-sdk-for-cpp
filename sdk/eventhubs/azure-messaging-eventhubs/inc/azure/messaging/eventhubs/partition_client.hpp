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
  /**brief PartitionClientOptions provides options for the CreatePartitionClient function.
   */
  struct PartitionClientOptions final
  {
    /**@brief StartPosition is the position we will start receiving events from,
     * either an offset (inclusive) with Offset, or receiving events received
     * after a specific time using EnqueuedTime.
     *
     *@remark NOTE: you can also use the [Processor], which will automatically manage the start
     * value using a [CheckpointStore]. See [example_consuming_with_checkpoints_test.go] for an
     * example.
     */
    Models::StartPosition StartPosition;

    /**@brief OwnerLevel is the priority for this partition client, also known as the 'epoch' level.
     * When used, a partition client with a higher OwnerLevel will take ownership of a partition
     * from partition clients with a lower OwnerLevel.
     * Default is off.
     */
    int64_t OwnerLevel;

    /**@brief Prefetch represents the size of the internal prefetch buffer. When set,
     * this client will attempt to always maintain an internal cache of events of
     * this size, asynchronously, increasing the odds that ReceiveEvents() will use
     * a locally stored cache of events, rather than having to wait for events to
     * arrive from the network.
     *
     * Defaults to 300 events if Prefetch == 0.
     * Disabled if Prefetch < 0.
     */

    int32_t Prefetch = 300;
  };

  /** PartitionClient is used to receive events from an Event Hub partition.
   *
   * This type is instantiated from the [ConsumerClient] type, using
   * [ConsumerClient.CreatePartitionClient].
   */
  class PartitionClient final {

  protected:
    /// The message receivers used to receive events from the partition.
    std::vector<Azure::Core::Amqp::_internal::MessageReceiver> m_receivers{};

    /// The name of the offset to start receiving events from.
    std::string m_offsetExpression;

    /// The level of the ownership.
    uint64_t m_ownerLevel;

    /// The number of events to prefetch at any time.
    int32_t m_prefetchCount;

    /// The options used to create the PartitionClient.
    PartitionClientOptions m_partitionOptions;

    /// The name of the partition.
    std::string m_partitionId;

    /** @brief RetryOptions controls how many times we should retry an operation in
     * response to being throttled or encountering a transient error.
     */
    Azure::Core::Http::Policies::RetryOptions RetryOptions{};

  public:
    /// Create a PartitionClient from another PartitionClient
    PartitionClient(PartitionClient const& other) = default;

    /// Assign a PartitionClient to another PartitionClient
    PartitionClient& operator=(PartitionClient const& other) = default;

    /** Receive events from the partition.
     *
     * @param maxMessages The maximum number of messages to receive.
     * @param context A context to control the request lifetime.
     * @return A vector of received events.
     *
     */
    std::vector<Models::ReceivedEventData> ReceiveEvents(
        uint32_t const& maxMessages,
        Core::Context const& context = {})
    {
      std::vector<Models::ReceivedEventData> messages;
      // bool prefetchDisabled = m_prefetchCount < 0;

      while (messages.size() < maxMessages && !context.IsCancelled())
      {
        auto message = m_receivers[0].WaitForIncomingMessage(context);
        if (message.first.HasValue())
        {
          messages.push_back(Models::ReceivedEventData{message.first.Value()});
        }
      }
      return messages;
    }

    /** @brief Closes the connection to the Event Hub service.
     */
    void Close()
    {
      for (size_t i = 0; i < m_receivers.size(); i++)
      {
        m_receivers[i].Close();
      }
    }

    /** Creates a new PartitionClient
     *
     * @param options The options used to create the PartitionClient.
     * @param retryOptions The retry options used to create the PartitionClient.
     *
     */
    PartitionClient(
        PartitionClientOptions options,
        Azure::Core::Http::Policies::RetryOptions retryOptions)
    {
      m_partitionOptions = options;
      RetryOptions = retryOptions;
    }

    /// @brief Push the message receiver back to the vector of receivers.
    void PushBackReceiver(Azure::Core::Amqp::_internal::MessageReceiver& receiver)
    {
      m_receivers.push_back(std::move(receiver));
    }
  };
}}} // namespace Azure::Messaging::EventHubs
