// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_header.hpp"
#include "amqp_properties.hpp"
#include "amqp_value.hpp"

struct TARGET_INSTANCE_TAG;

template <> struct Azure::Core::_internal::UniqueHandleHelper<TARGET_INSTANCE_TAG>
{
  static void FreeMessageTarget(TARGET_INSTANCE_TAG* obj);

  using type = Azure::Core::_internal::BasicUniqueHandle<TARGET_INSTANCE_TAG, FreeMessageTarget>;
};

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  class MessageTarget final {
  public:
    MessageTarget();
    ~MessageTarget() = default;

    MessageTarget(MessageTarget const&);
    MessageTarget& operator=(MessageTarget const&);
    MessageTarget(MessageTarget&&) noexcept;
    MessageTarget& operator=(MessageTarget&&) noexcept;

    MessageTarget(std::string const& value);
    MessageTarget(char const* value);

    MessageTarget(Azure::Core::Amqp::Models::AmqpValue const& value);
    operator const Azure::Core::Amqp::Models::AmqpValue() const;

    Azure::Core::Amqp::Models::AmqpValue GetAddress() const;
    void SetAddress(Azure::Core::Amqp::Models::AmqpValue const& address);

    TerminusDurability GetTerminusDurability() const;
    void SetTerminusDurability(TerminusDurability terminusDurability);

    TerminusExpiryPolicy GetExpiryPolicy() const;
    void SetExpiryPolicy(TerminusExpiryPolicy expiryPolicy);

    std::chrono::system_clock::time_point GetTimeout() const;
    void SetTimeout(std::chrono::system_clock::time_point const& timeout);

    bool GetDynamic() const;
    void SetDynamic(bool dynamic);

    Azure::Core::Amqp::Models::AmqpValue GetDynamicNodeProperties() const;
    void SetDynamicNodeProperties(
        Azure::Core::Amqp::Models::AmqpValue const& dynamicNodeProperties);

    /** @brief Retrieve the capabilities on this message target. */
    Azure::Core::Amqp::Models::AmqpArray GetCapabilities() const;
    /**
     * @brief Set the capabilities supported by this message target.
     */
    void SetCapabilities(Azure::Core::Amqp::Models::AmqpArray const& capabilities);
    /**
     * @brief Set a single capability for this message target.
     */
    void SetCapabilities(Azure::Core::Amqp::Models::AmqpValue const& capabilities);

    // uAMQP interoperability functions, do not use.
  public:
    MessageTarget(TARGET_INSTANCE_TAG* message);
    operator TARGET_INSTANCE_TAG*() const { return m_target.get(); }

  private:
    Azure::Core::_internal::UniqueHandle<TARGET_INSTANCE_TAG> m_target;
  };
  std::ostream& operator<<(std::ostream&, MessageTarget const&);

}}}}} // namespace Azure::Core::Amqp::Models::_internal
