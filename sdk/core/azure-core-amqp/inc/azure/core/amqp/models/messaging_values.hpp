// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_value.hpp"
namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  struct Messaging
  {
    static Azure::Core::Amqp::Models::AmqpValue CreateSource(std::string const& address);
    static Azure::Core::Amqp::Models::AmqpValue CreateTarget(std::string const& address);
    static Azure::Core::Amqp::Models::AmqpValue DeliveryReceived(
        uint32_t sectionNumber,
        uint64_t sectionOffset);
    static Azure::Core::Amqp::Models::AmqpValue DeliveryAccepted();
    static Azure::Core::Amqp::Models::AmqpValue DeliveryRejected(
        std::string const& errorCondition,
        std::string const& errorDescription);
    static Azure::Core::Amqp::Models::AmqpValue DeliveryReleased();
    static Azure::Core::Amqp::Models::AmqpValue DeliveryModified(
        bool deliveryFailed,
        bool undeliverableHere,
        Azure::Core::Amqp::Models::AmqpValue annotations);
  };

}}}}} // namespace Azure::Core::Amqp::Models::_internal
