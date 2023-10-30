// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/models/message_source.hpp"
#include "azure/core/amqp/internal/models/message_target.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  class Session;
  class LinkEndpoint;
  enum class SenderSettleMode;
  enum class ReceiverSettleMode;

  enum class LinkDurability
  {
    None,
    Configuration,
    UnsettledState,
  };

  enum class SessionRole
  {
    Sender,
    Receiver,
  };
}}}} // namespace Azure::Core::Amqp::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  class LinkImpl;

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
    Busy,
  };

  enum class LinkDeliverySettleReason
  {
    DispositionReceived,
    Settled,
    NotDelivered,
    Timeout,
    Cancelled,
  };

#if defined(TESTING_BUILD)
  class Link final {
  public:
    Link(
        _internal::Session const& session,
        std::string const& name,
        _internal::SessionRole role,
        Models::_internal::MessageSource const& source,
        Models::_internal::MessageTarget const& target);
    Link(
        _internal::Session const& session,
        _internal::LinkEndpoint& linkEndpoint,
        std::string const& name,
        _internal::SessionRole role,
        Models::_internal::MessageSource const& source,
        Models::_internal::MessageTarget const& target);
    ~Link() noexcept;

    Link(Link const&) = default;
    Link& operator=(Link const&) = default;
    Link(Link&&) noexcept = default;
    Link& operator=(Link&&) noexcept = default;

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

    std::string GetName() const;

    Models::_internal::MessageTarget const& GetTarget() const;
    Models::_internal::MessageSource const& GetSource() const;

    uint32_t GetReceivedMessageId() const;

    void Attach();

    void Detach(
        bool close,
        std::string const& errorCondition,
        std::string const& errorDescription,
        Models::AmqpValue& info);

  private:
    Link(std::shared_ptr<LinkImpl> impl) : m_impl{impl} {}

    std::shared_ptr<LinkImpl> m_impl;
  };
#endif // defined(TESTING_BUILD)
}}}} // namespace Azure::Core::Amqp::_detail
