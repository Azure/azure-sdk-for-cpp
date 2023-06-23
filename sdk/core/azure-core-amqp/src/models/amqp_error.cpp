// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/models/amqp_error.hpp"

#include "azure/core/amqp/models/amqp_value.hpp"

#include <azure_uamqp_c/amqp_definitions_fields.h>

#include <azure_uamqp_c/amqp_definitions_amqp_error.h>
#include <azure_uamqp_c/amqp_definitions_error.h>

#include <iostream>

namespace Azure { namespace Core { namespace _internal {
  void UniqueHandleHelper<ERROR_INSTANCE_TAG>::FreeAmqpError(ERROR_HANDLE handle)
  {
    error_destroy(handle);
  }
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  /*
   * Note that this does not take a unique handle to an AMQP Error - that is because the AMQP
   * code will NOT take ownership of the underlying ERROR_HANDLE object.
   */
  AmqpError AmqpErrorFactory::FromUamqp(ERROR_HANDLE handle)
  {
    AmqpError rv;
    const char* stringValue;
    if (!error_get_condition(handle, &stringValue))
    {
      rv.Condition = AmqpErrorCondition(stringValue);
    }

    if (!error_get_description(handle, &stringValue))
    {
      rv.Description = stringValue;
    }

    AMQP_VALUE info;
    if (!error_get_info(handle, &info) && info)
    {
      rv.Info = AmqpValue{info}.AsMap();
    }

    return rv;
  }

  AmqpValue AmqpErrorFactory::ToAmqp(AmqpError const& error)
  {
    UniqueAmqpErrorHandle errorHandle(error_create(error.Condition.ToString().data()));
    if (!error.Description.empty())
    {
      error_set_description(errorHandle.get(), error.Description.data());
    }
    if (!error.Info.empty())
    {
      error_set_info(errorHandle.get(), AmqpValue{error.Info});
    }
    // amqpvalue_create_error clones the error handle, so we remember it separately.
    _detail::UniqueAmqpValueHandle handleAsValue{amqpvalue_create_error(errorHandle.get())};

    // The AmqpValue constructor will clone the handle passed into it.
    // The UniqueAmqpValueHandle will take care of freeing the cloned handle.
    return handleAsValue.get();
  }
  std::ostream& operator<<(std::ostream& os, AmqpError const& error)
  {
    os << "Error {";
    os << "Condition =" << error.Condition.ToString();
    os << ", Description=" << error.Description;
    os << ", Info=" << error.Info;
    os << "}";
    return os;
  }

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

}}}}} // namespace Azure::Core::Amqp::Models::_internal
