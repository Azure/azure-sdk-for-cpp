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
        Core::Context const& context = {})
    {
      (void)partitionOwnership;
      (void)context;
      throw std::runtime_error("Not Implemented");
    }

    /**@brief  ListCheckpoints lists all the available checkpoints.
     */
    virtual std::vector<Models::Checkpoint> ListCheckpoints(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        Core::Context const& context = {})
    {
      (void)fullyQualifiedNamespace;
      (void)consumerGroup;
      (void)eventHubName;
      (void)context;
      throw std::runtime_error("Not Implemented");
    }

    /**@brief  ListOwnership lists all ownerships.
     */
    virtual std::vector<Models::Ownership> ListOwnership(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        Core::Context const& context = {})
    {
      (void)fullyQualifiedNamespace;
      (void)eventHubName;
      (void)consumerGroup;
      (void)context;
      throw std::runtime_error("Not Implemented");
    }

    /**@brief  UpdateCheckpoint updates a specific checkpoint with a sequence and offset.
     */
    virtual void UpdateCheckpoint(
        Models::Checkpoint const& checkpoint,
        Core::Context const& context = {})
    {
      (void)checkpoint;
      (void)context;
      throw std::runtime_error("Not Implemented");
    }

    virtual ~CheckpointStore() = default;
  };

  /** @brief BlobCheckpointStore is an implementation of a CheckpointStore backed by Azure Blob
   * Storage.
   */
  class BlobCheckpointStore final : public CheckpointStore {

    std::string m_connectionString;
    std::string m_containerName;
    Azure::Storage::Blobs::BlobContainerClient m_containerClient;

    void UpdateCheckpointImpl(
        Azure::Storage::Metadata const& metadata,
        Models::Checkpoint& checkpoint);

    void UpdateOwnership(
        Azure::Storage::Blobs::Models::BlobItem const& blob,
        Models::Ownership& ownership);

    Azure::Storage::Metadata CreateCheckpointBlobMetadata(Models::Checkpoint const& checkpoint);

    std::pair<Azure::DateTime, Azure::ETag> SetMetadata(
        std::string const& blobName,
        Azure::Storage::Metadata const& metadata,
        Azure::ETag const& etag,
        Core::Context const& context = {});

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
        Core::Context const& context = {}) override;

    std::vector<Models::Checkpoint> ListCheckpoints(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        Core::Context const& context = {}) override;

    /**@brief  ListOwnership lists all ownerships.
     */
    std::vector<Models::Ownership> ListOwnership(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        Core::Context const& context = {}) override;

    /**@brief  UpdateCheckpoint updates a specific checkpoint with a sequence and offset.
     */
    void UpdateCheckpoint(Models::Checkpoint const& checkpoint, Core::Context const& context = {})
        override;
  };
}}} // namespace Azure::Messaging::EventHubs
