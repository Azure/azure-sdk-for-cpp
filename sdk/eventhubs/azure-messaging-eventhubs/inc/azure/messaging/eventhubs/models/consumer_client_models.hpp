// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

// cspell: word myservicebus

#include <azure/core/amqp.hpp>
#include <azure/core/context.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/messaging/eventhubs.hpp>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

  /**@brief Contains options for the ConsumerClient creation
   */
  struct ConsumerClientDetails final
  {
    /**@brief The Fully Qualified Namespace that the Event Hub exists in.
     */
    std::string HostName;

    /**@brief The name of the consumer group that this consumer is associated with. Events will be
     * read only in the context of this group.
     */
    std::string ConsumerGroup;

    /**@brief The name of the Event Hub that the consumer is connected to.
     */
    std::string EventHubName;

    /**@brief A unique name used to identify this consumer.
     */
    std::string ClientID;
  };

}}}} // namespace Azure::Messaging::EventHubs::Models
