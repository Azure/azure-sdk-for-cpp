// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/models/amqp_header.hpp"
#include "azure/core/amqp/models/amqp_properties.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
  class MessageTargetImpl;
  class AmqpTargetFactory;
}}}}} // namespace Azure::Core::Amqp::Models::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

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

  /**
   * @brief Represents an AMQP message target.
   *
   * The MessageTarget class provides methods to create and manipulate an AMQP message target.
   * A message target is the intended destination of an AMQP message.
   *
   * @remarks See
   * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-target)
   * for more information about the fields in a message target.
   */
  class MessageTarget final {
  public:
    /** @brief Creates a default message target.
     */
    MessageTarget();
    /** @brief Deletes a message target. */
    ~MessageTarget();

    /** @brief Copies a MessageTarget */
    MessageTarget(MessageTarget const& that);

    /** @brief Copy assignment operator */
    MessageTarget& operator=(MessageTarget const& that);

    /** @brief Moves a MessageTarget */
    MessageTarget(MessageTarget&&) noexcept;

    /** @brief Moves assignment operator */
    MessageTarget& operator=(MessageTarget&&) noexcept;

    /* Note: These constructors should NOT be marked as explicit, because we want to enable the
     * implicit construction of the MessageTarget from a string - this allows callers to construct
     * Link, MessageSender, and MessageReceiver objects without forcing the creation of a
     * MessageSource object. */

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
    std::unique_ptr<_detail::MessageTargetImpl> m_impl;

    // Declared as friend so it can use the TARGET_INSTANCE_TAG* overload.
    friend std::ostream& operator<<(std::ostream&, MessageTarget const&);

    friend Models::_detail::AmqpTargetFactory;
  };

}}}}} // namespace Azure::Core::Amqp::Models::_internal
