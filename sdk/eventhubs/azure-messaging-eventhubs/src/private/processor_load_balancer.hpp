// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words lbinfo
#pragma once
#include "azure/messaging/eventhubs/checkpoint_store.hpp"
#include "azure/messaging/eventhubs/models/consumer_client_models.hpp"
#include "azure/messaging/eventhubs/models/partition_client_models.hpp"
#include "azure/messaging/eventhubs/models/processor_load_balancer_models.hpp"

#include <azure/core/context.hpp>

#include <chrono>

#ifdef _azure_TESTING_BUILD_AMQP
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

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models { namespace _detail {
  /**
   * @brief Configuration options for the load balancer.
   */
  struct LoadBalancerInfo final
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

    /// Raw ownerships are the raw ownerships from the checkpoint store.
    std::vector<Ownership> Raw;
  };
}}}}} // namespace Azure::Messaging::EventHubs::Models::_detail

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {

  /**@brief ProcessorLoadBalancer is used by [Processor] to provide automatic load balancing
   * between multiple Processor instances, even in separate processes or on separate machines.
   */
  class ProcessorLoadBalancer final {
#ifdef _azure_TESTING_BUILD_AMQP
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
    Models::ProcessorStrategy m_strategy;
    std::chrono::minutes m_duration;

    /**@brief  GetAvailablePartitions finds all partitions that are either completely unowned _or_
     * their ownership is stale.
     */
    Models::_detail::LoadBalancerInfo GetAvailablePartitions(
        std::vector<std::string> const& partitionIDs,
        Core::Context const& context);

    std::vector<Models::Ownership> GetRandomOwnerships(
        std::vector<Models::Ownership> const& ownerships,
        size_t const count);

    Models::Ownership ResetOwnership(Models::Ownership ownership);

    /**@brief BalancedLoadBalancer attempts to split the partition load out between the available
     *consumers so each one has an even amount (or even + 1, if the # of consumers and #
     * of partitions doesn't divide evenly).
     *
     *@remark: the checkpoint store itself does not have a concept of 'presence' that doesn't
     * ALSO involve owning a partition. It's possible for a consumer to get boxed out for a
     *bit until it manages to steal at least one partition since the other consumers don't
     *know it exists until then.
     */
    std::vector<Models::Ownership> BalancedLoadBalancer(
        Models::_detail::LoadBalancerInfo const& lbinfo,
        Core::Context const& context);

    std::vector<Models::Ownership> GreedyLoadBalancer(
        Models::_detail::LoadBalancerInfo const& lbInfo,
        Core::Context const& context);

  public:
    /**@brief ProcessorLoadBalancer is used by [Processor] to provide automatic load balancing
     * between multiple Processor instances, even in separate processes or on separate machines.
     *
     *@param checkpointStore the checkpoint store used by the Processor instances to store
     * their checkpoints.
     *@param consumerClientDetails the details of the consumer client, including its identifier
     * and the maximum number of partitions it can own at a time.
     *@param strategy the load balancing strategy to use.
     *@param duration the duration of time that a partition ownership is valid before it is
     * considered stale and can be stolen by another consumer.
     */
    ProcessorLoadBalancer(
        std::shared_ptr<CheckpointStore> checkpointStore,
        Models::ConsumerClientDetails const& consumerClientDetails,
        Models::ProcessorStrategy const& strategy,
        std::chrono::minutes const& duration)
        : m_checkpointStore(checkpointStore), m_consumerClientDetails(consumerClientDetails),
          m_strategy(strategy), m_duration(duration)
    { // seed the rand generator
      std::srand(static_cast<uint32_t>(std::time(nullptr)));
    };

    /** Construct a ProcessorLoadBalancer from another ProcessorLoadBalancer. */
    ProcessorLoadBalancer(ProcessorLoadBalancer const& other) = default;

    /** Assign a ProcessorLoadBalancer to another ProcessorLoadBalancer. */
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
    /**@brief LoadBalance is called by the Processor to get a list of partitions that it should
     * begin processing.
     *
     *@param partitionIDs the list of all partitions that are available to be owned.
     *@param context the context to pass to the checkpoint store.
     *
     *@return a list of partitions that the Processor should begin processing.
     */
    std::vector<Models::Ownership> LoadBalance(
        std::vector<std::string> const& partitionIDs,
        Core::Context const& context = {});
  };
}}}} // namespace Azure::Messaging::EventHubs::_detail


