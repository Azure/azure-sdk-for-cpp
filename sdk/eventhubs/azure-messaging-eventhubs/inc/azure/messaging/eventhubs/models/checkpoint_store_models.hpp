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

  /**@brief  Ownership tracks which consumer owns a particular partition.
   */
  struct Ownership
  {
    /// @brief The consumer group name.
    std::string ConsumerGroup;
    /// @brief The event hub name.
    std::string EventHubName;
    /// @brief The fully qualified namespace for the event hub.
    std::string FullyQualifiedNamespace;
    /// @brief The partition ID for the corresponding ownership.
    std::string PartitionID{};
    /// @brief The owner ID for the corresponding ownership.
    std::string OwnerID{};
    /// the ETag, used when attempting to claim or update ownership of a partition.
    Azure::Nullable<Azure::ETag> ETag{};

    /// @brief The last modified time for the corresponding ownership. Used to calculate if
    /// ownership has expired.
    Azure::Nullable<Azure::DateTime> LastModifiedTime{};
  };

  /**@brief Checkpoint tracks the last successfully processed event in a partition.
   */
  struct Checkpoint
  {
    /// @brief The consumer group name.
    std::string ConsumerGroup{};
    /// @brief The event hub name.
    std::string EventHubName{};
    /// @brief The fully qualified namespace for the event hub.
    std::string EventHubHostName{};
    /// @brief The partition ID for the corresponding checkpoint.
    std::string PartitionID{};
    /// @brief The offset of the last successfully processed event.
    Azure::Nullable<int64_t> Offset{};
    /// @brief The sequence number of the last successfully processed event.
    Azure::Nullable<int64_t> SequenceNumber{};
  };
}}}} // namespace Azure::Messaging::EventHubs::Models
