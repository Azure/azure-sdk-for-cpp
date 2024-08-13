// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/models/message_source.hpp"

#include "private/source_impl.hpp"
#include "private/value_impl.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/amqp_definitions_fields.h>

#include <azure_uamqp_c/amqp_definitions_terminus_durability.h>
#include <azure_uamqp_c/amqp_definitions_terminus_expiry_policy.h>

#include <azure_uamqp_c/amqp_definitions_filter_set.h>
#include <azure_uamqp_c/amqp_definitions_node_properties.h>
#include <azure_uamqp_c/amqp_definitions_seconds.h>
#include <azure_uamqp_c/amqp_definitions_source.h>
#include <azure_uamqp_c/amqpvalue.h>
#endif // ENABLE_UAMQP

#include <iostream>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  // @cond
  void UniqueHandleHelper<SOURCE_INSTANCE_TAG>::FreeMessageSource(SOURCE_HANDLE value)
  {
    source_destroy(value);
  }
// @endcond
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  MessageSource::~MessageSource() = default;

  MessageSource::MessageSource(Models::AmqpValue const& source)
      : m_impl{std::make_unique<_detail::MessageSourceImpl>(source)}
  {
  }

  /* Note: This constructor should NOT be marked as explicit, because we want to enable the
   * implicit construction of the MessageSource from a string - this allows callers to construct
   * Link, MessageSender, and MessageReceiver objects without forcing the creation of a
   * MessageSource object. */
  MessageSource::MessageSource(std::string const& address)
      : m_impl{std::make_unique<_detail::MessageSourceImpl>(address)}
  {
  }

  /* Note: This constructor should NOT be marked as explicit, because we want to enable the
   * implicit construction of the MessageSource from a string - this allows callers to construct
   * Link, MessageSender, and MessageReceiver objects without forcing the creation of a
   * MessageSource object. */
  MessageSource::MessageSource(char const* address)
      : m_impl{std::make_unique<_detail::MessageSourceImpl>(address)}
  {
  }

  MessageSource::MessageSource(MessageSource const& that)
      : m_impl{std::make_unique<_detail::MessageSourceImpl>(*that.m_impl)}
  {
  }

  MessageSource& MessageSource::operator=(MessageSource const& that)
  {
    m_impl = std::make_unique<_detail::MessageSourceImpl>(*that.m_impl);
    return *this;
  }

  MessageSource::MessageSource(MessageSource&& that) noexcept : m_impl(std::move(that.m_impl)) {}

  MessageSource& MessageSource::operator=(MessageSource&& that) noexcept
  {
    m_impl = std::move(that.m_impl);
    return *this;
  }

  MessageSource::MessageSource() : m_impl{std::make_unique<_detail::MessageSourceImpl>()} {}

  MessageSource::MessageSource(MessageSourceOptions const& options)
      : m_impl{std::make_unique<_detail::MessageSourceImpl>(options)}
  {
  }

  // Convert the MessageSource into a Value.
  Models::AmqpValue MessageSource::AsAmqpValue() const { return m_impl->AsAmqpValue(); }

  Models::AmqpValue MessageSource::GetAddress() const { return m_impl->GetAddress(); }

  TerminusDurability MessageSource::GetTerminusDurability() const
  {
    return m_impl->GetTerminusDurability();
  }

  TerminusExpiryPolicy MessageSource::GetExpiryPolicy() const { return m_impl->GetExpiryPolicy(); }

  std::chrono::system_clock::time_point MessageSource::GetTimeout() const
  {
    return m_impl->GetTimeout();
  }

  bool MessageSource::GetDynamic() const { return m_impl->GetDynamic(); }

  AmqpMap MessageSource::GetDynamicNodeProperties() const
  {
    return m_impl->GetDynamicNodeProperties();
  }
  std::string MessageSource::GetDistributionMode() const { return m_impl->GetDistributionMode(); }

  AmqpMap MessageSource::GetFilter() const { return m_impl->GetFilter(); }

  AmqpValue MessageSource::GetDefaultOutcome() const { return m_impl->GetDefaultOutcome(); }

  AmqpArray MessageSource::GetOutcomes() const { return m_impl->GetOutcomes(); }

  AmqpArray MessageSource::GetCapabilities() const { return m_impl->GetCapabilities(); }

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

  std::ostream& operator<<(std::ostream& os, MessageSource const& source)
  {
    os << *source.m_impl;
    return os;
  }

}}}}} // namespace Azure::Core::Amqp::Models::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {

  MessageSourceImpl::MessageSourceImpl(Models::AmqpValue const& source)
  {
    if (source.IsNull())
    {
      throw std::invalid_argument("source cannot be null");
    }
#if ENABLE_UAMQP
    {
      SOURCE_HANDLE sourceHandle;
      if (amqpvalue_get_source(_detail::AmqpValueFactory::ToImplementation(source), &sourceHandle))
      {
        throw std::runtime_error("Could not retrieve source from value.");
      }
      m_source.reset(sourceHandle);
    }
#endif
  }

  /* Note: This constructor should NOT be marked as explicit, because we want to enable the
   * implicit construction of the MessageSource from a string - this allows callers to construct
   * Link, MessageSender, and MessageReceiver objects without forcing the creation of a
   * MessageSource object. */
  MessageSourceImpl::MessageSourceImpl(std::string const& address)
