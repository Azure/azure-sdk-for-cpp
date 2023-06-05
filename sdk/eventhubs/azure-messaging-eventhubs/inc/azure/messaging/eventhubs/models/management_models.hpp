// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {
  /**@brief EventHubProperties represents properties of the Event Hub, like the number of
   * partitions.
   */
  struct EventHubProperties
  {
    Azure::DateTime CreatedOn;
    std::string Name;
    std::vector<std::string> PartitionIDs;
  };

  /**@brief EventHubPartitionProperties represents properties of an Event Hub partition
   */
  struct EventHubPartitionProperties final
  {
    std::string Name;
    std::string PartitionId;
    int64_t BeginningSequenceNumber{};
    int64_t LastEnqueuedSequenceNumber{};
    std::string LastEnqueuedOffset;
    Azure::DateTime LastEnqueuedTimeUtc;
    bool IsEmpty{};
  };

  /**@brief GetEventHubPropertiesOptions contains optional parameters for the GetEventHubProperties
   * function
   */
  struct GetEventHubPropertiesOptions
  {
    // For future expansion
  };

  /**@brief GetPartitionPropertiesOptions contains optional parameters for the
   * GetPartitionProperties function
   */
  struct GetPartitionPropertiesOptions
  {
    // For future expansion
  };

}}}} // namespace Azure::Messaging::EventHubs::Models