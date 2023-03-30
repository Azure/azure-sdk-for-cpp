// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "azure/core/amqp/models/amqp_value.hpp"
#include <azure_uamqp_c/link.h>
#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace _detail {

  class LinkImpl final : public std::enable_shared_from_this<LinkImpl> {
  public:
    using OnLinkDetachReceived = std::function<void(Error& error)>;

    LinkImpl(
        Session const& session,
        std::string const& name,
        SessionRole role,
        std::string const& source,
        std::string const& target);
    LinkImpl(
        Session const& session,
        LinkEndpoint& linkEndpoint,
        std::string const& name,
        SessionRole role,
        std::string const& source,
        std::string const& target);
    ~LinkImpl() noexcept;

    LinkImpl(LinkImpl const&) = delete;
    LinkImpl& operator=(LinkImpl const&) = delete;
    LinkImpl(LinkImpl&&) noexcept = delete;
    LinkImpl& operator=(LinkImpl&&) noexcept = delete;

    operator LINK_HANDLE() const { return m_link; }

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

    void Attach();

    void Detach(
        bool close,
        std::string const& errorCondition,
        std::string const& errorDescription,
        Azure::Core::Amqp::Models::Value& info);

  private:
    LINK_HANDLE m_link;
    Session const& m_session;
    std::string m_source;
    std::string m_target;

#if 0
MOCKABLE_FUNCTION(, int, link_send_disposition, LINK_HANDLE, link, delivery_number, message_number, AMQP_VALUE, delivery_state);
MOCKABLE_FUNCTION(, ASYNC_OPERATION_HANDLE, link_transfer_async, LINK_HANDLE, handle, message_format, message_format, PAYLOAD*, payloads, size_t, payload_count, ON_DELIVERY_SETTLED, on_delivery_settled, void*, callback_context, LINK_TRANSFER_RESULT*, link_transfer_result,tickcounter_ms_t, timeout);
MOCKABLE_FUNCTION(, ON_LINK_DETACH_EVENT_SUBSCRIPTION_HANDLE, link_subscribe_on_link_detach_received, LINK_HANDLE, link, ON_LINK_DETACH_RECEIVED, on_link_detach_received, void*, context);
MOCKABLE_FUNCTION(, void, link_unsubscribe_on_link_detach_received, ON_LINK_DETACH_EVENT_SUBSCRIPTION_HANDLE, event_subscription);

#endif
  };
}}}}} // namespace Azure::Core::_internal::Amqp::_detail
