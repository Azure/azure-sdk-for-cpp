// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/models/message_source.hpp"

#include <azure_uamqp_c/amqp_definitions_fields.h>
#include <azure_uamqp_c/amqp_definitions_filter_set.h>
#include <azure_uamqp_c/amqp_definitions_node_properties.h>
#include <azure_uamqp_c/amqp_definitions_seconds.h>
#include <azure_uamqp_c/amqp_definitions_terminus_durability.h>
#include <azure_uamqp_c/amqp_definitions_terminus_expiry_policy.h>
#include <azure_uamqp_c/amqpvalue.h>
#include <iostream>

#include <azure_uamqp_c/amqp_definitions_source.h>

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  MessageSource::MessageSource(SOURCE_HANDLE handle) : m_source{handle} {}

  MessageSource::MessageSource(Azure::Core::Amqp::Models::Value const& source)
  {
    if (source.IsNull())
    {
      throw std::invalid_argument("source cannot be null");
    }
    if (amqpvalue_get_source(source, &m_source))
    {
      throw std::runtime_error("Could not retrieve source from value.");
    }
  }

  MessageSource::~MessageSource()
  {
    if (m_source != nullptr)
    {
      source_destroy(m_source);
      m_source = nullptr;
    }
  }

  // Convert the MessageSource into a Value.
  MessageSource::operator Azure::Core::Amqp::Models::Value() const
  {
    return amqpvalue_create_source(m_source);
  }

  Azure::Core::Amqp::Models::Value MessageSource::GetAddress() const
  {
    AMQP_VALUE address;
    if (source_get_address(m_source, &address))
    {
      throw std::runtime_error("Could not retrieve address from source.");
    }
    return address;
  }
  void MessageSource::SetAddress(Azure::Core::Amqp::Models::Value const& value)
  {
    if (source_set_address(m_source, value))
    {
      throw std::runtime_error("Could not set value.");
    }
  }

  Azure::Core::Amqp::Models::TerminusDurability MessageSource::GetTerminusDurability() const
  {
    terminus_durability value;
    if (source_get_durable(m_source, &value))
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
  void MessageSource::SetTerminusDurability(Azure::Core::Amqp::Models::TerminusDurability value)
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
    if (source_set_durable(m_source, durability))
    {
      throw std::runtime_error("Could not set durable.");
    }
  }

  Azure::Core::Amqp::Models::TerminusExpiryPolicy MessageSource::GetExpiryPolicy() const
  {
    terminus_expiry_policy value;
    if (source_get_expiry_policy(m_source, &value))
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
  void MessageSource::SetExpiryPolicy(Azure::Core::Amqp::Models::TerminusExpiryPolicy value)
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
    if (source_set_expiry_policy(m_source, policy))
    {
      throw std::runtime_error("Could not set durable.");
    }
  }

  std::chrono::system_clock::time_point MessageSource::GetTimeout() const
  {
    seconds value;
    if (source_get_timeout(m_source, &value))
    {
      throw std::runtime_error("Could not get timeout from source.");
    }
    return std::chrono::system_clock::from_time_t(value);
  }
  void MessageSource::SetTimeout(std::chrono::system_clock::time_point const& value)
  {
    if (source_set_timeout(
            m_source,
            static_cast<uint32_t>(
                std::chrono::duration_cast<std::chrono::seconds>(value.time_since_epoch())
                    .count())))
    {
      throw std::runtime_error("Could not set value.");
    }
  }

  bool MessageSource::GetDynamic() const
  {
    bool value;
    if (source_get_dynamic(m_source, &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
    return value;
  }
  void MessageSource::SetDynamic(bool value)
  {
    if (source_set_dynamic(m_source, value))
    {
      throw std::runtime_error("Could not set dynamic.");
    }
  }

  Azure::Core::Amqp::Models::Value MessageSource::GetDynamicNodeProperties() const
  {
    AMQP_VALUE value;
    if (source_get_dynamic_node_properties(m_source, &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
    return value;
  }
  void MessageSource::SetDynamicNodeProperties(Azure::Core::Amqp::Models::Value const& value)
  {
    if (source_set_dynamic_node_properties(m_source, value))
    {
      throw std::runtime_error("Could not set dynamic node properties.");
    }
  }

  std::string MessageSource::GetDistributionMode() const
  {
    const char* value;
    if (source_get_distribution_mode(m_source, &value))
    {
      throw std::runtime_error("Could not get distribution mode.");
    }
    return value;
  }
  void MessageSource::SetDistributionMode(std::string const& value)
  {
    if (source_set_distribution_mode(m_source, value.c_str()))
    {
      throw std::runtime_error("Could not set dynamic node properties.");
    }
  }

  Azure::Core::Amqp::Models::Value MessageSource::GetFilter() const
  {
    AMQP_VALUE value;
    if (source_get_filter(m_source, &value))
    {
      throw std::runtime_error("Could not get filter set.");
    }
    return value;
  }
  void MessageSource::SetFilter(Azure::Core::Amqp::Models::Value const& value)
  {
    if (source_set_filter(m_source, value))
    {
      throw std::runtime_error("Could not set filter set.");
    }
  }

  Azure::Core::Amqp::Models::Value MessageSource::GetDefaultOutcome() const
  {
    AMQP_VALUE value;
    if (source_get_default_outcome(m_source, &value))
    {
      throw std::runtime_error("Could not get default outcome.");
    }
    return value;
  }
  void MessageSource::SetDefaultOutcome(Azure::Core::Amqp::Models::Value const& value)
  {
    if (source_set_default_outcome(m_source, value))
    {
      throw std::runtime_error("Could not set default outcome.");
    }
  }

  Azure::Core::Amqp::Models::Value MessageSource::GetOutcomes() const
  {
    AMQP_VALUE value;
    if (source_get_outcomes(m_source, &value))
    {
      throw std::runtime_error("Could not get outcomes.");
    }
    return value;
  }
  void MessageSource::SetOutcomes(Azure::Core::Amqp::Models::Value const& value)
  {
    if (source_set_outcomes(m_source, value))
    {
      throw std::runtime_error("Could not set outcomes.");
    }
  }

  Azure::Core::Amqp::Models::Value MessageSource::GetCapabilities() const
  {
    AMQP_VALUE value;
    if (source_get_capabilities(m_source, &value))
    {
      throw std::runtime_error("Could not get capabilities.");
    }
    return value;
  }
  void MessageSource::SetCapabilities(Azure::Core::Amqp::Models::Value const& value)
  {
    if (source_set_capabilities(m_source, value))
    {
      throw std::runtime_error("Could not set outcomes.");
    }
  }

  const char* StringFromTerminusDurability(TerminusDurability durability)
  {
    switch (durability)
    {
      case TerminusDurability::None:
        return "None";
      case TerminusDurability::Configuration:
        return "Configuration";
      case TerminusDurability::UnsettledState:
        return "Unsettled State";
    }
    throw std::runtime_error("Unknown terminus durability");
  }

  const char* StringFromTerminusExpiryPolicy(TerminusExpiryPolicy policy)
  {
    switch (policy)
    {
      case TerminusExpiryPolicy::LinkDetach:
        return "LinkDetach";
      case TerminusExpiryPolicy::SessionEnd:
        return "Session End";
      case TerminusExpiryPolicy::ConnectionClose:
        return "Connection Close";
      case TerminusExpiryPolicy::Never:
        return "Never";
    }
    throw std::runtime_error("Unknown terminus expiry policy");
  }

  std::ostream& operator<<(std::ostream& os, MessageSource const& source)
  {
    os << "Source{ " << std::endl;
    {
      AMQP_VALUE value;
      if (!source_get_address(source, &value))
      {
        os << "Address: " << source.GetAddress() << std::endl;
      }
    }
    {
      uint32_t value;
      if (!source_get_durable(source, &value))
      {
        os << ", Durable: " << StringFromTerminusDurability(source.GetTerminusDurability());
      }
    }
    {
      terminus_expiry_policy policy;
      if (!source_get_expiry_policy(source, &policy))
      {
        os << ", Expiry Policy: " << StringFromTerminusExpiryPolicy(source.GetExpiryPolicy());
      }
    }
    {
      seconds timeout;
      if (!source_get_timeout(source, &timeout))
      {
        os << ", Timeout: " << source.GetTimeout().time_since_epoch().count();
      }
    }
    {
      bool dynamic;
      if (!source_get_dynamic(source, &dynamic))
      {
        os << ", Dynamic: " << std::boolalpha << dynamic;
      }
    }
    {
      AMQP_VALUE dynamicProperties;
      if (!source_get_dynamic_node_properties(source, &dynamicProperties))
      {
        os << ", Dynamic Node Properties: " << source.GetDynamicNodeProperties();
      }
    }
    {
      AMQP_VALUE capabilities;
      if (!source_get_capabilities(source, &capabilities))
      {
        os << ", Capabilities: " << source.GetCapabilities();
      }
    }
    {
      const char* distributionMode;
      if (!source_get_distribution_mode(source, &distributionMode))
        os << ", Distribution Mode: " << distributionMode;
    }

    {
      AMQP_VALUE filter;
      if (!source_get_filter(source, &filter))
      {
        os << ", Filter: " << source.GetFilter();
      }
    }
    {
      AMQP_VALUE outcome;
      if (!source_get_default_outcome(source, &outcome))
      {
        os << ", Default Outcome: " << source.GetDefaultOutcome();
      }
    }
    {
      AMQP_VALUE outcome;
      if (!source_get_outcomes(source, &outcome))
      {
        os << ", Outcome: " << source.GetOutcomes();
      }
    }
    os << "}";
    return os;
  }

}}}}} // namespace Azure::Core::Amqp::Models::_internal
