// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/models/amqp_header.hpp"
#include "azure/core/amqp/models/amqp_properties.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"

#include <chrono>
#include <string>

struct SOURCE_INSTANCE_TAG;

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
  class MessageSourceImpl;
}}}}} // namespace Azure::Core::Amqp::Models::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  struct MessageSourceOptions final
  {
    AmqpValue Address;
    Nullable<TerminusDurability> SourceTerminusDurability;
    Nullable<TerminusExpiryPolicy> SourceTerminusExpiryPolicy;
    Nullable<std::chrono::system_clock::time_point> Timeout;
    Nullable<bool> Dynamic;
    AmqpMap DynamicNodeProperties;
    Nullable<AmqpSymbol> DistributionMode;
    AmqpMap Filter;
    AmqpValue DefaultOutcome;
    AmqpArray Outcomes;
    AmqpArray Capabilities;
  };

  /**
   * @brief Represents an AMQP message source.
   *
   * An AMQP message source is a node that originates messages. It is the source of messages for a
   * link. The message source is identified by its address, which is a string that uniquely
   * identifies the node within the scope of the AMQP container.
   *
   * @remarks See
   * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-source)
   * for more information about the fields in a message source.
   */
  class MessageSource final {
  public:
    /** @brief Creates a default message target.
     */
    MessageSource();
    /** @brief Deletes a message target. */
    ~MessageSource();

    // Create a described source from an AMQP Value - used in the OnLinkAttached.
    MessageSource(AmqpValue const& value);

    /** @brief Copies a MessageSource */
    MessageSource(MessageSource const& that);

    /** Assigns a message source from another.
     */
    MessageSource& operator=(MessageSource const& that);

    /** @brief Move constructor */
    MessageSource(MessageSource&& other) noexcept;

    /** @brief Move assignment operator */
    MessageSource& operator=(MessageSource&& other) noexcept;

    /** @brief Creates a message source with detailed options.
     *
     * @param options Options used constructing the message source.
     */

    MessageSource(MessageSourceOptions const& options);

    /* Note: These constructors should NOT be marked as explicit, because we want to enable the
     * implicit construction of the MessageSource from a string - this allows callers to construct
     * Link, MessageSender, and MessageReceiver objects without forcing the creation of a
     * MessageSource object. */

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
     * @remarks Creates an AMQP Described value with the descriptor being the message source
     * (0x29).
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
    AmqpSymbol GetDistributionMode() const;

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
    std::unique_ptr<_detail::MessageSourceImpl> m_impl;
    // Declared as friend so it can access the private m_impl member.
    friend std::ostream& operator<<(std::ostream&, MessageSource const&);
  };
}}}}} // namespace Azure::Core::Amqp::Models::_internal
