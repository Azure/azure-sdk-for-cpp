// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include "azure/core/amqp/message_receiver.hpp"
#include "claims_based_security_impl.hpp"
#include "connection_impl.hpp"
#include "message_receiver_impl.hpp"
#include "session_impl.hpp"

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
        std::shared_ptr<_detail::SessionImpl> session,
        std::string const& receiverSource,
        _internal::MessageReceiverOptions const& options,
        _internal::MessageReceiverEvents* receiverEvents = nullptr);
    MessageReceiverImpl(
        std::shared_ptr<_detail::SessionImpl> session,
        std::shared_ptr<_internal::ConnectionStringCredential> credentials,
        std::string const& receiverSource,
        _internal::MessageReceiverOptions const& options,
        _internal::MessageReceiverEvents* receiverEvents = nullptr);
    MessageReceiverImpl(
        std::shared_ptr<_detail::SessionImpl> session,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credentials,
        std::string const& receiverSource,
        _internal::MessageReceiverOptions const& options,
        _internal::MessageReceiverEvents* receiverEvents = nullptr);
    MessageReceiverImpl(
        std::shared_ptr<_detail::SessionImpl> session,
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

    void Open(Azure::Core::Context const& context);
    void Close();
    std::string GetLinkName() const;
    std::string GetSourceName() const { return m_source; }
    uint32_t GetReceivedMessageId();
    void SendMessageDisposition(
        const char* linkName,
        uint32_t messageNumber,
        Azure::Core::Amqp::Models::AmqpValue deliveryState);
    void SetTrace(bool traceEnabled);

    template <class... Waiters>
    Azure::Core::Amqp::Models::AmqpMessage WaitForIncomingMessage(
        Azure::Core::Context context,
        Waiters&... waiters)
    {
      auto result = m_messageQueue.WaitForPolledResult(
          context, *m_session->GetConnectionToPoll(), waiters...);
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
    std::shared_ptr<_detail::SessionImpl> m_session;
    std::shared_ptr<_internal::ConnectionStringCredential> m_connectionCredential;
    std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_tokenCredential;
    std::unique_ptr<ClaimsBasedSecurityImpl> m_claimsBasedSecurity;
    bool m_cbsOpen{false};

    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
        Azure::Core::Amqp::Models::AmqpMessage>
        m_messageQueue;

    _internal::MessageReceiverEvents* m_eventHandler{};

    static AMQP_VALUE OnMessageReceivedFn(const void* context, MESSAGE_HANDLE message);

    virtual Azure::Core::Amqp::Models::AmqpValue OnMessageReceived(
        Azure::Core::Amqp::Models::AmqpMessage message);

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
        std::string const& token,
        Azure::Core::Context const& context);
  };
}}}} // namespace Azure::Core::Amqp::_detail
