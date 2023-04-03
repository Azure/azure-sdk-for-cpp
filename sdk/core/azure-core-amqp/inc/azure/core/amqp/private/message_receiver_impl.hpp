// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "azure/core/amqp/message_receiver.hpp"
#include "azure/core/amqp/private/message_receiver_impl.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure_uamqp_c/amqpvalue.h>
#include <azure_uamqp_c/message.h>
#include <azure_uamqp_c/message_receiver.h>
#include <memory>
#include <vector>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  class MessageReceiverImpl final : public std::enable_shared_from_this<MessageReceiverImpl> {
  public:
    MessageReceiverImpl(
        _internal::Session& session,
        std::string const& receiverSource,
        _internal::MessageReceiverOptions const& options,
        _internal::MessageReceiverEvents* receiverEvents = nullptr);
    MessageReceiverImpl(
        _internal::Session& session,
        _internal::Connection const& connectionToPoll,
        std::shared_ptr<_internal::ConnectionStringCredential> credentials,
        std::string const& receiverSource,
        _internal::MessageReceiverOptions const& options,
        _internal::MessageReceiverEvents* receiverEvents = nullptr);
    MessageReceiverImpl(
        _internal::Session& session,
        _internal::Connection const& connectionToPoll,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credentials,
        std::string const& receiverSource,
        _internal::MessageReceiverOptions const& options,
        _internal::MessageReceiverEvents* receiverEvents = nullptr);
    MessageReceiverImpl(
        _internal::Session const& session,
        _internal::LinkEndpoint& linkEndpoint,
        std::string const& receiverSource,
        _internal::MessageReceiverOptions const& options,
        _internal::MessageReceiverEvents* receiverEvents = nullptr);
    ~MessageReceiverImpl() noexcept;

    MessageReceiverImpl(MessageReceiverImpl const&) = delete;
    MessageReceiverImpl& operator=(MessageReceiverImpl const&) = delete;
    MessageReceiverImpl(MessageReceiverImpl&&) = delete;
    MessageReceiverImpl& operator=(MessageReceiverImpl&&) = delete;

    operator bool() const { return (m_messageReceiver != nullptr); }

    void Open();
    void Close();
    std::string GetLinkName() const;
    uint32_t GetReceivedMessageId();
    void SendMessageDisposition(
        const char* linkName,
        uint32_t messageNumber,
        Azure::Core::Amqp::Models::AmqpValue deliveryState);
    void SetTrace(bool traceEnabled);

    //    Models::Message WaitForIncomingMessage();
    template <class... Waiters>
    Azure::Core::Amqp::Models::Message WaitForIncomingMessage(
        Azure::Core::Context context,
        Waiters&... waiters)
    {
      auto result = m_messageQueue.WaitForPolledResult(context, waiters...);
      if (result)
      {
        return std::move(std::get<0>(*result));
      }
      return nullptr;
    }

  private:
    MESSAGE_RECEIVER_HANDLE m_messageReceiver{};
    std::unique_ptr<_detail::Link> m_link;
    _internal::MessageReceiverOptions m_options;
    std::string m_source;
    _internal::Session const& m_session;
    _internal::Connection const* m_connection;
    std::shared_ptr<_internal::ConnectionStringCredential> m_connectionCredential;
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_tokenCredential;
    std::unique_ptr<ClaimsBasedSecurity> m_claimsBasedSecurity;

    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<Azure::Core::Amqp::Models::Message>
        m_messageQueue;

    _internal::MessageReceiverEvents* m_eventHandler{};

    static AMQP_VALUE OnMessageReceivedFn(const void* context, MESSAGE_HANDLE message);

    virtual Azure::Core::Amqp::Models::AmqpValue OnMessageReceived(
        Azure::Core::Amqp::Models::Message message);

    static void OnMessageReceiverStateChangedFn(
        const void* context,
        MESSAGE_RECEIVER_STATE newState,
        MESSAGE_RECEIVER_STATE oldState);

    void CreateLink();
    void CreateLink(_internal::LinkEndpoint& endpoint);
    void PopulateLinkProperties();

    void Authenticate(
        _internal::CredentialType type,
        std::string const& audience,
        std::string const& token);
  };
}}}} // namespace Azure::Core::Amqp::_detail
