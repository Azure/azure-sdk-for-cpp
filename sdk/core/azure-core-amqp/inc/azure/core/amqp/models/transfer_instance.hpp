// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include "amqp_value.hpp"
#include <chrono>
#include <exception>
#include <stdexcept>

struct TRANSFER_INSTANCE_TAG;
namespace Azure { namespace Core { namespace _internal { namespace Amqp {
  enum class ReceiverSettleMode;

  namespace Models {

    class TransferInstance {
      TRANSFER_INSTANCE_TAG* m_instance;

    public:
      // uAMQP integration functions.
      TransferInstance(TRANSFER_INSTANCE_TAG* instance) : m_instance(instance) {}
      operator TRANSFER_INSTANCE_TAG*() const { return m_instance; }

    public:
      TransferInstance();
      operator bool() const { return m_instance != nullptr; }

      virtual ~TransferInstance();

      //    bool IsTransferTypeByDescriptor(Value const& value);
      uint32_t GetHandle() const;
      void SetHandle(uint32_t handle);

      uint32_t GetDeliveryId() const;
      void SetDeliveryId(uint32_t value);

      Azure::Core::Amqp::Models::BinaryData GetDeliveryTag() const;
      void SetDeliveryTag(Azure::Core::Amqp::Models::BinaryData value);

      uint32_t GetMessageFormat() const;
      void SetMessageFormat(uint32_t format);

      bool GetSettled() const;
      void SetSettled(bool isSettled);

      bool GetMore() const;
      void SetMore(bool isMore);

      ReceiverSettleMode GetReceiverSettleMode() const;
      void SetReceiverSettleMode(ReceiverSettleMode settleMode);

      Azure::Core::Amqp::Models::Value GetState() const;
      void SetState(Azure::Core::Amqp::Models::Value const&);

      bool GetResume() const;
      void SetResume(bool isResume);

      bool GetAborted() const;
      void SetAborted(bool isAborted);

      bool GetBatchable() const;
      void SetBatchable(bool isBatchable);

      friend std::ostream& operator<<(std::ostream&, TransferInstance const&);
    };
#if 0
    MOCKABLE_FUNCTION(, TRANSFER_HANDLE, transfer_create , handle, handle_value);
    MOCKABLE_FUNCTION(, TRANSFER_HANDLE, transfer_clone, TRANSFER_HANDLE, value);
    MOCKABLE_FUNCTION(, void, transfer_destroy, TRANSFER_HANDLE, transfer);
    MOCKABLE_FUNCTION(, int, amqpvalue_get_transfer, AMQP_VALUE, value, TRANSFER_HANDLE*, TRANSFER_handle);
    MOCKABLE_FUNCTION(, AMQP_VALUE, amqpvalue_create_transfer, TRANSFER_HANDLE, transfer);
#endif
  } // namespace Models
}}}} // namespace Azure::Core::_internal::Amqp
