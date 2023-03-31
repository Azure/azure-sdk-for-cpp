// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "cancellable.hpp"
#include "claims_based_security.hpp"
#include "common/async_operation_queue.hpp"
#include "connection.hpp"
#include "link.hpp"
#include "models/amqp_message.hpp"
#include "models/amqp_value.hpp"
#include <azure/core/nullable.hpp>
#include <tuple>

namespace Azure { namespace Core { namespace Amqp {
  namespace _detail {
    class MessageSenderImpl;
  }
  namespace _internal {
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

    class MessageSender;
    struct MessageSenderEvents
    {
      virtual void OnMessageSenderStateChanged(
          MessageSender const& sender,
          MessageSenderState newState,
          MessageSenderState oldState)
          = 0;
    };

    struct MessageSenderOptions
    {
      std::string Name;
      SenderSettleMode SettleMode{};
      std::string SourceAddress;
      std::vector<std::string> AuthenticationScopes;
      Azure::Nullable<uint64_t> MaxMessageSize;
      bool EnableTrace{false};
      Azure::Nullable<uint32_t> InitialDeliveryCount;

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
      ExpiryPolicy TargetExpiryPolicy{};
      std::chrono::seconds TargetExpiryTimeout{std::chrono::seconds(0)};
    };

    class MessageSender {
    public:
      using MessageSendCompleteCallback = std::function<void(
          MessageSendResult sendResult,
          Azure::Core::Amqp::Models::Value const& deliveryState)>;

      MessageSender(
          Session const& session,
          std::string const& target,
          Connection const& connectionToPoll,
          MessageSenderOptions const& options,
          MessageSenderEvents* events);

      /** @brief Specialization of MessageSender class intended for use in a Message receiving
       * handler.
       */
      MessageSender(
          Session const& session,
          LinkEndpoint& newLinkEndpoint,
          std::string const& target,
          Connection const& connectionToPoll,
          MessageSenderOptions const& options,
          MessageSenderEvents* events);
      MessageSender(
          Session const& session,
          std::shared_ptr<ServiceBusSasConnectionStringCredential> credential,
          std::string const& target,
          Connection const& connectionToPoll,
          MessageSenderOptions const& options,
          MessageSenderEvents* events);
      MessageSender(
          Session const& session,
          std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
          std::string const& target,
          Connection const& connectionToPoll,
          MessageSenderOptions const& options,
          MessageSenderEvents* events);
      MessageSender(std::shared_ptr<Azure::Core::Amqp::_detail::MessageSenderImpl> sender)
          : m_impl{sender}
      {
      }
      virtual ~MessageSender() noexcept;

      MessageSender() = default;
      MessageSender(MessageSender const&) = default;
      MessageSender& operator=(MessageSender const&) = default;
      MessageSender(MessageSender&&) noexcept = default;
      MessageSender& operator=(MessageSender&&) noexcept = default;

      operator bool() const;

      void Open();
      void Close();
      std::tuple<MessageSendResult, Azure::Core::Amqp::Models::Value> Send(
          Azure::Core::Amqp::Models::Message const& message,
          Azure::Core::Context context = {});
      void SendAsync(
          Azure::Core::Amqp::Models::Message const& message,
          MessageSendCompleteCallback onSendComplete,
          Azure::Core::Context context = {});
      void SetTrace(bool traceEnabled);

    private:
      std::shared_ptr<Azure::Core::Amqp::_detail::MessageSenderImpl> m_impl;
    };
  } // namespace _internal
}}} // namespace Azure::Core::Amqp
