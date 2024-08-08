// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/models/amqp_error.hpp"

#include "../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "private/error_impl.hpp"
#include "private/value_impl.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/amqp_definitions_fields.h>

#include <azure_uamqp_c/amqp_definitions_error.h>

#include <azure_uamqp_c/amqp_definitions_amqp_error.h>
#endif

#include <iostream>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  // @cond
  void UniqueHandleHelper<ERROR_INSTANCE_TAG>::FreeAmqpError(ERROR_HANDLE handle)
  {
    error_destroy(handle);
  }
// @endcond
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
#if ENABLE_UAMQP
  /*
   * Note that this does not take a unique handle to an AMQP Error - that is because the AMQP
   * code will NOT take ownership of the underlying ERROR_HANDLE object.
   */
  _internal::AmqpError AmqpErrorFactory::FromUamqp(ERROR_HANDLE handle)
  {
    _internal::AmqpError rv;
    const char* stringValue;
    if (!error_get_condition(handle, &stringValue))
    {
      rv.Condition = _internal::AmqpErrorCondition(stringValue);
    }

    if (!error_get_description(handle, &stringValue))
    {
      rv.Description = stringValue;
    }

    AMQP_VALUE info;
    if (!error_get_info(handle, &info) && info)
    {
      // error_get_info returns the AMQP value in place, so we clone it before passing it to the
      // UniqueAmqpValueHandle.
      rv.Info = _detail::AmqpValueFactory::FromUamqp(
                    _detail::UniqueAmqpValueHandle{amqpvalue_clone(info)})
                    .AsMap();
    }

    return rv;
  }

  UniqueAmqpErrorHandle AmqpErrorFactory::ToAmqpError(_internal::AmqpError const& error)
  {
    _detail::UniqueAmqpErrorHandle errorHandle(error_create(error.Condition.ToString().data()));
    if (!error.Description.empty())
    {
      error_set_description(errorHandle.get(), error.Description.data());
    }
    if (!error.Info.empty())
    {
      AmqpValue infoValue(error.Info.AsAmqpValue());
      error_set_info(errorHandle.get(), _detail::AmqpValueFactory::ToUamqp(infoValue));
    }
    return errorHandle;
  }

  AmqpValue AmqpErrorFactory::ToAmqp(_internal::AmqpError const& error)
  {
    _detail::UniqueAmqpErrorHandle errorHandle(ToAmqpError(error));

    // amqpvalue_create_error clones the error handle, so we remember it separately.
    _detail::UniqueAmqpValueHandle handleAsValue{amqpvalue_create_error(errorHandle.get())};

    // The AmqpValue constructor will clone the handle passed into it.
    // The UniqueAmqpValueHandle will take care of freeing the cloned handle.
    return _detail::AmqpValueFactory::FromUamqp(handleAsValue);
  }
#endif
}}}}} // namespace Azure::Core::Amqp::Models::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  std::ostream& operator<<(std::ostream& os, AmqpError const& error)
  {
    if (error)
    {
      os << "Error {";
      os << "Condition =" << error.Condition.ToString();
      os << ", Description=" << error.Description;
      os << ", Info=" << error.Info;
      os << "}";
    }
    else
    {
      os << "Error: {No Error}";
    }
    return os;
  }

#if ENABLE_UAMQP
#else
#define amqp_error_internal_error "amqp:internal-error"
#define amqp_error_not_found "amqp:not-found"
#define amqp_error_unauthorized_access "amqp:unauthorized-access"
#define amqp_error_decode_error "amqp:decode-error"
#define amqp_error_resource_limit_exceeded "amqp:resource-limit-exceeded"
#define amqp_error_not_allowed "amqp:not-allowed"
#define amqp_error_invalid_field "amqp:invalid-field"
#define amqp_error_not_implemented "amqp:not-implemented"
#define amqp_error_resource_locked "amqp:resource-locked"
#define amqp_error_precondition_failed "amqp:precondition-failed"
#define amqp_error_resource_deleted "amqp:resource-deleted"
#define amqp_error_illegal_state "amqp:illegal-state"
#define amqp_error_frame_size_too_small "amqp:frame-size-too-small"

