// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "checkpoint_store_models.hpp"
#include "eventhubs.hpp"

#include <azure/core/context.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <chrono>
namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

  /**@brief StartPositions are used if there is no checkpoint for a partition in
   *         the checkpoint store.
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

  /**@brief ProcessorOptions are the options for the NewProcessor
   * function.
   */
  struct ProcessorOptions
  {
    /**@briefLoadBalancingStrategy dictates how concurrent Processor instances distribute
     * ownership of partitions between them.
     * The default strategy is ProcessorStrategyBalanced.
     */
    ProcessorStrategy LoadBalancingStrategy;

    /**@brief UpdateInterval controls how often attempt to claim partitions.
     * The default value is 10 seconds.
     */
    Azure::DateTime::duration UpdateInterval;

    /**@brief PartitionExpirationDuration is the amount of time before a partition is
     * considered unowned. The default value is 60 seconds.
     */
    Azure::DateTime::duration PartitionExpirationDuration;

    /**@brief StartPositions are the default start positions (configurable per
     * partition, or with an overall default value) if a checkpoint is not found
     * in the CheckpointStore. The default position is Latest.
     */
    StartPositions StartPositions;

    /**@brief Prefetch represents the size of the internal prefetch buffer for
     * each ProcessorPartitionClient created by this Processor. When
     * set, this client will attempt to always maintain an internal
     * cache of events of this size, asynchronously, increasing the odds
     * that ReceiveEvents() will use a locally stored cache of events,
     * rather than having to wait for events to arrive from the network.
     *
     * Defaults to 300 events.
     * Disabled if Prefetch < 0.
     */
    int32_t Prefetch = 300;
  };
}}}} // namespace Azure::Messaging::EventHubs::Models
