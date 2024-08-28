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
#elif ENABLE_RUST_AMQP
#include "../rust_amqp/rust_wrapper/rust_amqp_wrapper.h"
using namespace Azure::Core::Amqp::_detail::RustInterop;
#endif // ENABLE_UAMQP

#include <iostream>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  // @cond
  void UniqueHandleHelper<AmqpSourceImplementation>::FreeMessageSource(
      AmqpSourceImplementation* value)
  {
    source_destroy(value);
  }
  // @endcond

#if ENABLE_RUST_AMQP
  using AmqpSourceBuilderImplementation
      = Azure::Core::Amqp::_detail::RustInterop::RustAmqpSourceBuilder;

  template <> struct UniqueHandleHelper<AmqpSourceBuilderImplementation>
  {
    static void FreeMessageSourceBuilder(AmqpSourceBuilderImplementation* obj);

    using type = Core::_internal::
        BasicUniqueHandle<AmqpSourceBuilderImplementation, FreeMessageSourceBuilder>;
  };

  void UniqueHandleHelper<AmqpSourceBuilderImplementation>::FreeMessageSourceBuilder(
      AmqpSourceBuilderImplementation* value)
  {
    source_builder_destroy(value);
  }
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
  using UniqueAmqpMessageSource = Azure::Core::Amqp::_detail::UniqueHandle<
      Azure::Core::Amqp::_detail::AmqpSourceImplementation>;
#if ENABLE_RUST_AMQP
  using UniqueAmqpMessageSourceBuilder = Azure::Core::Amqp::_detail::UniqueHandle<
      Azure::Core::Amqp::_detail::AmqpSourceBuilderImplementation>;
#endif
  MessageSourceImpl::MessageSourceImpl(Models::AmqpValue const& source)
  {
    if (source.IsNull())
    {
      throw std::invalid_argument("source cannot be null");
    }
#if ENABLE_UAMQP
    {
      Azure::Core::Amqp::_detail::AmqpSourceImplementation* sourceHandle;
      if (amqpvalue_get_source(_detail::AmqpValueFactory::ToImplementation(source), &sourceHandle))
      {
        throw std::runtime_error("Could not retrieve source from value.");
      }
      m_source.reset(sourceHandle);
    }
#endif
  }

  /* Note: This constructor should NOT be marked as explicit, because we want to enable the
   * implicit construction of the MessageSource from a string - this allows callers to
   * construct Link, MessageSender, and MessageReceiver objects without forcing the creation
   * of a MessageSource object. */
  MessageSourceImpl::MessageSourceImpl(std::string const& address)
  {
#if ENABLE_UAMQP
    UniqueAmqpMessageSource source{source_create()};
#elif ENABLE_RUST_AMQP
    UniqueAmqpMessageSourceBuilder source{source_builder_create()};
#endif

    if (source == nullptr)
    {
      throw std::runtime_error("Could not create source.");
    }

    if (source_set_address(
            source.get(),
            _detail::UniqueAmqpValueHandle{amqpvalue_create_string(address.c_str())}.get()))
    {
      throw std::runtime_error("Could not set address.");
    }
#if ENABLE_UAMQP
    m_source = std::move(source);
#elif ENABLE_RUST_AMQP
    Azure::Core::Amqp::_detail::AmqpSourceImplementation* sourceHandle;
    if (source_builder_build(source.get(), &sourceHandle))
    {
      throw std::runtime_error("Could not build source.");
    }
    m_source = UniqueAmqpMessageSource{sourceHandle};
#endif
  }

  /* Note: This constructor should NOT be marked as explicit, because we want to enable the
   * implicit construction of the MessageSource from a string - this allows callers to
   * construct Link, MessageSender, and MessageReceiver objects without forcing the creation
   * of a MessageSource object. */
  MessageSourceImpl::MessageSourceImpl(char const* address)
  {
#if ENABLE_UAMQP
    UniqueAmqpMessageSource source{source_create()};
#elif ENABLE_RUST_AMQP
    UniqueAmqpMessageSourceBuilder source{source_builder_create()};
#endif

    if (source == nullptr)
    {
      throw std::runtime_error("Could not create source.");
    }

    if (m_source == nullptr)
    {
      throw std::runtime_error("Could not create source.");
    }
    if (source_set_address(
            source.get(), _detail::UniqueAmqpValueHandle{amqpvalue_create_string(address)}.get()))
    {
      throw std::runtime_error("Could not set address.");
    }
#if ENABLE_UAMQP
    m_source = std::move(source);
#elif ENABLE_RUST_AMQP
    Azure::Core::Amqp::_detail::AmqpSourceImplementation* sourceHandle;
    if (source_builder_build(source.get(), &sourceHandle))
    {
      throw std::runtime_error("Could not build source.");
    }
    m_source = UniqueAmqpMessageSource{sourceHandle};
#endif
  }

  MessageSourceImpl::MessageSourceImpl(MessageSourceImpl const& that)
      : m_source{source_clone(that.m_source.get())}
  {
  }

  MessageSourceImpl& MessageSourceImpl::operator=(MessageSourceImpl const& that)
  {
    m_source.reset(source_clone(that.m_source.get()));
    return *this;
  }

  MessageSourceImpl::MessageSourceImpl(MessageSourceImpl&& that) noexcept
      : m_source(std::move(that.m_source))
  {
  }

  MessageSourceImpl& MessageSourceImpl::operator=(MessageSourceImpl&& that) noexcept
  {
    m_source = std::move(that.m_source);
    return *this;
  }

  MessageSourceImpl::MessageSourceImpl()
