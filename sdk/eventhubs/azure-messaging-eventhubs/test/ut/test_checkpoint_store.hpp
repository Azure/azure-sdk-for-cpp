// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <azure/core/context.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <mutex>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  class TestCheckpointStore : public Azure::Messaging::EventHubs::CheckpointStore {
  public:
    TestCheckpointStore()
    {
      m_checkpoints = std::map<std::string, Azure::Messaging::EventHubs::Models::Checkpoint>();
      m_ownerships = std::map<std::string, Azure::Messaging::EventHubs::Models::Ownership>();
    }

    std::vector<Azure::Messaging::EventHubs::Models::Checkpoint> ListCheckpoints(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        Core::Context const& = {}) override
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      std::string prefix = Models::Checkpoint{consumerGroup, eventHubName, fullyQualifiedNamespace}
                               .GetCheckpointBlobPrefixName();
      GTEST_LOG_(INFO) << "List checkpoints: List checkpoints for prefix " << prefix;
      std::vector<Azure::Messaging::EventHubs::Models::Checkpoint> checkpoints;
      for (auto const& checkpoint : m_checkpoints)
      {
        // If the key to the checkpoint matches the prefix, add that checkpoint.
        if (checkpoint.first.find(prefix) == 0)
        {
          checkpoints.push_back(checkpoint.second);
        }
      }
      GTEST_LOG_(INFO) << "List checkpoints: " << checkpoints.size() << " checkpoints found";
      return checkpoints;
    }

    std::vector<Azure::Messaging::EventHubs::Models::Ownership> ListOwnership(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        Core::Context const& = {}) override
    {
      std::string prefix = Models::Ownership{consumerGroup, eventHubName, fullyQualifiedNamespace}
                               .GetOwnershipPrefixName();
      GTEST_LOG_(INFO) << "ListOwnership: List ownership for prefix " << prefix;
      if (prefix.find("testeventhub") == 0)
      {
        GTEST_LOG_(ERROR) << "Fully qualified namespace is not valid.";
      }

      std::unique_lock<std::mutex> lock(m_mutex);
      std::vector<Azure::Messaging::EventHubs::Models::Ownership> ownerships;
      for (auto const& ownership : m_ownerships)
      {
        GTEST_LOG_(INFO) << "Check ownership " << ownership.first << " for prefix " << prefix
                         << ".";
        // If the key to the ownership matches the prefix, add that ownership.
        if (ownership.first.find(prefix) == 0)
        {
          ownerships.push_back(ownership.second);
        }
      }
      GTEST_LOG_(INFO) << "ListOwnership: " << ownerships.size() << " ownerships found";
      if (ownerships.size() != 0)
      {
        for (auto const& ownership : ownerships)
        {
          GTEST_LOG_(INFO) << "ListOwnership: Ownership found: " << ownership;
        }
      }
      return ownerships;
    }

    std::vector<Azure::Messaging::EventHubs::Models::Ownership> ClaimOwnership(
        std::vector<Models::Ownership> const& partitionOwnership,
        Core::Context const& = {}) override
    {
      GTEST_LOG_(INFO) << "Claim Ownership for: " << partitionOwnership.size() << " partitions";
      for (auto const& ownership : partitionOwnership)
      {
        GTEST_LOG_(INFO) << "Claim Ownership for: " << ownership;
      }
      std::unique_lock<std::mutex> lock(m_mutex);
      std::vector<Models::Ownership> owned;
      for (auto const& ownership : partitionOwnership)
      {
        Azure::Messaging::EventHubs::Models::Ownership newOwnership = UpdateOwnership(ownership);
        if (newOwnership.ETag.HasValue())
        {
          owned.push_back(newOwnership);
        }
      }
      GTEST_LOG_(INFO) << "Claim Ownership: " << owned.size() << " ownerships claimed";
      return owned;
    }

    // *** TEST HOOK *** */
    void ExpireOwnership(Azure::Messaging::EventHubs::Models::Ownership const& o)
    {
      std::unique_lock<std::mutex> lock(m_mutex);

      Models::Ownership temp = o;
      temp.LastModifiedTime
          = temp.LastModifiedTime.ValueOr(std::chrono::system_clock::now()) - std::chrono::hours(6);
      std::string key = temp.GetOwnershipPrefixName() + temp.PartitionId;

      m_ownerships[key] = temp;
    }

  private:
    std::map<std::string, Azure::Messaging::EventHubs::Models::Checkpoint> m_checkpoints;
    std::map<std::string, Azure::Messaging::EventHubs::Models::Ownership> m_ownerships;
    std::mutex m_mutex;

    Azure::Messaging::EventHubs::Models::Ownership UpdateOwnership(
        Azure::Messaging::EventHubs::Models::Ownership const& ownership)
    {
      if (ownership.ConsumerGroup.empty() || ownership.EventHubName.empty()
          || ownership.FullyQualifiedNamespace.empty() || ownership.PartitionId.empty())
      {
        throw std::runtime_error("Invalid ownership");
      }

      std::string key = ownership.GetOwnershipPrefixName() + ownership.PartitionId;
      GTEST_LOG_(INFO) << "Update Ownership for key " << key;

      // If the ownership exists, check the ETag to see if it matches.
      {
        auto foundOwnership = m_ownerships.find(key);
        if (foundOwnership != m_ownerships.end())
        {
          if (ownership.ETag.HasValue() && !foundOwnership->second.ETag.HasValue())
          {
            throw std::runtime_error("ETag mismatch in partition ownership.");
          }

          if (ownership.ETag.HasValue()
              && (ownership.ETag.Value() != foundOwnership->second.ETag.Value()))
          {
            return Models::Ownership{};
          }
        }
      }
      Azure::Messaging::EventHubs::Models::Ownership newOwnership = ownership;
      newOwnership.ETag = Azure::ETag(Azure::Core::Uuid::CreateUuid().ToString());
      newOwnership.LastModifiedTime = std::chrono::system_clock::now();

      m_ownerships[key] = Models::Ownership(newOwnership);
      return newOwnership;
    }

    void UpdateCheckpoint(
        Azure::Messaging::EventHubs::Models::Checkpoint const& checkpoint,
        Core::Context const& context = {}) override
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      GTEST_LOG_(INFO) << "UpdateCheckpoint for " << checkpoint;
      (void)context;
      if (checkpoint.ConsumerGroup.empty() || checkpoint.EventHubName.empty()
          || checkpoint.FullyQualifiedNamespaceName.empty() || checkpoint.PartitionId.empty())
      {
        throw std::runtime_error("Invalid checkpoint");
      }
      std::string key = checkpoint.GetCheckpointBlobPrefixName() + checkpoint.PartitionId;
      m_checkpoints[key] = checkpoint;
    }
  };

}}}} // namespace Azure::Messaging::EventHubs::Test
