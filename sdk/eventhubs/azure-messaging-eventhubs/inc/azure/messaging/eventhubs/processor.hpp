// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "checkpoint_store.hpp"
#include "consumer_client.hpp"
#include "models/processor_models.hpp"
#include "processor_partition_client.hpp"

#include <azure/core/context.hpp>

#include <chrono>

namespace Azure { namespace Messaging { namespace EventHubs {

  /**@brief Processor uses a [ConsumerClient] and [CheckpointStore] to provide automatic
   * load balancing between multiple Processor instances, even in separate
   *processes or on separate machines.
   */
  class Processor {
    Azure::DateTime::duration m_ownershipUpdateInterval;
    StartPositions m_defaultStartPositions;
    std::unique_ptr<CheckpointStore> m_checkpointStore;
    int32_t m_prefetch;
    std::unique_ptr<ConsumerClient> m_ConsumerClient;
    std::vector<std::shared_ptr<ProcessorPartitionClient>> m_nextPartitionClients;
    uint32_t m_currentPartitionClient;
    ConsumerClientDetails m_consumerClientDetails;
    std::unique_ptr<ProcessorLoadBalancer> m_loadBalancer;

    typedef std::map<std::string, std::shared_ptr<ProcessorPartitionClient>> ConsumersType;

  public:
    Processor(
        std::unique_ptr<ConsumerClient> consumerClient,
        std::unique_ptr<CheckpointStore> checkpointStore,
        Models::ProcessorOptions const& options)
        : m_ConsumerClient(std::move(consumerClient)),
          m_checkpointStore(std::move(checkpointStore)), m_prefetch(options.Prefetch),
          m_defaultStartPositions(std::move(options.StartPositions))
    {
      m_ownershipUpdateInterval = options.UpdateInterval == Azure::DateTime::duration::zero()
          ? std::chrono::seconds(10)
          : options.UpdateInterval;

      m_consumerClientDetails = m_ConsumerClient->GetDetails();
      m_loadBalancer = std::make_unique<ProcessorLoadBalancer>(
          std::move(m_checkpointStore),
          m_consumerClientDetails,
          options.LoadBalancingStrategy,
          options.PartitionExpirationDuration == Azure::DateTime::duration::zero()
              ? std::chrono::minutes(1)
              : std::chrono::duration_cast<std::chrono::minutes>(
                  options.PartitionExpirationDuration));
    }

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

    void Run(Azure::Core::Context const& ctx = {})
    {
      EventHubProperties eventHubProperties = m_ConsumerClient->GetEventHubProperties();
      ConsumersType consumers;
      Dispatch(eventHubProperties, consumers, ctx);
    }

    void Dispatch(
        EventHubProperties const& eventHubProperties,
        ConsumersType& consumers,
        Azure::Core::Context const& ctx)
    {
      std::vector<Models::Ownership> ownerships
          = m_loadBalancer->LoadBalance(eventHubProperties.PartitionIDs, ctx);

      std::map<std::string, Models::Checkpoint> checkpoints = GetCheckpointsMap(ctx);

      for (auto ownership : ownerships)
      {
        AddPartitionClient(ownership, checkpoints, consumers);
      }
    }
    int64_t m_processorOwnerLevel = 0;

    void CloseConsumers(ConsumersType consumers)
    {
      for (auto consumer : consumers)
      {
        consumer.second->Close();
      }
    }

  private:
    void AddPartitionClient(
        Models::Ownership& ownership,
        std::map<std::string, Models::Checkpoint>& checkpoints,
        ConsumersType& consumers
        )
    {
      Models::StartPosition startPosition = GetStartPosition(ownership, checkpoints);
      
      std::shared_ptr<ProcessorPartitionClient> processorPartitionClient = std::make_shared<ProcessorPartitionClient>(
          ownership.PartitionID,
          m_ConsumerClient->NewPartitionClient(ownership.PartitionID, {startPosition,m_processorOwnerLevel,m_prefetch}),
          std::move(m_checkpointStore),
          m_consumerClientDetails,
          [&]() { consumers.erase(ownership.PartitionID); });

       if (consumers.find(ownership.PartitionID) == consumers.end())
      {
        consumers.emplace(
            ownership.PartitionID,
            processorPartitionClient);
      }

      m_nextPartitionClients.push_back(processorPartitionClient);
    }

    Models::StartPosition GetStartPosition(
        Models::Ownership const& ownership,
        std::map<std::string, Models::Checkpoint>& checkpoints )
    {
      Models::StartPosition startPosition = m_defaultStartPositions.Default;

      if (checkpoints.find(ownership.PartitionID) != checkpoints.end())
      {
        Models::Checkpoint checkpoint = checkpoints.at(ownership.PartitionID);

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
              "invalid checkpoint" + ownership.PartitionID + "no offset or sequence number");
        }
      }
      else if (
          m_defaultStartPositions.PerPartition.find(ownership.PartitionID)
          != m_defaultStartPositions.PerPartition.end())
      {
        startPosition = m_defaultStartPositions.PerPartition.at(ownership.PartitionID);
      }
      return startPosition;
    }

    std::map<std::string, Models::Checkpoint> GetCheckpointsMap(Azure::Core::Context const& ctx)
    {
      std::vector<Models::Checkpoint> checkpoints = m_checkpointStore->ListCheckpoints(
          m_consumerClientDetails.FullyQualifiedNamespace,
          m_consumerClientDetails.EventHubName,
          m_consumerClientDetails.ConsumerGroup,
          ctx);

      std::map<std::string, Models::Checkpoint> checkpointsMap;
      for (auto checkpoint : checkpoints)
      {
        checkpointsMap.emplace(checkpoint.PartitionID, checkpoint);
      }

      return checkpointsMap;
    }


  };
}}} // namespace Azure::Messaging::EventHubs
