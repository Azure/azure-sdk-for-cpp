// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/models/message_target.hpp"

#include "private/target_impl.hpp"
#include "private/value_impl.hpp"

#include <azure/core/internal/diagnostics/log.hpp>

#if ENABLE_UAMQP
#include <azure_uamqp_c/amqp_definitions_fields.h>

#include <azure_uamqp_c/amqp_definitions_terminus_durability.h>
#include <azure_uamqp_c/amqp_definitions_terminus_expiry_policy.h>

#include <azure_uamqp_c/amqp_definitions_filter_set.h>
#include <azure_uamqp_c/amqp_definitions_node_properties.h>
#include <azure_uamqp_c/amqp_definitions_seconds.h>
#include <azure_uamqp_c/amqp_definitions_target.h>
#include <azure_uamqp_c/amqpvalue.h>
#elif ENABLE_RUST_AMQP
using namespace Azure::Core::Amqp::_detail::RustInterop;
#endif // ENABLE_UAMQP

#include <iostream>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  // @cond
  void UniqueHandleHelper<AmqpTargetImplementation>::FreeMessageTarget(
      AmqpTargetImplementation* value)
  {
    target_destroy(value);
  }
  // @endcond

#if ENABLE_RUST_AMQP
  template <> struct UniqueHandleHelper<RustInterop::RustAmqpTargetBuilder>
  {
    static void FreeMessageTargetBuilder(RustInterop::RustAmqpTargetBuilder* obj);

    using type = Core::_internal::
        BasicUniqueHandle<RustInterop::RustAmqpTargetBuilder, FreeMessageTargetBuilder>;
  };

  void UniqueHandleHelper<RustInterop::RustAmqpTargetBuilder>::FreeMessageTargetBuilder(
      RustInterop::RustAmqpTargetBuilder* obj)
  {
    target_builder_destroy(obj);
  }
#endif

}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  extern const char* StringFromTerminusDurability(TerminusDurability);

  MessageTarget::~MessageTarget() {}

  MessageTarget::MessageTarget(Models::AmqpValue const& target)
      : m_impl{std::make_unique<_detail::MessageTargetImpl>(target)}
  {
  }

  MessageTarget::MessageTarget(std::string const& address)
      : m_impl{std::make_unique<_detail::MessageTargetImpl>(address)}
  {
  }
  MessageTarget::MessageTarget(char const* address)
      : m_impl{std::make_unique<_detail::MessageTargetImpl>(address)}
  {
  }

  MessageTarget::MessageTarget() : m_impl{std::make_unique<_detail::MessageTargetImpl>()} {}

  MessageTarget::MessageTarget(MessageTarget const& that)
      : m_impl{std::make_unique<_detail::MessageTargetImpl>(*that.m_impl)}
  {
  }

  MessageTarget& MessageTarget::operator=(MessageTarget const& that)
  {
    m_impl = std::make_unique<_detail::MessageTargetImpl>(*that.m_impl);
    return *this;
  }

  MessageTarget::MessageTarget(MessageTarget&& that) noexcept : m_impl{std::move(that.m_impl)} {}

  MessageTarget& MessageTarget::operator=(MessageTarget&& that) noexcept
  {
    m_impl = std::move(that.m_impl);
    return *this;
  }

  MessageTarget::MessageTarget(MessageTargetOptions const& options)
      : m_impl{std::make_unique<_detail::MessageTargetImpl>(options)}
  {
  }

  // Convert the MessageSource into a Value.
  Models::AmqpValue MessageTarget::AsAmqpValue() const { return m_impl->AsAmqpValue(); }

  Models::AmqpValue MessageTarget::GetAddress() const { return m_impl->GetAddress(); }

  TerminusDurability MessageTarget::GetTerminusDurability() const
  {
    return m_impl->GetTerminusDurability();
  }

  TerminusExpiryPolicy MessageTarget::GetExpiryPolicy() const { return m_impl->GetExpiryPolicy(); }

  std::chrono::system_clock::time_point MessageTarget::GetTimeout() const
  {
    return m_impl->GetTimeout();
  }

  bool MessageTarget::GetDynamic() const { return m_impl->GetDynamic(); }

  Models::AmqpMap MessageTarget::GetDynamicNodeProperties() const
  {
    return m_impl->GetDynamicNodeProperties();
  }

  Models::AmqpArray MessageTarget::GetCapabilities() const { return m_impl->GetCapabilities(); }

  std::ostream& operator<<(std::ostream& os, MessageTarget const& target)
  {
    os << *target.m_impl;
    return os;
  }
}}}}} // namespace Azure::Core::Amqp::Models::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
#if ENABLE_RUST_AMQP
  using UniqueMessageTargetBuilderHandle = Azure::Core::Amqp::_detail::UniqueHandle<
      Azure::Core::Amqp::_detail::RustInterop::RustAmqpTargetBuilder>;
