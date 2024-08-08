// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../../../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/internal/models/performatives/amqp_transfer.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/amqp_definitions_fields.h>

#include <azure_uamqp_c/amqp_definitions_handle.h>
#include <azure_uamqp_c/amqp_definitions_sequence_no.h>

#include <azure_uamqp_c/amqp_definitions_delivery_number.h>
#include <azure_uamqp_c/amqp_definitions_delivery_tag.h>
#include <azure_uamqp_c/amqp_definitions_message_format.h>
#include <azure_uamqp_c/amqp_definitions_receiver_settle_mode.h>
#include <azure_uamqp_c/amqp_definitions_transfer.h>
#endif

#include <type_traits>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  template <> struct UniqueHandleHelper<std::remove_pointer<TRANSFER_HANDLE>::type>
  {
    static void FreeAmqpTransfer(TRANSFER_HANDLE obj);

    using type = Core::_internal::
        BasicUniqueHandle<std::remove_pointer<TRANSFER_HANDLE>::type, FreeAmqpTransfer>;
  };
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
#if ENABLE_UAMQP
  using UniqueAmqpTransferHandle
      = Amqp::_detail::UniqueHandle<std::remove_pointer<TRANSFER_HANDLE>::type>;

#endif
  /**
   * @brief uAMQP interoperability functions to convert an AmqpTransfer to a uAMQP AMQP_TRANSFER
   * and back.
   *
   * @remarks This class should not be used directly. It is used by the uAMQP interoperability
   * layer.
   */
  struct AmqpTransferFactory
  {
#if ENABLE_UQAMP
    static _internal::Performatives::AmqpTransfer FromUamqp(TRANSFER_HANDLE error);
#endif
    static AmqpValue ToAmqp(_internal::Performatives::AmqpTransfer const& error);
    AmqpTransferFactory() = delete;
  };
}}}}} // namespace Azure::Core::Amqp::Models::_detail
