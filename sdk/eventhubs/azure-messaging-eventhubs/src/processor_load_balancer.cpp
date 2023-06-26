// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/messaging/eventhubs.hpp"

// cspell: words lbinfo
Azure::Messaging::EventHubs::Models::LoadBalancerInfo
Azure::Messaging::EventHubs::ProcessorLoadBalancer::GetAvailablePartitions(
    std::vector<std::string> const& partitionIDs,
    Azure::Core::Context& ctx)
{

  std::vector<Ownership> ownerships = m_checkpointStore->ListOwnership(
      m_consumerClientDetails.EventHubName,
      m_consumerClientDetails.ConsumerGroup,
      m_consumerClientDetails.ClientID,
      ctx);

  std::vector<Ownership> unownedOrExpired;
  std::map<std::string, bool> alreadyProcessed;
  std::map<std::string, std::vector<Ownership>> groupedByOwner;
  groupedByOwner[m_consumerClientDetails.ClientID] = std::vector<Ownership>();
  for (auto& ownership : ownerships)
  {
    if (alreadyProcessed.find(ownership.PartitionID) != alreadyProcessed.end())
    {
      continue;
    }
    alreadyProcessed[ownership.PartitionID] = true;
    Azure::DateTime now(Azure::_detail::Clock::now());

    if (ownership.OwnerID.empty()
        || std::chrono::duration_cast<std::chrono::minutes>(
               now - ownership.LastModifiedTime.Value())
            > m_duration)
    {
      unownedOrExpired.push_back(ownership);
      continue;
    }
    groupedByOwner[ownership.OwnerID].push_back(ownership);
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
        m_consumerClientDetails.FullyQualifiedNamespace,
        partitionID,
        m_consumerClientDetails.ClientID,
    });
  }

  uint64_t maxAllowed = partitionIDs.size() / groupedByOwner.size();
  bool hasRemainder = (partitionIDs.size() % groupedByOwner.size()) > 0;
  if (hasRemainder)
  {
    maxAllowed++;
  }

  std::vector<Ownership> aboveMax;

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

  return LoadBalancerInfo{
      groupedByOwner[m_consumerClientDetails.ClientID],
      unownedOrExpired,
      aboveMax,
      maxAllowed,
      hasRemainder,
      ownerships};
}

std::vector<Azure::Messaging::EventHubs::Models::Ownership>
Azure::Messaging::EventHubs::ProcessorLoadBalancer::GetRandomOwnerships(
    std::vector<Ownership> const& ownerships,
    uint64_t const& count)
{
  std::vector<Ownership> randomOwnerships;
  std::vector<Ownership> remainingOwnerships = ownerships;

  uint64_t numOwnerships = std::min(ownerships.size(), count);

  for (uint64_t i = 0; i < numOwnerships; i++)
  {
    uint64_t randomIndex = std::rand() % remainingOwnerships.size();
    randomOwnerships.push_back(remainingOwnerships[randomIndex]);
    remainingOwnerships.erase(remainingOwnerships.begin() + randomIndex);
  }

  return randomOwnerships;
}

Azure::Messaging::EventHubs::Models::Ownership
Azure::Messaging::EventHubs::ProcessorLoadBalancer::ResetOwnership(Ownership ownership)
{
  ownership.OwnerID = m_consumerClientDetails.ClientID;
  return ownership;
}

std::vector<Azure::Messaging::EventHubs::Models::Ownership>
Azure::Messaging::EventHubs::ProcessorLoadBalancer::BalancedLoadBalancer(
    LoadBalancerInfo const& lbinfo,
    Azure::Core::Context& ctx)
{
  (void)ctx;
  std::vector<Ownership> ours;
  if (lbinfo.UnownedOrExpired.size() > 0)
  {
    uint64_t index = std::rand() % lbinfo.UnownedOrExpired.size();
    Ownership ownership = ResetOwnership(lbinfo.UnownedOrExpired[index]);
    ours.push_back(ownership);
  }

  if (lbinfo.AboveMax.size() > 0)
  {
    uint64_t index = std::rand() % lbinfo.AboveMax.size();
    Ownership ownership = ResetOwnership(lbinfo.AboveMax[index]);
    ours.push_back(ownership);
  }

  return ours;
}

std::vector<Azure::Messaging::EventHubs::Models::Ownership>
Azure::Messaging::EventHubs::ProcessorLoadBalancer::GreedyLoadBalancer(
    LoadBalancerInfo const& lbInfo,
    Azure::Core::Context ctx)
{
  std::vector<Ownership> ours = lbInfo.Current;
  // try claiming from the completely unowned or expires ownerships _first_
  std::vector<Ownership> randomOwneships
      = GetRandomOwnerships(lbInfo.UnownedOrExpired, lbInfo.MaxAllowed - ours.size());
  ours.insert(ours.end(), randomOwneships.begin(), randomOwneships.end());

  if (ours.size() < lbInfo.MaxAllowed)
  { // try claiming from the completely unowned or expires ownerships _first_
    std::vector<Ownership> randomOwnerships
        = GetRandomOwnerships(lbInfo.AboveMax, lbInfo.MaxAllowed - ours.size());
    ours.insert(ours.end(), randomOwnerships.begin(), randomOwnerships.end());
  }
  for (Ownership& ownership : ours)
  {
    ownership = ResetOwnership(ownership);
  }

  return ours;
}

std::vector<Azure::Messaging::EventHubs::Models::Ownership>
Azure::Messaging::EventHubs::ProcessorLoadBalancer::LoadBalance(
    std::vector<std::string> const& partitionIDs,
    Azure::Core::Context ctx)
{
  LoadBalancerInfo lbInfo = GetAvailablePartitions(partitionIDs, ctx);

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

  std::vector<Ownership> ownerships = lbInfo.Current;

  if (claimMore)
  {
    switch (m_strategy)
    {
      case ProcessorStrategy::ProcessorStrategyGreedy: {
        ownerships = GreedyLoadBalancer(lbInfo, ctx);
      }
      break;
      case ProcessorStrategy::ProcessorStrategyBalanced: {

        std::vector<Ownership> newOwnership = BalancedLoadBalancer(lbInfo, ctx);
        ownerships.insert(ownerships.end(), newOwnership.begin(), newOwnership.end());
      }
      break;
      default:
        throw std::exception("unknown strategy");
        break;
    }
  }

  std::vector<Ownership> actual = m_checkpointStore->ClaimOwnership(ownerships, ctx);
  return actual;
}