// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/internal/models/amqp_error.hpp"

#include <azure_uamqp_c/amqp_definitions_fields.h>

#include <azure_uamqp_c/amqp_definitions_error.h>

#include <type_traits>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  template <> struct UniqueHandleHelper<std::remove_pointer<ERROR_HANDLE>::type>
  {
    static void FreeAmqpError(ERROR_HANDLE obj);

    using type = Core::_internal::
        BasicUniqueHandle<std::remove_pointer<ERROR_HANDLE>::type, FreeAmqpError>;
  };
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
  using UniqueAmqpErrorHandle
      = Amqp::_detail::UniqueHandle<std::remove_pointer<ERROR_HANDLE>::type>;

  /**
   * @brief uAMQP interoperability functions to convert an AmqpError to a uAMQP AMQP_ERROR
   * and back.
   *
   * @remarks This class should not be used directly. It is used by the uAMQP interoperability
   * layer.
   */
  struct AmqpErrorFactory
  {
    static _internal::AmqpError FromUamqp(ERROR_HANDLE error);
    static AmqpValue ToAmqp(_internal::AmqpError const& error);
    AmqpErrorFactory() = delete;
  };
}}}}} // namespace Azure::Core::Amqp::Models::_detail
