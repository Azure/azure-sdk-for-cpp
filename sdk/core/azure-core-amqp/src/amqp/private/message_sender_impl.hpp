// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "azure/core/amqp/message_sender.hpp"
#include "claims_based_security_impl.hpp"
#include "link_impl.hpp"
#include <azure_uamqp_c/message_sender.h>
#include <tuple>

template <> struct Azure::Core::_internal::UniqueHandleHelper<MESSAGE_SENDER_INSTANCE_TAG>
{
  static void FreeMessageSender(MESSAGE_SENDER_HANDLE obj);

  using type
      = Azure::Core::_internal::BasicUniqueHandle<MESSAGE_SENDER_INSTANCE_TAG, FreeMessageSender>;
};

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  using UniqueMessageSender = Azure::Core::_internal::UniqueHandle<MESSAGE_SENDER_INSTANCE_TAG>;

  class MessageSenderFactory {
  public:
    static Azure::Core::Amqp::_internal::MessageSender CreateFromInternal(
        std::shared_ptr<MessageSenderImpl> senderImpl)
    {
      return Azure::Core::Amqp::_internal::MessageSender(senderImpl);
    }
  };

  class MessageSenderImpl : public std::enable_shared_from_this<MessageSenderImpl> {
  public:
    MessageSenderImpl(
        std::shared_ptr<_detail::SessionImpl> session,
        Models::_internal::MessageTarget const& target,
        _internal::MessageSenderOptions const& options,
        _internal::MessageSenderEvents* events);
    MessageSenderImpl(
        std::shared_ptr<_detail::SessionImpl> session,
        _internal::LinkEndpoint& endpoint,
        Models::_internal::MessageTarget const& target,
        _internal::MessageSenderOptions const& options,
        _internal::MessageSenderEvents* events);
    MessageSenderImpl(
        std::shared_ptr<_detail::SessionImpl> session,
        std::shared_ptr<_internal::ServiceBusSasConnectionStringCredential> credential,
        Models::_internal::MessageTarget const& target,
        _internal::MessageSenderOptions const& options,
        _internal::MessageSenderEvents* events);
    MessageSenderImpl(
        std::shared_ptr<_detail::SessionImpl> session,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
        Models::_internal::MessageTarget const& target,
        _internal::MessageSenderOptions const& options,
        _internal::MessageSenderEvents* events);
    virtual ~MessageSenderImpl() noexcept;

    MessageSenderImpl(MessageSenderImpl const&) = delete;
    MessageSenderImpl& operator=(MessageSenderImpl const&) = delete;
    MessageSenderImpl(MessageSenderImpl&&) noexcept = delete;
    MessageSenderImpl& operator=(MessageSenderImpl&&) noexcept = delete;

    void Open(Azure::Core::Context const& context);
    void Close();
    std::tuple<_internal::MessageSendStatus, Azure::Core::Amqp::Models::AmqpValue> Send(
        Azure::Core::Amqp::Models::AmqpMessage const& message,
        Azure::Core::Context context);
    void QueueSend(
        Azure::Core::Amqp::Models::AmqpMessage const& message,
        Azure::Core::Amqp::_internal::MessageSender::MessageSendCompleteCallback onSendComplete,
        Azure::Core::Context context);

  private:
    static void OnMessageSenderStateChangedFn(
        void* context,
        MESSAGE_SENDER_STATE newState,
        MESSAGE_SENDER_STATE oldState);

    void Authenticate(
        _internal::CredentialType type,
        std::string const& audience,
        std::string const& token,
        Azure::Core::Context const& context);
    void CreateLink();
    void CreateLink(_internal::LinkEndpoint& endpoint);
    void PopulateLinkProperties();

    UniqueMessageSender m_messageSender{};
    std::shared_ptr<_detail::LinkImpl> m_link;
    _internal::MessageSenderEvents* m_events;

    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
        Azure::Core::Amqp::Models::AmqpMessage>
        m_messageQueue;

    std::shared_ptr<_detail::SessionImpl> m_session;
    std::shared_ptr<_internal::ConnectionStringCredential> m_connectionCredential;
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_tokenCredential;
    std::unique_ptr<ClaimsBasedSecurityImpl> m_claimsBasedSecurity;
    Models::_internal::MessageTarget m_target;
    _internal::MessageSenderOptions m_options;
    bool m_cbsOpen{false};
  };
}}}} // namespace Azure::Core::Amqp::_detail
