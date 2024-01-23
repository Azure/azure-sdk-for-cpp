// Copyright(c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/models/amqp_transfer.hpp"

#include "../amqp/private/unique_handle.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "private/transfer_impl.hpp"
#include "private/value_impl.hpp"

#include <azure_uamqp_c/amqp_definitions_delivery_number.h>
#include <azure_uamqp_c/amqp_definitions_delivery_tag.h>
#include <azure_uamqp_c/amqp_definitions_handle.h>
#include <azure_uamqp_c/amqp_definitions_transfer.h>

#include <iostream>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  // @cond
  void UniqueHandleHelper<TRANSFER_INSTANCE_TAG>::FreeAmqpTransfer(TRANSFER_HANDLE handle)
  {
    transfer_destroy(handle);
  }
  // @endcond
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {

    /*
     * Note that this does not take a unique handle to an AMQP Error - that is because the AMQP
     * code will NOT take ownership of the underlying ERROR_HANDLE object.
     */
    _internal::AmqpTransfer AmqpTransferFactory::FromUamqp(TRANSFER_HANDLE transferHandle)
    {
      _internal::AmqpTransfer rv;
      handle handle_value;
      if (!transfer_get_handle(transferHandle, &handle_value))
      {
        rv.HandleValue = handle_value;
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
        rv.ReceiverSettleMode = settleMode;
      }
      AMQP_VALUE amqpValue;
      if (!transfer_get_state(transferHandle, &amqpValue))
      {
        rv.State = _detail::AmqpValueFactory::FromUamqp(_detail::UniqueAmqpValueHandle{amqpValue});
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

    AmqpValue AmqpTransferFactory::ToAmqp(_internal::AmqpTransfer const& transfer)
    {
      _detail::UniqueAmqpTransferHandle transferHandle(transfer_create(transfer.HandleValue));

      // amqpvalue_create_error clones the error handle, so we remember it separately.
      _detail::UniqueAmqpValueHandle handleAsValue{amqpvalue_create_transfer(transferHandle.get())};

      // The AmqpValue constructor will clone the handle passed into it.
      // The UniqueAmqpValueHandle will take care of freeing the cloned handle.
      return _detail::AmqpValueFactory::FromUamqp(handleAsValue);
    }
}}}}} // namespace Azure::Core::Amqp::Models::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {
    std::ostream& operator<<(std::ostream& os, AmqpTransfer const& transfer)
    {
      os << "Transfer {";
      os << "Handle =" << transfer.HandleValue;
      if (transfer.DeliveryId.HasValue())
      {
        os << ", DeliveryId=" << transfer.DeliveryId.Value();
      }
      os << ", DeliveryTag " << transfer.DeliveryTag;
      os << ", MessageFormat=" << transfer.MessageFormat;
      os << ", Settled=" << transfer.Settled;
      os << ", More=" << transfer.More;
      os << ", RcvSettleMode=" << transfer.ReceiverSettleMode;
      os << ", State=" << transfer.State;
      os << ", Resume=" << transfer.Resume;
      os << ", Aborted=" << transfer.Aborted;
      os << ", Batchable=" << transfer.Batchable;
      os << "}";
      return os;
    }
}}}}} // namespace Azure::Core::Amqp::Models::_internal
