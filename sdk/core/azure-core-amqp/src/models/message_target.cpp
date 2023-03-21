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

#include <azure_uamqp_c/amqp_definitions_target.h>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Models {

  MessageTarget::MessageTarget(TARGET_HANDLE handle) : m_target{handle} {}

  MessageTarget::MessageTarget(Azure::Core::Amqp::Models::Value const& source)
  {
    if (source.IsNull())
    {
      throw std::invalid_argument("source cannot be null");
    }
    if (amqpvalue_get_target(source, &m_target))
    {
      throw std::runtime_error("Could not retrieve source from value.");
    }
  }

  MessageTarget::~MessageTarget()
  {
    if (m_target != nullptr)
    {
      target_destroy(m_target);
      m_target = nullptr;
    }
  }

  // Convert the MessageSource into a Value.
  MessageTarget::operator Azure::Core::Amqp::Models::Value() const
  {
    return amqpvalue_create_target(m_target);
  }

  Azure::Core::Amqp::Models::Value MessageTarget::GetAddress() const
  {
    AMQP_VALUE address;
    if (target_get_address(m_target, &address))
    {
      throw std::runtime_error("Could not retrieve address from source.");
    }
    return address;
  }
  void MessageTarget::SetAddress(Azure::Core::Amqp::Models::Value const& value)
  {
    if (target_set_address(m_target, value))
    {
      throw std::runtime_error("Could not set value.");
    }
  }

  Azure::Core::Amqp::Models::TerminusDurability MessageTarget::GetTerminusDurability() const
  {
    terminus_durability value;
    if (target_get_durable(m_target, &value))
    {
      throw std::runtime_error("Could not get durable from source.");
    }
    switch (value)
    {
      case terminus_durability_configuration:
        return Azure::Core::Amqp::Models::TerminusDurability::Configuration;
      case terminus_durability_none:
        return Azure::Core::Amqp::Models::TerminusDurability::None;
      case terminus_durability_unsettled_state:
        return Azure::Core::Amqp::Models::TerminusDurability::UnsettledState;
      default:
        throw std::logic_error("Unknown terminus durability.");
    }
  }
  void MessageTarget::SetTerminusDurability(Azure::Core::Amqp::Models::TerminusDurability value)
  {
    terminus_durability durability;
    switch (value)
    {
      case Azure::Core::Amqp::Models::TerminusDurability::None:
        durability = terminus_durability_none;
        break;
      case Azure::Core::Amqp::Models::TerminusDurability::Configuration:
        durability = terminus_durability_configuration;
        break;
      case Azure::Core::Amqp::Models::TerminusDurability::UnsettledState:
        durability = terminus_durability_unsettled_state;
        break;
      default:
        throw std::logic_error("Unknown terminus durability.");
    }
    if (target_set_durable(m_target, durability))
    {
      throw std::runtime_error("Could not set durable.");
    }
  }

  Azure::Core::Amqp::Models::TerminusExpiryPolicy MessageTarget::GetExpiryPolicy() const
  {
    terminus_expiry_policy value;
    if (target_get_expiry_policy(m_target, &value))
    {
      throw std::runtime_error("Could not get durable from source.");
    }
    if (strcmp(value, terminus_expiry_policy_connection_close) == 0)
    {
      return Azure::Core::Amqp::Models::TerminusExpiryPolicy::ConnectionClose;
    }
    if (strcmp(value, terminus_expiry_policy_link_detach) == 0)
    {
      return Azure::Core::Amqp::Models::TerminusExpiryPolicy::LinkDetach;
    }
    if (strcmp(value, terminus_expiry_policy_never) == 0)
    {
      return Azure::Core::Amqp::Models::TerminusExpiryPolicy::Never;
    }
    if (strcmp(value, terminus_expiry_policy_session_end) == 0)
    {
      return Azure::Core::Amqp::Models::TerminusExpiryPolicy::SessionEnd;
    }
    throw std::logic_error(std::string("Unknown terminus expiry policy: ") + value);
  }
  void MessageTarget::SetExpiryPolicy(Azure::Core::Amqp::Models::TerminusExpiryPolicy value)
  {
    terminus_expiry_policy policy;
    switch (value)
    {
      case Azure::Core::Amqp::Models::TerminusExpiryPolicy::LinkDetach:
        policy = terminus_expiry_policy_link_detach;
        break;
      case Azure::Core::Amqp::Models::TerminusExpiryPolicy::SessionEnd:
        policy = terminus_expiry_policy_session_end;
        break;
      case Azure::Core::Amqp::Models::TerminusExpiryPolicy::ConnectionClose:
        policy = terminus_expiry_policy_connection_close;
        break;
      case Azure::Core::Amqp::Models::TerminusExpiryPolicy::Never:
        policy = terminus_expiry_policy_never;
        break;
      default:
        throw std::logic_error("Unknown terminus durability.");
    }
    if (target_set_expiry_policy(m_target, policy))
    {
      throw std::runtime_error("Could not set durable.");
    }
  }

  std::chrono::system_clock::time_point MessageTarget::GetTimeout() const
  {
    seconds value;
    if (target_get_timeout(m_target, &value))
    {
      throw std::runtime_error("Could not get timeout from source.");
    }
    return std::chrono::system_clock::from_time_t(value);
  }
  void MessageTarget::SetTimeout(std::chrono::system_clock::time_point const& value)
  {
    if (target_set_timeout(
            m_target,
            static_cast<uint32_t>(
                std::chrono::duration_cast<std::chrono::seconds>(value.time_since_epoch())
                    .count())))
    {
      throw std::runtime_error("Could not set value.");
    }
  }

  bool MessageTarget::GetDynamic() const
  {
    bool value;
    if (target_get_dynamic(m_target, &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
    return value;
  }
  void MessageTarget::SetDynamic(bool value)
  {
    if (target_set_dynamic(m_target, value))
    {
      throw std::runtime_error("Could not set dynamic.");
    }
  }

  Azure::Core::Amqp::Models::Value MessageTarget::GetDynamicNodeProperties() const
  {
    AMQP_VALUE value;
    if (target_get_dynamic_node_properties(m_target, &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
    return value;
  }
  void MessageTarget::SetDynamicNodeProperties(Azure::Core::Amqp::Models::Value const& value)
  {
    if (target_set_dynamic_node_properties(m_target, value))
    {
      throw std::runtime_error("Could not set dynamic node properties.");
    }
  }

  Azure::Core::Amqp::Models::Value MessageTarget::GetCapabilities() const
  {
    AMQP_VALUE value;
    if (target_get_capabilities(m_target, &value))
    {
      throw std::runtime_error("Could not get capabilities.");
    }
    return value;
  }
  void MessageTarget::SetCapabilities(Azure::Core::Amqp::Models::Value const& value)
  {
    if (target_set_capabilities(m_target, value))
    {
      throw std::runtime_error("Could not set outcomes.");
    }
  }

}}}}} // namespace Azure::Core::_internal::Amqp::Models
