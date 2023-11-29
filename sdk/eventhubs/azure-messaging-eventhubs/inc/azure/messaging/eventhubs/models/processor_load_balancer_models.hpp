// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include "checkpoint_store_models.hpp"

#include <azure/core/context.hpp>

#include <chrono>
#include <vector>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {
  enum class ProcessorStrategy
  {
    // ProcessorStrategyBalanced will attempt to claim a single partition at a time, until each
    // active owner has an equal share of partitions. This is the default strategy.
    ProcessorStrategyBalanced,

    // ProcessorStrategyGreedy will attempt to claim as many partitions at a time as it can,
    // ignoring balance.
    ProcessorStrategyGreedy
  };

}}}} // namespace Azure::Messaging::EventHubs::Models


