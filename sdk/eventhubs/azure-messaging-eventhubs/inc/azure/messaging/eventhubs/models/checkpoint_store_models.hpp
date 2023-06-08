// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/nullable.hpp>
#include <azure/storage/blobs.hpp>

#include <sstream>
#include <vector>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {
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
  /**@brief  Ownership tracks which consumer owns a particular partition.
   */
  struct Ownership
  {
    std::string ConsumerGroup;
    std::string EventHubName;
    std::string FullyQualifiedNamespace;
    std::string PartitionID;
    std::string OwnerID; // the owner ID of the Processor
    Azure::Nullable<Azure::ETag>
        ETag; // the ETag, used when attempting to claim or update ownership of a partition.
    Azure::Nullable<Azure::DateTime>
        LastModifiedTime; // used when calculating if ownership has expired
  };

  /**@brief Checkpoint tracks the last successfully processed event in a partition.
   */
  struct Checkpoint
  {
    std::string ConsumerGroup;
    std::string EventHubName;
    std::string FullyQualifiedNamespace;
    std::string PartitionID;
    Azure::Nullable<int64_t> Offset; // the last successfully processed Offset.
    Azure::Nullable<int64_t> SequenceNumber; // the last successfully processed SequenceNumber.
  };
}}}} // namespace Azure::Messaging::EventHubs::Models
