// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_header.hpp"
#include "amqp_properties.hpp"
#include "amqp_value.hpp"

struct TARGET_INSTANCE_TAG;
namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  // enum class MessageBodyType
  //{
  //   None,
  //   Data,
  //   Sequence,
  //   Value,
  // };

  // enum class TerminusDurability : uint32_t
  //{
  //   None = 0,
  //   Configuration = 1,
  //   UnsettledState = 2
  // };

  //// Note: Should be an extendable Enumeration.
  // enum class TerminusExpiryPolicy
  //{
  //   LinkDetach,
  //   SessionEnd,
  //   ConnectionClose,
  //   Never
  // };

  class MessageTarget {
  public:
    MessageTarget();
    ~MessageTarget();

    MessageTarget(MessageTarget const&);
    MessageTarget& operator=(MessageTarget const&);
    MessageTarget(MessageTarget&&) noexcept;
    MessageTarget& operator=(MessageTarget&&) noexcept;

    MessageTarget(TARGET_INSTANCE_TAG* message);
    // Create a described source from an AMQP Value - used in the OnLinkAttached.
    MessageTarget(Azure::Core::Amqp::Models::Value const& value);

    MessageTarget(std::string const& value);

    operator TARGET_INSTANCE_TAG*() const { return m_target; }
    operator Azure::Core::Amqp::Models::Value() const;

    Azure::Core::Amqp::Models::Value GetAddress() const;
    void SetAddress(Azure::Core::Amqp::Models::Value const& address);

    Azure::Core::Amqp::Models::TerminusDurability GetTerminusDurability() const;
    void SetTerminusDurability(Azure::Core::Amqp::Models::TerminusDurability terminusDurability);

    Azure::Core::Amqp::Models::TerminusExpiryPolicy GetExpiryPolicy() const;
    void SetExpiryPolicy(Azure::Core::Amqp::Models::TerminusExpiryPolicy expiryPolicy);

    std::chrono::system_clock::time_point GetTimeout() const;
    void SetTimeout(std::chrono::system_clock::time_point const& timeout);

    bool GetDynamic() const;
    void SetDynamic(bool dynamic);

    Azure::Core::Amqp::Models::Value GetDynamicNodeProperties() const;
    void SetDynamicNodeProperties(Azure::Core::Amqp::Models::Value const& dynamicNodeProperties);

    Azure::Core::Amqp::Models::Value GetCapabilities() const;
    void SetCapabilities(Azure::Core::Amqp::Models::Value const& capabilities);

    friend std::ostream& operator<<(std::ostream&, MessageTarget const&);

  private:
    TARGET_INSTANCE_TAG* m_target;
  };
}}}}} // namespace Azure::Core::Amqp::Models::_internal
