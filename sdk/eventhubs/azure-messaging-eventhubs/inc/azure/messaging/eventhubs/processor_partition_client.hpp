// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "checkpoint_store.hpp"
#include "partition_client.hpp"
#include <azure/core/amqp.hpp>
namespace Azure { namespace Messaging { namespace EventHubs { namespace Azure {



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
    CheckpointStore m_checkpointStore;
    std::function<void> m_cleanupFunc;
    ConsumerClientDetails m_consumerClientDetails;


  };
}}}} // namespace Azure::Messaging::EventHubs::Azure