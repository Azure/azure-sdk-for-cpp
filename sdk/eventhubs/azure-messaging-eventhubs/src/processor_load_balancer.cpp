// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/processor_load_balancer.hpp"

#include "azure/messaging/eventhubs/checkpoint_store.hpp"
#include "azure/messaging/eventhubs/models/processor_load_balancer_models.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <iomanip>
#include <set>
#include <stdexcept>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

using namespace Azure::Messaging::EventHubs::Models;
using namespace Azure::Messaging::EventHubs::_detail;
using namespace Azure::Messaging::EventHubs::Models::_detail;

LoadBalancerInfo ProcessorLoadBalancer::GetAvailablePartitions(
    std::vector<std::string> const& partitionIDs,
    Core::Context const& context)
{
  Log::Stream(Logger::Level::Verbose)
      << "[" << m_consumerClientDetails.ClientId
      << "] Get Available Partitions for: " << m_consumerClientDetails.FullyQualifiedNamespace
      << "/" << m_consumerClientDetails.EventHubName << "/"
      << m_consumerClientDetails.ConsumerGroup;

  std::vector<Models::Ownership> ownerships = m_checkpointStore->ListOwnership(
      m_consumerClientDetails.FullyQualifiedNamespace,
      m_consumerClientDetails.EventHubName,
      m_consumerClientDetails.ConsumerGroup,
      context);

  std::vector<Models::Ownership> unownedOrExpired;
  std::set<std::string> alreadyProcessed;
  std::map<std::string, std::vector<Models::Ownership>> groupedByOwner;
  groupedByOwner.emplace(m_consumerClientDetails.ClientId, std::vector<Models::Ownership>{});

  for (auto& ownership : ownerships)
  {
    if (alreadyProcessed.find(ownership.PartitionId) != alreadyProcessed.end())
    {
      continue;
    }

    alreadyProcessed.emplace(ownership.PartitionId);

    if (std::chrono::duration_cast<std::chrono::minutes>(
            Azure::_detail::Clock::now() - ownership.LastModifiedTime.Value())
        > m_duration)
    {
      unownedOrExpired.push_back(ownership);
      continue;
    }
    groupedByOwner[ownership.OwnerId].push_back(ownership);
  }

  Log::Stream(Logger::Level::Verbose)
      << "Number of expired partitions: " << unownedOrExpired.size();

  for (auto& partitionID : partitionIDs)
  {
    if (alreadyProcessed.find(partitionID) != alreadyProcessed.end())
    {
      continue;
    }

    Ownership newOwnership;
    newOwnership.ConsumerGroup = m_consumerClientDetails.ConsumerGroup;
    newOwnership.EventHubName = m_consumerClientDetails.EventHubName;
    newOwnership.FullyQualifiedNamespace = m_consumerClientDetails.FullyQualifiedNamespace;
    newOwnership.PartitionId = partitionID;
    newOwnership.OwnerId = m_consumerClientDetails.ClientId;
    unownedOrExpired.push_back(newOwnership);
  }

  Log::Stream(Logger::Level::Verbose)
      << "Number of unowned partitions: " << unownedOrExpired.size();

  size_t maxAllowed = partitionIDs.size() / groupedByOwner.size();
  bool hasRemainder = (partitionIDs.size() % groupedByOwner.size()) > 0;
  if (hasRemainder)
  {
    maxAllowed++;
  }

  std::vector<Models::Ownership> aboveMax;

  for (auto& entry : groupedByOwner)
  {
    if (entry.first == m_consumerClientDetails.ClientId)
    {
      continue;
    }
    if (entry.second.size() > maxAllowed)
    {
      for (auto& ownership : entry.second)
      {
        aboveMax.push_back(ownership);
      }
    }
  }

  LoadBalancerInfo rv;
  rv.Current = groupedByOwner[m_consumerClientDetails.ClientId];
  rv.UnownedOrExpired = unownedOrExpired;
  rv.AboveMax = aboveMax;
  rv.MaxAllowed = maxAllowed;
  rv.ExtraPartitionPossible = hasRemainder;
  rv.Raw = ownerships;
  return rv;
}

std::vector<Azure::Messaging::EventHubs::Models::Ownership> ProcessorLoadBalancer::
    GetRandomOwnerships(std::vector<Models::Ownership> const& ownerships, size_t const count)
{
  std::vector<Models::Ownership> randomOwnerships;
  std::vector<Models::Ownership> remainingOwnerships = ownerships;

  size_t numOwnerships = (std::min)(ownerships.size(), count);

  for (size_t i = 0; i < numOwnerships; i++)
  {
    size_t randomIndex = std::rand() % remainingOwnerships.size();
    randomOwnerships.push_back(remainingOwnerships[randomIndex]);
    remainingOwnerships.erase(remainingOwnerships.begin() + randomIndex);
  }

  return randomOwnerships;
}

Ownership ProcessorLoadBalancer::ResetOwnership(Models::Ownership ownership)
{
  ownership.OwnerId = m_consumerClientDetails.ClientId;
  return ownership;
}

