// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "claims_based_security.hpp"
#include "common/async_operation_queue.hpp"
#include "connection_string_credential.hpp"
#include "link.hpp"
#include "models/amqp_message.hpp"
#include "models/amqp_value.hpp"
#include "session.hpp"
#include <azure/core/credentials/credentials.hpp>
#include <vector>

namespace Azure { namespace Core { namespace _internal { namespace Amqp {
  namespace _detail {
    class MessageReceiverImpl;
  }

  enum class MessageReceiverState
  {
    Invalid,
    Idle,
    Opening,
    Open,
    Closing,
    Error
  };

  enum class ReceiverSettleMode
  {
    First,
    Second
  };

  class MessageReceiver;

  struct MessageReceiverOptions
  {
    std::string Name;
    std::vector<std::string> AuthenticationScopes;
    ReceiverSettleMode SettleMode{ReceiverSettleMode::First};
    std::string TargetName;
    bool EnableTrace{false};

    bool Batching{false};
    std::chrono::seconds BatchMaxAge{std::chrono::seconds(5)};
    std::vector<std::string> Capabilities;
    uint32_t Credit{1};
    LinkDurability Durability{};
    bool DynamicAddress{false};
    ExpiryPolicy SenderExpiryPolicy{};
    ExpiryPolicy ReceiverExpiryPolicy{};
    std::chrono::seconds ExpiryTimeout{std::chrono::seconds(0)};
    // LinkFilter
    bool ManualCredits{};
    uint64_t MaxMessageSize{0};
    Azure::Core::Amqp::Models::Value Properties;

    std::vector<std::string> SenderCapabilities;
    LinkDurability SenderDurability{};
    std::chrono::seconds SenderExpiryTimeout{std::chrono::seconds(0)};
  };

  struct MessageReceiverEvents
  {
    virtual void OnMessageReceiverStateChanged(
        MessageReceiver const& receiver,
        MessageReceiverState newState,
        MessageReceiverState oldState)
        = 0;
    virtual Azure::Core::Amqp::Models::Value OnMessageReceived(
        Azure::Core::Amqp::Models::Message message)
        = 0;
  };

  class MessageReceiver final {
  public:
    MessageReceiver(
        Session& session,
        std::string const& receiverSource,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* receiverEvents = nullptr);
    MessageReceiver(
        Session& session,
        Connection const& connectionToPoll,
        std::shared_ptr<ConnectionStringCredential> credentials,
        std::string const& receiverSource,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* receiverEvents = nullptr);
    MessageReceiver(
        Session& session,
        Connection const& connectionToPoll,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credentials,
        std::string const& receiverSource,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* receiverEvents = nullptr);
    MessageReceiver(
        Session const& session,
        LinkEndpoint& linkEndpoint,
        std::string const& receiverSource,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* receiverEvents = nullptr);

    MessageReceiver(std::shared_ptr<_detail::MessageReceiverImpl> impl) : m_impl{impl} {}
    ~MessageReceiver() noexcept;

    MessageReceiver(MessageReceiver const&) = default;
    MessageReceiver& operator=(MessageReceiver const&) = default;
    MessageReceiver(MessageReceiver&&) = default;
    MessageReceiver& operator=(MessageReceiver&&) = default;

    void Open();
    void Close();
    std::string GetLinkName();
    uint32_t GetReceivedMessageId();
    void SendMessageDisposition(
        const char* linkName,
        uint32_t messageNumber,
        Azure::Core::Amqp::Models::Value deliveryState);
    void SetTrace(bool traceEnabled);

    Azure::Core::Amqp::Models::Message WaitForIncomingMessage(
        Connection& connection,
        Azure::Core::Context context = {});

  private:
    std::shared_ptr<_detail::MessageReceiverImpl> m_impl;
  };
}}}} // namespace Azure::Core::_internal::Amqp