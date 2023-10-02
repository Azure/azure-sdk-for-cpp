// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include "models/checkpoint_store_models.hpp"

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>

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
     * returns the actual partitions that were claimed.
     */
    virtual std::vector<Models::Ownership> ClaimOwnership(
        std::vector<Models::Ownership> const& partitionOwnership,
        Core::Context const& context = {})
        = 0;

    /**@brief  ListCheckpoints lists all the available checkpoints.
     */
    virtual std::vector<Models::Checkpoint> ListCheckpoints(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        Core::Context const& context = {})
        = 0;

    /**@brief  ListOwnership lists all ownerships.
     */
    virtual std::vector<Models::Ownership> ListOwnership(
        std::string const& fullyQualifiedNamespace,
        std::string const& eventHubName,
        std::string const& consumerGroup,
        Core::Context const& context = {})
        = 0;

    /**@brief  UpdateCheckpoint updates a specific checkpoint with a sequence and offset.
     */
    virtual void UpdateCheckpoint(
        Models::Checkpoint const& checkpoint,
        Core::Context const& context = {})
        = 0;

    virtual ~CheckpointStore() = default;
  };

}}} // namespace Azure::Messaging::EventHubs
