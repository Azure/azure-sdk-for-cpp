// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "checkpoint_store.hpp"
#include "consumer_client.hpp"

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
  class ProcessorPartitionClient {
    std::string m_partitionId;
    PartitionClient m_partitionClient;
    std::unique_ptr<CheckpointStore> m_checkpointStore;
    std::function<void()> m_cleanupFunc;
    ConsumerClientDetails m_consumerClientDetails;
    const Azure::Core::Amqp::Models::AmqpValue sequenceNumberAnnotation = "x-opt-sequence-number";
    const Azure::Core::Amqp::Models::AmqpValue offsetNumberAnnotation = "x-opt-offset";

  public:
    ProcessorPartitionClient(
        std::string partitionId,
        PartitionClient partitionClient,
        std::unique_ptr<CheckpointStore> checkpointStore,
        ConsumerClientDetails consumerClientDetails,
        std::function<void()> cleanupFunc)
        : m_partitionId(std::move(partitionId)), m_partitionClient(std::move(partitionClient)),
          m_checkpointStore(std::move(checkpointStore)), m_cleanupFunc(std::move(cleanupFunc)),
          m_consumerClientDetails(std::move(consumerClientDetails))
    {
    }
    

    std::vector<Azure::Core::Amqp::Models::AmqpMessage> ReceiveEvents(
        uint32_t maxBatchSize,
        Azure::Core::Context ctx = {})
    {
      return m_partitionClient.ReceiveEvents(maxBatchSize, ctx);
    }

    void UpdateCheckpoint(
        Azure::Core::Amqp::Models::AmqpMessage const& amqpMessage,
        Azure::Core::Context ctx = {},
        UpdateCheckpointOptions options = {})
    {
      Azure::Nullable<int64_t> sequenceNumber;

      Azure::Nullable<int64_t> offsetNumber;

      for (auto pair : amqpMessage.MessageAnnotations)
      {
        if (pair.first == sequenceNumberAnnotation)
        {
          if (pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Int
              || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Uint
              || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Long
              || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Ulong)
            sequenceNumber = (size_t)pair.second;
        }
        if (pair.first == offsetNumberAnnotation)
        {
          if (pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Int
              || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Uint
              || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Long
              || pair.second.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Ulong)
            offsetNumber = (size_t)pair.second;
        }
      }

      Checkpoint checkpoint
          = {m_consumerClientDetails.ConsumerGroup,
             m_consumerClientDetails.EventHubName,
             m_consumerClientDetails.FullyQualifiedNamespace,
             m_partitionId,
             sequenceNumber,
             offsetNumber};

      m_checkpointStore->UpdateCheckpoint(checkpoint, ctx, options);
    }

    std::string GetPartitionId() { return m_partitionId; }

    void Close()
    {
      if (m_cleanupFunc != nullptr)
      {
        m_cleanupFunc();
      }
      m_partitionClient.Close();
    }
  };
}}} // namespace Azure::Messaging::EventHubs
