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

  struct MessageSourceOptions
  {
    Azure::Core::Amqp::Models::AmqpValue Address;
    Azure::Nullable<TerminusDurability> SourceTerminusDurability;
    Azure::Nullable<TerminusExpiryPolicy> SourceTerminusExpiryPolicy;
    Azure::Nullable<std::chrono::system_clock::time_point> Timeout;
    Azure::Nullable<bool> Dynamic;
    Azure::Core::Amqp::Models::AmqpMap DynamicNodeProperties;
    Azure::Nullable<std::string> DistributionMode;
    Azure::Core::Amqp::Models::AmqpMap Filter;
    Azure::Core::Amqp::Models::AmqpValue DefaultOutcome;
    Azure::Core::Amqp::Models::AmqpArray Outcomes;
    Azure::Core::Amqp::Models::AmqpArray Capabilities;
  };

  class MessageSource final {
  public:
    /** @brief Creates a default message target.
     */
    MessageSource();
    /** @brief Deletes a message target. */
    ~MessageSource() = default;

    // Create a described source from an AMQP Value - used in the OnLinkAttached.
    MessageSource(Azure::Core::Amqp::Models::AmqpValue const& value);

    /** @brief Creates a message source with detailed options.
     *
     * @param options Options used constructing the message source.
     */

    MessageSource(MessageSourceOptions const& options);

    /** @brief Creates a message source with the given address.
     *
     * @param address The address of the source.
     */
    MessageSource(std::string const& value);

    /** @brief Creates a message source with the given address.
     *
     * @param address The address of the source.
     */
    MessageSource(char const* value);

    /** @brief Creates an AMQP value from a message source.
     *
     * @remarks Creates an AMQP Described value with the descriptor being the message source (0x29).
     */
    operator const Azure::Core::Amqp::Models::AmqpValue() const;

    /** @brief Gets the address of the source.
     *
     * @return The address of the source.
     */
    Azure::Core::Amqp::Models::AmqpValue GetAddress() const;

    /** @brief Gets the durability of the source.
     *
     * @return The durability of the source.
     */
    TerminusDurability GetTerminusDurability() const;

    /** @brief Gets the expiry policy of the source.
     *
     * @return The expiry policy of the source.
     */
    TerminusExpiryPolicy GetExpiryPolicy() const;

    /** @brief Duration that an expiring source will be retained.
     *
     * @return The timeout of the source.
     *
     * @remarks The source starts expiring as indicated by the expiry-policy.
     */
    std::chrono::system_clock::time_point GetTimeout() const;

    /** @brief Requests dynamic creation of a remote node.
     *
     * @return Whether the source is dynamic.
     */
    bool GetDynamic() const;

    /** @brief Retrieve the dynamic node properties on this message source.
     * @remarks See
     * http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-node-properties
     * for more information.
     */
    Azure::Core::Amqp::Models::AmqpMap GetDynamicNodeProperties() const;

    /** @brief Gets the distribution mode of the source.
     *
     * @return The distribution mode of the source.
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
    Azure::Core::Amqp::Models::AmqpMap GetFilter() const;

    /** @brief Gets the default outcome of the source.
     *
     * @return The default outcome of the source.
     *
     */
    Azure::Core::Amqp::Models::AmqpValue GetDefaultOutcome() const;

    /** @brief Gets the outcomes of the source.
     *
     * @return The outcomes of the source.
     *
     */
    Azure::Core::Amqp::Models::AmqpArray GetOutcomes() const;

    /** @brief Gets the capabilities of the source.
     *
     * @return The capabilities of the source.
     *
     */
    Azure::Core::Amqp::Models::AmqpArray GetCapabilities() const;

  private:
    operator SOURCE_INSTANCE_TAG*() const { return m_source.get(); }
    Azure::Core::_internal::UniqueHandle<SOURCE_INSTANCE_TAG> m_source;

    // Declared as friend so it can access the private operator SOURCE_INSTANCE_TAG member.
    friend std::ostream& operator<<(std::ostream&, MessageSource const&);
  };
}}}}} // namespace Azure::Core::Amqp::Models::_internal
