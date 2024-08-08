// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/models/messaging_values.hpp"

#include "private/value_impl.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/amqp_definitions_fields.h>

#include <azure_uamqp_c/messaging.h>
#endif
/* Common Message values */
namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  Models::AmqpValue Messaging::DeliveryAccepted()
  {
#if ENABLE_UAMQP
    Models::_detail::UniqueAmqpValueHandle rv{messaging_delivery_accepted()};
    if (!rv)
    {
      throw std::runtime_error("Could not allocate delivery accepted described value.");
    }
    return _detail::AmqpValueFactory::FromUamqp(rv);
#else
    return {};
#endif
  }

  Models::AmqpValue Messaging::DeliveryReleased()
  {
#if ENABLE_UAMQP
    Models::_detail::UniqueAmqpValueHandle rv{messaging_delivery_released()};
    if (!rv)
    {
      throw std::runtime_error("Could not allocate delivery released described value.");
    }
    return _detail::AmqpValueFactory::FromUamqp(rv);
#else
    return {};

#endif
  }
  Models::AmqpValue Messaging::DeliveryReceived(uint32_t sectionNumber, uint64_t sectionOffset)
  {
#if ENABLE_UAMQP
    Models::_detail::UniqueAmqpValueHandle rv{
        messaging_delivery_received(sectionNumber, sectionOffset)};
    if (!rv)
    {
      throw std::runtime_error("Could not allocate delivery received described value.");
    }
    return _detail::AmqpValueFactory::FromUamqp(rv);
#else
    (void)sectionNumber;
    (void)sectionOffset;
    return {};
#endif
  }
  AmqpValue Messaging::DeliveryRejected(
      std::string const& errorCondition,
      std::string const& errorDescription,
      AmqpValue const& errorInfo)
  {
#if ENABLE_UAMQP
    _detail::UniqueAmqpValueHandle rv{messaging_delivery_rejected(
        errorCondition.empty() ? nullptr : errorCondition.c_str(),
        errorDescription.empty() ? nullptr : errorDescription.c_str(),
        _detail::AmqpValueFactory::ToUamqp(errorInfo))};
    if (!rv)
    {
      throw std::runtime_error("Could not allocate delivery rejected described value.");
    }
    return _detail::AmqpValueFactory::FromUamqp(rv);
#else
    (void)errorCondition;
    (void)errorDescription;
    (void)errorInfo;
    return {};
#endif
  }
  AmqpValue Messaging::DeliveryModified(
      bool deliveryFailed,
      bool undeliverableHere,
      AmqpValue annotations)
  {
#if ENABLE_UAMQP
    Models::_detail::UniqueAmqpValueHandle rv{messaging_delivery_modified(
        deliveryFailed, undeliverableHere, _detail::AmqpValueFactory::ToUamqp(annotations))};
    if (!rv)
    {
      throw std::runtime_error("Could not allocate delivery modified described value.");
    }
    return _detail::AmqpValueFactory::FromUamqp(rv);
#else
    (void)deliveryFailed;
    (void)undeliverableHere;
    (void)annotations;
    return {};
#endif
  }
}}}}} // namespace Azure::Core::Amqp::Models::_internal
