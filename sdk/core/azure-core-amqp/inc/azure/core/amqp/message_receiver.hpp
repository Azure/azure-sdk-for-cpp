// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "claims_based_security.hpp"
#include "common/async_operation_queue.hpp"
#include "connection_string_credential.hpp"
#include "link.hpp"
#include "models/amqp_error.hpp"
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
    Error,
  };

  enum class ReceiverSettleMode
  {
    First,
    Second,
  };

  class MessageReceiver;

  struct MessageReceiverOptions
  {
    std::string Name;
    std::vector<std::string> AuthenticationScopes;
    ReceiverSettleMode SettleMode{ReceiverSettleMode::First};
    Models::_internal::MessageTarget MessageTarget;
    bool EnableTrace{false};
    Nullable<uint32_t> InitialDeliveryCount;
    Nullable<uint64_t> MaxMessageSize;

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
    Models::AmqpValue Properties;

    std::vector<std::string> SenderCapabilities;
    LinkDurability SenderDurability{};
    std::chrono::seconds SenderExpiryTimeout{std::chrono::seconds(0)};
  };

  class MessageReceiverEvents {
  protected:
    ~MessageReceiverEvents() = default;

  public:
    virtual void OnMessageReceiverStateChanged(
        MessageReceiver const& receiver,
        MessageReceiverState newState,
        MessageReceiverState oldState)
        = 0;
    virtual Models::AmqpValue OnMessageReceived(
        MessageReceiver const& receiver,
        Models::AmqpMessage const& message)
        = 0;
    virtual void OnMessageReceiverDisconnected(Models::_internal::AmqpError const& error) = 0;
  };

  class MessageReceiver final {
  public:
    ~MessageReceiver() noexcept;

    MessageReceiver(MessageReceiver const&) = default;
    MessageReceiver& operator=(MessageReceiver const&) = default;
    MessageReceiver(MessageReceiver&&) = default;
    MessageReceiver& operator=(MessageReceiver&&) = default;

    operator bool() const;

    void Open(Context const& context = {});
    void Close();
    std::string GetLinkName() const;
    std::string GetSourceName() const;

    std::pair<Azure::Nullable<Models::AmqpMessage>, Models::_internal::AmqpError>
    WaitForIncomingMessage(Context const& context = {});

  private:
    MessageReceiver(std::shared_ptr<_detail::MessageReceiverImpl> impl) : m_impl{impl} {}
    friend class _detail::MessageReceiverFactory;
    std::shared_ptr<_detail::MessageReceiverImpl> m_impl;
  };
}}}} // namespace Azure::Core::Amqp::_internal
