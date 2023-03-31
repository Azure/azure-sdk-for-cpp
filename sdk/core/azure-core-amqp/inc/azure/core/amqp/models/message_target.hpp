// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_header.hpp"
#include "amqp_properties.hpp"
#include "amqp_value.hpp"

struct TARGET_INSTANCE_TAG;
namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  class MessageTarget {
  public:
    MessageTarget();
    ~MessageTarget();

    MessageTarget(MessageTarget const&);
    MessageTarget& operator=(MessageTarget const&);
    MessageTarget(MessageTarget&&) noexcept;
    MessageTarget& operator=(MessageTarget&&) noexcept;

    MessageTarget(TARGET_INSTANCE_TAG* message);
    MessageTarget(std::string const& value);
    MessageTarget(char const* value);

    MessageTarget(Azure::Core::Amqp::Models::AmqpValue const& value);
    operator const Azure::Core::Amqp::Models::AmqpValue() const;

    operator TARGET_INSTANCE_TAG*() const { return m_target; }

    Azure::Core::Amqp::Models::AmqpValue GetAddress() const;
    void SetAddress(Azure::Core::Amqp::Models::AmqpValue const& address);

    Azure::Core::Amqp::Models::TerminusDurability GetTerminusDurability() const;
    void SetTerminusDurability(Azure::Core::Amqp::Models::TerminusDurability terminusDurability);

    Azure::Core::Amqp::Models::TerminusExpiryPolicy GetExpiryPolicy() const;
    void SetExpiryPolicy(Azure::Core::Amqp::Models::TerminusExpiryPolicy expiryPolicy);

    std::chrono::system_clock::time_point GetTimeout() const;
    void SetTimeout(std::chrono::system_clock::time_point const& timeout);

    bool GetDynamic() const;
    void SetDynamic(bool dynamic);

    Azure::Core::Amqp::Models::AmqpValue GetDynamicNodeProperties() const;
    void SetDynamicNodeProperties(Azure::Core::Amqp::Models::AmqpValue const& dynamicNodeProperties);

    Azure::Core::Amqp::Models::AmqpValue GetCapabilities() const;
    void SetCapabilities(Azure::Core::Amqp::Models::AmqpValue const& capabilities);

    friend std::ostream& operator<<(std::ostream&, MessageTarget const&);

  private:
    TARGET_INSTANCE_TAG* m_target;
  };
}}}}} // namespace Azure::Core::Amqp::Models::_internal