#if ENABLE_UAMQP
      : m_source(source_create())
#endif
  {
#if ENABLE_UAMQP
    if (m_source == nullptr)
    {
      throw std::runtime_error("Could not create source.");
    }
    if (source_set_address(
            m_source.get(),
            _detail::UniqueAmqpValueHandle{amqpvalue_create_string(address.c_str())}.get()))
    {
      throw std::runtime_error("Could not set address.");
    }
#else
    (void)address;
#endif
  }

  /* Note: This constructor should NOT be marked as explicit, because we want to enable the
   * implicit construction of the MessageSource from a string - this allows callers to construct
   * Link, MessageSender, and MessageReceiver objects without forcing the creation of a
   * MessageSource object. */
  MessageSourceImpl::MessageSourceImpl(char const* address)
#if ENABLE_UAMQP
      : m_source(source_create())
#endif
  {
#if ENABLE_UAMQP
    if (m_source == nullptr)
    {
      throw std::runtime_error("Could not create source.");
    }
    if (source_set_address(
            m_source.get(), _detail::UniqueAmqpValueHandle{amqpvalue_create_string(address)}.get()))
    {
      throw std::runtime_error("Could not set address.");
    }
#else
    (void)address;
#endif
  }

  MessageSourceImpl::MessageSourceImpl(MessageSourceImpl const& that)
#if ENABLE_UAMQP
      : m_source
  {
    source_clone(that.m_source.get())
  }
#endif
  {
    (void)that;
  }

  MessageSourceImpl& MessageSourceImpl::operator=(MessageSourceImpl const& that)
  {
#if ENABLE_UAMQP
    m_source.reset(source_clone(that.m_source.get()));
#else
    (void)that;

#endif
    return *this;
  }

  MessageSourceImpl::MessageSourceImpl(MessageSourceImpl&& that) noexcept
#if ENABLE_UAMQP
      : m_source(std::move(that.m_source))
#endif
  {
    (void)that;
  }

  MessageSourceImpl& MessageSourceImpl::operator=(MessageSourceImpl&& that) noexcept
  {
#if ENABLE_UAMQP
    m_source = std::move(that.m_source);
#else
    (void)that;

#endif
    return *this;
  }

  MessageSourceImpl::MessageSourceImpl()
#if ENABLE_UAMQP
      : m_source(source_create())
#endif
  {
  }

  MessageSourceImpl::MessageSourceImpl(_internal::MessageSourceOptions const& options)
#if ENABLE_UAMQP
      : m_source(source_create())