#endif

  MessageTargetImpl::MessageTargetImpl(Models::AmqpValue const& target)
  {
    if (target.IsNull())
    {
      throw std::invalid_argument("target cannot be null");
    }
    Azure::Core::Amqp::_detail::AmqpTargetImplementation* targetHandle;
    if (amqpvalue_get_target(_detail::AmqpValueFactory::ToImplementation(target), &targetHandle))
    {
      throw std::runtime_error("Could not retrieve target from value.");
    }
#if ENABLE_UAMQP
    m_target.reset(targetHandle);
#elif ENABLE_RUST_AMQP
    m_target.reset(targetHandle);
#endif
  }

  MessageTargetImpl::MessageTargetImpl(std::string const& address)
#if ENABLE_UAMQP
      : m_target
  {
    target_create()
  }
#endif
  {
#if ENABLE_UAMQP
    if (!m_target)
    {
      throw std::runtime_error("Could not create source.");
    }
    if (target_set_address(
            m_target.get(),
            _detail::UniqueAmqpValueHandle(amqpvalue_create_string(address.c_str())).get()))
    {
      throw std::runtime_error("Could not set address.");
    }
#elif ENABLE_RUST_AMQP
    UniqueMessageTargetBuilderHandle builder{target_builder_create()};
    if (!builder)
    {
      throw std::runtime_error("Could not create source.");
    }

    builder.reset(target_set_address(
        builder.release(),
        _detail::UniqueAmqpValueHandle(amqpvalue_create_string(address.c_str())).get()));
    if (!builder)
    {
      throw std::runtime_error("Could not set address.");
    }
    Azure::Core::Amqp::_detail::AmqpTargetImplementation* targetHandle;
    if (target_builder_build(builder.release(), &targetHandle) != 0)
    {
      throw std::runtime_error("Could not build target.");
    }
    m_target.reset(targetHandle);
#endif
  }

  MessageTargetImpl::MessageTargetImpl(char const* address)
  {
#if ENABLE_UAMQP
    UniqueMessageTargetHandle target{target_create()};

    if (!target)
    {
      throw std::runtime_error("Could not create source.");
    }

    if (target_set_address(
            target.get(), _detail::UniqueAmqpValueHandle(amqpvalue_create_string(address)).get()))
    {
      throw std::runtime_error("Could not set address.");
    }
    m_target = std::move(target);
#elif ENABLE_RUST_AMQP
    UniqueMessageTargetBuilderHandle builder{target_builder_create()};

    if (!builder)
    {
      throw std::runtime_error("Could not create source.");
    }

    builder.reset(target_set_address(
        builder.release(), _detail::UniqueAmqpValueHandle(amqpvalue_create_string(address)).get()));
    if (!builder)
    {
      throw std::runtime_error("Could not set address.");
    }
    Azure::Core::Amqp::_detail::AmqpTargetImplementation* targetHandle;
    if (target_builder_build(builder.release(), &targetHandle) != 0)
    {
      throw std::runtime_error("Could not build target.");
    }
    m_target.reset(targetHandle);
#endif
  }

  MessageTargetImpl::MessageTargetImpl()
#if ENABLE_UAMQP
      : m_target
  {
    target_create()
  }
