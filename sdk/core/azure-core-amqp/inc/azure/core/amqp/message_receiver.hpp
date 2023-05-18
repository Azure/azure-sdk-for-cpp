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
#include <azure/core/nullable.hpp>
#include <vector>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  class MessageReceiverImpl;
  class MessageReceiverFactory;
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _internal {
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
    Models::_internal::MessageTarget MessageTarget;
    bool EnableTrace{false};
    Azure::Nullable<uint32_t> InitialDeliveryCount;
    Azure::Nullable<uint64_t> MaxMessageSize;

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
    Azure::Core::Amqp::Models::AmqpValue Properties;

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
    virtual Azure::Core::Amqp::Models::AmqpValue OnMessageReceived(
        MessageReceiver const& receiver,
        Azure::Core::Amqp::Models::AmqpMessage const& message)
        = 0;
  };

  class MessageReceiver final {
  public:
    MessageReceiver(
        Session& session,
        Models::_internal::MessageSource const& receiverSource,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* receiverEvents = nullptr);
    MessageReceiver(
        Session& session,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credentials,
        Models::_internal::MessageSource const& receiverSource,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* receiverEvents = nullptr);
    MessageReceiver(
        Session const& session,
        LinkEndpoint& linkEndpoint,
        Models::_internal::MessageSource const& receiverSource,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* receiverEvents = nullptr);

    MessageReceiver() = default;
    ~MessageReceiver() noexcept;

    MessageReceiver(MessageReceiver const&) = default;
    MessageReceiver& operator=(MessageReceiver const&) = default;
    MessageReceiver(MessageReceiver&&) = default;
    MessageReceiver& operator=(MessageReceiver&&) = default;

    operator bool() const;

    void Open(Azure::Core::Context const& context = {});
    void Close();
    std::string GetLinkName() const;
    std::string GetSourceName() const;
    uint32_t GetReceivedMessageId();
    void SendMessageDisposition(
        const char* linkName,
        uint32_t messageNumber,
        Azure::Core::Amqp::Models::AmqpValue deliveryState);

    Azure::Core::Amqp::Models::AmqpMessage WaitForIncomingMessage(
        Azure::Core::Context context = {});

  private:
    MessageReceiver(std::shared_ptr<Azure::Core::Amqp::_detail::MessageReceiverImpl> impl)
        : m_impl{impl}
    {
    }
    friend class Azure::Core::Amqp::_detail::MessageReceiverFactory;
    std::shared_ptr<Azure::Core::Amqp::_detail::MessageReceiverImpl> m_impl;
  };
}}}} // namespace Azure::Core::Amqp::_internal
