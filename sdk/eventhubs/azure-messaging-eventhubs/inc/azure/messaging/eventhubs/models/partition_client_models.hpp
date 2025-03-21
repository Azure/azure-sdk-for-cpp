// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

#include <azure/core/amqp.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/nullable.hpp>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

  /**@brief StartPosition indicates the position to start receiving events within a partition.
   * The default position is Latest.
   *
   * @remark You can set this in the options for #ConsumerClient::CreatePartitionClient.
   */
  struct StartPosition final
  {
    /**@brief Offset will start the consumer after the specified offset. Can be exclusive
     * or inclusive, based on the Inclusive property.
     *@remark NOTE: offsets are not stable values, and might refer to different events over time
     * as the Event Hub events reach their age limit and are discarded.
     */
    Azure::Nullable<std::string> Offset;

    /**@brief SequenceNumber will start the consumer after the specified sequence number. Can be
     * exclusive or inclusive, based on the Inclusive property.
     */
    Azure::Nullable<int64_t> SequenceNumber;

    /**@brief EnqueuedTime will start the consumer before events that were enqueued on or after
     * EnqueuedTime. Can be exclusive or inclusive, based on the Inclusive property.
     */
    Azure::Nullable<Azure::DateTime> EnqueuedTime;

    /**@brief Inclusive configures whether the events directly at Offset,
     * SequenceNumber or EnqueuedTime will be included (true) or excluded
     * (false). The default is false.
     */
    bool Inclusive{false};

    /**@brief Earliest will start the consumer at the earliest event.
     */
    Azure::Nullable<bool> Earliest;

    /**@brief Latest will start the consumer after the last event.
     */
    Azure::Nullable<bool> Latest;
  };
  std::ostream& operator<<(std::ostream&, StartPosition const&);

}}}} // namespace Azure::Messaging::EventHubs::Models
