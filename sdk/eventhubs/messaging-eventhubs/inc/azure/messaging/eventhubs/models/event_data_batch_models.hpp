// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#pragma once
#include <azure/core/amqp.hpp>
#include <azure/core/amqp/models/amqp_message.hpp>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

  /** @brief EventDataBatchOptions contains optional parameters for the
   * [ProducerClient.NewEventDataBatch] function.
   *
   * @remark If both PartitionKey and PartitionID are nil, Event Hubs will choose an arbitrary
   * partition for any events in this [EventDataBatch].
   */
  struct EventDataBatchOptions
  {

    /** @brief MaxBytes overrides the max size (in bytes) for a batch.
     * By default NewEventDataBatch will use the max message size provided by the service.
     */
    uint32_t MaxBytes = std::numeric_limits<int32_t>::max();

    /** @brief PartitionKey is hashed to calculate the partition assignment.Messages and message
     * batches with the same PartitionKey are guaranteed to end up in the same partition.
     * Note that if you use this option then PartitionID cannot be set.
     */
    std::string PartitionKey;

    /** @brief PartitionID is the ID of the partition to send these messages to.
     * Note that if you use this option then PartitionKey cannot be set.
     */
    std::string PartitionID;
  };
}}}} // namespace Azure::Messaging::EventHubs::Models
