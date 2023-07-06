// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include "models/checkpoint_store_models.hpp"

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>
#include <azure/storage/blobs.hpp>

#include <sstream>
#include <stdexcept>
#include <vector>
using namespace Azure::Messaging::EventHubs::Models;
namespace Azure { namespace Messaging { namespace EventHubs {

  /**@brief  CheckpointStore is used by multiple consumers to coordinate progress and ownership for
   * partitions.
   */
  class CheckpointStore {
  public:
    CheckpointStore() = default;

    /** @brief  Construct a CheckpointStore from another CheckpointStore.
     */
    CheckpointStore(CheckpointStore const& other) = default;

    /** @brief  Construct a CheckpointStore from another CheckpointStore.
     */
    CheckpointStore& operator=(CheckpointStore const& other) = default;
    /**@brief ClaimOwnership attempts to claim ownership of the partitions in partitionOwnership and
     * returns the actual partitions that    were claimed.
     */
    virtual std::vector<Ownership> ClaimOwnership(
        std::vector<Ownership> partitionOwnership,
        Azure::Core::Context ctx = {},
        ClaimOwnershipOptions const& options = {})
    {
      (void)partitionOwnership;
      (void)ctx;
      (void)options;
      throw std::runtime_error("Not Implemented");
    }

    /**@brief  ListCheckpoints lists all the available checkpoints.
     */
    virtual std::vector<Checkpoint> ListCheckpoints(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        Azure::Core::Context ctx = {},
        ListCheckpointsOptions options = {})
    {
      (void)fullyQualifiedNamespace;
      (void)consumerGroup;
      (void)eventHubName;
      (void)ctx;
      (void)options;
      throw std::runtime_error("Not Implemented");
    }

    /**@brief  ListOwnership lists all ownerships.
     */
    virtual std::vector<Ownership> ListOwnership(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        Azure::Core::Context ctx = {},
        ListOwnershipOptions options = {})
    {
      (void)fullyQualifiedNamespace;
      (void)eventHubName;
      (void)consumerGroup;
      (void)ctx;
      (void)options;
      throw std::runtime_error("Not Implemented");
    }

    /**@brief  UpdateCheckpoint updates a specific checkpoint with a sequence and offset.
     */
    virtual void UpdateCheckpoint(
        Checkpoint const& checkpoint,
        Azure::Core::Context ctx = {},
        UpdateCheckpointOptions options = {})
    {
      (void)checkpoint;
      (void)ctx;
      (void)options;
      throw std::runtime_error("Not Implemented");
    }

    virtual ~CheckpointStore() = default;
  };

  /** @brief BlobCheckpointStore is an implementation of a CheckpointStore backed by Azure Blob
   * Storage.
   */
  class BlobCheckpointStore : public CheckpointStore {

    std::string m_connectionString;
    std::string m_containerName;
    Azure::Storage::Blobs::BlobContainerClient m_containerClient;

    std::string GetOwnershipName(Ownership const& ownership);

    std::string GetOwnershipPrefixName(Ownership const& ownership);

    std::string GetCheckpointBlobPrefixName(Checkpoint const& checkpoint);

    std::string GetCheckpointBlobName(Checkpoint const& checkpoint);

    void UpdateCheckpointImpl(Azure::Storage::Metadata const& metadata, Checkpoint& checkpoint);

    void UpdateOwnership(Azure::Storage::Blobs::Models::BlobItem const& blob, Ownership& ownership);

    Azure::Storage::Metadata NewCheckpointBlobMetadata(Checkpoint const& checkpoint);

    std::pair<Azure::DateTime, Azure::ETag> SetMetadata(
        std::string const& blobName,
        Azure::Storage::Metadata const& metadata,
        Azure::ETag const& etag,
        Azure::Core::Context const& context = Azure::Core::Context());

  public:
    /** @brief  Construct a BlobCheckpointStore from another BlobCheckpointStore.
     */
    BlobCheckpointStore(BlobCheckpointStore const& other) = default;

    /** @brief  Assign a BlobCheckpointStore to another BlobCheckpointStore.
     */
    BlobCheckpointStore& operator=(BlobCheckpointStore const& other) = default;

    /**@brief  Construct a BlobCheckpointStore.
     *
     * @param connectionString  The connection string of an Azure Storage account.
     * @param containerName  The name of the blob container.
     */
    BlobCheckpointStore(std::string const& connectionString, std::string const& containerName)
        : CheckpointStore(), m_connectionString(connectionString), m_containerName(containerName),
          m_containerClient(Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
              connectionString,
              containerName))
    {
      m_containerClient.CreateIfNotExists();
    }

    std::vector<Ownership> ClaimOwnership(
        std::vector<Ownership> partitionOwnership,
        Azure::Core::Context ctx = {},
        ClaimOwnershipOptions const& options = {}) override;

    std::vector<Checkpoint> ListCheckpoints(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        Azure::Core::Context ctx = {},
        ListCheckpointsOptions options = {}) override;

    /**@brief  ListOwnership lists all ownerships.
     */
    std::vector<Ownership> ListOwnership(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        Azure::Core::Context ctx = {},
        ListOwnershipOptions options = {}) override;

    /**@brief  UpdateCheckpoint updates a specific checkpoint with a sequence and offset.
     */
    void UpdateCheckpoint(
        Checkpoint const& checkpoint,
        Azure::Core::Context ctx = {},
        UpdateCheckpointOptions options = {}) override;
  };
}}} // namespace Azure::Messaging::EventHubs
