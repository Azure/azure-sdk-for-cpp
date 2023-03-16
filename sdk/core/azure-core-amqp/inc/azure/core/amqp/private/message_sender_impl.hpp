// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include "azure/core/amqp/message_sender.hpp"
#include <tuple>

struct MESSAGE_SENDER_INSTANCE_TAG;
enum MESSAGE_SENDER_STATE_TAG : int;
struct AMQP_VALUE_DATA_TAG;
struct MESSAGE_INSTANCE_TAG;

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace _detail {

  class MessageSenderImpl {
  public:
    using MessageSendCompleteCallback = std::function<
        void(MessageSendResult sendResult, Azure::Core::Amqp::Models::Value deliveryState)>;

    MessageSenderImpl(
        Session const& session,
        std::string const& target,
        Connection const& connectionToPoll,
        MessageSenderOptions const& options);
    MessageSenderImpl(
        Session const& session,
        std::shared_ptr<ServiceBusSasConnectionStringCredential> credential,
        std::string const& target,
        Connection const& connectionToPoll,
        MessageSenderOptions const& options);
    MessageSenderImpl(
        Session const& session,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
        std::string const& target,
        Connection const& connectionToPoll,
        MessageSenderOptions const& options);
    virtual ~MessageSenderImpl() noexcept;

    MessageSenderImpl(MessageSenderImpl const&) = delete;
    MessageSenderImpl& operator=(MessageSenderImpl const&) = delete;
    MessageSenderImpl(MessageSenderImpl&&) noexcept = delete;
    MessageSenderImpl& operator=(MessageSenderImpl&&) noexcept = delete;

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
}}}}} // namespace Azure::Core::_internal::Amqp::_detail
