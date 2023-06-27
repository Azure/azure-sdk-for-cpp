// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include <azure/core/context.hpp>

#include <chrono>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {
  enum ProcessorStrategy
  {
    // ProcessorStrategyBalanced will attempt to claim a single partition at a time, until each
    // active owner has an equal share of partitions. This is the default strategy.
    ProcessorStrategyBalanced,

    // ProcessorStrategyGreedy will attempt to claim as many partitions at a time as it can,
    // ignoring balance.
    ProcessorStrategyGreedy
  };

  struct LoadBalancerInfo
  {
    /// current are the partitions that _we_ own
    std::vector<Ownership> Current;

    /// unownedOrExpired partitions either had no claim _ever_ or were once
    /// owned but the ownership claim has expired.
    std::vector<Ownership> UnownedOrExpired;

    /// aboveMax are ownerships where the specific owner has too many partitions
    /// it contains _all_ the partitions for that particular consumer.
    std::vector<Ownership> AboveMax;

    /// maxAllowed is the maximum number of partitions a consumer should have
    /// If partitions do not divide evenly this will be the "theoretical" max
    /// with the assumption that this particular consumer will get an extra
    /// partition.
    size_t MaxAllowed;

    /// extraPartitionPossible is true if the partitions cannot split up evenly
    /// amongst all the known consumers.
    bool ExtraPartitionPossible;

    std::vector<Ownership> Raw;
  };
}}}} // namespace Azure::Messaging::EventHubs::Models
