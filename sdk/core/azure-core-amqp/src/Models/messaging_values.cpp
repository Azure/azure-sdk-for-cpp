// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#include "azure/core/amqp/models/messaging_values.hpp"

#include <azure_uamqp_c/amqp_definitions_fields.h>
#include <azure_uamqp_c/messaging.h>
/* Common Message values */
namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Models {
  Azure::Core::Amqp::Models::Value Messaging::DeliveryAccepted()
  {
    return messaging_delivery_accepted();
  }
  Azure::Core::Amqp::Models::Value Messaging::DeliveryReleased()
  {
    return messaging_delivery_released();
  }
  Azure::Core::Amqp::Models::Value Messaging::CreateSource(std::string const& address)
  {
    return messaging_create_source(address.c_str());
  }
  Azure::Core::Amqp::Models::Value Messaging::CreateTarget(std::string const& address)
  {
    return messaging_create_target(address.c_str());
  }

}}}}} // namespace Azure::Core::_internal::Amqp::Models
