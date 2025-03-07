// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/internal/models/performatives/amqp_detach.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/amqp_definitions_fields.h>

#include <azure_uamqp_c/amqp_definitions_error.h>
#include <azure_uamqp_c/amqp_definitions_handle.h>
#include <azure_uamqp_c/amqp_definitions_sequence_no.h>

#include <azure_uamqp_c/amqp_definitions_detach.h>
#endif

#include <type_traits>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  template <> struct UniqueHandleHelper<std::remove_pointer<DETACH_HANDLE>::type>
  {
    static void FreeAmqpDetach(DETACH_HANDLE obj);

    using type = Core::_internal::
        BasicUniqueHandle<std::remove_pointer<DETACH_HANDLE>::type, FreeAmqpDetach>;
  };
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
#if ENABLE_UAMQP
  using UniqueAmqpDetachHandle
      = Amqp::_detail::UniqueHandle<std::remove_pointer<DETACH_HANDLE>::type>;
#endif

  /**
   * @brief uAMQP interoperability functions to convert an AmqpTransfer to a uAMQP AMQP_TRANSFER
   * and back.
   *
   * @remarks This class should not be used directly. It is used by the uAMQP interoperability
   * layer.
   */
  struct AmqpDetachFactory
  {
#if ENABLE_UAMQP
    static _internal::Performatives::AmqpDetach ToImplementation(DETACH_HANDLE error);
    static UniqueAmqpDetachHandle ToAmqpDetach(_internal::Performatives::AmqpDetach const& error);
#endif // ENABLE_UAMQP
    AmqpDetachFactory() = delete;
  };
}}}}} // namespace Azure::Core::Amqp::Models::_detail
