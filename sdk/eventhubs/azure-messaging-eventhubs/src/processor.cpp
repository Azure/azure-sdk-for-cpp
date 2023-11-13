// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs/processor.hpp"

#include "azure/messaging/eventhubs/models/management_models.hpp"
#include "azure/messaging/eventhubs/models/partition_client_models.hpp"
#include "private/processor_load_balancer.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <iomanip>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Messaging { namespace EventHubs {

  Processor::Processor(
      std::shared_ptr<ConsumerClient> consumerClient,
      std::shared_ptr<CheckpointStore> checkpointStore,
      ProcessorOptions const& options)
      : m_defaultStartPositions(options.StartPositions),
        m_maximumNumberOfPartitions{options.MaximumNumberOfPartitions},
        m_checkpointStore(checkpointStore), m_consumerClient(consumerClient),
        m_prefetch(options.Prefetch), m_nextPartitionClients{}
  {
    m_ownershipUpdateInterval = options.UpdateInterval == Azure::DateTime::duration::zero()
        ? std::chrono::seconds(10)
        : options.UpdateInterval;

    m_consumerClientDetails = m_consumerClient->GetDetails();
    m_loadBalancer = std::make_shared<_detail::ProcessorLoadBalancer>(
        m_checkpointStore,
        m_consumerClientDetails,
        options.LoadBalancingStrategy,
        options.PartitionExpirationDuration == Azure::DateTime::duration::zero()
            ? std::chrono::minutes(1)
            : std::chrono::duration_cast<std::chrono::minutes>(
                options.PartitionExpirationDuration));
  }

  Processor::~Processor()
  {
    Log::Stream(Logger::Level::Verbose) << "~Processor.";
    Stop();
    m_consumerClient.reset();
  }

  void Processor::Start(Azure::Core::Context const& context)
  {
    m_processorThread = std::thread([this, context]() {
      try
      {

        this->RunInternal(context, false);
      }
      catch (std::exception& ex)
      {
        Log::Stream(Logger::Level::Warning) << "Exception caught running processor: " << ex.what();
      }
    });
    m_isRunning = true;
  }

  // Stop the running processor, waiting for the processor to terminate.
  void Processor::Stop()
  {
    Log::Stream(Logger::Level::Verbose) << "Stop processor.";
    m_isRunning = false;

    if (m_processorThread.joinable())
    {
      m_processorThread.join();
    }
  }

  void Processor::Run(Core::Context const& context) { RunInternal(context, true); }

  void Processor::RunInternal(Core::Context const& context, bool publicInvocation)
  {
    Models::EventHubProperties eventHubProperties
        = m_consumerClient->GetEventHubProperties(context);

    if (m_maximumNumberOfPartitions != 0)
    {
      eventHubProperties.PartitionIds.resize(m_maximumNumberOfPartitions);
    }

    // Establish the maximum depth of the partition clients channel.
    m_nextPartitionClients.SetMaximumDepth(eventHubProperties.PartitionIds.size());

    auto consumers = std::make_shared<ConsumersType>();

    try
    {
      // If this is a public invocation (the caller directly called "Run"), then we want to ignore
      // the m_isRunning boolean.
      while (!context.IsCancelled() && (publicInvocation ? true : m_isRunning))
      {
        Dispatch(eventHubProperties, consumers, context);

        Log::Stream(Logger::Level::Verbose)
            << "Processor Sleeping for "
            << std::chrono::duration_cast<std::chrono::milliseconds>(m_ownershipUpdateInterval)
                   .count()
            << "  milliseconds. ";
        std::this_thread::sleep_for(m_ownershipUpdateInterval);
      }
    }
    catch (std::exception& ex)
    {
      Log::Stream(Logger::Level::Warning) << "Exception caught running processor: " << ex.what();
    }
  }

  void Processor::Dispatch(
      Models::EventHubProperties const& eventHubProperties,
      std::shared_ptr<Processor::ConsumersType> consumers,
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

  std::map<std::string, Models::Checkpoint> Processor::GetCheckpointsMap(
      Core::Context const& context)
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

  void Processor::AddPartitionClient(
      Models::Ownership const& ownership,
      std::map<std::string, Models::Checkpoint>& checkpoints,
      std::weak_ptr<ConsumersType> consumers)
  {
    Log::Stream(Logger::Level::Verbose) << "Add partition client for " << ownership;

    std::shared_ptr<ProcessorPartitionClient> processorPartitionClient
        = std::make_shared<ProcessorPartitionClient>(ProcessorPartitionClient(
            ownership.PartitionId,
            m_checkpointStore,
            m_consumerClientDetails,
            [consumers, ownership]() {
              if (auto strongConsumers = consumers.lock())
              {
                strongConsumers->erase(ownership.PartitionId);
              }
            }));

    // Try to add the partition client to the map. If it's already there, we discard the one we just
    // created in favor of the existing processor partition client.
    if (auto strongConsumers = consumers.lock())
    {
      auto added = strongConsumers->emplace(ownership.PartitionId, processorPartitionClient);
      if (!added.second)
      {
        Log::Stream(Logger::Level::Verbose)
            << "Partition client already in consumers map, ignoring.";
        return;
      }
    }

    // Now that we've verified there is no active processor partition client for this partition, we
    // can create a partition client to make the processor partition client fully functional.
    Models::StartPosition startPosition = GetStartPosition(ownership, checkpoints);
    PartitionClientOptions partitionClientOptions;
    partitionClientOptions.StartPosition = startPosition;
    partitionClientOptions.Prefetch = m_prefetch;
    partitionClientOptions.OwnerLevel = m_processorOwnerLevel;

    auto partitionClient{std::make_unique<PartitionClient>(
        m_consumerClient->CreatePartitionClient(ownership.PartitionId, partitionClientOptions))};
    processorPartitionClient->SetPartitionClient(partitionClient);

    // Add the new processor partition client to the next partitions client queue. If the queue
    // is full, discard the client.
    if (!m_nextPartitionClients.Insert(processorPartitionClient))
    {
      Log::Stream(Logger::Level::Verbose)
          << "nextPartitionClients is full, discarding partition client..";
      processorPartitionClient->Close();
    }
  }

  std::shared_ptr<ProcessorPartitionClient> Processor::NextPartitionClient(
      Azure::Core::Context const& context)
  {
    Log::Stream(Logger::Level::Verbose) << "NextPartitionClient: Retrieve next client";
    auto nextClient = m_nextPartitionClients.Remove(context);
    return nextClient;
  }
}}} // namespace Azure::Messaging::EventHubs
