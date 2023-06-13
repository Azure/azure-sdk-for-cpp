// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "checkpoint_store.hpp"
#include "models/partition_client_models.hpp"
#include "models/processor_load_balancer_models.hpp"

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
  std::shared_ptr<CheckpointStore> m_checkpointStore;
  Models::ConsumerClientDetails m_consumerClientDetails;
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
      std::shared_ptr<CheckpointStore> checkpointStore,
      ConsumerClientDetails const& consumerClientDetails,
      ProcessorStrategy const& strategy,
      std::chrono::minutes const& duration)
      : m_checkpointStore(checkpointStore), m_consumerClientDetails(consumerClientDetails),
        m_strategy(strategy), m_duration(duration)
  { // seed the rand generator
    std::srand((uint32_t)std::time(nullptr));
  };

  ProcessorLoadBalancer(ProcessorLoadBalancer const& other)
      : m_checkpointStore(other.m_checkpointStore),
        m_consumerClientDetails(other.m_consumerClientDetails), m_strategy(other.m_strategy),
        m_duration(other.m_duration)
  {
  }

  ProcessorLoadBalancer operator=(ProcessorLoadBalancer const& other)
  {
    if (this != &other)
    {
      m_checkpointStore = other.m_checkpointStore;
      m_consumerClientDetails = other.m_consumerClientDetails;
      m_strategy = other.m_strategy;
      m_duration = other.m_duration;
    }
    return *this;
  }

  std::vector<Ownership> LoadBalance(
      std::vector<std::string> const& partitionIDs,
      Azure::Core::Context ctx = Azure::Core::Context());
};
}}} // namespace Azure::Messaging::EventHubs
