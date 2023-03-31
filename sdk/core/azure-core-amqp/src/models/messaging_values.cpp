// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/models/messaging_values.hpp"

#include <azure_uamqp_c/amqp_definitions_fields.h>
#include <azure_uamqp_c/messaging.h>
/* Common Message values */
namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  Azure::Core::Amqp::Models::AmqpValue Messaging::DeliveryAccepted()
  {
    return messaging_delivery_accepted();
  }
  Azure::Core::Amqp::Models::AmqpValue Messaging::DeliveryReleased()
  {
    return messaging_delivery_released();
  }
  Azure::Core::Amqp::Models::AmqpValue Messaging::DeliveryReceived(
      uint32_t sectionNumber,
      uint64_t sectionOffset)
  {
    return messaging_delivery_received(sectionNumber, sectionOffset);
  }
  Azure::Core::Amqp::Models::AmqpValue Messaging::DeliveryRejected(
      std::string const& errorCondition,
      std::string const& errorDescription)
  {
    return messaging_delivery_rejected(
        errorCondition.empty() ? nullptr : errorCondition.c_str(),
        errorDescription.empty() ? nullptr : errorDescription.c_str());
  }
  Azure::Core::Amqp::Models::AmqpValue Messaging::DeliveryModified(
      bool deliveryFailed,
      bool undeliverableHere,
      Azure::Core::Amqp::Models::AmqpValue annotations)
  {
    return messaging_delivery_modified(deliveryFailed, undeliverableHere, annotations);
  }

  Azure::Core::Amqp::Models::AmqpValue Messaging::CreateSource(std::string const& address)
  {
    return messaging_create_source(address.c_str());
  }
  Azure::Core::Amqp::Models::AmqpValue Messaging::CreateTarget(std::string const& address)
  {
    return messaging_create_target(address.c_str());
  }

}}}}} // namespace Azure::Core::Amqp::Models::_internal
