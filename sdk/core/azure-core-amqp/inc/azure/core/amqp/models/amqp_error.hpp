// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_header.hpp"
#include "amqp_properties.hpp"
#include "amqp_value.hpp"
#include "azure/core/amqp/dll_import_export.hpp"

#include <azure/core/internal/extendable_enumeration.hpp>
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic push
#elif defined(__clang__) // !__clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
#endif // _MSC_VER

/// @cond HIDDEN
struct ERROR_INSTANCE_TAG;

template <> struct Azure::Core::_internal::UniqueHandleHelper<ERROR_INSTANCE_TAG>
{
  static void FreeAmqpError(ERROR_INSTANCE_TAG* obj);

  using type = Azure::Core::_internal::BasicUniqueHandle<ERROR_INSTANCE_TAG, FreeAmqpError>;
};
/// @endcond
#if defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic pop
#elif defined(__clang__) // !__clang__
#pragma clang diagnostic pop
#endif // _MSC_VER

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  using UniqueAmqpErrorHandle = Azure::Core::_internal::UniqueHandle<ERROR_INSTANCE_TAG>;

  class AmqpErrorCondition final
      : public Azure::Core::_internal::ExtendableEnumeration<AmqpErrorCondition> {
  public:
    AmqpErrorCondition() : ExtendableEnumeration(){};
    explicit AmqpErrorCondition(std::string const& value) : ExtendableEnumeration(value) {}

    /** @brief An AMQP Internal Error
     *
     * An internal error occurred. Operator intervention might be necessary to resume normal
     * operation.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     */
    AZ_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition InternalError;

    /** @brief Entity Not Found
     * A peer attempted to work with a remote entity that does not
     * exist.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     */
    AZ_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition NotFound;

    /** @brief Unauthorized Access.
     *
     * A peer attempted to work with a remote entity to which it has no access due to security
     * settings.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     */
    AZ_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition UnauthorizedAccess;

    /** @brief Decoder Error
     * Data could not be decoded.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     */
    AZ_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition DecodeError;

    /** @brief Resource allocation exceeded.
     *
     * A peer exceeded its resource allocation.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     */
    AZ_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition ResourceLimitExceeded;

    /** @brief Not Allowed.
     *
     * The peer tried to use a frame in a manner that is inconsistent with the semantics defined in
     * the specification. For more information, see
     *
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     */
    AZ_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition NotAllowed;

    /** @brief Invalid Field.
     *
     * An invalid field was passed in a frame body, and the operation could not proceed.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     *
     */
    AZ_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition InvalidField;

    /** @brief Not Implemented.
     * The peer tried to use functionality that is not implemented in its partner.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     *
     */
    AZ_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition NotImplemented;

    /** @brief Resource Locked.
     *
     * The client attempted to work with a server entity to which it has no access because another
     * client is working with it.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     *
     */
    AZ_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition ResourceLocked;

    /** @brief Precondition Failed.
     *
     * The client made a request that was not allowed because some precondition failed.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     *
     */
    AZ_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition PreconditionFailed;
    /** @brief Resource Deleted.
     *
     * A server entity the client is working with has been deleted.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     *
     */
    AZ_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition ResourceDeleted;

    /** @brief Illegal State.
     *
     * The peer sent a frame that is not permitted in the current state.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     *
     */
    AZ_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition IllegalState;

    /** @brief Frame Size Too Small.
     *
     * The peer cannot send a frame because the smallest encoding of the performative with the
     * currently valid values would be too large to fit within a frame of the agreed maximum frame
     * size. When transferring a message the message data can be sent in multiple transfer frames
     * thereby avoiding this error. Similarly when attaching a link with a large unsettled map the
     * endpoint MAY make use of the incomplete-unsettled flag to avoid the need for overly large
     * frames.
     *
     * For more information, see
     * [AmqpError](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-amqp-error).
     *
     */
    AZ_CORE_AMQP_DLLEXPORT static const AmqpErrorCondition FrameSizeTooSmall;
  };

  struct AmqpError final
  {
    AmqpError() = default;
    ~AmqpError() = default;

    /** @brief A symbolic value indicating the error condition.
     *
     * @remarks For more information, see [AMQP
     * Section 2.8.14](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-error).
     */
    AmqpErrorCondition Condition;

    /** @brief This text supplies any supplementary details not indicated by the condition field.
     * This text can be logged as an aid to resolving issues.
     *
     * @remarks For more information, see [AMQP
     * Section 2.8.14](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-error).
     */
    std::string Description;

    /** @brief A map containing information about the error condition.
     *
     * @remarks For more information, see [AMQP
     * Section 2.8.14](https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-transport-v1.0-os.html#type-error).
     */
    AmqpMap Info;

    /** @brief Returns true if the AmqpError has a value.
     *
     * @return true if the AmqpError has a value.
     *
     */
    operator bool() const
    {
      return !(Condition.ToString().empty() && Description.empty() && Info.empty());
    }
  };
  std::ostream& operator<<(std::ostream&, AmqpError const&);

  /**
   * @brief uAMQP interoperability functions to convert an AmqpError to a uAMQP AMQP_ERROR
   * and back.
   *
   * @remarks This class should not be used directly. It is used by the uAMQP interoperability
   * layer.
   */
  struct AmqpErrorFactory
  {
    static AmqpError FromUamqp(ERROR_INSTANCE_TAG* error);
    static AmqpValue ToAmqp(AmqpError const& error);
    AmqpErrorFactory() = delete;
  };

}}}}} // namespace Azure::Core::Amqp::Models::_internal
