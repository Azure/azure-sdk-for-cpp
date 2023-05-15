// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "checkpoint_store.hpp"
#include <azure/core/context.hpp>
#include <chrono>

#ifdef TESTING_BUILD_AMQP
namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  class ProcessorLoadBalancerTest_Greedy_EnoughUnownedPartitions_Test;
  class ProcessorLoadBalancerTest_Balanced_UnownedPartitions_Test;
  class ProcessorLoadBalancerTest_Greedy_ForcedToSteal_Test;
  class ProcessorLoadBalancerTest_AnyStrategy_GetExpiredPartition_Test;
  class ProcessorLoadBalancerTest_AnyStrategy_FullyBalancedOdd_Test;
  class ProcessorLoadBalancerTest_AnyStrategy_FullyBalancedEven_Test;
  class ProcessorLoadBalancerTest_AnyStrategy_GrabExtraPartitionBecauseAboveMax_Test;
  class ProcessorLoadBalancerTest_AnyStrategy_StealsToBalance_Test;
}}}} // namespace Azure::Messaging::EventHubs::Test
#endif

namespace Azure { namespace Messaging { namespace EventHubs {
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
  // current are the partitions that _we_ own
  std::vector<Ownership> Current;

  // unownedOrExpired partitions either had no claim _ever_ or were once
  // owned but the ownership claim has expired.
  std::vector<Ownership> UnownedOrExpired;

  // aboveMax are ownerships where the specific owner has too many partitions
  // it contains _all_ the partitions for that particular consumer.
  std::vector<Ownership> AboveMax;

  // maxAllowed is the maximum number of partitions a consumer should have
  // If partitions do not divide evenly this will be the "theoretical" max
  // with the assumption that this particular consumer will get an extra
  // partition.
  uint64_t MaxAllowed;

  // extraPartitionPossible is true if the partitions cannot split up evenly
  // amongst all the known consumers.
  bool ExtraPartitionPossible;

  std::vector<Ownership> Raw;
};

class ProcessorLoadBalancer {
#ifdef TESTING_BUILD_AMQP
  friend class Test::ProcessorLoadBalancerTest_Greedy_EnoughUnownedPartitions_Test;
  friend class Test::ProcessorLoadBalancerTest_Balanced_UnownedPartitions_Test;
  friend class Test::ProcessorLoadBalancerTest_Greedy_ForcedToSteal_Test;
  friend class Test::ProcessorLoadBalancerTest_AnyStrategy_GetExpiredPartition_Test;
  friend class Test::ProcessorLoadBalancerTest_AnyStrategy_FullyBalancedOdd_Test;
  friend class Test::ProcessorLoadBalancerTest_AnyStrategy_FullyBalancedEven_Test;
  friend class Test::ProcessorLoadBalancerTest_AnyStrategy_GrabExtraPartitionBecauseAboveMax_Test;
  friend class Test::ProcessorLoadBalancerTest_AnyStrategy_StealsToBalance_Test;
#endif
  std::unique_ptr<CheckpointStore> m_checkpointStore;
  ConsumerClientDetails m_consumerClientDetails;
  ProcessorStrategy m_strategy;
  std::chrono::minutes m_duration;

  /**@brief  GetAvailablePartitions finds all partitions that are either completely unowned _or_
   * their ownership is stale.
   */
  LoadBalancerInfo GetAvailablePartitions(
      std::vector<std::string> const& partitionIDs,
      Azure::Core::Context& ctx);

  std::vector<Ownership> GetRandomOwnerships(
      std::vector<Ownership> const& ownerships,
      uint64_t const& count);

  Ownership ResetOwnership(Ownership ownership);

  /**@brief BalancedLoadBalancer attempts to split the partition load out between the available
   *consumers so each one has an even amount (or even + 1, if the # of consumers and #
   * of partitions doesn't divide evenly).
   *
   *@remark: the checkpoint store itself does not have a concept of 'presence' that doesn't
   * ALSO involve owning a partition. It's possible for a consumer to get boxed out for a
   *bit until it manages to steal at least one partition since the other consumers don't
   *know it exists until then.
   */
  std::vector<Ownership> BalancedLoadBalancer(
      LoadBalancerInfo const& lbinfo,
      Azure::Core::Context& ctx);

  std::vector<Ownership> GreedyLoadBalancer(
      LoadBalancerInfo const& lbInfo,
      Azure::Core::Context ctx);

public:
  ProcessorLoadBalancer(
      std::unique_ptr<CheckpointStore> checkpointStore,
      ConsumerClientDetails const& consumerClientDetails,
      ProcessorStrategy const& strategy,
      std::chrono::minutes const& duration)
      : m_checkpointStore(std::move(checkpointStore)),
        m_consumerClientDetails(consumerClientDetails), m_strategy(strategy), m_duration(duration)
  { // seed the rand generator
    std::srand((uint32_t)std::time(nullptr));
  };

  std::vector<Ownership> LoadBalance(
      std::vector<std::string> const& partitionIDs,
      Azure::Core::Context ctx = Azure::Core::Context());
};
}}} // namespace Azure::Messaging::EventHubs
