// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/internal/models/amqp_error.hpp"
#include "azure/core/amqp/internal/models/message_source.hpp"
#include "azure/core/amqp/internal/models/message_target.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"

#include <azure_uamqp_c/link.h>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  class LinkImpl final : public std::enable_shared_from_this<LinkImpl>,
                         public Common::_detail::Pollable {

    using OnLinkDetachEvent = std::function<void(Models::_internal::AmqpError)>;

  public:
    LinkImpl(
        std::shared_ptr<_detail::SessionImpl> session,
        std::string const& name,
        _internal::SessionRole role,
        Models::_internal::MessageSource const& source,
        Models::_internal::MessageTarget const& target);
    LinkImpl(
        std::shared_ptr<_detail::SessionImpl> session,
        _internal::LinkEndpoint& linkEndpoint,
        std::string const& name,
        _internal::SessionRole role,
        Models::_internal::MessageSource const& source,
        Models::_internal::MessageTarget const& target);
    ~LinkImpl() noexcept;

    LinkImpl(LinkImpl const&) = delete;
    LinkImpl& operator=(LinkImpl const&) = delete;
    LinkImpl(LinkImpl&&) noexcept = delete;
    LinkImpl& operator=(LinkImpl&&) noexcept = delete;

    operator LINK_HANDLE() const { return m_link; }

    void SetSenderSettleMode(_internal::SenderSettleMode senderSettleMode);
    _internal::SenderSettleMode GetSenderSettleMode() const;

    void SetReceiverSettleMode(_internal::ReceiverSettleMode receiverSettleMode);
    _internal::ReceiverSettleMode GetReceiverSettleMode() const;

    void SetInitialDeliveryCount(uint32_t initialDeliveryCount);
    uint32_t GetInitialDeliveryCount() const;

    void SetMaxMessageSize(uint64_t maxMessageSize);
    uint64_t GetMaxMessageSize() const;

    uint64_t GetPeerMaxMessageSize() const;

    void SetAttachProperties(Models::AmqpValue attachProperties);
    void SetMaxLinkCredit(uint32_t maxLinkCredit);

    void SetDesiredCapabilities(Models::AmqpValue desiredCapabilities);
    Models::AmqpValue GetDesiredCapabilities() const;

    /** @brief Subscribe to link detach events. */
    void SubscribeToDetachEvent(OnLinkDetachEvent onLinkDetachEvent);
    void UnsubscribeFromDetachEvent();

    std::string GetName() const;

    Models::_internal::MessageTarget const& GetTarget() const;
    Models::_internal::MessageSource const& GetSource() const;

    uint32_t GetReceivedMessageId() const;

    std::shared_ptr<_detail::SessionImpl> const& GetSession() const { return m_session; }

    void ResetLinkCredit(std::uint32_t linkCredit, bool drain);

    void Attach();

    void Detach(
        bool close,
        std::string const& errorCondition,
        std::string const& errorDescription,
        Models::AmqpValue& info);

  private:
    LINK_HANDLE m_link;
    std::shared_ptr<_detail::SessionImpl> m_session;
    Models::_internal::MessageSource m_source;
    Models::_internal::MessageTarget m_target;
    OnLinkDetachEvent m_onLinkDetachEvent;
    ON_LINK_DETACH_EVENT_SUBSCRIPTION_HANDLE m_linkSubscriptionHandle{};

    static void OnLinkDetachEventFn(void* context, ERROR_HANDLE error);

    // Inherited via Pollable
    void Poll() override;
  };
}}}} // namespace Azure::Core::Amqp::_detail
