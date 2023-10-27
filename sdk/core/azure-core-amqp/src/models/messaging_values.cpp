// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/models/messaging_values.hpp"

#include <azure_uamqp_c/amqp_definitions_fields.h>

#include <azure_uamqp_c/messaging.h>
/* Common Message values */
namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  Models::AmqpValue Messaging::DeliveryAccepted()
  {
    auto rv = messaging_delivery_accepted();
    if (!rv)
    {
      throw std::runtime_error("Could not allocate delivery accepted described value.");
    }
    return rv;
  }
  Models::AmqpValue Messaging::DeliveryReleased()
  {
    auto rv = messaging_delivery_released();
    if (!rv)
    {
      throw std::runtime_error("Could not allocate delivery released described value.");
    }
    return rv;
  }
  Models::AmqpValue Messaging::DeliveryReceived(uint32_t sectionNumber, uint64_t sectionOffset)
  {
    auto rv = messaging_delivery_received(sectionNumber, sectionOffset);
    if (!rv)
    {
      throw std::runtime_error("Could not allocate delivery received described value.");
    }
    return rv;
  }
  Models::AmqpValue Messaging::DeliveryRejected(
      std::string const& errorCondition,
      std::string const& errorDescription,
      AmqpValue const& errorInfo)
  {
    auto rv = messaging_delivery_rejected(
        errorCondition.empty() ? nullptr : errorCondition.c_str(),
        errorDescription.empty() ? nullptr : errorDescription.c_str(),
        errorInfo);
    if (!rv)
    {
      throw std::runtime_error("Could not allocate delivery rejected described value.");
    }
    return rv;
  }
  Models::AmqpValue Messaging::DeliveryModified(
      bool deliveryFailed,
      bool undeliverableHere,
      Models::AmqpValue annotations)
  {
    auto rv = messaging_delivery_modified(deliveryFailed, undeliverableHere, annotations);
    if (!rv)
    {
      throw std::runtime_error("Could not allocate delivery modified described value.");
    }
    return rv;
  }

}}}}} // namespace Azure::Core::Amqp::Models::_internal
