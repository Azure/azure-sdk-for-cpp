// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once

// cspell: word myservicebus
#include <string>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

  /**@brief Contains options for the ConsumerClient creation
   */
  struct ConsumerClientDetails final
  {
    /**@brief The Fully Qualified Namespace that the Event Hub exists in.
     */
    std::string FullyQualifiedNamespace;

    /**@brief The name of the consumer group that this consumer is associated with. Events will be
     * read only in the context of this group.
     */
    std::string ConsumerGroup;

    /**@brief The name of the Event Hub that the consumer is connected to.
     */
    std::string EventHubName;

    /**@brief A unique name used to identify this consumer.
     */
    std::string ClientId;
  };

}}}} // namespace Azure::Messaging::EventHubs::Models


