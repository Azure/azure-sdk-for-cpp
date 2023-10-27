// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/amqp/models/amqp_value.hpp>

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  class Messaging final {
  public:
    static Models::AmqpValue DeliveryReceived(uint32_t sectionNumber, uint64_t sectionOffset);
    static Models::AmqpValue DeliveryAccepted();
    static Models::AmqpValue DeliveryRejected(
        std::string const& errorCondition,
        std::string const& errorDescription,
        Models::AmqpValue const& errorInformation);
    static Models::AmqpValue DeliveryReleased();
    static Models::AmqpValue DeliveryModified(
        bool deliveryFailed,
        bool undeliverableHere,
        Models::AmqpValue annotations);

  private:
    Messaging() = delete;
    ~Messaging() = delete;
  };

}}}}} // namespace Azure::Core::Amqp::Models::_internal
