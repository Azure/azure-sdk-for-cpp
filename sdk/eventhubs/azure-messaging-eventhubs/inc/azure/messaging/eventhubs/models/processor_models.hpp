// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "checkpoint_store.hpp"
#include "eventhubs.hpp"
#include <azure/core/context.hpp>
#include <chrono>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

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
}}}} // namespace Azure::Messaging::EventHubs::Models
