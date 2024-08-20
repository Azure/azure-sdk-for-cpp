// Copyright(c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/models/performatives/amqp_detach.hpp"

#include "../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "private/error_impl.hpp"
#include "private/performatives/detach_impl.hpp"
#include "private/value_impl.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/amqp_definitions_handle.h>

#include <azure_uamqp_c/amqp_definitions_delivery_number.h>
#include <azure_uamqp_c/amqp_definitions_delivery_tag.h>
#include <azure_uamqp_c/amqp_definitions_detach.h>
#endif

#include <iostream>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  // @cond
  void UniqueHandleHelper<DETACH_INSTANCE_TAG>::FreeAmqpDetach(DETACH_HANDLE handle)
  {
    detach_destroy(handle);
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
  _internal::Performatives::AmqpDetach AmqpDetachFactory::ToImplementation(DETACH_HANDLE detachHandle)
  {
    _internal::Performatives::AmqpDetach rv;
    handle handle_value;
    if (!detach_get_handle(detachHandle, &handle_value))
    {
      rv.Handle = handle_value;
    }
    bool boolValue;
    if (!detach_get_closed(detachHandle, &boolValue))
    {
      rv.Closed = boolValue;
    }

    {
      ERROR_HANDLE amqpErrorHandle;
      if (!detach_get_error(detachHandle, &amqpErrorHandle))
      {
        UniqueAmqpErrorHandle error{amqpErrorHandle};
        amqpErrorHandle = nullptr;
        rv.Error = _detail::AmqpErrorFactory::FromImplementation(error.get());
      }
    }
    return rv;
  }

  _detail::UniqueAmqpDetachHandle AmqpDetachFactory::ToAmqpDetach(
      _internal::Performatives::AmqpDetach const& detach)
  {
    _detail::UniqueAmqpDetachHandle detachHandle(detach_create(detach.Handle));
    if (detach_set_closed(detachHandle.get(), detach.Closed))
    {
      throw std::runtime_error("Could not set closed state on detach item.");
    }

    if (detach_set_error(detachHandle.get(), AmqpErrorFactory::ToAmqpError(detach.Error).get()))
    {
      throw std::runtime_error("Could not set error on detach item.");
    }
    return detachHandle;
  }
#endif
}}}}} // namespace Azure::Core::Amqp::Models::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  namespace Performatives {
    std::ostream& operator<<(std::ostream& os, AmqpDetach const& detach)
    {
      os << "Detach {";
      os << "Handle =" << detach.Handle;
      os << ", Closed: " << detach.Closed;
      os << ", Error: " << detach.Error;
      os << "}";
      return os;
    }
}}}}}} // namespace Azure::Core::Amqp::Models::_internal::Performatives
