// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include "checkpoint_store.hpp"
#include "consumer_client.hpp"
#include "models/processor_models.hpp"
#include "processor_load_balancer.hpp"
#include "processor_partition_client.hpp"

#include <azure/core/context.hpp>

#include <chrono>

#ifdef TESTING_BUILD_AMQP
namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  class ProcessorTest_LoadBalancing_Test;
}}}} // namespace Azure::Messaging::EventHubs::Test
#endif
namespace Azure { namespace Messaging { namespace EventHubs {
  /**@brief ProcessorOptions are the options for the CreateProcessor
   * function.
   */
  struct ProcessorOptions final
  {
    /**@brief LoadBalancingStrategy dictates how concurrent Processor instances distribute
     * ownership of partitions between them.
     * The default strategy is ProcessorStrategyBalanced.
     */
    Models::ProcessorStrategy LoadBalancingStrategy;

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
    Models::StartPositions StartPositions;

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

  /**@brief Processor uses a [ConsumerClient] and [CheckpointStore] to provide automatic
   * load balancing between multiple Processor instances, even in separate
   *processes or on separate machines.
   */
  class Processor final {
#ifdef TESTING_BUILD_AMQP
    friend class Test::ProcessorTest_LoadBalancing_Test;
#endif

    Azure::DateTime::duration m_ownershipUpdateInterval;
    Models::StartPositions m_defaultStartPositions;
    std::shared_ptr<CheckpointStore> m_checkpointStore;
    int32_t m_prefetch;
    std::shared_ptr<ConsumerClient> m_consumerClient;
    std::vector<std::shared_ptr<ProcessorPartitionClient>> m_nextPartitionClients;
    uint32_t m_currentPartitionClient;
    Models::ConsumerClientDetails m_consumerClientDetails;
    std::shared_ptr<ProcessorLoadBalancer> m_loadBalancer;
    int64_t m_processorOwnerLevel = 0;

    typedef std::map<std::string, std::shared_ptr<ProcessorPartitionClient>> ConsumersType;

  public:
    /** @brief Construct a new Processor object.
     *
     * @param consumerClient A [ConsumerClient] that is used to receive events from the Event Hub.
     * @param checkpointStore A [CheckpointStore] that is used to load and update checkpoints.
     * @param options Optional configuration for the processor.
     */
    Processor(
        std::shared_ptr<ConsumerClient> consumerClient,
        std::shared_ptr<CheckpointStore> checkpointStore,
        ProcessorOptions const& options = {})
        : m_defaultStartPositions(options.StartPositions), m_checkpointStore(checkpointStore),
          m_prefetch(options.Prefetch), m_consumerClient(consumerClient)
    {
      m_ownershipUpdateInterval = options.UpdateInterval == Azure::DateTime::duration::zero()
          ? std::chrono::seconds(10)
          : options.UpdateInterval;

      m_consumerClientDetails = m_consumerClient->GetDetails();
      m_loadBalancer = std::make_shared<ProcessorLoadBalancer>(
          m_checkpointStore,
          m_consumerClientDetails,
          options.LoadBalancingStrategy,
          options.PartitionExpirationDuration == Azure::DateTime::duration::zero()
              ? std::chrono::minutes(1)
              : std::chrono::duration_cast<std::chrono::minutes>(
                  options.PartitionExpirationDuration));
    }

    /** Construct a Processor from another Processor. */
    Processor(Processor const& other) = default;

    /** Assign a Processor to another Processor. */
    Processor& operator=(Processor const& other) = default;

    /** Move to the next partition client */
    std::shared_ptr<ProcessorPartitionClient> NextPartitionClient()
    {
      uint32_t currentPartition = m_currentPartitionClient;
      if (currentPartition > m_nextPartitionClients.size() - 1)
      {
        currentPartition = 0;
      }
      m_currentPartitionClient = currentPartition + 1;
      return m_nextPartitionClients[currentPartition];
    }

