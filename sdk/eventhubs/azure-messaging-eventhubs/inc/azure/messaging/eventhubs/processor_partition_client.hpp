// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "checkpoint_store.hpp"
#include "consumer_client.hpp"
#include "eventhub_constants.hpp"

#include <azure/core/amqp.hpp>
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
    std::string m_partitionId;
    PartitionClient m_partitionClient;
    std::shared_ptr<CheckpointStore> m_checkpointStore;
    std::function<void()> m_cleanupFunc;
    Models::ConsumerClientDetails m_consumerClientDetails;

  public:
    /**  Constructs a new instance of the ProcessorPartitionClient.
     * @param partitionId The identifier of the partition to connect the client to.
     * @param partitionClient The [PartitionClient] to use for receiving events.
     * @param checkpointStore The [CheckpointStore] to use for storing checkpoints.
     * @param consumerClientDetails The [ConsumerClientDetails] to use for storing checkpoints.
     * @param cleanupFunc The function to call when the ProcessorPartitionClient is closed.
     */
    ProcessorPartitionClient(
        std::string partitionId,
        PartitionClient partitionClient,
        std::shared_ptr<CheckpointStore> checkpointStore,
        Models::ConsumerClientDetails consumerClientDetails,
        std::function<void()> cleanupFunc)
        : m_partitionId(partitionId), m_partitionClient(partitionClient),
          m_checkpointStore(checkpointStore), m_cleanupFunc(cleanupFunc),
          m_consumerClientDetails(consumerClientDetails)
    {
    }

    /// Copy a ProcessorPartitionClient to another ProcessorPartitionClient.
    ProcessorPartitionClient(ProcessorPartitionClient const& other) = default;

    /// Assignment operator.
    ProcessorPartitionClient& operator=(ProcessorPartitionClient const& other) = default;

    /** Receives Events from the partition.
     * @param maxBatchSize The maximum number of events to receive in a single call to the service.
     * @param ctx The context to pass to the update checkpoint operation.
     */
    std::vector<Models::ReceivedEventData> ReceiveEvents(
        uint32_t maxBatchSize,
        Core::Context const& context = {})
    {
      return m_partitionClient.ReceiveEvents(maxBatchSize, context);
    }

    /** Closes the partition client.
     */
    void Close()
    {
      if (m_cleanupFunc != nullptr)
      {
        m_cleanupFunc();
      }
      m_partitionClient.Close();
    }

  private:
    void UpdateCheckpoint(
        Azure::Core::Amqp::Models::AmqpMessage const& amqpMessage,
        Core::Context const& context = {})
    {
      Azure::Nullable<int64_t> sequenceNumber;

      Azure::Nullable<int64_t> offsetNumber;

      for (auto const& pair : amqpMessage.MessageAnnotations)
      {
        if (pair.first == _detail::SequenceNumberAnnotation)
        {
          if (pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Int
              || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Uint
              || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Long
              || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Ulong)
            sequenceNumber = static_cast<int64_t>(pair.second);
        }
        if (pair.first == _detail::OffsetNumberAnnotation)
        {
          if (pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Int
              || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Uint
              || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Long
              || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Ulong)
            offsetNumber = static_cast<int64_t>(pair.second);
        }
      }

      Models::Checkpoint checkpoint
          = {m_consumerClientDetails.ConsumerGroup,
             m_consumerClientDetails.EventHubName,
             m_consumerClientDetails.HostName,
             m_partitionId,
             sequenceNumber,
             offsetNumber};

      m_checkpointStore->UpdateCheckpoint(checkpoint, context);
    }

    std::string GetPartitionId() { return m_partitionId; }
  };
}}} // namespace Azure::Messaging::EventHubs
