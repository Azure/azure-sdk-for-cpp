// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include "eventhubs_exception.hpp"
#include "models/event_data.hpp"
#include "models/partition_client_models.hpp"

#include <azure/core/amqp.hpp>
#include <azure/core/amqp/internal/message_receiver.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/nullable.hpp>

#include <memory>

namespace Azure { namespace Messaging { namespace EventHubs {
  namespace _detail {
    class PartitionClientFactory;
    struct PartitionClientState;
  } // namespace _detail

  class ConsumerClient;
  /**brief PartitionClientOptions provides options for the ConsumerClient::CreatePartitionClient
   * function.
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
    Azure::Nullable<std::int64_t> OwnerLevel{};

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

  public:
    /// Create a PartitionClient from another PartitionClient
    PartitionClient(PartitionClient const& other) = delete;
    /// Create a PartitionClient moving from another PartitionClient
    PartitionClient(PartitionClient&& other) = default;

    /// Assign a PartitionClient to another PartitionClient
    PartitionClient& operator=(PartitionClient const& other) = delete;
    /// Move a PartitionClient to another PartitionClient
    PartitionClient& operator=(PartitionClient&& other);

    /** Destroy this partition client.
     */
    virtual ~PartitionClient();

    /** Receive events from the partition.
     *
     * @param maxMessages The maximum number of messages to receive.
     * @param context A context to control the request lifetime.
     * @return A vector of received events.
     *
     */
    std::vector<std::shared_ptr<const Models::ReceivedEventData>> ReceiveEvents(
        uint32_t maxMessages,
        Core::Context const& context = {});

    /** @brief Closes the connection to the Event Hub service.
     */
    void Close(Core::Context const& context);

  private:
    friend class _detail::PartitionClientFactory;
    friend class ConsumerClient;

    std::shared_ptr<_detail::PartitionClientState> m_state;

    /// The message receiver used to receive events from the partition.
    Azure::Core::Amqp::_internal::MessageReceiver m_receiver;

    /// The AMQP session that carries the receiver. A rebuild reattaches to this session.
    Azure::Core::Amqp::_internal::Session m_session;
    /// The address of the partition. A rebuild reuses this address.
    std::string m_partitionUrl;
    /// The link name of the receiver. A rebuild reuses this name.
    std::string m_receiverName;

    /// The offset of the last event received. A rebuild starts just after it.
    Azure::Nullable<std::string> m_lastReceivedOffset;

    /// The error that ended the last ReceiveEvents call. The next call recovers from it.
    Azure::Nullable<Azure::Core::Amqp::Models::_internal::AmqpError> m_pendingError;

    /// The options used to create the PartitionClient.
    PartitionClientOptions m_partitionOptions;

    /// The name of the partition.
    //    std::string m_partitionId;

    /** @brief RetryOptions controls how many times we should retry an operation in
     * response to being throttled or encountering a transient error.
     */
    Azure::Core::Http::Policies::RetryOptions m_retryOptions{};

#if ENABLE_UAMQP
    explicit PartitionClient(std::shared_ptr<_detail::PartitionClientState> state);
    std::shared_ptr<_detail::PartitionClientState> GetState() const { return m_state; }
#endif

#if ENABLE_RUST_AMQP
    /** Creates a new PartitionClient
     *
     * @param messageReceiver Message Receiver for the partition client.
     * @param session The AMQP session that carries the message receiver.
     * @param partitionUrl The address of the partition.
     * @param receiverName The link name of the message receiver.
     * @param options options used to create the PartitionClient.
     * @param retryOptions controls how many times we should retry an operation in response to being
     * throttled or encountering a transient error.
     */
    PartitionClient(
        Azure::Core::Amqp::_internal::MessageReceiver const& messageReceiver,
        Azure::Core::Amqp::_internal::Session const& session,
        std::string partitionUrl,
        std::string receiverName,
        PartitionClientOptions options,
        Core::Http::Policies::RetryOptions retryOptions);
#endif

#if ENABLE_RUST_AMQP || ENABLE_UAMQP
    /// Closes the faulted receiver and attaches a new one starting after the last offset.
    void RebuildReceiver(Core::Context const& context);
#endif

#if ENABLE_RUST_AMQP
    std::string GetStartExpression(Models::StartPosition const& startPosition);
#endif
  };
}}} // namespace Azure::Messaging::EventHubs
