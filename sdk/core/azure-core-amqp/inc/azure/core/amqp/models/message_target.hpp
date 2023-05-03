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
using UniqueMessageTargetHandle = Azure::Core::_internal::UniqueHandle<TARGET_INSTANCE_TAG>;

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  struct MessageTargetOptions
  {
    AmqpValue Address;
    Azure::Nullable<TerminusDurability> TerminusDurabilityValue;
    Azure::Nullable<TerminusExpiryPolicy> TerminusExpiryPolicyValue;
    Azure::Nullable<std::chrono::system_clock::time_point> Timeout;
    Azure::Nullable<bool> Dynamic;
    AmqpMap DynamicNodeProperties;
    AmqpArray Capabilities;
  };

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
    MessageTarget(MessageTargetOptions const& options);

    // Construct a Message Target from an AMQP value.
    MessageTarget(Azure::Core::Amqp::Models::AmqpValue const& value);

    // Represent the Message Target as an AMQP value.
    operator const Azure::Core::Amqp::Models::AmqpValue() const;

    /** @brief The address of the target.
     *
     * @returns The address of the target.
     */
    Azure::Core::Amqp::Models::AmqpValue GetAddress() const;

    /** @brief The durability of the target. */
    TerminusDurability GetTerminusDurability() const;

    /** @brief The expiry policy of the target. */
    TerminusExpiryPolicy GetExpiryPolicy() const;

    /** @brief Duration that an expiring target will be retained. */
    std::chrono::system_clock::time_point GetTimeout() const;

    /** @brief Does the target request that the remote node be dynamically created? */
    bool GetDynamic() const;

    /** @brief Retrieve the dynamic node properties on this message target.
     * @remarks See
     * http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-node-properties
     * for more information.
     */
    Azure::Core::Amqp::Models::AmqpMap GetDynamicNodeProperties() const;

    /** @brief Retrieve the capabilities on this message target. */
    Azure::Core::Amqp::Models::AmqpArray GetCapabilities() const;

  private:
    UniqueMessageTargetHandle m_target;

    operator TARGET_INSTANCE_TAG*() const { return m_target.get(); }

    // Declared as friend so it can use the TARGET_INSTANCE_TAG* overload.
    friend std::ostream& operator<<(std::ostream&, MessageTarget const&);
  };

}}}}} // namespace Azure::Core::Amqp::Models::_internal
