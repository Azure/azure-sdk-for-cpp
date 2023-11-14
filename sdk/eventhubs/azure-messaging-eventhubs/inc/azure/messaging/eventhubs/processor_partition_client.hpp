// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include "checkpoint_store.hpp"
#include "consumer_client.hpp"

namespace Azure { namespace Messaging { namespace EventHubs {

  /**@brief  ProcessorPartitionClient allows you to receive events, similar to a [PartitionClient],
   * with a checkpoint store for tracking progress.
   * This type is instantiated from [Processor.NextPartitionClient], which handles load balancing
   * of partition ownership between multiple [Processor] instances.
   *
   *@remark If you do NOT want to use dynamic load balancing, and would prefer to track state and
   * ownership manually, use the [ConsumerClient] instead.
   */
  class ProcessorPartitionClient final {
    friend class Processor;

  public:
    /// Copy a ProcessorPartitionClient to another ProcessorPartitionClient.
    ProcessorPartitionClient(ProcessorPartitionClient const& other) = delete;

    /// Move a ProcessorPartitionClient to another.
    ProcessorPartitionClient(ProcessorPartitionClient&& other) = default;

    /// Assignment operator.
    ProcessorPartitionClient& operator=(ProcessorPartitionClient const& other) = delete;

    /// Move a ProcessorPartitionClient to another.
    ProcessorPartitionClient& operator=(ProcessorPartitionClient&& other) = default;

    ~ProcessorPartitionClient();

    /** Receives Events from the partition.
     * @param maxBatchSize The maximum number of events to receive in a single call to the service.
     * @param context The context to pass to the update checkpoint operation.
     */
    std::vector<std::shared_ptr<const Models::ReceivedEventData>> ReceiveEvents(
        uint32_t maxBatchSize,
        Core::Context const& context = {})
    {
      return m_partitionClient->ReceiveEvents(maxBatchSize, context);
    }

    /**
     * @brief Updates the checkpoint for this partition using the given event data.
     *
     * Subsequent partition client reads will start from this event.
     *
     * @param eventData The event data to use for updating the checkpoint.
     * @param context The context to pass to the update checkpoint operation.
     */
    void UpdateCheckpoint(
        std::shared_ptr<const Models::ReceivedEventData> const& eventData,
        Core::Context const& context = {});

    /// Returns the partition ID associated with this ProcessorPartitionClient.
    std::string PartitionId() const { return m_partitionId; }

    /** Closes the partition client.     */
    void Close()
    {
      if (m_cleanupFunc)
      {
        m_cleanupFunc();
      }
      m_partitionClient->Close();
    }

  private:
    std::string m_partitionId;
    std::unique_ptr<PartitionClient> m_partitionClient{};
    std::shared_ptr<CheckpointStore> m_checkpointStore;
    std::function<void()> m_cleanupFunc;
    Models::ConsumerClientDetails m_consumerClientDetails;

    /**  Constructs a new instance of the ProcessorPartitionClient.
     * @param partitionId The identifier of the partition to connect the client to.
     * @param checkpointStore The [CheckpointStore] to use for storing checkpoints.
     * @param consumerClientDetails The [ConsumerClientDetails] to use for storing checkpoints.
     * @param cleanupFunc The function to call when the ProcessorPartitionClient is closed.
     */
    ProcessorPartitionClient(
        std::string partitionId,
        std::shared_ptr<CheckpointStore> checkpointStore,
        Models::ConsumerClientDetails consumerClientDetails,
        std::function<void()> cleanupFunc)
        : m_partitionId(partitionId), m_checkpointStore(checkpointStore),
          m_cleanupFunc(cleanupFunc), m_consumerClientDetails(consumerClientDetails)
    {
    }

    void SetPartitionClient(std::unique_ptr<PartitionClient>& partitionClient)
    {
      m_partitionClient = std::move(partitionClient);
    }

    void UpdateCheckpoint(
        Azure::Core::Amqp::Models::AmqpMessage const& amqpMessage,
        Core::Context const& context = {});
    std::string GetPartitionId() { return m_partitionId; }
  };
}}} // namespace Azure::Messaging::EventHubs
