// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include <azure/core/amqp.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/nullable.hpp>
namespace Azure { namespace Messaging { namespace EventHubs {
  class ConsumerClient;

  struct ConsumerClientDetails
  {
    std::string FullyQualifiedNamespace;
    std::string ConsumerGroup;
    std::string EventHubName;
    std::string ClientID;
  };

  /**@brief StartPosition indicates the position to start receiving events within a partition.
   * The default position is Latest.
   *
   * @remark You can set this in the options for [ConsumerClient.NewPartitionClient].
   */
  struct StartPosition
  {
    /**@brief Offset will start the consumer after the specified offset. Can be exclusive
     * or inclusive, based on the Inclusive property.
     *@remark NOTE: offsets are not stable values, and might refer to different events over time
     * as the Event Hub events reach their age limit and are discarded.
     */
    Azure::Nullable<int64_t> Offset;

    /**@brief SequenceNumber will start the consumer after the specified sequence number. Can be
     * exclusive or inclusive, based on the Inclusive property.
     */
    Azure::Nullable<int64_t> SequenceNumber;

    /**@brief EnqueuedTime will start the consumer before events that were enqueued on or after
     * EnqueuedTime. Can be exclusive or inclusive, based on the Inclusive property.
     */
    Azure::Nullable<Azure::DateTime> EnqueuedTime;

    /**@brief Inclusive configures whether the events directly at Offset,
     * SequenceNumber or EnqueuedTime will be included (true) or excluded
     * (false).
     */
    bool Inclusive;

    /**@brief Earliest will start the consumer at the earliest event.
     */
    Azure::Nullable<bool> Earliest;

    /**@brief Latest will start the consumer after the last event.
     */
    Azure::Nullable<bool> Latest;
  };

  /**brief PartitionClientOptions provides options for the NewPartitionClient function.
   */
  struct PartitionClientOptions
  {
    /**@brief StartPosition is the position we will start receiving events from,
     * either an offset (inclusive) with Offset, or receiving events received
     * after a specific time using EnqueuedTime.
     *
     *@remark NOTE: you can also use the [Processor], which will automatically manage the start
     * value using a [CheckpointStore]. See [example_consuming_with_checkpoints_test.go] for an
     * example.
     */
    StartPosition StartPosition;

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
  /**@brief ReceiveEventsOptions contains optional parameters for the ReceiveEvents function
   */
  struct ReceiveEventsOptions
  {
    // For future expansion
  };

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
    uint32_t m_prefetchCount;
    Azure::Core::Http::Policies::RetryOptions RetryOptions{};
    PartitionClientOptions m_partitionOptions;
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
        int const& maxMessages,
        Azure::Core::Context ctx = Azure::Core::Context(),
        ReceiveEventsOptions options = ReceiveEventsOptions())
    {
      (void)options;
      std::vector<Azure::Core::Amqp::Models::AmqpMessage> messages;
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