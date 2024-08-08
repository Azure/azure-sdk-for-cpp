// Copyright(c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/models/performatives/amqp_transfer.hpp"

#include "../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "private/performatives/transfer_impl.hpp"
#include "private/value_impl.hpp"

#if ENABLE_UAMQP
#include <azure_uamqp_c/amqp_definitions_handle.h>

#include <azure_uamqp_c/amqp_definitions_delivery_number.h>
#include <azure_uamqp_c/amqp_definitions_delivery_tag.h>
#include <azure_uamqp_c/amqp_definitions_transfer.h>
#endif

#include <iostream>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  // @cond
  void UniqueHandleHelper<TRANSFER_INSTANCE_TAG>::FreeAmqpTransfer(TRANSFER_HANDLE handle)
  {
    transfer_destroy(handle);
  }
// @endcond
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {

  /*
   * Note that this does not take a unique handle to an AMQP Error - that is because the AMQP
   * code will NOT take ownership of the underlying ERROR_HANDLE object.
   */
#if ENABLE_UAMQP
  _internal::Performatives::AmqpTransfer AmqpTransferFactory::FromUamqp(
      TRANSFER_HANDLE transferHandle)
  {
    _internal::Performatives::AmqpTransfer rv;
    handle handle_value;
    if (!transfer_get_handle(transferHandle, &handle_value))
    {
      rv.Handle = handle_value;
    }
    uint32_t uint32val;
    if (!transfer_get_delivery_id(transferHandle, &uint32val))
    {
      rv.DeliveryId = uint32val;
    }
    amqp_binary binaryValue;
    if (!transfer_get_delivery_tag(transferHandle, &binaryValue))
    {
      std::vector<uint8_t> binaryData(
          reinterpret_cast<const uint8_t*>(binaryValue.bytes),
          reinterpret_cast<const uint8_t*>(binaryValue.bytes) + binaryValue.length);
      rv.DeliveryTag = AmqpBinaryData{binaryData};
    }
    if (!transfer_get_message_format(transferHandle, &uint32val))
    {
      rv.MessageFormat = uint32val;
    }
    bool boolValue;
    if (!transfer_get_settled(transferHandle, &boolValue))
    {
      rv.Settled = boolValue;
    }
    if (!transfer_get_more(transferHandle, &boolValue))
    {
      rv.More = boolValue;
    }
    receiver_settle_mode settleMode;
    if (!transfer_get_rcv_settle_mode(transferHandle, &settleMode))
    {
      switch (settleMode)
      {
        case receiver_settle_mode_first:
          rv.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
          break;
        case receiver_settle_mode_second:
          rv.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::Second;
      }
    }
    AMQP_VALUE amqpValue;
    if (!transfer_get_state(transferHandle, &amqpValue))
    {
      rv.State = _detail::AmqpValueFactory::FromUamqp(
          _detail::UniqueAmqpValueHandle{amqpvalue_clone(amqpValue)});
    }
    if (!transfer_get_resume(transferHandle, &boolValue))
    {
      rv.Resume = boolValue;
    }
    if (!transfer_get_aborted(transferHandle, &boolValue))

    {
      rv.Aborted = boolValue;
    }
    if (!transfer_get_batchable(transferHandle, &boolValue))
    {
      rv.Batchable = boolValue;
    }
    return rv;
  }
#endif

#if ENABLE_UAMQP
  AmqpValue AmqpTransferFactory::ToAmqp(_internal::Performatives::AmqpTransfer const& transfer)
  {
    _detail::UniqueAmqpTransferHandle transferHandle(transfer_create(transfer.Handle));

    // amqpvalue_create_error clones the error handle, so we remember it separately.
    _detail::UniqueAmqpValueHandle handleAsValue{amqpvalue_create_transfer(transferHandle.get())};

    // The AmqpValue constructor will clone the handle passed into it.
    // The UniqueAmqpValueHandle will take care of freeing the cloned handle.
    return _detail::AmqpValueFactory::FromUamqp(handleAsValue);
  }
#endif
}}}}} // namespace Azure::Core::Amqp::Models::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
  namespace Performatives {
    std::ostream& operator<<(std::ostream& os, AmqpTransfer const& transfer)
    {
      os << "Transfer {";
      os << "Handle: " << transfer.Handle;
      if (transfer.DeliveryId)
      {
        os << ", DeliveryId: " << transfer.DeliveryId.Value();
      }
      if (transfer.DeliveryTag)
      {
        os << ", DeliveryTag: " << transfer.DeliveryTag.Value();
      }
      os << ", MessageFormat: " << transfer.MessageFormat;
      if (transfer.Settled)
      {
        os << ", Settled, " << transfer.Settled.Value();
      }
      os << ", More: " << transfer.More;
      if (transfer.SettleMode)
      {
        os << ", RcvSettleMode=" << transfer.SettleMode.Value();
      }
      os << ", State=" << transfer.State;
      os << ", Resume=" << transfer.Resume;
      os << ", Aborted=" << transfer.Aborted;
      os << ", Batchable=" << transfer.Batchable;
      os << "}";
      return os;
    }
}}}}}} // namespace Azure::Core::Amqp::Models::_internal::Performatives
