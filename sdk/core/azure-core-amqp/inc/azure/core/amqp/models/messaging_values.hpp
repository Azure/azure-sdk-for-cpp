// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include "amqp_value.hpp"
namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Models {
  struct Messaging
  {
    static Azure::Core::Amqp::Models::Value CreateSource(std::string const& address);
    static Azure::Core::Amqp::Models::Value CreateTarget(std::string const& address);
    static Azure::Core::Amqp::Models::Value DeliveryReceived(
        uint32_t sectionNumber,
        uint64_t sectionOffset);
    static Azure::Core::Amqp::Models::Value DeliveryAccepted();
    static Azure::Core::Amqp::Models::Value DeliveryRejected(
        std::string const& errorCondition,
        std::string const& errorDescription);
    static Azure::Core::Amqp::Models::Value DeliveryReleased();
    static Azure::Core::Amqp::Models::Value DeliveryModified(
        bool deliveryFailed,
        bool undeliverableHere,
        Azure::Core::Amqp::Models::Value annotations);
  };

}}}}} // namespace Azure::Core::_internal::Amqp::Models
