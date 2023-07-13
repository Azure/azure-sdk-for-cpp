// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "amqp_header.hpp"
#include "amqp_properties.hpp"
#include "amqp_value.hpp"

struct TARGET_INSTANCE_TAG;

namespace Azure { namespace Core { namespace _internal {
  template <> struct UniqueHandleHelper<TARGET_INSTANCE_TAG>
  {
    static void FreeMessageTarget(TARGET_INSTANCE_TAG* obj);

    using type = BasicUniqueHandle<TARGET_INSTANCE_TAG, FreeMessageTarget>;
  };
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  using UniqueMessageTargetHandle = Azure::Core::_internal::UniqueHandle<TARGET_INSTANCE_TAG>;

  struct MessageTargetOptions final
  {
    AmqpValue Address;
    Nullable<TerminusDurability> TerminusDurabilityValue;
    Nullable<TerminusExpiryPolicy> TerminusExpiryPolicyValue;
    Nullable<std::chrono::system_clock::time_point> Timeout;
    Nullable<bool> Dynamic;
    AmqpMap DynamicNodeProperties;
    AmqpArray Capabilities;
  };

  class MessageTarget final {
  public:
    /** @brief Creates a default message target.
     */
    MessageTarget();
    /** @brief Deletes a message target. */
    ~MessageTarget() = default;

    /** @brief Copies a MessageTarget */
    MessageTarget(MessageTarget const& that);

    /** @brief Creates a message target with the given address.
     *
     * @param address The address of the target.
     */
    MessageTarget(std::string const& address);

    /** @brief Creates a message target with the given address.
     *
     * @param address The address of the target.
     */
    MessageTarget(char const* address);

    /** @brief Creates a message target with detailed options.
     *
     * @param options Options used constructing the message target.
     */
    MessageTarget(MessageTargetOptions const& options);

    /** @brief Creates a message target from an AMQP value.
     *
     * @param value The AMQP value to create the message target from.
     *
     * @remarks Normally used in the OnLinkAttached callback.
     */
    MessageTarget(Models::AmqpValue const& value);

    MessageTarget& operator=(MessageTarget const& that);

    /** @brief Creates an AMQP value from a message target.
     *
     * @remarks Creates an AMQP Described value with the descriptor being the message target
     * (0x29).
     */
    AmqpValue AsAmqpValue() const;

    /** @brief The address of the target.
     *
     * @returns The address of the target.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-target)
     * for more information about the fields in a message target.
     *
     */
    AmqpValue GetAddress() const;

    /** @brief The durability of the target.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-target)
     * for more information about the fields in a message target.
     *
     */
    TerminusDurability GetTerminusDurability() const;

    /** @brief The expiry policy of the target.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-target)
     * for more information about the fields in a message target.
     *
     */
    TerminusExpiryPolicy GetExpiryPolicy() const;

    /** @brief Duration that an expiring target will be retained.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-target)
     * for more information about the fields in a message target.
     *
     */
    std::chrono::system_clock::time_point GetTimeout() const;

    /** @brief Does the target request that the remote node be dynamically created?
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-target)
     * for more information about the fields in a message target.
     *
     */
    bool GetDynamic() const;

    /** @brief Retrieve the dynamic node properties on this message target.
     *
     * @remarks See
     * http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-node-properties
     * for more information.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-target)
     * for more information about the fields in a message target.
     *
     */
    AmqpMap GetDynamicNodeProperties() const;

    /** @brief Retrieve the capabilities on this message target.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-target)
     * for more information about the fields in a message target.
     *
     */
    AmqpArray GetCapabilities() const;

  private:
    UniqueMessageTargetHandle m_target;

    operator TARGET_INSTANCE_TAG*() const { return m_target.get(); }

    // Declared as friend so it can use the TARGET_INSTANCE_TAG* overload.
    friend std::ostream& operator<<(std::ostream&, MessageTarget const&);
  };

}}}}} // namespace Azure::Core::Amqp::Models::_internal
