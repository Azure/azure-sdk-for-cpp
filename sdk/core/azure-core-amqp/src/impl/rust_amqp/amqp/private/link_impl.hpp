// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "../private/session_impl.hpp"
#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/internal/link.hpp"
#include "azure/core/amqp/internal/models/amqp_error.hpp"
#include "azure/core/amqp/internal/models/message_source.hpp"
#include "azure/core/amqp/internal/models/message_target.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  class LinkImpl final : public std::enable_shared_from_this<LinkImpl> {
  public:
    LinkImpl(
        std::shared_ptr<_detail::SessionImpl> session,
        std::string const& name,
        _internal::SessionRole role,
        Models::_internal::MessageSource const& source,
        Models::_internal::MessageTarget const& target);
    ~LinkImpl() noexcept;

    LinkImpl(LinkImpl const&) = delete;
    LinkImpl& operator=(LinkImpl const&) = delete;
    LinkImpl(LinkImpl&&) noexcept = delete;
    LinkImpl& operator=(LinkImpl&&) noexcept = delete;

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
        Models::AmqpValue const& info);

    std::tuple<uint32_t, LinkDeliverySettleReason, Models::AmqpValue> Transfer(
        std::vector<uint8_t> const& payload,
        Azure::Core::Context const& context);

  private:
    std::shared_ptr<_detail::SessionImpl> m_session;
    Models::_internal::MessageSource m_source;
    Models::_internal::MessageTarget m_target;
    Common::_internal::AsyncOperationQueue<uint32_t, LinkDeliverySettleReason, Models::AmqpValue>
        m_transferCompleteQueue;
  };
}}}} // namespace Azure::Core::Amqp::_detail
