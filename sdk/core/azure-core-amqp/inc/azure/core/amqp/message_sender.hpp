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

struct MESSAGE_SENDER_INSTANCE_TAG;
enum MESSAGE_SENDER_STATE_TAG : int;
struct AMQP_VALUE_DATA_TAG;
struct MESSAGE_INSTANCE_TAG;
namespace Azure { namespace Core { namespace Credentials {
  class TokenCredential;
}}} // namespace Azure::Core::Credentials

namespace Azure { namespace Core { namespace _internal { namespace Amqp {

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
    SenderSettleMode SenderSettleMode{};
    std::string SourceAddress;
    std::vector<std::string> AuthenticationScopes;
    uint32_t MaxMessageSize{};
    bool EnableTrace{false};

    // Copied from Go, not sure if they're needed.
    std::vector<std::string> Capabilities;
    LinkDurability Durability{};
    bool DynamicAddress{false};
    ExpiryPolicy ExpiryPolicy{};
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

    MessageSender(MessageSender const&) = delete;
    MessageSender& operator=(MessageSender const&) = delete;
    MessageSender(MessageSender&&) noexcept = delete;
    MessageSender& operator=(MessageSender&&) noexcept = delete;

    void Open();
    void Close();
    std::tuple<MessageSendResult, Azure::Core::Amqp::Models::Value> Send(
        Azure::Core::Amqp::Models::Message const& message);
    void SendAsync(
        Azure::Core::Amqp::Models::Message const& message,
        MessageSendCompleteCallback onSendComplete);
    void SetTrace(bool traceEnabled);

  private:
    static void OnMessageSenderStateChangedFn(
        void* context,
        MESSAGE_SENDER_STATE_TAG newState,
        MESSAGE_SENDER_STATE_TAG oldState);

    virtual void OnStateChanged(MessageSenderState newState, MessageSenderState oldState);
    void Authenticate(CredentialType type, std::string const& audience, std::string const& token);

    MESSAGE_SENDER_INSTANCE_TAG* m_messageSender{};
    std::unique_ptr<_detail::Link> m_link;

    Common::AsyncOperationQueue<Azure::Core::Amqp::Models::Message> m_messageQueue;
    Common::AsyncOperationQueue<MessageSenderState, MessageSenderState> m_stateChangeQueue;

    Connection const& m_connection;
    Session const& m_session;
    std::shared_ptr<ConnectionStringCredential> m_connectionCredential;
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_tokenCredential;
    std::unique_ptr<Cbs> m_claimsBasedSecurity;
    std::string m_target;
    MessageSenderOptions m_options;
  };
}}}} // namespace Azure::Core::_internal::Amqp