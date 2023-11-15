// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/models/message_target.hpp"

#include <azure/core/internal/diagnostics/log.hpp>

#include <azure_uamqp_c/amqp_definitions_fields.h>
#include <azure_uamqp_c/amqp_definitions_terminus_durability.h>
#include <azure_uamqp_c/amqp_definitions_terminus_expiry_policy.h>

#include <azure_uamqp_c/amqp_definitions_filter_set.h>
#include <azure_uamqp_c/amqp_definitions_node_properties.h>
#include <azure_uamqp_c/amqp_definitions_seconds.h>
#include <azure_uamqp_c/amqp_definitions_target.h>
#include <azure_uamqp_c/amqpvalue.h>

#include <iostream>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace _detail {
  void UniqueHandleHelper<TARGET_INSTANCE_TAG>::FreeMessageTarget(TARGET_HANDLE value)
  {
    target_destroy(value);
  }
}}} // namespace Azure::Core::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  extern const char* StringFromTerminusDurability(TerminusDurability);

  MessageTarget::MessageTarget(Models::AmqpValue const& target)
  {
    if (target.IsNull())
    {
      throw std::invalid_argument("target cannot be null");
    }
    TARGET_HANDLE targetHandle;
    if (amqpvalue_get_target(_detail::AmqpValueFactory::ToUamqp(target), &targetHandle))
    {
      throw std::runtime_error("Could not retrieve target from value.");
    }
    m_target.reset(targetHandle);
  }

  MessageTarget::MessageTarget(std::string const& address) : m_target{target_create()}
  {
    if (m_target == nullptr)
    {
      throw std::runtime_error("Could not create source.");
    }
    if (target_set_address(
            m_target.get(),
            _detail::UniqueAmqpValueHandle(amqpvalue_create_string(address.c_str())).get()))
    {
      throw std::runtime_error("Could not set address.");
    }
  }
  MessageTarget::MessageTarget(char const* address) : m_target{target_create()}
  {
    if (m_target == nullptr)
    {
      throw std::runtime_error("Could not create source.");
    }
    if (target_set_address(
            m_target.get(), _detail::UniqueAmqpValueHandle(amqpvalue_create_string(address)).get()))
    {
      throw std::runtime_error("Could not set address.");
    }
  }

  MessageTarget::MessageTarget() : m_target{target_create()} {}

  MessageTarget::MessageTarget(MessageTarget const& that) : m_target{target_clone(that)} {}

  MessageTarget& MessageTarget::operator=(MessageTarget const& that)
  {
    m_target.reset(target_clone(that));
    return *this;
  }

  MessageTarget::MessageTarget(MessageTarget&& that) noexcept : m_target{std::move(that.m_target)}
  {
  }

  MessageTarget& MessageTarget::operator=(MessageTarget&& that) noexcept
  {
    m_target = std::move(that.m_target);
    return *this;
  }

  MessageTarget::MessageTarget(MessageTargetOptions const& options) : m_target{target_create()}
  {
    if (!options.Address.IsNull())
    {
      if (target_set_address(m_target.get(), _detail::AmqpValueFactory::ToUamqp(options.Address)))
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
        default:
          throw std::logic_error("Unknown terminus durability.");
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
        default:
          throw std::logic_error("Unknown terminus durability.");
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
              _detail::AmqpValueFactory::ToUamqp(options.DynamicNodeProperties.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set dynamic node properties.");
      }
    }

    if (!options.Capabilities.empty())
    {
      if (target_set_capabilities(
              m_target.get(),
              _detail::AmqpValueFactory::ToUamqp(options.Capabilities.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set capabilities.");
      }
    }
  }

  // Convert the MessageSource into a Value.
  Models::AmqpValue MessageTarget::AsAmqpValue() const
  {
    Models::_detail::UniqueAmqpValueHandle targetValue{amqpvalue_create_target(m_target.get())};
    return _detail::AmqpValueFactory::FromUamqp(targetValue);
  }

  Models::AmqpValue MessageTarget::GetAddress() const
  {
    AMQP_VALUE address;
    if (target_get_address(m_target.get(), &address))
    {
      throw std::runtime_error("Could not retrieve address from source.");
    }
    // target_get_address does not reference the underlying address so we need to addref it here so
    // it gets freed properly.
    return _detail::AmqpValueFactory::FromUamqp(
        Models::_detail::UniqueAmqpValueHandle{amqpvalue_clone(address)});
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
      throw std::runtime_error("Could not get expiry policy from target.");
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
    throw std::logic_error(std::string("Unknown terminus expiry policy: ") + value);
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

  Models::AmqpMap MessageTarget::GetDynamicNodeProperties() const
  {
    AMQP_VALUE value;
    // Note: target_get_dynamic_node_properties does NOT reference the value.
    if (target_get_dynamic_node_properties(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
    // We clone the value before converting it to a UniqueAmqpValueHandle because the destructor for
    // UniqueAmqpValueHandle will remove the reference.
    return _detail::AmqpValueFactory::FromUamqp(
               Models::_detail::UniqueAmqpValueHandle{amqpvalue_clone(value)})
        .AsMap();
  }

  Models::AmqpArray MessageTarget::GetCapabilities() const
  {
    AMQP_VALUE value;
    if (target_get_capabilities(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get capabilities.");
    }
    return _detail::AmqpValueFactory::FromUamqp(
               _detail::UniqueAmqpValueHandle(amqpvalue_clone(value)))
        .AsArray();
  }

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