std::vector<Azure::Messaging::EventHubs::Models::Ownership> ProcessorLoadBalancer::
    BalancedLoadBalancer(LoadBalancerInfo const& loadBalancerInfo, Core::Context const& context)
{
  (void)context;
  std::vector<Models::Ownership> ours;
  if (loadBalancerInfo.UnownedOrExpired.size() > 0)
  {
    size_t index = std::rand() % loadBalancerInfo.UnownedOrExpired.size();
    Models::Ownership ownership = ResetOwnership(loadBalancerInfo.UnownedOrExpired[index]);
    ours.push_back(ownership);
  }

  if (loadBalancerInfo.AboveMax.size() > 0)
  {
    size_t index = std::rand() % loadBalancerInfo.AboveMax.size();
    Models::Ownership ownership = ResetOwnership(loadBalancerInfo.AboveMax[index]);
    ours.push_back(ownership);
  }

  return ours;
}

std::vector<Ownership> ProcessorLoadBalancer::GreedyLoadBalancer(
    LoadBalancerInfo const& loadBalancerInfo,
    Core::Context const& context)
{
  (void)context;
  std::vector<Models::Ownership> ours = loadBalancerInfo.Current;
  // try claiming from the completely unowned or expires ownerships _first_
  std::vector<Models::Ownership> randomOwnerships = GetRandomOwnerships(
      loadBalancerInfo.UnownedOrExpired, loadBalancerInfo.MaxAllowed - ours.size());
  ours.insert(ours.end(), randomOwnerships.begin(), randomOwnerships.end());

  if (ours.size() < loadBalancerInfo.MaxAllowed)
  { // try claiming from the completely unowned or expires ownerships _first_
    std::vector<Models::Ownership> additionalRandomOwnerships
        = GetRandomOwnerships(loadBalancerInfo.AboveMax, loadBalancerInfo.MaxAllowed - ours.size());
    ours.insert(ours.end(), additionalRandomOwnerships.begin(), additionalRandomOwnerships.end()); 
  }
  for (Models::Ownership& ownership : ours)
  {
    ownership = ResetOwnership(ownership);
  }

  return ours;
}

namespace {

std::string partitionsForOwnerships(std::vector<Ownership> const& ownerships)
{
  std::stringstream ss;
  ss << "[";
  bool first = true;
  for (auto& ownership : ownerships)
  {
    if (!first)
    {
      ss << ", ";
    }
    first = false;
    ss << ownership.PartitionId;
  }
  ss << "]";
  return ss.str();
}
} // namespace

std::vector<Ownership> ProcessorLoadBalancer::LoadBalance(
    std::vector<std::string> const& partitionIDs,
    Core::Context const& context)
{
  LoadBalancerInfo loadBalancerInfo = GetAvailablePartitions(partitionIDs, context);

  bool claimMore = true;

  if (loadBalancerInfo.Current.size() >= loadBalancerInfo.MaxAllowed)
  {
    // - I have _exactly_ the right amount
    // or
    // - I have too many. We expect to have some stolen from us, but we'll maintain
    //    ownership for now.
    claimMore = false;
    Log::Stream(Logger::Level::Verbose)
        << "Owns " << loadBalancerInfo.Current.size() << " of " << partitionIDs.size()
        << " partitions.  Max allowed is " << loadBalancerInfo.MaxAllowed << std::endl;
  }
  else if (
      loadBalancerInfo.ExtraPartitionPossible
      && loadBalancerInfo.Current.size() == loadBalancerInfo.MaxAllowed - 1)
  {
    // In the 'extraPartitionPossible' scenario, some consumers will have an extra partition
    // since things don't divide up evenly. We're one under the max, which means we _might_
    // be able to claim another one.
    //
    // We will attempt to grab _one_ more but only if there are free partitions available
    // or if one of the consumers has more than the max allowed.

    claimMore
        = loadBalancerInfo.UnownedOrExpired.size() > 0 || loadBalancerInfo.AboveMax.size() > 0;
    Log::Stream(Logger::Level::Verbose)
        << "Unowned/expired: " << loadBalancerInfo.UnownedOrExpired.size()
        << " Above max: " << loadBalancerInfo.AboveMax.size()
        << "Need to claim more: " << std::boolalpha << claimMore;
  }

  std::vector<Models::Ownership> ownerships = loadBalancerInfo.Current;

  if (claimMore)
  {
    switch (m_strategy)
    {
      case Models::ProcessorStrategy::ProcessorStrategyGreedy: {
        ownerships = GreedyLoadBalancer(loadBalancerInfo, context);
      }
      break;
      case Models::ProcessorStrategy::ProcessorStrategyBalanced: {

        std::vector<Models::Ownership> newOwnership
            = BalancedLoadBalancer(loadBalancerInfo, context);
        ownerships.insert(ownerships.end(), newOwnership.begin(), newOwnership.end());
      }
      break;
      default:
        throw std::runtime_error("unknown strategy");
        break;
    }
  }

  std::vector<Models::Ownership> actual = m_checkpointStore->ClaimOwnership(ownerships, context);
  Log::Stream(Logger::Level::Verbose)
      << "[" << m_consumerClientDetails.ClientId << "] Asked for "
      << partitionsForOwnerships(ownerships) << ", got " << partitionsForOwnerships(actual);

  return actual;
}