#endif

  const AmqpErrorCondition AmqpErrorCondition::DecodeError(amqp_error_decode_error);
  const AmqpErrorCondition AmqpErrorCondition::FrameSizeTooSmall(amqp_error_frame_size_too_small);
  const AmqpErrorCondition AmqpErrorCondition::IllegalState(amqp_error_illegal_state);
  const AmqpErrorCondition AmqpErrorCondition::InternalError(amqp_error_internal_error);
  const AmqpErrorCondition AmqpErrorCondition::InvalidField(amqp_error_invalid_field);
  const AmqpErrorCondition AmqpErrorCondition::NotAllowed(amqp_error_not_allowed);
  const AmqpErrorCondition AmqpErrorCondition::NotFound(amqp_error_not_found);
  const AmqpErrorCondition AmqpErrorCondition::NotImplemented(amqp_error_not_implemented);
  const AmqpErrorCondition AmqpErrorCondition::PreconditionFailed(amqp_error_precondition_failed);
  const AmqpErrorCondition AmqpErrorCondition::ResourceDeleted(amqp_error_resource_deleted);
  const AmqpErrorCondition AmqpErrorCondition::ResourceLimitExceeded(
      amqp_error_resource_limit_exceeded);
  const AmqpErrorCondition AmqpErrorCondition::ResourceLocked(amqp_error_resource_locked);
  const AmqpErrorCondition AmqpErrorCondition::UnauthorizedAccess(amqp_error_unauthorized_access);

  const AmqpErrorCondition AmqpErrorCondition::LinkStolen("amqp:link:stolen");
  const AmqpErrorCondition AmqpErrorCondition::LinkPayloadSizeExceeded(
      "amqp:link:message-size-exceeded");
  const AmqpErrorCondition AmqpErrorCondition::LinkDetachForced("amqp:link:detach-forced");
  const AmqpErrorCondition AmqpErrorCondition::ConnectionForced("amqp:connection:forced");

  // These are errors that are specific to Azure services.
  const AmqpErrorCondition AmqpErrorCondition::ServerBusyError("com.microsoft:server-busy");
  const AmqpErrorCondition AmqpErrorCondition::ArgumentError("com.microsoft:argument-error");
  const AmqpErrorCondition AmqpErrorCondition::ArgumentOutOfRangeError(
      "com.microsoft:argument-out-of-range");
  const AmqpErrorCondition AmqpErrorCondition::EntityDisabledError("com.microsoft:entity-disabled");
  const AmqpErrorCondition AmqpErrorCondition::PartitionNotOwnedError(
      "com.microsoft:partition-not-owned");
  const AmqpErrorCondition AmqpErrorCondition::StoreLockLostError("com.microsoft:store-lock-lost");
  const AmqpErrorCondition AmqpErrorCondition::PublisherRevokedError(
      "com.microsoft:publisher-revoked");
  const AmqpErrorCondition AmqpErrorCondition::TimeoutError("com.microsoft:timeout");
  const AmqpErrorCondition AmqpErrorCondition::TrackingIdProperty("com.microsoft:tracking-id");
  const AmqpErrorCondition AmqpErrorCondition::ProtonIo("proton:io");
  const AmqpErrorCondition AmqpErrorCondition::ConnectionFramingError(
      "amqp:connection:framing-error");
  const AmqpErrorCondition AmqpErrorCondition::OperationCancelled(
      "com.microsoft:operation-cancelled");
  const AmqpErrorCondition AmqpErrorCondition::MessageLockLost("com.microsoft:message-lock-lost");
  const AmqpErrorCondition AmqpErrorCondition::SessionLockLost("com.microsoft:session-lock-lost");
  const AmqpErrorCondition AmqpErrorCondition::SessionCannotBeLocked(
      "com.microsoft:session-cannot-be-locked");
  const AmqpErrorCondition AmqpErrorCondition::MessageNotFound("com.microsoft:message-not-found");
  const AmqpErrorCondition AmqpErrorCondition::SessionNotFound("com.microsoft:session-not-found");
  const AmqpErrorCondition AmqpErrorCondition::EntityAlreadyExists(
      "com.microsoft:entity-already-exists");
  const AmqpErrorCondition AmqpErrorCondition::ConnectionRedirect("amqp:connection:redirect");
  const AmqpErrorCondition AmqpErrorCondition::LinkRedirect("amqp:link:redirect");
  const AmqpErrorCondition AmqpErrorCondition::TransferLimitExceeded(
      "amqp:link:transfer-limit-exceeded");

}}}}} // namespace Azure::Core::Amqp::Models::_internal