#if ENABLE_UAMQP
      : m_source(source_create())
#endif
  {
  }

  MessageSourceImpl::MessageSourceImpl(_internal::MessageSourceOptions const& options)
  {
#if ENABLE_UAMQP
    UniqueAmqpMessageSource source{source_create()};
#elif ENABLE_RUST_AMQP
    UniqueAmqpMessageSourceBuilder source{source_builder_create()};
#endif

    if (!options.Address.IsNull())
    {
      if (source_set_address(
              source.get(), _detail::AmqpValueFactory::ToImplementation(options.Address)))
      {
        throw std::runtime_error("Could not set source address.");
      }
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
#elif ENABLE_RUST_AMQP
      RustTerminusDurability durability;
      switch (options.SourceTerminusDurability.Value())
      {
        case _internal::TerminusDurability::None:
          durability = RustTerminusDurability::None;
          break;
        case _internal::TerminusDurability::Configuration:
          durability = RustTerminusDurability::Configuration;
          break;
        case _internal::TerminusDurability::UnsettledState:
          durability = RustTerminusDurability::UnsettledState;
          break;
        default:
          throw std::logic_error("Unknown terminus durability.");
      }
#endif

      if (source_set_durable(source.get(), durability))
      {
        throw std::runtime_error("Could not set durable.");
      }
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
#elif ENABLE_RUST_AMQP
      RustExpiryPolicy policy;
      switch (options.SourceTerminusExpiryPolicy.Value())
      {
        case _internal::TerminusExpiryPolicy::LinkDetach:
          policy = RustExpiryPolicy::LinkDetach;
          break;
        case _internal::TerminusExpiryPolicy::SessionEnd:
          policy = RustExpiryPolicy::SessionEnd;
          break;
        case _internal::TerminusExpiryPolicy::ConnectionClose:
          policy = RustExpiryPolicy::ConnectionClose;
          break;
        case _internal::TerminusExpiryPolicy::Never:
          policy = RustExpiryPolicy::Never;
          break;
        default:
          throw std::logic_error("Unknown terminus durability.");
      }
#endif
      if (source_set_expiry_policy(source.get(), policy))
      {
        throw std::runtime_error("Could not set durable.");
      }
    }
    if (options.Timeout.HasValue())
    {
      if (source_set_timeout(
              source.get(),
              static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(
                                        options.Timeout.Value().time_since_epoch())
                                        .count())))
      {
        throw std::runtime_error("Could not set value.");
      }
    }
    if (options.Dynamic.HasValue())
    {
      if (source_set_dynamic(source.get(), *options.Dynamic))
      {
        throw std::runtime_error("Could not set dynamic.");
      }
    }
    if (!options.DynamicNodeProperties.empty())
    {
      AmqpValue dynamicNodeProperties(options.DynamicNodeProperties.AsAmqpValue());
      if (source_set_dynamic_node_properties(
              source.get(), _detail::AmqpValueFactory::ToImplementation(dynamicNodeProperties)))
      {
        throw std::runtime_error("Could not set dynamic node properties.");
      }
    }
    if (options.DistributionMode.HasValue())
    {
#if ENABLE_UAMQP
      if (source_set_distribution_mode(source.get(), options.DistributionMode.Value().c_str()))
      {
        throw std::runtime_error("Could not set distribution mode.");
      }
#elif ENABLE_RUST_AMQP
      uint8_t mode = 0xff;
      if (options.DistributionMode.Value() == "Move")
      {
        mode = 0;
      }
      else if (options.DistributionMode.Value() == "Copy")
      {
        mode = 1;
      }
      if (source_set_distribution_mode(source.get(), mode))
      {
        throw std::runtime_error("Could not set distribution mode.");
      }
#endif
    }
    if (!options.Filter.empty())
    {
      if (source_set_filter(
              source.get(),
              _detail::AmqpValueFactory::ToImplementation(options.Filter.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set filter set.");
      }
    }
    if (!options.DefaultOutcome.IsNull())
    {
#if ENABLE_UAMQP
      if (source_set_default_outcome(
              source.get(), _detail::AmqpValueFactory::ToImplementation(options.DefaultOutcome)))
      {
        throw std::runtime_error("Could not set default outcome.");
      }
#elif ENABLE_RUST_AMQP
      if (options.DefaultOutcome.GetType() != AmqpValueType::Symbol)
      {
        throw std::runtime_error("Default outcome must be a Symbol.");
      }
      if (options.DefaultOutcome.AsSymbol() == "amqp:accepted:list")
      {
        if (source_set_default_outcome(source.get(), RustDeliveryOutcome::Accepted))
        {
          throw std::runtime_error("Could not set default outcome.");
        }
      }
      else if (options.DefaultOutcome.AsSymbol() == "amqp:rejected:list")
      {
        if (source_set_default_outcome(source.get(), RustDeliveryOutcome::Rejected))
        {
          throw std::runtime_error("Could not set default outcome.");
        }
      }
      else if (options.DefaultOutcome.AsSymbol() == "amqp:released:list")
      {
        if (source_set_default_outcome(source.get(), RustDeliveryOutcome::Released))
        {
          throw std::runtime_error("Could not set default outcome.");
        }
      }
      else if (options.DefaultOutcome.AsSymbol() == "amqp:modified:list")
      {
        if (source_set_default_outcome(source.get(), RustDeliveryOutcome::Modified))
        {
          throw std::runtime_error("Could not set default outcome.");
        }
      }
      else
      {
        throw std::runtime_error("Unknown default outcome.");
      }
#endif
    }
    if (!options.Outcomes.empty())
    {
      if (source_set_outcomes(
              source.get(),
              _detail::AmqpValueFactory::ToImplementation(options.Outcomes.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set outcomes.");
      }
    }
    if (!options.Capabilities.empty())
    {
      if (source_set_capabilities(
              source.get(),
              _detail::AmqpValueFactory::ToImplementation(options.Capabilities.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set capabilities.");
      }
    }
#if ENABLE_UAMQP
    m_source = std::move(source);
#elif ENABLE_RUST_AMQP
    Azure::Core::Amqp::_detail::AmqpSourceImplementation* sourceHandle;
    if (source_builder_build(source.get(), &sourceHandle))
    {
      throw std::runtime_error("Could not build source.");
    }
    m_source = UniqueAmqpMessageSource{sourceHandle};
#endif
  }

  // Convert the MessageSource into a Value.
  Models::AmqpValue MessageSourceImpl::AsAmqpValue() const
  {
#if ENABLE_UAMQP
    Models::_detail::UniqueAmqpValueHandle sourceValue{amqpvalue_create_source(m_source.get())};
#elif ENABLE_RUST_AMQP
    Azure::Core::Amqp::_detail::AmqpValueImplementation* source;
    if (amqpvalue_create_source(m_source.get(), &source))
    {
      throw std::runtime_error("Could not build source.");
    }
    Models::_detail::UniqueAmqpValueHandle sourceValue{source};
#endif
    return _detail::AmqpValueFactory::FromImplementation(sourceValue);
  }

  Models::AmqpValue MessageSourceImpl::GetAddress() const
  {
    Azure::Core::Amqp::_detail::AmqpValueImplementation* address;
    if (source_get_address(m_source.get(), &address))
    {
      throw std::runtime_error("Could not retrieve address from source.");
    }
    // source_get_address does not reference its value, so we need to reference it before
    // creating an AmqpValueHandle.
    return _detail::AmqpValueFactory::FromImplementation(
        Models::_detail::UniqueAmqpValueHandle{amqpvalue_clone(address)});
  }

  _internal::TerminusDurability MessageSourceImpl::GetTerminusDurability() const
  {
#if ENABLE_UAMQP
    terminus_durability value;
#elif ENABLE_RUST_AMQP
    RustTerminusDurability value;
#endif
    if (source_get_durable(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get durable from source.");
    }
#if ENABLE_UAMQP
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
#elif ENABLE_RUST_AMQP
    switch (value)
    {
      case RustTerminusDurability::None:
        return _internal::TerminusDurability::None;
      case RustTerminusDurability::Configuration:
        return _internal::TerminusDurability::Configuration;
      case RustTerminusDurability::UnsettledState:
        return _internal::TerminusDurability::UnsettledState;
      default:
        throw std::logic_error("Unknown terminus durability.");
    }
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
#elif ENABLE_RUST_AMQP
    RustExpiryPolicy expiryPolicy;
    if (source_get_expiry_policy(m_source.get(), &expiryPolicy))
    {
      throw std::runtime_error("Could not get expiry policy from source.");
    }

    switch (expiryPolicy)
    {
      case RustExpiryPolicy::ConnectionClose:
        return _internal::TerminusExpiryPolicy::ConnectionClose;
      case RustExpiryPolicy::LinkDetach:
        return _internal::TerminusExpiryPolicy::LinkDetach;
      case RustExpiryPolicy::Never:
        return _internal::TerminusExpiryPolicy::Never;
      case RustExpiryPolicy::SessionEnd:
        return _internal::TerminusExpiryPolicy::SessionEnd;
      default:
        throw std::logic_error("Unknown terminus expiry policy.");
    }
#endif
  }

  std::chrono::system_clock::time_point MessageSourceImpl::GetTimeout() const
  {
#if ENABLE_UAMQP
    seconds value;
#elif ENABLE_RUST_AMQP
    uint32_t value;
#endif
    if (source_get_timeout(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get timeout from source.");
    }
    return std::chrono::system_clock::from_time_t(value);
  }

  bool MessageSourceImpl::GetDynamic() const
  {
    bool value = {};
    if (source_get_dynamic(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
    return value;
  }

  AmqpMap MessageSourceImpl::GetDynamicNodeProperties() const
  {
    Azure::Core::Amqp::_detail::AmqpValueImplementation* value;
    if (source_get_dynamic_node_properties(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
#if ENABLE_UAMQP
    return _detail::AmqpValueFactory::FromImplementation(
               _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)})
#elif ENABLE_RUST_AMQP
    return _detail::AmqpValueFactory::FromImplementation(_detail::UniqueAmqpValueHandle{value})
#endif
        .AsMap();
  }

  std::string MessageSourceImpl::GetDistributionMode() const
  {
#if ENABLE_UAMQP
    const char* value = {};
    if (source_get_distribution_mode(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get distribution mode.");
    }
    return value;
#elif ENABLE_RUST_AMQP
    int8_t value;
    if (source_get_distribution_mode(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get distribution mode.");
    }
    if (value == 0)
    {
      return "Move";
    }
    else if (value == 1)
    {
      return "Copy";
    }
    else
    {
      return "Unknown";
    }
#endif
  }

  AmqpMap MessageSourceImpl::GetFilter() const
  {
    Azure::Core::Amqp::_detail::AmqpValueImplementation* value;
    if (source_get_filter(m_source.get(), &value))
    {
      throw std::runtime_error("Could not get filter set.");
    }
#if ENABLE_UAMQP
    return _detail::AmqpValueFactory::FromImplementation(
               _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)})
#elif ENABLE_RUST_AMQP
    return _detail::AmqpValueFactory::FromImplementation(_detail::UniqueAmqpValueHandle{value})
#endif
        .AsMap();
  }

  AmqpValue MessageSourceImpl::GetDefaultOutcome() const
  {
// source_get_default_outcome does not reference the value returned, we reference it so it can
// be put into a UniqueAmqpValueHandle.
#if ENABLE_UAMQP
          Azure::Core::Amqp::_detail::AmqpValueImplementation* value;
          if (source_get_default_outcome(m_source.get(), &value))
          {
            throw std::runtime_error("Could not get default outcome.");
          }
          return _detail::AmqpValueFactory::FromImplementation(
              _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)});
#elif ENABLE_RUST_AMQP
          RustDeliveryOutcome value;
          if (source_get_default_outcome(m_source.get(), &value))
          {
            throw std::runtime_error("Could not get default outcome.");
          }
          return AmqpValue{};
//          return _detail::AmqpValueFactory::FromImplementation(
//              _detail::UniqueAmqpValueHandle{value});
#endif
        }

        AmqpArray MessageSourceImpl::GetOutcomes() const
        {
          Azure::Core::Amqp::_detail::AmqpValueImplementation* value;
          if (source_get_outcomes(m_source.get(), &value))
          {
            throw std::runtime_error("Could not get outcomes.");
          }
          return _detail::AmqpValueFactory::FromImplementation(
                     _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)})
              .AsArray();
        }

        AmqpArray MessageSourceImpl::GetCapabilities() const
        {
          Azure::Core::Amqp::_detail::AmqpValueImplementation* value;
          if (source_get_capabilities(m_source.get(), &value))
          {
            throw std::runtime_error("Could not get capabilities.");
          }
#if ENABLE_UAMQP
          return _detail::AmqpValueFactory::FromImplementation(
                     _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)})
#elif ENABLE_RUST_AMQP
          return _detail::AmqpValueFactory::FromImplementation(
                     _detail::UniqueAmqpValueHandle{value})
#endif
              .AsArray();
        }
        // const char* StringFromTerminusDurability(_internal::TerminusDurability durability)
        //{
        //   switch (durability)
        //   {
        //     case _internal::TerminusDurability::None:
        //       return "None";
        //     case _internal::TerminusDurability::Configuration:
        //       return "Configuration";
        //     case _internal::TerminusDurability::UnsettledState:
        //       return "Unsettled State";
        //   }
        //   throw std::runtime_error("Unknown terminus durability");
        // }

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
              // source_get_default_outcome does not reference the value returned, we reference it
              // so it can be put into a UniqueAmqpValueHandle.
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
              // composite value at location 9 is a symbol, it does reference it. As a consequence,
              // this will leak an AMQPSymbol if the value at location 9 is a symbol (as opposed to
              // being an array).
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
