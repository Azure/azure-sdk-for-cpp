// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs.hpp"

#include <stdexcept>

// cspell: words lbinfo

using namespace Azure::Messaging::EventHubs::Models;

Azure::Messaging::EventHubs::Models::LoadBalancerInfo
Azure::Messaging::EventHubs::ProcessorLoadBalancer::GetAvailablePartitions(
    std::vector<std::string> const& partitionIDs,
    Core::Context const& context)
{

  std::vector<Models::Ownership> ownerships = m_checkpointStore->ListOwnership(
      m_consumerClientDetails.EventHubName,
      m_consumerClientDetails.ConsumerGroup,
      m_consumerClientDetails.ClientId,
      context);

  std::vector<Models::Ownership> unownedOrExpired;
  std::map<std::string, bool> alreadyProcessed;
  std::map<std::string, std::vector<Models::Ownership>> groupedByOwner;
  groupedByOwner[m_consumerClientDetails.ClientId] = std::vector<Models::Ownership>();
  for (auto& ownership : ownerships)
  {
    if (alreadyProcessed.find(ownership.PartitionId) != alreadyProcessed.end())
    {
      continue;
    }
    alreadyProcessed[ownership.PartitionId] = true;
    Azure::DateTime now(Azure::_detail::Clock::now());

    if (ownership.OwnerId.empty()
        || std::chrono::duration_cast<std::chrono::minutes>(
               now - ownership.LastModifiedTime.Value())
            > m_duration)
    {
      unownedOrExpired.push_back(ownership);
      continue;
    }
    groupedByOwner[ownership.OwnerId].push_back(ownership);
  }

  for (auto& partitionID : partitionIDs)
  {
    if (alreadyProcessed.find(partitionID) != alreadyProcessed.end())
    {
      continue;
    }

    unownedOrExpired.push_back(Ownership{
        m_consumerClientDetails.ConsumerGroup,
        m_consumerClientDetails.EventHubName,
        m_consumerClientDetails.HostName,
        partitionID,
        m_consumerClientDetails.ClientId,
    });
  }

  size_t maxAllowed = partitionIDs.size() / groupedByOwner.size();
  bool hasRemainder = (partitionIDs.size() % groupedByOwner.size()) > 0;
  if (hasRemainder)
  {
    maxAllowed++;
  }

  std::vector<Models::Ownership> aboveMax;

  for (auto& entry : groupedByOwner)
  {
    if (entry.second.size() > maxAllowed)
    {
      for (auto& ownership : entry.second)
      {
        aboveMax.push_back(ownership);
      }
    }
  }

  return Models::LoadBalancerInfo{
      groupedByOwner[m_consumerClientDetails.ClientId],
      unownedOrExpired,
      aboveMax,
      maxAllowed,
      hasRemainder,
      ownerships};
}

std::vector<Azure::Messaging::EventHubs::Models::Ownership>
Azure::Messaging::EventHubs::ProcessorLoadBalancer::GetRandomOwnerships(
    std::vector<Models::Ownership> const& ownerships,
    size_t const count)
{
  std::vector<Models::Ownership> randomOwnerships;
  std::vector<Models::Ownership> remainingOwnerships = ownerships;

  size_t numOwnerships = std::min(ownerships.size(), count);

  for (size_t i = 0; i < numOwnerships; i++)
  {
    size_t randomIndex = std::rand() % remainingOwnerships.size();
    randomOwnerships.push_back(remainingOwnerships[randomIndex]);
    remainingOwnerships.erase(remainingOwnerships.begin() + randomIndex);
  }

  return randomOwnerships;
}

Azure::Messaging::EventHubs::Models::Ownership
Azure::Messaging::EventHubs::ProcessorLoadBalancer::ResetOwnership(Models::Ownership ownership)
{
  ownership.OwnerId = m_consumerClientDetails.ClientId;
  return ownership;
}

