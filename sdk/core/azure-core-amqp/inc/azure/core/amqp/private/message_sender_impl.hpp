// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "azure/core/amqp/message_sender.hpp"
#include <azure_uamqp_c/message_sender.h>
#include <tuple>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace _detail {

  class MessageSenderImpl : public std::enable_shared_from_this<MessageSenderImpl> {
  public:
    MessageSenderImpl(
        Session const& session,
        std::string const& target,
        Connection const& connectionToPoll,
        MessageSenderOptions const& options,
        MessageSenderEvents* events);
    MessageSenderImpl(
        Session const& session,
        LinkEndpoint& endpoint,
        std::string const& target,
        Connection const& connectionToPoll,
        MessageSenderOptions const& options,
        MessageSenderEvents* events);
    MessageSenderImpl(
        Session const& session,
        std::shared_ptr<ServiceBusSasConnectionStringCredential> credential,
        std::string const& target,
        Connection const& connectionToPoll,
        MessageSenderOptions const& options,
        MessageSenderEvents* events);
    MessageSenderImpl(
        Session const& session,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
        std::string const& target,
        Connection const& connectionToPoll,
        MessageSenderOptions const& options,
        MessageSenderEvents* events);
    virtual ~MessageSenderImpl() noexcept;

    MessageSenderImpl(MessageSenderImpl const&) = delete;
    MessageSenderImpl& operator=(MessageSenderImpl const&) = delete;
    MessageSenderImpl(MessageSenderImpl&&) noexcept = delete;
    MessageSenderImpl& operator=(MessageSenderImpl&&) noexcept = delete;

    operator bool() const { return (m_messageSender != nullptr); }

    void Open();
    void Close();
    std::tuple<MessageSendResult, Azure::Core::Amqp::Models::Value> Send(
        Azure::Core::Amqp::Models::Message const& message,
        Azure::Core::Context context);
    void SendAsync(
        Azure::Core::Amqp::Models::Message const& message,
        Azure::Core::_internal::Amqp::MessageSender::MessageSendCompleteCallback onSendComplete,
        Azure::Core::Context context);
    void SetTrace(bool traceEnabled);

  private:
    static void OnMessageSenderStateChangedFn(
        void* context,
        MESSAGE_SENDER_STATE newState,
        MESSAGE_SENDER_STATE oldState);

    void Authenticate(CredentialType type, std::string const& audience, std::string const& token);
    void CreateLink();
    void CreateLink(LinkEndpoint& endpoint);
    void PopulateLinkProperties();

    MESSAGE_SENDER_HANDLE m_messageSender{};
    std::unique_ptr<_detail::Link> m_link;
    MessageSenderEvents* m_events;

    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<Azure::Core::Amqp::Models::Message>
        m_messageQueue;

    Connection const& m_connection;
    Session const& m_session;
    std::shared_ptr<ConnectionStringCredential> m_connectionCredential;
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_tokenCredential;
    std::unique_ptr<ClaimsBasedSecurity> m_claimsBasedSecurity;
    std::string m_target;
    MessageSenderOptions m_options;
  };
}}}}} // namespace Azure::Core::_internal::Amqp::_detail
