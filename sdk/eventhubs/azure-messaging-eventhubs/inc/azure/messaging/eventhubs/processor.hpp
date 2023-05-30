// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "checkpoint_store.hpp"
#include "eventhubs.hpp"
#include <azure/core/context.hpp>
#include <chrono>

namespace Azure { namespace Messaging { namespace EventHubs {

  /**@brief StartPositions are used if there is no checkpoint for a partition in
   * the checkpoint store.
   */
  struct StartPositions
  {
    /**@brief PerPartition controls the start position for a specific partition,
     * by partition ID. If a partition is not configured here it will default
     * to Default start position.
     */
    std::map<std::string, StartPosition> PerPartition;

    /**@brief  Default is used if the partition is not found in the PerPartition map.
     */
    StartPosition Default;
  };

  /**@brief Processor uses a [ConsumerClient] and [CheckpointStore] to provide automatic
   * load balancing between multiple Processor instances, even in separate
   *processes or on separate machines.
   */
  class Processor {
    Azure::DateTime m_ownershipUpdateInterval;
    StartPositions m_defaultStartPositions;
    std::unique_ptr<CheckpointStore> m_checkpointStore;
    uint32_t m_prefetch;
    PartitionClient m_partitionClient;
    std::vector<PartitionClient> m_nextPartitionClients;
    ConsumerClientDetails m_consumerClientDetails;
    ProcessorLoadBalancer m_loadBalancer;

  };
}}} // namespace Azure::Messaging::EventHubs