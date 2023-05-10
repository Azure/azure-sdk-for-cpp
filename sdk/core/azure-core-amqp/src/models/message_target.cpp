// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/models/message_target.hpp"

#include <azure_uamqp_c/amqp_definitions_fields.h>
#include <azure_uamqp_c/amqp_definitions_filter_set.h>
#include <azure_uamqp_c/amqp_definitions_node_properties.h>
#include <azure_uamqp_c/amqp_definitions_seconds.h>
#include <azure_uamqp_c/amqp_definitions_terminus_durability.h>
#include <azure_uamqp_c/amqp_definitions_terminus_expiry_policy.h>
#include <azure_uamqp_c/amqpvalue.h>
#include <iostream>

#include <azure_uamqp_c/amqp_definitions_target.h>

void Azure::Core::_internal::UniqueHandleHelper<TARGET_INSTANCE_TAG>::FreeMessageTarget(
    TARGET_HANDLE value)
{
  target_destroy(value);
}

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  //  MessageTarget::MessageTarget(TARGET_HANDLE handle) : m_target{handle} {}

  MessageTarget::MessageTarget(Azure::Core::Amqp::Models::AmqpValue const& source)
  {
    if (source.IsNull())
    {
      throw std::invalid_argument("source cannot be null");
    }
    TARGET_HANDLE targetHandle;
    if (amqpvalue_get_target(source, &targetHandle))
    {
      throw std::runtime_error("Could not retrieve source from value.");
    }
    m_target.reset(targetHandle);
  }

  MessageTarget::MessageTarget(std::string const& address) : m_target{target_create()}
  {
    if (m_target == nullptr)
    {
      throw std::runtime_error("Could not create source."); // LCOV_EXCL_LINE
    }
    if (target_set_address(m_target.get(), AmqpValue{address}))
    {
      throw std::runtime_error("Could not set address."); // LCOV_EXCL_LINE
    }
  }
  MessageTarget::MessageTarget(char const* address) : m_target{target_create()}
  {
    if (m_target == nullptr)
    {
      throw std::runtime_error("Could not create source."); // LCOV_EXCL_LINE
    }
    if (target_set_address(m_target.get(), AmqpValue{address}))
    {
      throw std::runtime_error("Could not set address."); // LCOV_EXCL_LINE
    }
  }

  MessageTarget::MessageTarget() : m_target{target_create()} {}

  MessageTarget::MessageTarget(MessageTargetOptions const& options) : m_target{target_create()}
  {
    if (!options.Address.IsNull())
    {
      if (target_set_address(m_target.get(), options.Address))
      {
        throw std::runtime_error("Could not set source address.");
      }
    }
    if (options.TerminusDurabilityValue.HasValue())
    {
      terminus_durability durability;
      switch (options.TerminusDurabilityValue.Value())
      {
        case TerminusDurability::None:
          durability = terminus_durability_none;
          break;
        case TerminusDurability::Configuration:
          durability = terminus_durability_configuration;
          break;
        case TerminusDurability::UnsettledState:
          durability = terminus_durability_unsettled_state;
          break;
        default: // LCOV_EXCL_LINE
          throw std::logic_error("Unknown terminus durability."); // LCOV_EXCL_LINE
      }
      if (target_set_durable(m_target.get(), durability))
      {
        throw std::runtime_error("Could not set durable.");
      }
    }
    if (options.TerminusExpiryPolicyValue.HasValue())
    {
      terminus_expiry_policy policy;
      switch (options.TerminusExpiryPolicyValue.Value())
      {
        case TerminusExpiryPolicy::LinkDetach:
          policy = terminus_expiry_policy_link_detach;
          break;
        case TerminusExpiryPolicy::SessionEnd:
          policy = terminus_expiry_policy_session_end;
          break;
        case TerminusExpiryPolicy::ConnectionClose:
          policy = terminus_expiry_policy_connection_close;
          break;
        case TerminusExpiryPolicy::Never:
          policy = terminus_expiry_policy_never;
          break;
        default: // LCOV_EXCL_LINE
          throw std::logic_error("Unknown terminus durability."); // LCOV_EXCL_LINE
      }
      if (target_set_expiry_policy(m_target.get(), policy))
      {
        throw std::runtime_error("Could not set durable.");
      }
    }
    if (options.Timeout.HasValue())
    {
      if (target_set_timeout(
              m_target.get(),
              static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(
                                        options.Timeout.Value().time_since_epoch())
                                        .count())))
      {
        throw std::runtime_error("Could not set value.");
      }
    }
    if (options.Dynamic.HasValue())
    {
      if (target_set_dynamic(m_target.get(), *options.Dynamic))
      {
        throw std::runtime_error("Could not set dynamic.");
      }
    }

    if (!options.DynamicNodeProperties.empty())
    {
      if (target_set_dynamic_node_properties(
              m_target.get(),
              static_cast<UniqueAmqpValueHandle>(options.DynamicNodeProperties).get()))
      {
        throw std::runtime_error("Could not set dynamic node properties.");
      }
    }

    if (!options.Capabilities.empty())
    {
      if (target_set_capabilities(
              m_target.get(), static_cast<UniqueAmqpValueHandle>(options.Capabilities).get()))
      {
        throw std::runtime_error("Could not set capabilities.");
      }
    }
  }

  // Convert the MessageSource into a Value.
  MessageTarget::operator const Azure::Core::Amqp::Models::AmqpValue() const
  {
    return amqpvalue_create_target(m_target.get());
  }

  Azure::Core::Amqp::Models::AmqpValue MessageTarget::GetAddress() const
  {
    AMQP_VALUE address;
    if (target_get_address(m_target.get(), &address))
    {
      throw std::runtime_error("Could not retrieve address from source.");
    }
    return address;
  }

  TerminusDurability MessageTarget::GetTerminusDurability() const
  {
    terminus_durability value;
    if (target_get_durable(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get durable from target.");
    }
    switch (value)
    {
      case terminus_durability_configuration:
        return TerminusDurability::Configuration;
      case terminus_durability_none:
        return TerminusDurability::None;
      case terminus_durability_unsettled_state:
        return TerminusDurability::UnsettledState;
      default:
        throw std::logic_error("Unknown terminus durability.");
    }
  }

  TerminusExpiryPolicy MessageTarget::GetExpiryPolicy() const
  {
    terminus_expiry_policy value;
    if (target_get_expiry_policy(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get expiry policy from target."); // LCOV_EXCL_LINE
    }
    if (std::strcmp(value, terminus_expiry_policy_connection_close) == 0)
    {
      return TerminusExpiryPolicy::ConnectionClose;
    }
    if (std::strcmp(value, terminus_expiry_policy_link_detach) == 0)
    {
      return TerminusExpiryPolicy::LinkDetach;
    }
    if (std::strcmp(value, terminus_expiry_policy_never) == 0)
    {
      return TerminusExpiryPolicy::Never;
    }
    if (std::strcmp(value, terminus_expiry_policy_session_end) == 0)
    {
      return TerminusExpiryPolicy::SessionEnd;
    }
    throw std::logic_error(
        std::string("Unknown terminus expiry policy: ") + value); // LCOV_EXCL_LINE
  }

  std::chrono::system_clock::time_point MessageTarget::GetTimeout() const
  {
    seconds value;
    if (target_get_timeout(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get timeout from source.");
    }
    return std::chrono::system_clock::from_time_t(value);
  }

  bool MessageTarget::GetDynamic() const
  {
    bool value;
    if (target_get_dynamic(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
    return value;
  }

  Azure::Core::Amqp::Models::AmqpMap MessageTarget::GetDynamicNodeProperties() const
  {
    AMQP_VALUE value;
    if (target_get_dynamic_node_properties(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
    return AmqpValue{value}.AsMap();
  }

  Azure::Core::Amqp::Models::AmqpArray MessageTarget::GetCapabilities() const
  {
    AMQP_VALUE value;
    if (target_get_capabilities(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get capabilities.");
    }
    return value;
  }

  extern const char* StringFromTerminusDurability(TerminusDurability);

  std::ostream& operator<<(std::ostream& os, MessageTarget const& target)
  {
    os << "Target{ ";
    {
      AMQP_VALUE value;
      if (!target_get_address(target, &value))
      {
        os << "Address: " << target.GetAddress();
      }
    }
    {
      uint32_t value;
      if (!target_get_durable(target, &value))
      {
        os << ", Durable: " << StringFromTerminusDurability(target.GetTerminusDurability());
      }
    }
    {
      terminus_expiry_policy policy;
      if (!target_get_expiry_policy(target, &policy))
      {
        os << ", Expiry Policy: " << policy;
      }
    }
    {
      seconds timeout;
      if (!target_get_timeout(target, &timeout))
      {
        os << ", Timeout: " << target.GetTimeout().time_since_epoch().count();
      }
    }
    {
      bool dynamic;
      if (!target_get_dynamic(target, &dynamic))
      {
        os << ", Dynamic: " << std::boolalpha << dynamic;
      }
    }
    {
      AMQP_VALUE dynamicProperties;
      if (!target_get_dynamic_node_properties(target, &dynamicProperties))
      {
        os << ", Dynamic Node Properties: " << target.GetDynamicNodeProperties();
      }
    }
    {
      AMQP_VALUE capabilities;
      if (!target_get_capabilities(target, &capabilities))
      {
        os << ", Capabilities: " << target.GetCapabilities();
      }
    }

    os << "}";
    return os;
  }

}}}}} // namespace Azure::Core::Amqp::Models::_internal
