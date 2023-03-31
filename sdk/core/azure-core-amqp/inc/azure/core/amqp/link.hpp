// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "models/amqp_value.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>
struct LINK_INSTANCE_TAG;
namespace Azure { namespace Core { namespace Amqp {
  namespace _internal {
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

    enum class SessionRole
    {
      Sender,
      Receiver
    };
  } // namespace _internal

  namespace _detail {
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

    class Error;
    class Link;

    class Link final {
    public:
      using OnLinkDetachReceived = std::function<void(Error& error)>;

      Link(
          _internal::Session const& session,
          std::string const& name,
          _internal::SessionRole role,
          std::string const& source,
          std::string const& target);
      Link(
          _internal::Session const& session,
          _internal::LinkEndpoint& linkEndpoint,
          std::string const& name,
          _internal::SessionRole role,
          std::string const& source,
          std::string const& target);
      Link(std::shared_ptr<LinkImpl> impl) : m_impl{impl} {}
      ~Link() noexcept;

      Link(Link const&) = default;
      Link& operator=(Link const&) = default;
      Link(Link&&) noexcept = default;
      Link& operator=(Link&&) noexcept = default;

      operator LINK_INSTANCE_TAG*() const;
      std::shared_ptr<_detail::LinkImpl> GetImpl() const { return m_impl; }

      void SetSenderSettleMode(_internal::SenderSettleMode senderSettleMode);
      _internal::SenderSettleMode GetSenderSettleMode() const;

      void SetReceiverSettleMode(_internal::ReceiverSettleMode receiverSettleMode);
      _internal::ReceiverSettleMode GetReceiverSettleMode() const;

      void SetInitialDeliveryCount(uint32_t initialDeliveryCount);
      uint32_t GetInitialDeliveryCount() const;

      void SetMaxMessageSize(uint64_t maxMessageSize);
      uint64_t GetMaxMessageSize() const;

      uint64_t GetPeerMaxMessageSize() const;

      void SetAttachProperties(Azure::Core::Amqp::Models::AmqpValue attachProperties);
      void SetMaxLinkCredit(uint32_t maxLinkCredit);

      std::string GetName() const;

      std::string const& GetTarget() const;
      std::string const& GetSource() const;

      uint32_t GetReceivedMessageId() const;

      void Attach();

      void Detach(
          bool close,
          std::string const& errorCondition,
          std::string const& errorDescription,
          Azure::Core::Amqp::Models::AmqpValue& info);

    private:
      std::shared_ptr<Azure::Core::Amqp::_detail::LinkImpl> m_impl;
    };
  } // namespace _detail
}}} // namespace Azure::Core::Amqp
