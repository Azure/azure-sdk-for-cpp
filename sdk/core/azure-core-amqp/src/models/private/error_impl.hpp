// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/internal/models/amqp_error.hpp"

#if ENABLE_UAMQP

#include <azure_uamqp_c/amqp_definitions_fields.h>

#include <azure_uamqp_c/amqp_definitions_error.h>

#endif // ENABLE_UAMQP

#include <type_traits>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  template <> struct UniqueHandleHelper<std::remove_pointer<ERROR_HANDLE>::type>
  {
    static void FreeAmqpError(ERROR_HANDLE obj);

    using type = Core::_internal::
        BasicUniqueHandle<std::remove_pointer<ERROR_HANDLE>::type, FreeAmqpError>;
  };
#endif // ENABLE_UAMQP
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
#if ENABLE_UAMQP
  using UniqueAmqpErrorHandle
      = Amqp::_detail::UniqueHandle<std::remove_pointer<ERROR_HANDLE>::type>;
#endif

  /**
   * @brief uAMQP interoperability functions to convert an AmqpError to a uAMQP AMQP_ERROR
   * and back.
   *
   * @remarks This class should not be used directly. It is used by the uAMQP interoperability
   * layer.
   */
  struct AmqpErrorFactory
  {
#if ENABLE_AMQP

    static _internal::AmqpError FromUamqp(ERROR_HANDLE error);
#endif
    static AmqpValue ToAmqp(_internal::AmqpError const& error);
#if ENABLE_AMQP
    static UniqueAmqpErrorHandle ToAmqpError(_internal::AmqpError const& error);
#endif
    AmqpErrorFactory() = delete;
  };
}}}}} // namespace Azure::Core::Amqp::Models::_detail