std::vector<Azure::Messaging::EventHubs::Models::Ownership>
Azure::Messaging::EventHubs::ProcessorLoadBalancer::BalancedLoadBalancer(
    Models::LoadBalancerInfo const& lbinfo,
    Core::Context const& context)
{
  (void)context;
  std::vector<Models::Ownership> ours;
  if (lbinfo.UnownedOrExpired.size() > 0)
  {
    size_t index = std::rand() % lbinfo.UnownedOrExpired.size();
    Models::Ownership ownership = ResetOwnership(lbinfo.UnownedOrExpired[index]);
    ours.push_back(ownership);
  }

  if (lbinfo.AboveMax.size() > 0)
  {
    size_t index = std::rand() % lbinfo.AboveMax.size();
    Models::Ownership ownership = ResetOwnership(lbinfo.AboveMax[index]);
    ours.push_back(ownership);
  }

  return ours;
}

std::vector<Azure::Messaging::EventHubs::Models::Ownership>
Azure::Messaging::EventHubs::ProcessorLoadBalancer::GreedyLoadBalancer(
    Models::LoadBalancerInfo const& lbInfo,
    Core::Context const& context)
{
  (void)context;
  std::vector<Models::Ownership> ours = lbInfo.Current;
  // try claiming from the completely unowned or expires ownerships _first_
  std::vector<Models::Ownership> randomOwneships
      = GetRandomOwnerships(lbInfo.UnownedOrExpired, lbInfo.MaxAllowed - ours.size());
  ours.insert(ours.end(), randomOwneships.begin(), randomOwneships.end());

  if (ours.size() < lbInfo.MaxAllowed)
  { // try claiming from the completely unowned or expires ownerships _first_
    std::vector<Models::Ownership> randomOwnerships
        = GetRandomOwnerships(lbInfo.AboveMax, lbInfo.MaxAllowed - ours.size());
    ours.insert(ours.end(), randomOwnerships.begin(), randomOwnerships.end());
  }
  for (Models::Ownership& ownership : ours)
  {
    ownership = ResetOwnership(ownership);
  }

  return ours;
}

std::vector<Azure::Messaging::EventHubs::Models::Ownership>
Azure::Messaging::EventHubs::ProcessorLoadBalancer::LoadBalance(
    std::vector<std::string> const& partitionIDs,
    Core::Context const& context)
{
  Models::LoadBalancerInfo lbInfo = GetAvailablePartitions(partitionIDs, context);

  bool claimMore = true;

  if (lbInfo.Current.size() >= lbInfo.MaxAllowed)
  {
    // - I have _exactly_ the right amount
    // or
    // - I have too many. We expect to have some stolen from us, but we'll maintain
    //    ownership for now.
    claimMore = false;
  }
  else if (lbInfo.ExtraPartitionPossible && lbInfo.Current.size() == lbInfo.MaxAllowed - 1)
  {
    // In the 'extraPartitionPossible' scenario, some consumers will have an extra partition
    // since things don't divide up evenly. We're one under the max, which means we _might_
    // be able to claim another one.
    //
    // We will attempt to grab _one_ more but only if there are free partitions available
    // or if one of the consumers has more than the max allowed.

    claimMore = lbInfo.UnownedOrExpired.size() > 0 || lbInfo.AboveMax.size() > 0;
  }

  std::vector<Models::Ownership> ownerships = lbInfo.Current;

  if (claimMore)
  {
    switch (m_strategy)
    {
      case Models::ProcessorStrategy::ProcessorStrategyGreedy: {
        ownerships = GreedyLoadBalancer(lbInfo, context);
      }
      break;
      case Models::ProcessorStrategy::ProcessorStrategyBalanced: {

        std::vector<Models::Ownership> newOwnership = BalancedLoadBalancer(lbInfo, context);
        ownerships.insert(ownerships.end(), newOwnership.begin(), newOwnership.end());
      }
      break;
      default:
        throw std::runtime_error("unknown strategy");
        break;
    }
  }

  std::vector<Models::Ownership> actual = m_checkpointStore->ClaimOwnership(ownerships, context);
  return actual;
}
