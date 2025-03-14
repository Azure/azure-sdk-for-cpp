// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <azure/core/datetime.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {
  /**@brief EventHubProperties represents properties of the Event Hub, like the number of
   * partitions.
   */
  struct EventHubProperties final
  {
    /// Time when the Event Hub was created.
    Azure::DateTime CreatedOn;
    /// The name of the Event Hub.
    std::string Name;

    /// A list of the partitions in the Event Hub.
    std::vector<std::string> PartitionIds;
  };
  std::ostream& operator<<(std::ostream&, EventHubProperties const&);

  /**@brief EventHubPartitionProperties represents properties of an Event Hub partition
   */
  struct EventHubPartitionProperties final
  {
    /** The name of the Event Hub where the partitions reside, specific to the Event Hubs namespace
     * that contains it.*/
    std::string Name;

    /** The identifier of the partition, unique to the Event Hub which contains it. */
    std::string PartitionId;

    /** The first sequence number available for events in the partition.
     */
    int64_t BeginningSequenceNumber{};
    /** The sequence number of the last observed event to be enqueued in the partition. */
    int64_t LastEnqueuedSequenceNumber{};
    /** The offset of the last observed event to be enqueued in the partition */
    std::string LastEnqueuedOffset{};

    /** The date and time, in UTC, that the last observed event was enqueued in the partition. */
    Azure::DateTime LastEnqueuedTimeUtc;

    /** Indicates whether or not the partition is currently empty. */
    bool IsEmpty{};
  };
  std::ostream& operator<<(std::ostream&, EventHubPartitionProperties const&);

}}}} // namespace Azure::Messaging::EventHubs::Models
