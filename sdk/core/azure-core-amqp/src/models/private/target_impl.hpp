// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/internal/models/message_target.hpp"
#if ENABLE_UAMQP
#include <azure_uamqp_c/amqp_definitions_fields.h>

#include <azure_uamqp_c/amqp_definitions_terminus_durability.h>
#include <azure_uamqp_c/amqp_definitions_terminus_expiry_policy.h>

#include <azure_uamqp_c/amqp_definitions_filter_set.h>
#include <azure_uamqp_c/amqp_definitions_node_properties.h>
#include <azure_uamqp_c/amqp_definitions_seconds.h>
#include <azure_uamqp_c/amqp_definitions_target.h>
#elif ENABLE_RUST_AMQP
#include "rust_amqp_wrapper.h"
#endif

#include <string>
#include <type_traits>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  using AmqpTargetImplementation = std::remove_pointer<TARGET_HANDLE>::type;
#elif ENABLE_RUST_AMQP
  using AmqpTargetImplementation = Azure::Core::Amqp::_detail::RustInterop::RustAmqpTarget;
#endif

  template <> struct UniqueHandleHelper<AmqpTargetImplementation>
  {
    static void FreeMessageTarget(AmqpTargetImplementation* obj);

    using type = Core::_internal::BasicUniqueHandle<AmqpTargetImplementation, FreeMessageTarget>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
  using UniqueMessageTargetHandle
      = Amqp::_detail::UniqueHandle<Azure::Core::Amqp::_detail::AmqpTargetImplementation>;

  class MessageTargetImpl final {
  public:
    /** @brief Creates a default message target.
     */
    MessageTargetImpl();
    /** @brief Deletes a message target. */
    ~MessageTargetImpl() = default;

    /** @brief Copies a MessageTargetImpl */
    MessageTargetImpl(MessageTargetImpl const& that);

    /** @brief Copy assignment operator */
    MessageTargetImpl& operator=(MessageTargetImpl const& that);

    /** @brief Moves a MessageTargetImpl */
    MessageTargetImpl(MessageTargetImpl&&) noexcept;

    /** @brief Moves assignment operator */
    MessageTargetImpl& operator=(MessageTargetImpl&&) noexcept;

    /** @brief Creates a message target with the given address.
     *
     * @param address The address of the target.
     */
    MessageTargetImpl(std::string const& address);

    /** @brief Creates a message target with the given address.
     *
     * @param address The address of the target.
     */
    MessageTargetImpl(char const* address);

    /** @brief Creates a message target with detailed options.
     *
     * @param options Options used constructing the message target.
     */
    MessageTargetImpl(_internal::MessageTargetOptions const& options);

    /** @brief Creates a message target from an AMQP value.
     *
     * @param value The AMQP value to create the message target from.
     *
     * @remarks Normally used in the OnLinkAttached callback.
     */
    MessageTargetImpl(Models::AmqpValue const& value);

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
    _internal::TerminusDurability GetTerminusDurability() const;

    /** @brief The expiry policy of the target.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-target)
     * for more information about the fields in a message target.
     *
     */
    _internal::TerminusExpiryPolicy GetExpiryPolicy() const;

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
    friend class AmqpTargetFactory;

    _detail::UniqueMessageTargetHandle m_target;

    operator Azure::Core::Amqp::_detail::AmqpTargetImplementation*() const
    {
      return m_target.get();
    }

    // Declared as friend so it can use the TARGET_INSTANCE_TAG* overload.
    friend std::ostream& operator<<(std::ostream&, MessageTargetImpl const&);
  };
  class AmqpTargetFactory final {
  public:
    // Returns the internal AMQP value handle, without referencing it.
    static Azure::Core::Amqp::_detail::AmqpTargetImplementation* ToImplementation(
        Azure::Core::Amqp::Models::_internal::MessageTarget const& value);
  };

}}}}} // namespace Azure::Core::Amqp::Models::_detail
