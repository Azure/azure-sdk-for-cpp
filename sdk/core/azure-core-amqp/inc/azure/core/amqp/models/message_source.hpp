// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_header.hpp"
#include "amqp_properties.hpp"
#include "amqp_value.hpp"

struct SOURCE_INSTANCE_TAG;

template <> struct Azure::Core::_internal::UniqueHandleHelper<SOURCE_INSTANCE_TAG>
{
  static void FreeMessageSource(SOURCE_INSTANCE_TAG* obj);

  using type = Azure::Core::_internal::BasicUniqueHandle<SOURCE_INSTANCE_TAG, FreeMessageSource>;
};

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  struct MessageSourceOptions final
  {
    AmqpValue Address;
    Nullable<TerminusDurability> SourceTerminusDurability;
    Nullable<TerminusExpiryPolicy> SourceTerminusExpiryPolicy;
    Nullable<std::chrono::system_clock::time_point> Timeout;
    Nullable<bool> Dynamic;
    AmqpMap DynamicNodeProperties;
    Nullable<std::string> DistributionMode;
    AmqpMap Filter;
    AmqpValue DefaultOutcome;
    AmqpArray Outcomes;
    AmqpArray Capabilities;
  };

  class MessageSource final {
  public:
    /** @brief Creates a default message target.
     */
    MessageSource();
    /** @brief Deletes a message target. */
    ~MessageSource() = default;

    // Create a described source from an AMQP Value - used in the OnLinkAttached.
    MessageSource(AmqpValue const& value);

    /** @brief Copies a MessageSource */
    MessageSource(MessageSource const& that);

    /** Assigns a message source from another.
     */
    MessageSource& operator=(MessageSource const& that);

    /** @brief Creates a message source with detailed options.
     *
     * @param options Options used constructing the message source.
     */

    MessageSource(MessageSourceOptions const& options);

    /** @brief Creates a message source with the given address.
     *
     * @param address The address of the source.
     */
    MessageSource(std::string const& address);

    /** @brief Creates a message source with the given address.
     *
     * @param address The address of the source.
     */
    MessageSource(char const* address);

    /** @brief Creates an AMQP value from a message source.
     *
     * @remarks Creates an AMQP Described value with the descriptor being the message source (0x29).
     */
    AmqpValue AsAmqpValue() const;

    /** @brief Gets the address of the source.
     *
     * @return The address of the source.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-source)
     * for more information about the fields in a message source.
     */
    AmqpValue GetAddress() const;

    /** @brief Gets the durability of the source.
     *
     * @return The durability of the source.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-source)
     * for more information about the fields in a message source.
     */
    TerminusDurability GetTerminusDurability() const;

    /** @brief Gets the expiry policy of the source.
     *
     * @return The expiry policy of the source.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-source)
     * for more information about the fields in a message source.
     */
    TerminusExpiryPolicy GetExpiryPolicy() const;

    /** @brief Duration that an expiring source will be retained.
     *
     * @return The timeout of the source.
     *
     * @remarks The source starts expiring as indicated by the expiry-policy.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-source)
     * for more information about the fields in a message source.
     */
    std::chrono::system_clock::time_point GetTimeout() const;

    /** @brief Requests dynamic creation of a remote node.
     *
     * @return Whether the source is dynamic.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-source)
     * for more information about the fields in a message source.
     */
    bool GetDynamic() const;

    /** @brief Retrieve the dynamic node properties on this message source.
     * @remarks See
     * http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-node-properties
     * for more information.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-source)
     * for more information about the fields in a message source.
     */
    AmqpMap GetDynamicNodeProperties() const;

    /** @brief Gets the distribution mode of the source.
     *
     * @return The distribution mode of the source.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-source)
     * for more information about the fields in a message source.
     */
    std::string GetDistributionMode() const;

    /** @brief Gets the filter of the source.
     *
     * @return The filter of the source.
     *
     * @remarks See [filter
     * set](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-filter-set)
     * for more information.
     */
    AmqpMap GetFilter() const;

    /** @brief Gets the default outcome of the source.
     *
     * @return The default outcome of the source.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-source)
     * for more information about the fields in a message source.
     *
     */
    AmqpValue GetDefaultOutcome() const;

    /** @brief Gets the outcomes of the source.
     *
     * @return The outcomes of the source.
     *
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-source)
     * for more information about the fields in a message source.
     */
    AmqpArray GetOutcomes() const;

    /** @brief Gets the capabilities of the source.
     *
     * @return The capabilities of the source.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-source)
     * for more information about the fields in a message source.
     *
     */
    AmqpArray GetCapabilities() const;

  private:
    operator SOURCE_INSTANCE_TAG*() const { return m_source.get(); }
    Azure::Core::_internal::UniqueHandle<SOURCE_INSTANCE_TAG> m_source;

    // Declared as friend so it can access the private operator SOURCE_INSTANCE_TAG member.
    friend std::ostream& operator<<(std::ostream&, MessageSource const&);
  };
}}}}} // namespace Azure::Core::Amqp::Models::_internal
