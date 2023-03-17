// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include "cancellable.hpp"
#include "claims_based_security.hpp"
#include "common/async_operation_queue.hpp"
#include "connection.hpp"
#include "link.hpp"
#include "models/amqp_message.hpp"
#include "models/amqp_value.hpp"
#include <tuple>

namespace Azure { namespace Core { namespace _internal { namespace Amqp {
  namespace _detail {
    class MessageSenderImpl;
  }

  enum class MessageSendResult
  {
    Invalid,
    Ok,
    Error,
    Timeout,
    Cancelled
  };
  enum class MessageSenderState
  {
    Invalid,
    Idle,
    Opening,
    Open,
    Closing,
    Error
  };

  enum class SenderSettleMode
  {
    Unsettled,
    Settled,
    Mixed
  };

  struct MessageSenderOptions
  {
    std::string Name;
    SenderSettleMode SettleMode{};
    std::string SourceAddress;
    std::vector<std::string> AuthenticationScopes;
    uint32_t MaxMessageSize{};
    bool EnableTrace{false};

    // Copied from Go, not sure if they're needed.
    std::vector<std::string> Capabilities;
    LinkDurability Durability{};
    bool DynamicAddress{false};
    ExpiryPolicy SenderExpiryPolicy{};
    std::chrono::seconds ExpiryTimeout{std::chrono::seconds(0)};
    bool IgnoreDispositionErrors{false};

    Azure::Core::Amqp::Models::Value Properties;

    ReceiverSettleMode RequestedReceiverSettleMode{};

    std::vector<std::string> TargetCapabilities;
    LinkDurability TargetDurability{};
    Azure::Core::_internal::Amqp::ExpiryPolicy TargetExpiryPolicy{};
    std::chrono::seconds TargetExpiryTimeout{std::chrono::seconds(0)};
  };

  class MessageSender {
  public:
    using MessageSendCompleteCallback = std::function<
        void(MessageSendResult sendResult, Azure::Core::Amqp::Models::Value deliveryState)>;

    MessageSender(
        Session const& session,
        std::string const& target,
        Connection const& connectionToPoll,
        MessageSenderOptions const& options);
    MessageSender(
        Session const& session,
        std::shared_ptr<ServiceBusSasConnectionStringCredential> credential,
        std::string const& target,
        Connection const& connectionToPoll,
        MessageSenderOptions const& options);
    MessageSender(
        Session const& session,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
        std::string const& target,
        Connection const& connectionToPoll,
        MessageSenderOptions const& options);
    virtual ~MessageSender() noexcept;

    MessageSender(MessageSender const&) = default;
    MessageSender& operator=(MessageSender const&) = default;
    MessageSender(MessageSender&&) noexcept = default;
    MessageSender& operator=(MessageSender&&) noexcept = default;

    void Open();
    void Close();
    std::tuple<MessageSendResult, Azure::Core::Amqp::Models::Value> Send(
        Azure::Core::Amqp::Models::Message const& message);
    void SendAsync(
        Azure::Core::Amqp::Models::Message const& message,
        MessageSendCompleteCallback onSendComplete);
    void SetTrace(bool traceEnabled);

  private:
    std::shared_ptr<_detail::MessageSenderImpl> m_impl;
  };
}}}} // namespace Azure::Core::_internal::Amqp