    /** @brief Starts the processor.
     *
     * @param context The context to control the request lifetime.
     */
    void Run(Core::Context const& context)
    {
      Models::EventHubProperties eventHubProperties
          = m_consumerClient->GetEventHubProperties(context);
      ConsumersType consumers;
      Dispatch(eventHubProperties, consumers, context);
      //      time_t timeNowSeconds
      //        = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      // const auto current = std::chrono::system_clock::from_time_t(timeNowSeconds);

      //// TODO : this is where we re load balance on the update interval
      // while (!context.IsCancelled())
      //{
      //  std::this_thread::sleep_for(m_ownershipUpdateInterval);
      //  Dispatch(eventHubProperties, consumers, context);
      //}
    }

    /** @brief Dispatches events to the appropriate partition clients.
     *
     * @param eventHubProperties The properties of the Event Hub.
     * @param consumers The map of partition id to partition client.
     * @param context The context to control the request lifetime.
     */
    void Dispatch(
        Models::EventHubProperties const& eventHubProperties,
        ConsumersType& consumers,
        Core::Context const& context)
    {
      std::vector<Models::Ownership> ownerships
          = m_loadBalancer->LoadBalance(eventHubProperties.PartitionIds, context);

      std::map<std::string, Models::Checkpoint> checkpoints = GetCheckpointsMap(context);

      for (auto const& ownership : ownerships)
      {
        AddPartitionClient(ownership, checkpoints, consumers);
      }
    }

    /** @brief Closes the processor and cancels any current operations.
     *
     *
     */
    void Close()
    {
      for (auto& consumer : m_nextPartitionClients)
      {
        consumer->Close();
      }
    }

  private:
    void AddPartitionClient(
        Models::Ownership const& ownership,
        std::map<std::string, Models::Checkpoint>& checkpoints,
        ConsumersType& consumers)
    {
      Models::StartPosition startPosition = GetStartPosition(ownership, checkpoints);

      // The consumers parameter is not stabilized across the lifetime of the partition client. Leak
      // the partition for now.
      std::shared_ptr<ProcessorPartitionClient> processorPartitionClient
          = std::make_shared<ProcessorPartitionClient>(
              ownership.PartitionId,
              m_consumerClient->CreatePartitionClient(
                  ownership.PartitionId, {startPosition, m_processorOwnerLevel, m_prefetch}),
              m_checkpointStore,
              m_consumerClientDetails,
              []() { /*
                      consumers.erase(ownership.PartitionId);*/
              });

      if (consumers.find(ownership.PartitionId) == consumers.end())
      {
        consumers.emplace(ownership.PartitionId, processorPartitionClient);
      }

      m_nextPartitionClients.push_back(processorPartitionClient);
    }

    Models::StartPosition GetStartPosition(
        Models::Ownership const& ownership,
        std::map<std::string, Models::Checkpoint> const& checkpoints)
    {
      Models::StartPosition startPosition = m_defaultStartPositions.Default;

      if (checkpoints.find(ownership.PartitionId) != checkpoints.end())
      {
        Models::Checkpoint checkpoint = checkpoints.at(ownership.PartitionId);

        if (checkpoint.Offset.HasValue())
        {
          startPosition.Offset = checkpoint.Offset;
        }
        else if (checkpoint.SequenceNumber.HasValue())
        {
          startPosition.SequenceNumber = checkpoint.SequenceNumber;
        }
        else
        {
          throw std::runtime_error(
              "invalid checkpoint" + ownership.PartitionId + "no offset or sequence number");
        }
      }
      else if (
          m_defaultStartPositions.PerPartition.find(ownership.PartitionId)
          != m_defaultStartPositions.PerPartition.end())
      {
        startPosition = m_defaultStartPositions.PerPartition.at(ownership.PartitionId);
      }
      return startPosition;
    }

    std::map<std::string, Models::Checkpoint> GetCheckpointsMap(Core::Context const& context)
    {
      std::vector<Models::Checkpoint> checkpoints = m_checkpointStore->ListCheckpoints(
          m_consumerClientDetails.FullyQualifiedNamespace,
          m_consumerClientDetails.EventHubName,
          m_consumerClientDetails.ConsumerGroup,
          context);

      std::map<std::string, Models::Checkpoint> checkpointsMap;
      for (auto& checkpoint : checkpoints)
      {
        checkpointsMap.emplace(checkpoint.PartitionId, checkpoint);
      }

      return checkpointsMap;
    }
  };
}}} // namespace Azure::Messaging::EventHubs