#endif
  {
    if (!options.Address.IsNull())
    {
#if ENABLE_UAMQP
      if (source_set_address(
              m_source.get(), _detail::AmqpValueFactory::ToImplementation(options.Address)))
      {
        throw std::runtime_error("Could not set source address.");
      }
#endif
    }
    if (options.SourceTerminusDurability.HasValue())
    {
#if ENABLE_UAMQP
      terminus_durability durability;
      switch (options.SourceTerminusDurability.Value())
      {
        case _internal::TerminusDurability::None:
          durability = terminus_durability_none;
          break;
        case _internal::TerminusDurability::Configuration:
          durability = terminus_durability_configuration;
          break;
        case _internal::TerminusDurability::UnsettledState:
          durability = terminus_durability_unsettled_state;
          break;
        default:
          throw std::logic_error("Unknown terminus durability.");
      }
      if (source_set_durable(m_source.get(), durability))
      {
        throw std::runtime_error("Could not set durable.");
      }
#endif
    }
    if (options.SourceTerminusExpiryPolicy.HasValue())
    {
#if ENABLE_UAMQP
      terminus_expiry_policy policy;
      switch (options.SourceTerminusExpiryPolicy.Value())
      {
        case _internal::TerminusExpiryPolicy::LinkDetach:
          policy = terminus_expiry_policy_link_detach;
          break;
        case _internal::TerminusExpiryPolicy::SessionEnd:
          policy = terminus_expiry_policy_session_end;
          break;
        case _internal::TerminusExpiryPolicy::ConnectionClose:
          policy = terminus_expiry_policy_connection_close;
          break;
        case _internal::TerminusExpiryPolicy::Never:
          policy = terminus_expiry_policy_never;
          break;
        default:
          throw std::logic_error("Unknown terminus durability.");
      }
      if (source_set_expiry_policy(m_source.get(), policy))
      {
        throw std::runtime_error("Could not set durable.");
      }
#endif
    }
    if (options.Timeout.HasValue())
    {
#if ENABLE_UAMQP
      if (source_set_timeout(
              m_source.get(),
              static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(
                                        options.Timeout.Value().time_since_epoch())
                                        .count())))
      {
        throw std::runtime_error("Could not set value.");
      }
#endif
    }
    if (options.Dynamic.HasValue())
    {
#if ENABLE_UAMQP
      if (source_set_dynamic(m_source.get(), *options.Dynamic))
      {
        throw std::runtime_error("Could not set dynamic.");
      }
#endif
    }
    if (!options.DynamicNodeProperties.empty())
    {
#if ENABLE_UAMQP
      AmqpValue dynamicNodeProperties(options.DynamicNodeProperties.AsAmqpValue());
      if (source_set_dynamic_node_properties(
              m_source.get(), _detail::AmqpValueFactory::ToImplementation(dynamicNodeProperties)))
      {
        throw std::runtime_error("Could not set dynamic node properties.");
      }
#endif
    }
    if (options.DistributionMode.HasValue())
    {
#if ENABLE_UAMQP
      if (source_set_distribution_mode(m_source.get(), options.DistributionMode.Value().c_str()))
      {
        throw std::runtime_error("Could not set distribution mode.");
      }
#endif
    }
    if (!options.Filter.empty())
    {
#if ENABLE_UAMQP
      if (source_set_filter(
              m_source.get(),
              _detail::AmqpValueFactory::ToImplementation(options.Filter.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set filter set.");
      }
#endif
    }
    if (!options.DefaultOutcome.IsNull())
    {
#if ENABLE_UAMQP
      if (source_set_default_outcome(
              m_source.get(), _detail::AmqpValueFactory::ToImplementation(options.DefaultOutcome)))
      {
        throw std::runtime_error("Could not set default outcome.");
      }
#endif
    }
    if (!options.Outcomes.empty())
    {
#if ENABLE_UAMQP
      if (source_set_outcomes(
              m_source.get(),
              _detail::AmqpValueFactory::ToImplementation(options.Outcomes.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set outcomes.");
      }
#endif
    }
    if (!options.Capabilities.empty())
    {
#if ENABLE_UAMQP
      if (source_set_capabilities(
              m_source.get(),
              _detail::AmqpValueFactory::ToImplementation(options.Capabilities.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set capabilities.");
      }
#endif
    }
  }

  // Convert the MessageSource into a Value.
  Models::AmqpValue MessageSourceImpl::AsAmqpValue() const
  {
#if ENABLE_UAMQP
    Models::_detail::UniqueAmqpValueHandle sourceValue{amqpvalue_create_source(m_source.get())};
    return _detail::AmqpValueFactory::FromImplementation(sourceValue);
#else
    return {};
#endif
  }

  Models::AmqpValue MessageSourceImpl::GetAddress() const
  {
#if ENABLE_UAMQP
    AMQP_VALUE address;
    if (source_get_address(m_source.get(), &address))
    {
      throw std::runtime_error("Could not retrieve address from source.");
    }
    // source_get_address does not reference its value, so we need to reference it before creating
    // an AmqpValueHandle.
    return _detail::AmqpValueFactory::FromImplementation(
        Models::_detail::UniqueAmqpValueHandle{amqpvalue_clone(address)});
#else
    return {};
#endif
  }

  _internal::TerminusDurability MessageSourceImpl::GetTerminusDurability() const
  {
#if ENABLE_UAMQP
    terminus_durability value;
    if (source_get_durable(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get durable from source.");
    }
    switch (value)
    {
      case terminus_durability_configuration:
        return _internal::TerminusDurability::Configuration;
      case terminus_durability_none:
        return _internal::TerminusDurability::None;
      case terminus_durability_unsettled_state:
        return _internal::TerminusDurability::UnsettledState;
      default:
        throw std::logic_error("Unknown terminus durability.");
    }
#else
    return {};
#endif
  }

  _internal::TerminusExpiryPolicy MessageSourceImpl::GetExpiryPolicy() const
  {
#if ENABLE_UAMQP
    terminus_expiry_policy value;
    if (source_get_expiry_policy(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get expiry policy from source.");
    }
    if (std::strcmp(value, terminus_expiry_policy_connection_close) == 0)
    {
      return _internal::TerminusExpiryPolicy::ConnectionClose;
    }
    if (std::strcmp(value, terminus_expiry_policy_link_detach) == 0)
    {
      return _internal::TerminusExpiryPolicy::LinkDetach;
    }
    if (std::strcmp(value, terminus_expiry_policy_never) == 0)
    {
      return _internal::TerminusExpiryPolicy::Never;
    }
    if (std::strcmp(value, terminus_expiry_policy_session_end) == 0)
    {
      return _internal::TerminusExpiryPolicy::SessionEnd;
    }
    throw std::logic_error(std::string("Unknown terminus expiry policy: ") + value);
#else
    return {};
#endif
  }

  std::chrono::system_clock::time_point MessageSourceImpl::GetTimeout() const
  {
#if ENABLE_UAMQP
    seconds value;
    if (source_get_timeout(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get timeout from source.");
    }
    return std::chrono::system_clock::from_time_t(value);
#else
    return {};
#endif
  }

  bool MessageSourceImpl::GetDynamic() const
  {
    bool value = {};
#if ENABLE_UAMQP
    if (source_get_dynamic(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
#endif
    return value;
  }

  AmqpMap MessageSourceImpl::GetDynamicNodeProperties() const
  {
#if ENABLE_UAMQP
    AMQP_VALUE value;
    if (source_get_dynamic_node_properties(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
    return _detail::AmqpValueFactory::FromImplementation(
               _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)})
        .AsMap();
#else
    return {};
#endif
  }
  std::string MessageSourceImpl::GetDistributionMode() const
  {
    const char* value = {};
#if ENABLE_UAMQP
    if (source_get_distribution_mode(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get distribution mode.");
    }
#endif
    return value;
  }

  AmqpMap MessageSourceImpl::GetFilter() const
  {
#if ENABLE_UAMQP
    AMQP_VALUE value;
    if (source_get_filter(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get filter set.");
    }
    return _detail::AmqpValueFactory::FromImplementation(
               _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)})
        .AsMap();
#else
    return {};
#endif
  }

  AmqpValue MessageSourceImpl::GetDefaultOutcome() const
  {
#if ENABLE_UAMQP
    AMQP_VALUE value;
    if (source_get_default_outcome(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get default outcome.");
    }
    // source_get_default_outcome does not reference the value returned, we reference it so it can
    // be put into a UniqueAmqpValueHandle.
    return _detail::AmqpValueFactory::FromImplementation(
        _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)});
#else
    return {};
#endif
  }

  AmqpArray MessageSourceImpl::GetOutcomes() const
  {
#if ENABLE_UAMQP
    AMQP_VALUE value;
    if (source_get_outcomes(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get outcomes.");
    }
    return _detail::AmqpValueFactory::FromImplementation(
               _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)})
        .AsArray();
#else
    return {};
#endif
  }

  AmqpArray MessageSourceImpl::GetCapabilities() const
  {
#if ENABLE_UAMQP
    AMQP_VALUE value;
    if (source_get_capabilities(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get capabilities.");
    }
    return _detail::AmqpValueFactory::FromImplementation(
               _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)})
        .AsArray();
#else
    return {};
#endif
  }
#if 0
  const char* StringFromTerminusDurability(_internal::TerminusDurability durability)
  {
    switch (durability)
    {
      case _internal::TerminusDurability::None:
        return "None";
      case _internal::TerminusDurability::Configuration:
        return "Configuration";
      case _internal::TerminusDurability::UnsettledState:
        return "Unsettled State";
    }
    throw std::runtime_error("Unknown terminus durability");
  }
#endif

  std::ostream& operator<<(std::ostream& os, MessageSourceImpl const& source)
  {
    os << "Source{ ";
#if ENABLE_UAMQP
    {
      AMQP_VALUE value;
      if (!source_get_address(source, &value))
      {
        os << "Address: " << source.GetAddress();
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
        // source_get_default_outcome does not reference the value returned, we reference it so it
        // can be put into a UniqueAmqpValueHandle.
        os << ", Default Outcome: "
           << _detail::AmqpValueFactory::FromImplementation(
                  Models::_detail::UniqueAmqpValueHandle{amqpvalue_clone(outcome)});
      }
    }
    {
      AMQP_VALUE outcomes;
      if (!source_get_outcomes(source, &outcomes))
      {
        // Most of the time, source_get_outcomes does not reference its input. However, if the
        // composite value at location 9 is a symbol, it does reference it. As a consequence, this
        // will leak an AMQPSymbol if the value at location 9 is a symbol (as opposed to being an
        // array).
        os << ", Outcomes: "
           << _detail::AmqpValueFactory::FromImplementation(
                  Models::_detail::UniqueAmqpValueHandle{amqpvalue_clone(outcomes)});
      }
    }
#else
    (void)source;
#endif
    os << "}";
    return os;
  }

}}}}} // namespace Azure::Core::Amqp::Models::_detail