#endif
  {
#if ENABLE_RUST_AMQP
    UniqueMessageTargetBuilderHandle target{target_builder_create()};
    if (!target)
    {
      throw std::runtime_error("Could not create source.");
    }
    Azure::Core::Amqp::_detail::AmqpTargetImplementation* targetHandle;
    target_builder_build(target.get(), &targetHandle);
    m_target.reset(targetHandle);

#endif
  }

  MessageTargetImpl::MessageTargetImpl(MessageTargetImpl const& that) : m_target{target_clone(that)}
  {
  }

  MessageTargetImpl& MessageTargetImpl::operator=(MessageTargetImpl const& that)
  {
    m_target.reset(target_clone(that));
    return *this;
  }

  MessageTargetImpl::MessageTargetImpl(MessageTargetImpl&& that) noexcept
      : m_target{std::move(that.m_target)}
  {
  }

  MessageTargetImpl& MessageTargetImpl::operator=(MessageTargetImpl&& that) noexcept
  {
    m_target = std::move(that.m_target);
    return *this;
  }

  MessageTargetImpl::MessageTargetImpl(_internal::MessageTargetOptions const& options)
  {
#if ENABLE_UAMQP
    UniqueMessageTargetHandle target{target_create()};
    if (!options.Address.IsNull())
    {
      if (target_set_address(
              target.get(), _detail::AmqpValueFactory::ToImplementation(options.Address)))
      {
        throw std::runtime_error("Could not set source address.");
      }
    }
    if (options.TerminusDurabilityValue.HasValue())
    {
#if ENABLE_UAMQP
      terminus_durability durability;
#elif ENABLE_RUST_AMQP
      RustTerminusDurability durability;
#endif
#if ENABLE_UAMQP
      switch (options.TerminusDurabilityValue.Value())
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
      switch (options.TerminusDurabilityValue.Value())
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
      if (target_set_durable(target.get(), durability))
      {
        throw std::runtime_error("Could not set durable.");
      }
    }
    if (options.TerminusExpiryPolicyValue.HasValue())
    {
#if ENABLE_UAMQP
      terminus_expiry_policy policy;
#elif ENABLE_RUST_AMQP
      RustExpiryPolicy policy;
#endif
#if ENABLE_UAMQP
      switch (options.TerminusExpiryPolicyValue.Value())
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
      switch (options.TerminusExpiryPolicyValue.Value())
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
      if (target_set_expiry_policy(target.get(), policy))
      {
        throw std::runtime_error("Could not set durable.");
      }
    }
    if (options.Timeout.HasValue())
    {
      if (target_set_timeout(
              target.get(),
              static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(
                                        options.Timeout.Value().time_since_epoch())
                                        .count())))
      {
        throw std::runtime_error("Could not set value.");
      }
    }
    if (options.Dynamic.HasValue())
    {
      if (target_set_dynamic(target.get(), *options.Dynamic))
      {
        throw std::runtime_error("Could not set dynamic.");
      }
    }

    if (!options.DynamicNodeProperties.empty())
    {
      if (target_set_dynamic_node_properties(
              target.get(),
              _detail::AmqpValueFactory::ToImplementation(
                  options.DynamicNodeProperties.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set dynamic node properties.");
      }
    }

    if (!options.Capabilities.empty())
    {
      if (target_set_capabilities(
              target.get(),
              _detail::AmqpValueFactory::ToImplementation(options.Capabilities.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set capabilities.");
      }
    }
    m_target = std::move(target);
#elif ENABLE_RUST_AMQP
    UniqueMessageTargetBuilderHandle builder{target_builder_create()};

    if (!options.Address.IsNull())
    {
      builder.reset(target_set_address(
          builder.release(), _detail::AmqpValueFactory::ToImplementation(options.Address)));
      if (!builder)
      {
        throw std::runtime_error("Could not set source address.");
      }
    }
    if (options.TerminusDurabilityValue.HasValue())
    {
      RustTerminusDurability durability;
      switch (options.TerminusDurabilityValue.Value())
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
      builder.reset(target_set_durable(builder.release(), durability));
      if (!builder)
      {
        throw std::runtime_error("Could not set durable.");
      }
    }
    if (options.TerminusExpiryPolicyValue.HasValue())
    {
      RustExpiryPolicy policy;
      switch (options.TerminusExpiryPolicyValue.Value())
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
      builder.reset(target_set_expiry_policy(builder.release(), policy));
      if (!builder)
      {
        throw std::runtime_error("Could not set durable.");
      }
    }
    if (options.Timeout.HasValue())
    {
      builder.reset(target_set_timeout(
          builder.release(),
          static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(
                                    options.Timeout.Value().time_since_epoch())
                                    .count())));
      if (!builder)
      {
        throw std::runtime_error("Could not set value.");
      }
    }
    if (options.Dynamic.HasValue())
    {
      builder.reset(target_set_dynamic(builder.release(), *options.Dynamic));
      if (!builder)
      {
        throw std::runtime_error("Could not set dynamic.");
      }
    }

    if (!options.DynamicNodeProperties.empty())
    {
      builder.reset(target_set_dynamic_node_properties(
          builder.release(),
          _detail::AmqpValueFactory::ToImplementation(
              options.DynamicNodeProperties.AsAmqpValue())));
      if (!builder)
      {
        throw std::runtime_error("Could not set dynamic node properties.");
      }
    }

    if (!options.Capabilities.empty())
    {
      builder.reset(target_set_capabilities(
          builder.release(),
          _detail::AmqpValueFactory::ToImplementation(options.Capabilities.AsAmqpValue())));
      if (!builder)
      {
        throw std::runtime_error("Could not set capabilities.");
      }
    }
    Azure::Core::Amqp::_detail::AmqpTargetImplementation* targetHandle;
    if (target_builder_build(builder.release(), &targetHandle) != 0)
    {
      throw std::runtime_error("Could not build target.");
    }
    m_target.reset(targetHandle);
#endif
  }

  // Convert the MessageSource into a Value.
  Models::AmqpValue MessageTargetImpl::AsAmqpValue() const
  {
    Models::_detail::UniqueAmqpValueHandle targetValue{amqpvalue_create_target(m_target.get())};
    return _detail::AmqpValueFactory::FromImplementation(targetValue);
  }

  Models::AmqpValue MessageTargetImpl::GetAddress() const
  {
    Azure::Core::Amqp::_detail::AmqpValueImplementation* address;
    if (target_get_address(m_target.get(), &address))
    {
      throw std::runtime_error("Could not retrieve address from target.");
    }
    // target_get_address does not reference the underlying address so we need to addref it here so
    // it gets freed properly.
#if ENABLE_UAMQP
    return _detail::AmqpValueFactory::FromImplementation(
        Models::_detail::UniqueAmqpValueHandle{amqpvalue_clone(address)});
#elif ENABLE_RUST_AMQP
    return _detail::AmqpValueFactory::FromImplementation(
        Models::_detail::UniqueAmqpValueHandle{address});
#endif
  }

  _internal::TerminusDurability MessageTargetImpl::GetTerminusDurability() const
  {
#if ENABLE_UAMQP
    terminus_durability value;
#elif ENABLE_RUST_AMQP
    RustTerminusDurability value;
#endif
    if (target_get_durable(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get durable from target.");
    }
    switch (value)
    {
#if ENABLE_UAMQP
      case terminus_durability_configuration:
        return _internal::TerminusDurability::Configuration;
      case terminus_durability_none:
        return _internal::TerminusDurability::None;
      case terminus_durability_unsettled_state:
        return _internal::TerminusDurability::UnsettledState;
#elif ENABLE_RUST_AMQP
      case RustTerminusDurability::Configuration:
        return _internal::TerminusDurability::Configuration;
      case RustTerminusDurability::None:
        return _internal::TerminusDurability::None;
      case RustTerminusDurability::UnsettledState:
        return _internal::TerminusDurability::UnsettledState;
#endif
      default:
        throw std::logic_error("Unknown terminus durability.");
    }
  }

  _internal::TerminusExpiryPolicy MessageTargetImpl::GetExpiryPolicy() const
  {
#if ENABLE_UAMQP
    terminus_expiry_policy value;
#elif ENABLE_RUST_AMQP
    RustExpiryPolicy value;
#endif

    if (target_get_expiry_policy(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get expiry policy from target.");
    }
#if ENABLE_UAMQP
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
    switch (value)
    {
      case RustExpiryPolicy::ConnectionClose:
        return _internal::TerminusExpiryPolicy::ConnectionClose;
      case RustExpiryPolicy::LinkDetach:
        return _internal::TerminusExpiryPolicy::LinkDetach;
      case RustExpiryPolicy::Never:
        return _internal::TerminusExpiryPolicy::Never;
      case RustExpiryPolicy::SessionEnd:
        return _internal::TerminusExpiryPolicy::SessionEnd;
    }
    throw std::logic_error(
        std::string("Unknown terminus expiry policy: ")
        + std::to_string(static_cast<std::underlying_type<RustExpiryPolicy>::type>(value)));
#endif
  }

  std::chrono::system_clock::time_point MessageTargetImpl::GetTimeout() const
  {
#if ENABLE_UAMQP
    seconds value;
#elif ENABLE_RUST_AMQP
    uint32_t value;
#endif
    if (target_get_timeout(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get timeout from source.");
    }
    return std::chrono::system_clock::from_time_t(value);
  }

  bool MessageTargetImpl::GetDynamic() const
  {
    bool value;
    if (target_get_dynamic(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
    return value;
  }

  Models::AmqpMap MessageTargetImpl::GetDynamicNodeProperties() const
  {
    Azure::Core::Amqp::_detail::AmqpValueImplementation* value;
    // Note: target_get_dynamic_node_properties does NOT reference the value.
    if (target_get_dynamic_node_properties(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
    // We clone the value before converting it to a UniqueAmqpValueHandle because the destructor for
    // UniqueAmqpValueHandle will remove the reference.
#if ENABLE_UAMQP
    return _detail::AmqpValueFactory::FromImplementation(
               _detail::UniqueAmqpValueHandle(amqpvalue_clone(value)))
#elif ENABLE_RUST_AMQP
    return _detail::AmqpValueFactory::FromImplementation(_detail::UniqueAmqpValueHandle(value))
#endif
        .AsMap();
  }

  Models::AmqpArray MessageTargetImpl::GetCapabilities() const
  {
    Azure::Core::Amqp::_detail::AmqpValueImplementation* value;
    if (target_get_capabilities(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get capabilities.");
    }
#if ENABLE_UAMQP
    return _detail::AmqpValueFactory::FromImplementation(
               _detail::UniqueAmqpValueHandle(amqpvalue_clone(value)))
#elif ENABLE_RUST_AMQP
    return _detail::AmqpValueFactory::FromImplementation(_detail::UniqueAmqpValueHandle(value))
#endif
        .AsArray();
  }

#if ENABLE_RUST_AMQP
  extern std::ostream& operator<<(std::ostream& os, RustExpiryPolicy value);
#endif

  std::ostream& operator<<(std::ostream& os, MessageTargetImpl const& target)
  {
    os << "Target{ ";
    {
      try
      {
        os << "Address: " << target.GetAddress();
      }
      catch (std::runtime_error const&)
      {
      }
    }
    {
#if ENABLE_UAMQP
      uint32_t value;
#elif ENABLE_RUST_AMQP
      RustTerminusDurability value;
#endif
      if (!target_get_durable(target, &value))
      {
        os << ", Durable: " << StringFromTerminusDurability(target.GetTerminusDurability());
      }
    }
    {
#if ENABLE_UAMQP
      terminus_expiry_policy policy;
#elif ENABLE_RUST_AMQP
      RustExpiryPolicy policy;
#endif
      if (!target_get_expiry_policy(target, &policy))
      {
        os << ", Expiry Policy: " << policy;
      }
    }

    {
#if ENABLE_UAMQP
      seconds timeout;
#elif ENABLE_RUST_AMQP
      uint32_t timeout;
#endif
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
      try
      {
        auto dynamicProperties = target.GetDynamicNodeProperties();
        os << ", Dynamic Node Properties: " << dynamicProperties;
      }
      catch (std::runtime_error&)
      {
      }
    }
    {
      try
      {
        auto capabilities = target.GetCapabilities();
        os << ", Capabilities: " << capabilities;
      }
      catch (std::runtime_error&)
      {
      }
    }
    os << "}";
    return os;
  }

  Azure::Core::Amqp::_detail::AmqpTargetImplementation* AmqpTargetFactory::ToImplementation(
      Models::_internal::MessageTarget const& target)
  {
    return target.m_impl->m_target.get();
  }

}}}}} // namespace Azure::Core::Amqp::Models::_detail
