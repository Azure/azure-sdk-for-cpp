// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

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

struct MESSAGE_RECEIVER_INSTANCE_TAG;
enum MESSAGE_RECEIVER_STATE_TAG : int;
struct AMQP_VALUE_DATA_TAG;
struct MESSAGE_INSTANCE_TAG;

namespace Azure { namespace Core { namespace _internal { namespace Amqp {

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
    ReceiverSettleMode ReceiverSettleMode{ReceiverSettleMode::First};
    std::string TargetName;
    bool EnableTrace{false};

    bool Batching{false};
    std::chrono::seconds BatchMaxAge{std::chrono::seconds(5)};
    std::vector<std::string> Capabilities;
    uint32_t Credit{1};
    LinkDurability Durability{};
    bool DynamicAddress{false};
    ExpiryPolicy SenderExpiryPolicy{};
    ExpiryPolicy ExpiryPolicy{};
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
    ~MessageReceiver() noexcept;

    MessageReceiver(MessageReceiver const&) = delete;
    MessageReceiver& operator=(MessageReceiver const&) = delete;
    MessageReceiver(MessageReceiver&&) noexcept;
    MessageReceiver& operator=(MessageReceiver&&) noexcept;

    void Open();
    void Close();
    std::string GetLinkName();
    uint32_t GetReceivedMessageId();
    void SendMessageDisposition(
        const char* linkName,
        uint32_t messageNumber,
        Azure::Core::Amqp::Models::Value deliveryState);
    void SetTrace(bool traceEnabled);

    //    Models::Message WaitForIncomingMessage();
    template <class... Waiters>
    Azure::Core::Amqp::Models::Message WaitForIncomingMessage(Waiters&... waiters)
    {
      auto result = m_messageQueue.WaitForPolledResult(waiters...);
      return std::move(std::get<0>(*result));
    }

  private:
    MESSAGE_RECEIVER_INSTANCE_TAG* m_messageReceiver{};
    std::unique_ptr<_detail::Link> m_link;
    MessageReceiverOptions m_options;
    std::string m_source;
    Session const& m_session;
    Connection const* m_connection;
    std::shared_ptr<ConnectionStringCredential> m_connectionCredential;
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_tokenCredential;
    std::unique_ptr<Cbs> m_claimsBasedSecurity;

    Common::AsyncOperationQueue<Azure::Core::Amqp::Models::Message> m_messageQueue;
    Common::AsyncOperationQueue<MessageReceiverState, MessageReceiverState> m_stateChangeQueue;

    MessageReceiverEvents* m_eventHandler{};

    static AMQP_VALUE_DATA_TAG* OnMessageReceivedFn(
        const void* context,
        MESSAGE_INSTANCE_TAG* message);

    virtual Azure::Core::Amqp::Models::Value OnMessageReceived(
        Azure::Core::Amqp::Models::Message message);

    static void OnMessageReceiverStateChangedFn(
        const void* context,
        MESSAGE_RECEIVER_STATE_TAG newState,
        MESSAGE_RECEIVER_STATE_TAG oldState);

    virtual void OnStateChanged(MessageReceiverState newState, MessageReceiverState oldState);

    void Authenticate(CredentialType type, std::string const& audience, std::string const& token);
  };
}}}} // namespace Azure::Core::_internal::Amqp