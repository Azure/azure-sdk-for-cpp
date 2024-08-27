// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/internal/models/message_source.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/amqp_definitions_fields.h>

#include <azure_uamqp_c/amqp_definitions_terminus_durability.h>
#include <azure_uamqp_c/amqp_definitions_terminus_expiry_policy.h>

#include <azure_uamqp_c/amqp_definitions_filter_set.h>
#include <azure_uamqp_c/amqp_definitions_node_properties.h>
#include <azure_uamqp_c/amqp_definitions_seconds.h>
#include <azure_uamqp_c/amqp_definitions_source.h>
#elif ENABLE_RUST_AMQP
#include "../rust_amqp/rust_wrapper/rust_amqp_wrapper.h"
#endif

#include <type_traits>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  using AmqpSourceImplementation = std::remove_pointer<SOURCE_HANDLE>::type;
#elif ENABLE_RUST_AMQP
  using AmqpSourceImplementation = Azure::Core::Amqp::_detail::RustInterop::RustAmqpSource;
#endif

  template <> struct UniqueHandleHelper<AmqpSourceImplementation>
  {
    static void FreeMessageSource(AmqpSourceImplementation* obj);

    using type = Core::_internal::BasicUniqueHandle<AmqpSourceImplementation, FreeMessageSource>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
  class MessageSourceImpl final {
  public:
    /** @brief Creates a default message target.
     */
    MessageSourceImpl();
    /** @brief Deletes a message target. */
    ~MessageSourceImpl() = default;

    // Create a described source from an AMQP Value - used in the OnLinkAttached.
    MessageSourceImpl(AmqpValue const& value);

    /** @brief Copies a MessageSource */
    MessageSourceImpl(MessageSourceImpl const& that);

    /** Assigns a message source from another.
     */
    MessageSourceImpl& operator=(MessageSourceImpl const& that);

    /** @brief Move constructor */
    MessageSourceImpl(MessageSourceImpl&& other) noexcept;

    /** @brief Move assignment operator */
    MessageSourceImpl& operator=(MessageSourceImpl&& other) noexcept;

    /** @brief Creates a message source with detailed options.
     *
     * @param options Options used constructing the message source.
     */

    MessageSourceImpl(Models::_internal::MessageSourceOptions const& options);

    /** @brief Creates a message source with the given address.
     *
     * @param address The address of the source.
     */
    MessageSourceImpl(std::string const& address);

    /** @brief Creates a message source with the given address.
     *
     * @param address The address of the source.
     */
    MessageSourceImpl(char const* address);

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
    _internal::TerminusDurability GetTerminusDurability() const;

    /** @brief Gets the expiry policy of the source.
     *
     * @return The expiry policy of the source.
     *
     * @remarks See
     * [source](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-source)
     * for more information about the fields in a message source.
     */
    _internal::TerminusExpiryPolicy GetExpiryPolicy() const;

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
    operator Azure::Core::Amqp::_detail::AmqpSourceImplementation*() const
    {
      return m_source.get();
    }
    Amqp::_detail::UniqueHandle<Azure::Core::Amqp::_detail::AmqpSourceImplementation> m_source;

    // Declared as friend so it can access the private operator SOURCE_INSTANCE_TAG member.
    friend std::ostream& operator<<(std::ostream&, MessageSourceImpl const&);
  };
}}}}} // namespace Azure::Core::Amqp::Models::_detail
