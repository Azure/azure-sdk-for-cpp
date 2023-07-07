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

namespace Azure { namespace Messaging { namespace EventHubs {
  /**@brief  ListCheckpointsOptions contains optional parameters for the ListCheckpoints
   * function
   */
  struct ListCheckpointsOptions
  {
    // For future expansion
  };

  /**@brief  ListOwnershipOptions contains optional parameters for the ListOwnership function
   */
  struct ListOwnershipOptions
  {
    // For future expansion
  };

  /**@brief  UpdateCheckpointOptions contains optional parameters for the UpdateCheckpoint
   * function
   */
  struct UpdateCheckpointOptions
  {
    // For future expansion
  };

  /**@brief  ClaimOwnershipOptions contains optional parameters for the ClaimOwnership function
   */
  struct ClaimOwnershipOptions
  {
    // For future expansion
  };

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
    virtual std::vector<Models::Ownership> ClaimOwnership(
        std::vector<Models::Ownership> partitionOwnership,
        ClaimOwnershipOptions const& options = {},
        Azure::Core::Context ctx = {})
    {
      (void)partitionOwnership;
      (void)ctx;
      (void)options;
      throw std::runtime_error("Not Implemented");
    }

    /**@brief  ListCheckpoints lists all the available checkpoints.
     */
    virtual std::vector<Models::Checkpoint> ListCheckpoints(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        ListCheckpointsOptions options = {},
        Azure::Core::Context ctx = {})
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
    virtual std::vector<Models::Ownership> ListOwnership(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        ListOwnershipOptions options = {},
        Azure::Core::Context ctx = {})
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
        Models::Checkpoint const& checkpoint,
        UpdateCheckpointOptions options = {},
        Azure::Core::Context ctx = {})
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

    std::string GetOwnershipName(Models::Ownership const& ownership);

    std::string GetOwnershipPrefixName(Models::Ownership const& ownership);

    std::string GetCheckpointBlobPrefixName(Models::Checkpoint const& checkpoint);

    std::string GetCheckpointBlobName(Models::Checkpoint const& checkpoint);

    void UpdateCheckpointImpl(
        Azure::Storage::Metadata const& metadata,
        Models::Checkpoint& checkpoint);

    void UpdateOwnership(
        Azure::Storage::Blobs::Models::BlobItem const& blob,
        Models::Ownership& ownership);

    Azure::Storage::Metadata NewCheckpointBlobMetadata(Models::Checkpoint const& checkpoint);

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

    std::vector<Models::Ownership> ClaimOwnership(
        std::vector<Models::Ownership> partitionOwnership,
        ClaimOwnershipOptions const& options = {},
        Azure::Core::Context ctx = {}) override;

    std::vector<Models::Checkpoint> ListCheckpoints(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        ListCheckpointsOptions options = {},
        Azure::Core::Context ctx = {}) override;

    /**@brief  ListOwnership lists all ownerships.
     */
    std::vector<Models::Ownership> ListOwnership(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        ListOwnershipOptions options = {},
        Azure::Core::Context ctx = {}) override;

    /**@brief  UpdateCheckpoint updates a specific checkpoint with a sequence and offset.
     */
    void UpdateCheckpoint(
        Models::Checkpoint const& checkpoint,
        UpdateCheckpointOptions options = {},
        Azure::Core::Context ctx = {}) override;
  };
}}} // namespace Azure::Messaging::EventHubs
