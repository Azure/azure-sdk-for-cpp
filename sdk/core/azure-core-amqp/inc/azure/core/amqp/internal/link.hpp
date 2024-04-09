// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "azure/core/amqp/internal/models/message_source.hpp"
#include "azure/core/amqp/internal/models/message_target.hpp"
#include "azure/core/amqp/internal/models/performatives/amqp_transfer.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include "azure/core/context.hpp"

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
    HalfAttachedAttachReceived,
    Attached,
    Error,
  };

  std::ostream& operator<<(std::ostream& stream, LinkState linkState);

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
    Invalid
  };

#if defined(_azure_TESTING_BUILD)
  class Link;
  class LinkImplEvents;
  class LinkImplEventsImpl;

  class LinkEvents {
  public:
    virtual Models::AmqpValue OnTransferReceived(
        Link const& link,
        Models::_internal::Performatives::AmqpTransfer transfer,
        uint32_t payloadSize,
        const unsigned char* payloadBytes)
        = 0;
    virtual void OnLinkStateChanged(
        Link const& link,
        LinkState newLinkState,
        LinkState previousLinkState)
        = 0;
    virtual void OnLinkFlowOn(Link const& link) = 0;
    virtual ~LinkEvents() = default;
  };

  class Link final {
  public:
    Link(
        _internal::Session const& session,
        std::string const& name,
        _internal::SessionRole role,
        Models::_internal::MessageSource const& source,
        Models::_internal::MessageTarget const& target,
        LinkEvents* events = nullptr);
    Link(
        _internal::Session const& session,
        _internal::LinkEndpoint& linkEndpoint,
        std::string const& name,
        _internal::SessionRole role,
        Models::_internal::MessageSource const& source,
        Models::_internal::MessageTarget const& target,
        LinkEvents* events = nullptr);
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

    void SetAttachProperties(Models::AmqpValue const& attachProperties);
    void SetMaxLinkCredit(uint32_t maxLinkCredit);

    void SetDesiredCapabilities(Models::AmqpValue const& desiredCapabilities);
    Models::AmqpValue GetDesiredCapabilities() const;

    void ResetLinkCredit(std::uint32_t linkCredit, bool drain);

    std::string GetName() const;

    Models::_internal::MessageTarget const& GetTarget() const;
    Models::_internal::MessageSource const& GetSource() const;

    uint32_t GetReceivedMessageId() const;

    void Attach();

    std::tuple<uint32_t, LinkDeliverySettleReason, Models::AmqpValue> Transfer(
        std::vector<uint8_t> const& payload,
        Azure::Core::Context const& context);

    void Detach(
        bool close,
        std::string const& errorCondition,
        std::string const& errorDescription,
        const Models::AmqpValue& info);

  private:
    friend class LinkImpl;
    friend class LinkImplEventsImpl;
    Link(std::shared_ptr<LinkImpl> impl) : m_impl{impl} {}
    std::shared_ptr<LinkImplEvents> m_implEvents;
    std::shared_ptr<LinkImpl> m_impl;
  };
#endif // _azure_TESTING_BUILD
}}}} // namespace Azure::Core::Amqp::_detail
