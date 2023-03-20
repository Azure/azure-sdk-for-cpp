// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#pragma once

#include "azure/core/amqp/message_receiver.hpp"
#include "azure/core/amqp/private/message_receiver_impl.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure_uamqp_c/amqpvalue.h>
#include <azure_uamqp_c/message.h>
#include <azure_uamqp_c/message_receiver.h>
#include <memory>
#include <vector>

namespace Azure { namespace Core { namespace _internal { namespace Amqp { namespace _detail {
  class MessageReceiverImpl final : public std::enable_shared_from_this<MessageReceiverImpl> {
  public:
    MessageReceiverImpl(
        Session& session,
        std::string const& receiverSource,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* receiverEvents = nullptr);
    MessageReceiverImpl(
        Session& session,
        Connection const& connectionToPoll,
        std::shared_ptr<ConnectionStringCredential> credentials,
        std::string const& receiverSource,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* receiverEvents = nullptr);
    MessageReceiverImpl(
        Session& session,
        Connection const& connectionToPoll,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credentials,
        std::string const& receiverSource,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* receiverEvents = nullptr);
    MessageReceiverImpl(
        Session const& session,
        LinkEndpoint& linkEndpoint,
        std::string const& receiverSource,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* receiverEvents = nullptr);
    ~MessageReceiverImpl() noexcept;

    MessageReceiverImpl(MessageReceiverImpl const&) = delete;
    MessageReceiverImpl& operator=(MessageReceiverImpl const&) = delete;
    MessageReceiverImpl(MessageReceiverImpl&&) noexcept;
    MessageReceiverImpl& operator=(MessageReceiverImpl&&) noexcept;

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
    Azure::Core::Amqp::Models::Message WaitForIncomingMessage(Azure::Core::Context context, Waiters&... waiters)
    {
      auto result = m_messageQueue.WaitForPolledResult(context, waiters...);
      return std::move(std::get<0>(*result));
    }

  private:
    MESSAGE_RECEIVER_HANDLE m_messageReceiver{};
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

    static AMQP_VALUE OnMessageReceivedFn(const void* context, MESSAGE_HANDLE message);

    virtual Azure::Core::Amqp::Models::Value OnMessageReceived(
        Azure::Core::Amqp::Models::Message message);

    static void OnMessageReceiverStateChangedFn(
        const void* context,
        MESSAGE_RECEIVER_STATE newState,
        MESSAGE_RECEIVER_STATE oldState);

    virtual void OnStateChanged(MessageReceiverState newState, MessageReceiverState oldState);

    void Authenticate(CredentialType type, std::string const& audience, std::string const& token);
  };
}}}}} // namespace Azure::Core::_internal::Amqp::_detail
