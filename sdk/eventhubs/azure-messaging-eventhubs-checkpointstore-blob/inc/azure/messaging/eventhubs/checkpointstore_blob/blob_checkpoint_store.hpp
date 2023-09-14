// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>
#include <azure/messaging/eventhubs/checkpoint_store.hpp>
#include <azure/messaging/eventhubs/models/checkpoint_store_models.hpp>
#include <azure/storage/blobs.hpp>

#include <sstream>
#include <stdexcept>
#include <vector>

namespace Azure { namespace Messaging { namespace EventHubs {

  /** @brief BlobCheckpointStore is an implementation of a CheckpointStore backed by Azure Blob
   * Storage.
   */
  class BlobCheckpointStore final : public Azure::Messaging::EventHubs::CheckpointStore {
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
     * @param containerClient An Azure Blob ContainerClient used to hold the checkpoints.
     */
    BlobCheckpointStore(Azure::Storage::Blobs::BlobContainerClient const& containerClient)
        : Azure::Messaging::EventHubs::CheckpointStore(), m_containerClient(containerClient)
    {
      m_containerClient.CreateIfNotExists();
    }

    std::vector<Models::Ownership> ClaimOwnership(
        std::vector<Models::Ownership> const& partitionOwnership,
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
