// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/etag.hpp>
#include <azure/core/nullable.hpp>

#include <sstream>
#include <vector>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

  /**@brief  Ownership tracks which consumer owns a particular partition.
   */
  class Ownership final {
  public:
    /// @brief The consumer group name.
    std::string ConsumerGroup;
    /// @brief The event hub name.
    std::string EventHubName;
    /// @brief The fully qualified namespace for the event hub.
    std::string FullyQualifiedNamespace;
    /// @brief The partition ID for the corresponding ownership.
    std::string PartitionId{};
    /// @brief The owner ID for the corresponding ownership.
    std::string OwnerId{};
    /// the ETag, used when attempting to claim or update ownership of a partition.
    Azure::Nullable<Azure::ETag> ETag{};

    /// @brief The last modified time for the corresponding ownership. Used to calculate if
    /// ownership has expired.
    Azure::Nullable<Azure::DateTime> LastModifiedTime{};

    /// @brief The blob storage name for this ownership.
    std::string GetOwnershipName() const;

    /// @brief The blob storage name prefix for this ownership.
    std::string GetOwnershipPrefixName() const;
  };
  std::ostream& operator<<(std::ostream& os, Ownership const& value);

  /**@brief Checkpoint tracks the last successfully processed event in a partition.
   */
  class Checkpoint final {
  public:
    /// @brief The consumer group name.
    std::string ConsumerGroup;
    /// @brief The event hub name.
    std::string EventHubName;
    /// @brief The fully qualified namespace for the event hub.
    std::string FullyQualifiedNamespaceName;
    /// @brief The partition ID for the corresponding checkpoint.
    std::string PartitionId{};
    /// @brief The offset of the last successfully processed event.
    Azure::Nullable<std::string> Offset{};
    /// @brief The sequence number of the last successfully processed event.
    Azure::Nullable<int64_t> SequenceNumber{};

    /// @brief Returns the prefix for the name of the blob that stores the checkpoint.
    std::string GetCheckpointBlobPrefixName() const;

    /// @brief Returns the name of the blob that stores the checkpoint.
    std::string GetCheckpointBlobName() const;
  };
  std::ostream& operator<<(std::ostream& os, Checkpoint const& value);
}}}} // namespace Azure::Messaging::EventHubs::Models
