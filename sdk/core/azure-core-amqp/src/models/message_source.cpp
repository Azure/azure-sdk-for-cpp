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

void Azure::Core::_internal::UniqueHandleHelper<SOURCE_INSTANCE_TAG>::FreeMessageSource(
    SOURCE_HANDLE value)
{
  source_destroy(value);
}

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  MessageSource::MessageSource(Azure::Core::Amqp::Models::AmqpValue const& source)
  {
    if (source.IsNull())
    {
      throw std::invalid_argument("source cannot be null");
    }
    {
      SOURCE_HANDLE sourceHandle;
      if (amqpvalue_get_source(source, &sourceHandle))
      {
        throw std::runtime_error("Could not retrieve source from value.");
      }
      m_source.reset(sourceHandle);
    }
  }

  MessageSource::MessageSource(std::string const& address) : m_source(source_create())
  {
    if (m_source == nullptr)
    {
      throw std::runtime_error("Could not create source.");
    }
    if (source_set_address(m_source.get(), amqpvalue_create_string(address.c_str())))
    {
      throw std::runtime_error("Could not set address.");
    }
  }
  MessageSource::MessageSource(char const* address) : m_source(source_create())
  {
    if (m_source == nullptr)
    {
      throw std::runtime_error("Could not create source.");
    }
    if (source_set_address(m_source.get(), amqpvalue_create_string(address)))
    {
      throw std::runtime_error("Could not set address.");
    }
  }

  MessageSource::MessageSource() : m_source(source_create()) {}

  MessageSource::MessageSource(MessageSourceOptions const& options) : m_source(source_create())
  {
    if (!options.Address.IsNull())
    {
      if (source_set_address(m_source.get(), options.Address))
      {
        throw std::runtime_error("Could not set source address.");
      }
    }
    if (options.SourceTerminusDurability.HasValue())
    {
      terminus_durability durability;
      switch (options.SourceTerminusDurability.Value())
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
      if (source_set_durable(m_source.get(), durability))
      {
        throw std::runtime_error("Could not set durable.");
      }
    }
    if (options.SourceTerminusExpiryPolicy.HasValue())
    {
      terminus_expiry_policy policy;
      switch (options.SourceTerminusExpiryPolicy.Value())
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
      if (source_set_expiry_policy(m_source.get(), policy))
      {
        throw std::runtime_error("Could not set durable.");
      }
    }
    if (options.Timeout.HasValue())
    {
      if (source_set_timeout(
              m_source.get(),
              static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(
                                        options.Timeout.Value().time_since_epoch())
                                        .count())))
      {
        throw std::runtime_error("Could not set value.");
      }
    }
    if (options.Dynamic.HasValue())
    {
      if (source_set_dynamic(m_source.get(), *options.Dynamic))
      {
        throw std::runtime_error("Could not set dynamic.");
      }
    }
    if (!options.DynamicNodeProperties.empty())
    {
      if (source_set_dynamic_node_properties(
              m_source.get(),
              static_cast<UniqueAmqpValueHandle>(options.DynamicNodeProperties).get()))
      {
        throw std::runtime_error("Could not set dynamic node properties.");
      }
    }
    if (options.DistributionMode.HasValue())
    {
      if (source_set_distribution_mode(m_source.get(), options.DistributionMode.Value().c_str()))
      {
        throw std::runtime_error("Could not set distribution mode.");
      }
    }
    if (!options.Filter.empty())
    {
      if (source_set_filter(
              m_source.get(), static_cast<UniqueAmqpValueHandle>(options.Filter).get()))
      {
        throw std::runtime_error("Could not set filter set.");
      }
    }
    if (!options.DefaultOutcome.IsNull())
    {
      if (source_set_default_outcome(m_source.get(), options.DefaultOutcome))
      {
        throw std::runtime_error("Could not set default outcome.");
      }
    }
    if (!options.Outcomes.empty())
    {
      if (source_set_outcomes(
              m_source.get(), static_cast<UniqueAmqpValueHandle>(options.Outcomes).get()))
      {
        throw std::runtime_error("Could not set outcomes.");
      }
    }
    if (!options.Capabilities.empty())
    {
      if (source_set_capabilities(
              m_source.get(), static_cast<UniqueAmqpValueHandle>(options.Capabilities).get()))
      {
        throw std::runtime_error("Could not set capabilities.");
      }
    }
  }

  // Convert the MessageSource into a Value.
  MessageSource::operator const Azure::Core::Amqp::Models::AmqpValue() const
  {
    return amqpvalue_create_source(m_source.get());
  }

  Azure::Core::Amqp::Models::AmqpValue MessageSource::GetAddress() const
  {
    AMQP_VALUE address;
    if (source_get_address(m_source.get(), &address))
    {
      throw std::runtime_error("Could not retrieve address from source.");
    }
    return address;
  }

  TerminusDurability MessageSource::GetTerminusDurability() const
  {
    terminus_durability value;
    if (source_get_durable(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get durable from source.");
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

  TerminusExpiryPolicy MessageSource::GetExpiryPolicy() const
  {
    terminus_expiry_policy value;
    if (source_get_expiry_policy(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get durable from source.");
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

  std::chrono::system_clock::time_point MessageSource::GetTimeout() const
  {
    seconds value;
    if (source_get_timeout(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get timeout from source.");
    }
    return std::chrono::system_clock::from_time_t(value);
  }

  bool MessageSource::GetDynamic() const
  {
    bool value;
    if (source_get_dynamic(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
    return value;
  }

  Azure::Core::Amqp::Models::AmqpMap MessageSource::GetDynamicNodeProperties() const
  {
    AMQP_VALUE value;
    if (source_get_dynamic_node_properties(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
    return value;
  }
  std::string MessageSource::GetDistributionMode() const
  {
    const char* value;
    if (source_get_distribution_mode(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get distribution mode.");
    }
    return value;
  }
  Azure::Core::Amqp::Models::AmqpMap MessageSource::GetFilter() const
  {
    AMQP_VALUE value;
    if (source_get_filter(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get filter set.");
    }
    return value;
  }

  Azure::Core::Amqp::Models::AmqpValue MessageSource::GetDefaultOutcome() const
  {
    AMQP_VALUE value;
    if (source_get_default_outcome(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get default outcome.");
    }
    return value;
  }

  Azure::Core::Amqp::Models::AmqpArray MessageSource::GetOutcomes() const
  {
    AMQP_VALUE value;
    if (source_get_outcomes(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get outcomes.");
    }
    return value;
  }

  Azure::Core::Amqp::Models::AmqpArray MessageSource::GetCapabilities() const
  {
    AMQP_VALUE value;
    if (source_get_capabilities(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get capabilities.");
    }
    return value;
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
    os << "Source{ ";
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
        os << ", Expiry Policy: " << policy;
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
        os << ", Default Outcome: " << AmqpValue{outcome};
      }
    }
    {
      AMQP_VALUE outcomes;
      if (!source_get_outcomes(source, &outcomes))
      {
        os << ", Outcomes: " << AmqpValue{outcomes};
      }
    }
    os << "}";
    return os;
  }

}}}}} // namespace Azure::Core::Amqp::Models::_internal
