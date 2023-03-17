// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#include "azure/core/amqp/models/transfer_instance.hpp"
#include "azure/core/amqp/message_receiver.hpp"

#include <azure_uamqp_c/amqp_definitions_sequence_no.h>

#include <azure_uamqp_c/amqp_definitions_delivery_number.h>
#include <azure_uamqp_c/amqp_definitions_delivery_tag.h>
#include <azure_uamqp_c/amqp_definitions_handle.h>
#include <azure_uamqp_c/amqp_definitions_message_format.h>
#include <azure_uamqp_c/amqp_definitions_receiver_settle_mode.h>
#include <azure_uamqp_c/amqp_definitions_sender_settle_mode.h>
#include <azure_uamqp_c/amqp_definitions_transfer.h>
#include <chrono>
#include <iostream>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace Models {

  TransferInstance::~TransferInstance() { transfer_destroy(m_instance); }

  uint32_t TransferInstance::GetDeliveryId() const
  {
    uint32_t deliveryId;
    if (transfer_get_delivery_id(m_instance, &deliveryId))
    {
      throw std::runtime_error("Failed to set handle");
    }
    return deliveryId;
  }

  void TransferInstance::SetDeliveryId(uint32_t deliveryId)
  {
    if (transfer_set_delivery_id(m_instance, deliveryId))
    {
      throw std::runtime_error("Failed to set delivery id");
    }
  }

  uint32_t TransferInstance::GetHandle() const
  {
    uint32_t handle;
    if (transfer_get_handle(m_instance, &handle))
    {
      throw std::runtime_error("Failed to set handle");
    }
    return handle;
  }
  void TransferInstance::SetHandle(uint32_t handle)
  {
    if (transfer_set_handle(m_instance, handle))
    {
      throw std::runtime_error("Failed to set handle");
    }
  }

  Azure::Core::Amqp::Models::BinaryData TransferInstance::GetDeliveryTag() const
  {
    delivery_tag deliveryTag;
    if (transfer_get_delivery_tag(m_instance, &deliveryTag))
    {
      throw std::runtime_error("Failed to set delivery tag");
    }
    Azure::Core::Amqp::Models::BinaryData rv{
        static_cast<const uint8_t*>(deliveryTag.bytes), deliveryTag.length};
    return rv;
  }

  void TransferInstance::SetDeliveryTag(Azure::Core::Amqp::Models::BinaryData deliveryTag)
  {
    delivery_tag tag{deliveryTag.bytes, static_cast<uint32_t>(deliveryTag.length)};
    if (transfer_set_delivery_tag(m_instance, tag))
    {
      throw std::runtime_error("Failed to set delivery tag");
    }
  }

  uint32_t TransferInstance::GetMessageFormat() const
  {
    uint32_t format;
    if (transfer_get_message_format(m_instance, &format))
    {
      throw std::runtime_error("Failed to get message format");
    }
    return format;
  }
  void TransferInstance::SetMessageFormat(uint32_t format)
  {
    if (transfer_set_message_format(m_instance, format))
    {
      throw std::runtime_error("Failed to set message format");
    }
  }

  bool TransferInstance::GetMore() const
  {
    bool more;
    if (transfer_get_more(m_instance, &more))
    {
      throw std::runtime_error("Failed to get more");
    }
    return more;
  }
  void TransferInstance::SetMore(bool more)
  {
    if (transfer_set_more(m_instance, more))
    {
      throw std::runtime_error("Failed to set more");
    }
  }

  bool TransferInstance::GetBatchable() const
  {
    bool batchable;
    if (transfer_get_batchable(m_instance, &batchable))
    {
      throw std::runtime_error("Failed to get batchable");
    }
    return batchable;
  }
  void TransferInstance::SetBatchable(bool batchable)
  {
    if (transfer_set_batchable(m_instance, batchable))
    {
      throw std::runtime_error("Failed to set more");
    }
  }

  bool TransferInstance::GetSettled() const
  {
    bool settled;
    if (transfer_get_settled(m_instance, &settled))
    {
      throw std::runtime_error("Failed to get settled");
    }
    return settled;
  }
  void TransferInstance::SetSettled(bool settled)
  {
    if (transfer_set_settled(m_instance, settled))
    {
      throw std::runtime_error("Failed to set settled");
    }
  }

  bool TransferInstance::GetResume() const
  {
    bool resume;
    if (transfer_get_resume(m_instance, &resume))
    {
      throw std::runtime_error("Failed to get resume");
    }
    return resume;
  }
  void TransferInstance::SetResume(bool resume)
  {
    if (transfer_set_resume(m_instance, resume))
    {
      throw std::runtime_error("Failed to set resume");
    }
  }

  ReceiverSettleMode TransferInstance::GetReceiverSettleMode() const
  {
    receiver_settle_mode receiverMode;
    if (transfer_get_rcv_settle_mode(m_instance, &receiverMode))
    {
      throw std::runtime_error("Failed to get settled");
    }
    switch (receiverMode)
    {
      case receiver_settle_mode_first:
        return ReceiverSettleMode::First;
      case receiver_settle_mode_second:
        return ReceiverSettleMode::Second;
      default:
        throw std::logic_error("Unknown settle mode.");
    }
  }
  void TransferInstance::SetReceiverSettleMode(ReceiverSettleMode settleMode)
  {
    receiver_settle_mode receiverMode;
    switch (settleMode)
    {
      case ReceiverSettleMode::First:
        receiverMode = receiver_settle_mode_first;
        break;
      case ReceiverSettleMode::Second:
        receiverMode = receiver_settle_mode_second;
        break;
      default:
        throw std::logic_error("Unknown settle mode.");
    }
    if (transfer_set_rcv_settle_mode(m_instance, receiverMode))
    {
      throw std::runtime_error("Failed to set settled");
    }
  }

  bool TransferInstance::GetAborted() const
  {
    bool aborted;
    if (transfer_get_aborted(m_instance, &aborted))
    {
      throw std::runtime_error("Failed to get aborted");
    }
    return aborted;
  }
  void TransferInstance::SetAborted(bool aborted)
  {
    if (transfer_set_aborted(m_instance, aborted))
    {
      throw std::runtime_error("Failed to set settled");
    }
  }

  Azure::Core::Amqp::Models::Value TransferInstance::GetState() const
  {
    AMQP_VALUE value;
    if (transfer_get_state(m_instance, &value))
    {
      throw std::runtime_error("Failed to get state");
    }
    return value;
  }
  void TransferInstance::SetState(Azure::Core::Amqp::Models::Value const& state)
  {
    if (transfer_set_state(m_instance, state))
    {
      throw std::runtime_error("Failed to set state");
    }
  }

  std::ostream& operator<<(std::ostream& os, TransferInstance const& instance)
  {
    os << "TransferInstance {"
       << "aborted=" << instance.GetAborted() << ", "
       << "batchable=" << instance.GetBatchable() << ", "
       << "deliveryId=" << instance.GetDeliveryId() << "deliveryTag=" << instance.GetDeliveryTag()
       << ", "
       << "handle=" << instance.GetHandle() << ", "
       << "messageFormat =" << instance.GetMessageFormat() << ", "
       << "more=" << instance.GetMore() << "resume=" << instance.GetResume() << ", "
       << "settled=" << instance.GetSettled() << ", "
       << "settleMode=" << static_cast<int>(instance.GetReceiverSettleMode()) << ", "
       << "state=" << instance.GetState() << "}";
    return os;
  }
}}}}} // namespace Azure::Core::_internal::Amqp::Models