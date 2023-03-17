// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include "models/amqp_value.hpp"
#include "models/transfer_instance.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

extern "C"
{
  struct LINK_INSTANCE_TAG;
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4471)
#endif
  enum LINK_STATE_TAG;
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
}
namespace Azure { namespace Core { namespace _internal { namespace Amqp {
  class Session;
  struct LinkEndpoint;
  enum class SenderSettleMode;
  enum class ReceiverSettleMode;

  enum class LinkDurability
  {
    None,
    Configuration,
    UnsettledState
  };

  namespace _detail {

    enum class LinkState
    {
      Invalid,
      Detached,
      HalfAttachedAttachSent,
      HalfAttachAttachReceived,
      Attached,
      Error,
    };
    enum class LinkTransferResult
    {
      Error,
      Busy
    };

    enum class LinkDeliverySettleReason
    {
      DispositionReceived,
      Settled,
      NotDelivered,
      Timeout,
      Cancelled,
    };

    enum class SessionRole
    {
      Sender,
      Receiver
    };

    class Error;
    class Link;

    struct LinkEvents
    {
      virtual void OnLinkStateChanged(Link const&, LinkState newState, LinkState oldState) = 0;
      virtual Azure::Core::Amqp::Models::Value OnTransferReceived(
          Link const&,
          Models::TransferInstance& transfer,
          uint32_t payloadSize,
          const uint8_t* payloadBytes)
          = 0;
      virtual void OnLinkFlowOn(Link const&) = 0;
    };

    class Link final {
    public:
      using OnLinkDetachReceived = std::function<void(Error& error)>;

      Link(
          Session const& session,
          std::string const& name,
          SessionRole role,
          std::string const& source,
          std::string const& target);
      Link(
          Session const& session,
          LinkEndpoint& linkEndpoint,
          std::string const& name,
          SessionRole role,
          std::string const& source,
          std::string const& target);
      //    Link(LINK_INSTANCE_TAG* instance) { m_link = instance; }
      ~Link() noexcept;

      Link(Link const&) = delete;
      Link& operator=(Link const&) = delete;
      Link(Link&&) noexcept = delete;
      Link& operator=(Link&&) noexcept = delete;

      LINK_INSTANCE_TAG* Release()
      {
        auto rv = m_link;
        m_link = nullptr;
        return rv;
      }
      LINK_INSTANCE_TAG* Get() const { return m_link; }

      void SetSenderSettleMode(SenderSettleMode senderSettleMode);
      SenderSettleMode GetSenderSettleMode() const;

      void SetReceiverSettleMode(ReceiverSettleMode receiverSettleMode);
      ReceiverSettleMode GetReceiverSettleMode() const;

      void SetInitialDeliveryCount(uint32_t initialDeliveryCount);
      uint32_t GetInitialDeliveryCount() const;

      void SetMaxMessageSize(uint64_t maxMessageSize);
      uint64_t GetMaxMessageSize() const;

      uint64_t GetPeerMaxMessageSize() const;

      void SetAttachProperties(Azure::Core::Amqp::Models::Value attachProperties);
      void SetMaxLinkCredit(uint32_t maxLinkCredit);

      std::string GetName() const;

      std::string const& GetTarget() const;
      std::string const& GetSource() const;

      uint32_t GetReceivedMessageId() const;

      Session const& GetSession() const { return m_session; }

      void Attach(LinkEvents* eventHandler);

      void Detach(
          bool close,
          std::string const& errorCondition,
          std::string const& errorDescription,
          Azure::Core::Amqp::Models::Value& info);

    private:
      LINK_INSTANCE_TAG* m_link;
      Session const& m_session;
      LinkEvents* m_eventHandler{nullptr};
      std::string m_source;
      std::string m_target;

      static AMQP_VALUE_DATA_TAG* OnTransferReceivedFn(
          void* context,
          TRANSFER_INSTANCE_TAG* transfer,
          uint32_t payload_size,
          const uint8_t* payload_bytes);
      static void OnLinkStateChangedFn(
          void* context,
          LINK_STATE_TAG newState,
          LINK_STATE_TAG oldState);
      static void OnLinkFlowOnFn(void* context);

#if 0
MOCKABLE_FUNCTION(, int, link_send_disposition, LINK_HANDLE, link, delivery_number, message_number, AMQP_VALUE, delivery_state);
MOCKABLE_FUNCTION(, ASYNC_OPERATION_HANDLE, link_transfer_async, LINK_HANDLE, handle, message_format, message_format, PAYLOAD*, payloads, size_t, payload_count, ON_DELIVERY_SETTLED, on_delivery_settled, void*, callback_context, LINK_TRANSFER_RESULT*, link_transfer_result,tickcounter_ms_t, timeout);
MOCKABLE_FUNCTION(, ON_LINK_DETACH_EVENT_SUBSCRIPTION_HANDLE, link_subscribe_on_link_detach_received, LINK_HANDLE, link, ON_LINK_DETACH_RECEIVED, on_link_detach_received, void*, context);
MOCKABLE_FUNCTION(, void, link_unsubscribe_on_link_detach_received, ON_LINK_DETACH_EVENT_SUBSCRIPTION_HANDLE, event_subscription);

#endif
    };
  } // namespace _detail
}}}} // namespace Azure::Core::_internal::Amqp