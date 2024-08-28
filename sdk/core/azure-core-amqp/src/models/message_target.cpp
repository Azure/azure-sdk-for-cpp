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
#endif // ENABLE_UAMQP

#include <iostream>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  // @cond
  void UniqueHandleHelper<TARGET_INSTANCE_TAG>::FreeMessageTarget(TARGET_HANDLE value)
  {
    target_destroy(value);
  }
// @endcond
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
  MessageTargetImpl::MessageTargetImpl(Models::AmqpValue const& target)
  {
    if (target.IsNull())
    {
      throw std::invalid_argument("target cannot be null");
    }
#if ENABLE_UAMQP
    TARGET_HANDLE targetHandle;
    if (amqpvalue_get_target(_detail::AmqpValueFactory::ToImplementation(target), &targetHandle))
    {
      throw std::runtime_error("Could not retrieve target from value.");
    }
    m_target.reset(targetHandle);
#endif
  }

  MessageTargetImpl::MessageTargetImpl(std::string const& address)
#if ENABLE_UAMQP
      : m_target{target_create()}
#endif
  {
#if ENABLE_UAMQP
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
#else
    (void)address;
#endif
  }
  MessageTargetImpl::MessageTargetImpl(char const* address)
#if ENABLE_UAMQP
      : m_target{target_create()}
#endif
  {
#if ENABLE_UAMQP
    if (m_target == nullptr)
    {
      throw std::runtime_error("Could not create source.");
    }
    if (target_set_address(
            m_target.get(), _detail::UniqueAmqpValueHandle(amqpvalue_create_string(address)).get()))
    {
      throw std::runtime_error("Could not set address.");
    }
#else
    (void)address;
#endif
  }

  MessageTargetImpl::MessageTargetImpl()
#if ENABLE_UAMQP
      : m_target{target_create()}
#endif
  {
  }

  MessageTargetImpl::MessageTargetImpl(MessageTargetImpl const& that)
#if ENABLE_UAMQP
      : m_target{target_clone(that)}
#endif
  {
    (void)that;
  }

  MessageTargetImpl& MessageTargetImpl::operator=(MessageTargetImpl const& that)
  {
#if ENABLE_UAMQP
    m_target.reset(target_clone(that));
#else
    (void)that;
#endif
    return *this;
  }

  MessageTargetImpl::MessageTargetImpl(MessageTargetImpl&& that) noexcept
#if ENABLE_UAMQP
      : m_target
  {
    std::move(that.m_target)
  }
#endif
  {
    (void)that;
  }

  MessageTargetImpl& MessageTargetImpl::operator=(MessageTargetImpl&& that) noexcept
  {
#if ENABLE_UAMQP
    m_target = std::move(that.m_target);
#else
    (void)that;
#endif
    return *this;
  }

  MessageTargetImpl::MessageTargetImpl(_internal::MessageTargetOptions const& options)
#if ENABLE_UAMQP
      : m_target
  {
    target_create()
  }
#endif
  {
#if ENABLE_UAMQP
    if (!options.Address.IsNull())
    {
      if (target_set_address(
              m_target.get(), _detail::AmqpValueFactory::ToImplementation(options.Address)))
      {
        throw std::runtime_error("Could not set source address.");
      }
    }
    if (options.TerminusDurabilityValue.HasValue())
    {
      terminus_durability durability;
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
              _detail::AmqpValueFactory::ToImplementation(
                  options.DynamicNodeProperties.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set dynamic node properties.");
      }
    }

    if (!options.Capabilities.empty())
    {
      if (target_set_capabilities(
              m_target.get(),
              _detail::AmqpValueFactory::ToImplementation(options.Capabilities.AsAmqpValue())))
      {
        throw std::runtime_error("Could not set capabilities.");
      }
    }
#else
    (void)options;

#endif
  }

  // Convert the MessageSource into a Value.
  Models::AmqpValue MessageTargetImpl::AsAmqpValue() const
  {
#if ENABLE_UAMQP
    Models::_detail::UniqueAmqpValueHandle targetValue{amqpvalue_create_target(m_target.get())};
    return _detail::AmqpValueFactory::FromImplementation(targetValue);
#else
    return {};
#endif
  }

  Models::AmqpValue MessageTargetImpl::GetAddress() const
  {
#if ENABLE_UAMQP
    AMQP_VALUE address;
    if (target_get_address(m_target.get(), &address))
    {
      throw std::runtime_error("Could not retrieve address from source.");
    }
    // target_get_address does not reference the underlying address so we need to addref it here so
    // it gets freed properly.
    return _detail::AmqpValueFactory::FromImplementation(
        Models::_detail::UniqueAmqpValueHandle{amqpvalue_clone(address)});
#else
    return {};
#endif
  }

  _internal::TerminusDurability MessageTargetImpl::GetTerminusDurability() const
  {
#if ENABLE_UAMQP
    terminus_durability value;
    if (target_get_durable(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get durable from target.");
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

  _internal::TerminusExpiryPolicy MessageTargetImpl::GetExpiryPolicy() const
  {
#if ENABLE_UAMQP
    terminus_expiry_policy value;
    if (target_get_expiry_policy(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get expiry policy from target.");
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

  std::chrono::system_clock::time_point MessageTargetImpl::GetTimeout() const
  {
#if ENABLE_UAMQP
    seconds value;
    if (target_get_timeout(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get timeout from source.");
    }
    return std::chrono::system_clock::from_time_t(value);
#else
    return {};
#endif
  }

  bool MessageTargetImpl::GetDynamic() const
  {
    bool value = {};
#if ENABLE_UAMQP
    if (target_get_dynamic(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
#endif
    return value;
  }

  Models::AmqpMap MessageTargetImpl::GetDynamicNodeProperties() const
  {
#if ENABLE_UAMQP
    AMQP_VALUE value;
    // Note: target_get_dynamic_node_properties does NOT reference the value.
    if (target_get_dynamic_node_properties(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get dynamic.");
    }
    // We clone the value before converting it to a UniqueAmqpValueHandle because the destructor for
    // UniqueAmqpValueHandle will remove the reference.
    return _detail::AmqpValueFactory::FromImplementation(
               Models::_detail::UniqueAmqpValueHandle{amqpvalue_clone(value)})
        .AsMap();
#else
    return {};
#endif
  }

  Models::AmqpArray MessageTargetImpl::GetCapabilities() const
  {
#if ENABLE_UAMQP
    AMQP_VALUE value;
    if (target_get_capabilities(m_target.get(), &value))
    {
      throw std::runtime_error("Could not get capabilities.");
    }
    return _detail::AmqpValueFactory::FromImplementation(
               _detail::UniqueAmqpValueHandle(amqpvalue_clone(value)))
        .AsArray();
#else
    return {};
#endif
  }

  std::ostream& operator<<(std::ostream& os, MessageTargetImpl const& target)
  {
#if ENABLE_UAMQP
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
#else
    (void)target;
#endif
    os << "}";
    return os;
  }
}}}}} // namespace Azure::Core::Amqp::Models::_detail
