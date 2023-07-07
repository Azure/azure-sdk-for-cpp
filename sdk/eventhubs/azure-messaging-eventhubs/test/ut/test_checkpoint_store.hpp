// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once

#include <azure/core/context.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  class TestCheckpointStore : public Azure::Messaging::EventHubs::CheckpointStore {
    std::map<std::string, Azure::Messaging::EventHubs::Models::Checkpoint> m_checkpoints;
    std::map<std::string, Azure::Messaging::EventHubs::Models::Ownership> m_ownerships;

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
        ListCheckpointsOptions options = ListCheckpointsOptions(),
        Azure::Core::Context ctx = Azure::Core::Context()) override
    {
      (void)fullyQualifiedNamespace;
      (void)eventHubName;
      (void)consumerGroup;
      (void)ctx;
      (void)options;
      std::vector<Azure::Messaging::EventHubs::Models::Checkpoint> checkpoints;
      for (auto const& checkpoint : m_checkpoints)
      {
        checkpoints.push_back(checkpoint.second);
      }
      return checkpoints;
    }

    std::vector<Azure::Messaging::EventHubs::Models::Ownership> ListOwnership(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        ListOwnershipOptions options = ListOwnershipOptions(),
        Azure::Core::Context ctx = Azure::Core::Context()) override
    {
      (void)fullyQualifiedNamespace;
      (void)eventHubName;
      (void)consumerGroup;
      (void)ctx;
      (void)options;
      std::vector<Azure::Messaging::EventHubs::Models::Ownership> ownerships;
      for (auto const& ownership : m_ownerships)
      {
        ownerships.push_back(ownership.second);
      }
      return ownerships;
    }

    std::vector<Azure::Messaging::EventHubs::Models::Ownership> ClaimOwnership(
        std::vector<Models::Ownership> partitionOwnership,
        ClaimOwnershipOptions const& options = ClaimOwnershipOptions(),
        Azure::Core::Context ctx = Azure::Core::Context()) override
    {
      (void)ctx;
      (void)options;
      std::vector<Models::Ownership> owned;
      for (auto& ownership : partitionOwnership)
      {
        Azure::Messaging::EventHubs::Models::Ownership newOwnership = UpdateOwnership(ownership);
        if (newOwnership.ETag.HasValue())
        {
          owned.push_back(newOwnership);
        }
      }
      return owned;
    }

    Azure::Messaging::EventHubs::Models::Ownership UpdateOwnership(
        Azure::Messaging::EventHubs::Models::Ownership ownership)
    {
      if (ownership.ConsumerGroup.empty() || ownership.EventHubName.empty()
          || ownership.FullyQualifiedNamespace.empty() || ownership.PartitionID.empty())
      {
        throw std::runtime_error("Invalid ownership");
      }

      std::string key = ownership.FullyQualifiedNamespace + "/" + ownership.EventHubName + "/"
          + ownership.ConsumerGroup + "/" + ownership.PartitionID;

      if (m_ownerships.find(key) != m_ownerships.end())
      {
        if (ownership.ETag.HasValue() == false)
        {
          throw std::runtime_error("ETag is required for claiming ownership");
        }

        if (ownership.ETag.Value() != m_ownerships[key].ETag.Value())
        {
          return Models::Ownership{};
        }
      }
      Azure::Messaging::EventHubs::Models::Ownership newOwnership = ownership;
      newOwnership.ETag = Azure::ETag(Azure::Core::Uuid::CreateUuid().ToString());
      newOwnership.LastModifiedTime = std::chrono::system_clock::now();

      m_ownerships[key] = Models::Ownership(newOwnership);
      return newOwnership;
    }

    void ExpireOwnership(Azure::Messaging::EventHubs::Models::Ownership& o)
    {
      Models::Ownership temp = o;
      temp.LastModifiedTime
          = temp.LastModifiedTime.ValueOr(std::chrono::system_clock::now()) - std::chrono::hours(6);
      std::string key = temp.FullyQualifiedNamespace + "/" + temp.EventHubName + "/"
          + temp.ConsumerGroup + "/" + temp.PartitionID;
      m_ownerships[key] = temp;
    }

    void UpdateCheckpoint(
        Azure::Messaging::EventHubs::Models::Checkpoint const& checkpoint,
        UpdateCheckpointOptions options = UpdateCheckpointOptions(),
        Azure::Core::Context ctx = Azure::Core::Context()) override
    {
      (void)ctx;
      (void)options;
      if (checkpoint.ConsumerGroup.empty() || checkpoint.EventHubName.empty()
          || checkpoint.EventHubHostName.empty() || checkpoint.PartitionID.empty())
      {
        throw std::runtime_error("Invalid checkpoint");
      }
      std::string key = checkpoint.EventHubHostName + "/" + checkpoint.EventHubName + "/"
          + checkpoint.ConsumerGroup + "/" + checkpoint.PartitionID;
      m_checkpoints[key] = checkpoint;
    }
  };

}}}} // namespace Azure::Messaging::EventHubs::Test
