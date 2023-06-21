// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "checkpoint_store.hpp"
#include "consumer_client.hpp"
#include "models/processor_models.hpp"
#include "processor_partition_client.hpp"

#include <azure/core/context.hpp>

#include <chrono>

#ifdef TESTING_BUILD_AMQP
namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  class ProcessorTest_LoadBalancing_Test;
}}}} // namespace Azure::Messaging::EventHubs::Test
#endif
namespace Azure { namespace Messaging { namespace EventHubs {

/**@brief Processor uses a [ConsumerClient] and [CheckpointStore] to provide automatic
 * load balancing between multiple Processor instances, even in separate
 *processes or on separate machines.
 */
class Processor {
#ifdef TESTING_BUILD_AMQP

  friend class Test::ProcessorTest_LoadBalancing_Test;
#endif

  Azure::DateTime::duration m_ownershipUpdateInterval;
  StartPositions m_defaultStartPositions;
  std::shared_ptr<CheckpointStore> m_checkpointStore;
  int32_t m_prefetch;
  std::shared_ptr<ConsumerClient> m_ConsumerClient;
  std::vector<std::shared_ptr<ProcessorPartitionClient>> m_nextPartitionClients;
  uint32_t m_currentPartitionClient;
  ConsumerClientDetails m_consumerClientDetails;
  std::shared_ptr<ProcessorLoadBalancer> m_loadBalancer;
  int64_t m_processorOwnerLevel = 0;

  typedef std::map<std::string, std::shared_ptr<ProcessorPartitionClient>> ConsumersType;

public:
  Processor(
      std::shared_ptr<ConsumerClient> consumerClient,
      std::shared_ptr<CheckpointStore> checkpointStore,
      Models::ProcessorOptions const& options = {})
      : m_ConsumerClient(consumerClient), m_checkpointStore(checkpointStore),
        m_prefetch(options.Prefetch), m_defaultStartPositions(options.StartPositions)
  {
    m_ownershipUpdateInterval = options.UpdateInterval == Azure::DateTime::duration::zero()
        ? std::chrono::seconds(10)
        : options.UpdateInterval;

    m_consumerClientDetails = m_ConsumerClient->GetDetails();
    m_loadBalancer = std::make_shared<ProcessorLoadBalancer>(
        m_checkpointStore,
        m_consumerClientDetails,
        options.LoadBalancingStrategy,
        options.PartitionExpirationDuration == Azure::DateTime::duration::zero()
            ? std::chrono::minutes(1)
            : std::chrono::duration_cast<std::chrono::minutes>(
                options.PartitionExpirationDuration));
  }

  Processor(Processor const& other)
      : m_ownershipUpdateInterval(other.m_ownershipUpdateInterval),
        m_defaultStartPositions(other.m_defaultStartPositions),
        m_checkpointStore(other.m_checkpointStore), m_prefetch(other.m_prefetch),
        m_ConsumerClient(other.m_ConsumerClient),
        m_consumerClientDetails(other.m_consumerClientDetails),
        m_loadBalancer(other.m_loadBalancer), m_nextPartitionClients(other.m_nextPartitionClients),
        m_currentPartitionClient(other.m_currentPartitionClient)
  {
  }

  Processor& operator=(Processor const& other)
  {
    if (this != &other)
    {
      m_ownershipUpdateInterval = other.m_ownershipUpdateInterval;
      m_defaultStartPositions = other.m_defaultStartPositions;
      m_checkpointStore = other.m_checkpointStore;
      m_prefetch = other.m_prefetch;
      m_ConsumerClient = other.m_ConsumerClient;
      m_consumerClientDetails = other.m_consumerClientDetails;
      m_loadBalancer = other.m_loadBalancer;
      m_nextPartitionClients = other.m_nextPartitionClients;
      m_currentPartitionClient = other.m_currentPartitionClient;
    }
    return *this;
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
    time_t timeNowSeconds = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const auto current = std::chrono::system_clock::from_time_t(timeNowSeconds);

    while (!ctx.IsCancelled())
    {
      std::this_thread::sleep_for(m_ownershipUpdateInterval);
      Dispatch(eventHubProperties, consumers, ctx);
    }
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

  void Close()
  {
    for (auto consumer : m_nextPartitionClients)
    {
      consumer->Close();
    }
  }

private:
  void AddPartitionClient(
      Models::Ownership& ownership,
      std::map<std::string, Models::Checkpoint>& checkpoints,
      ConsumersType& consumers)
  {
    Models::StartPosition startPosition = GetStartPosition(ownership, checkpoints);

    std::shared_ptr<ProcessorPartitionClient> processorPartitionClient
        = std::make_shared<ProcessorPartitionClient>(
            ownership.PartitionID,
            m_ConsumerClient->NewPartitionClient(
                ownership.PartitionID, {startPosition, m_processorOwnerLevel, m_prefetch}),
            std::move(m_checkpointStore),
            m_consumerClientDetails,
            [&]() { consumers.erase(ownership.PartitionID); });

    if (consumers.find(ownership.PartitionID) == consumers.end())
    {
      consumers.emplace(ownership.PartitionID, processorPartitionClient);
    }

    m_nextPartitionClients.push_back(processorPartitionClient);
  }

  Models::StartPosition GetStartPosition(
      Models::Ownership const& ownership,
      std::map<std::string, Models::Checkpoint>& checkpoints)
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
