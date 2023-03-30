// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/claims_based_security.hpp"
#include "azure/core/amqp/common/completion_operation.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "azure/core/amqp/models/messaging_values.hpp"
#include "azure/core/amqp/private/message_sender_impl.hpp"
#include "azure/core/amqp/session.hpp"
#include <azure/core/credentials/credentials.hpp>

#include <azure_uamqp_c/message_sender.h>
#include <memory>

namespace Azure { namespace Core { namespace Amqp {
  using namespace Azure::Core::Amqp::_internal;

  namespace _internal {

    MessageSender::MessageSender(
        Session const& session,
        std::string const& target,
        Connection const& connection,
        MessageSenderOptions const& options,
        MessageSenderEvents* events)
        : m_impl{std::make_shared<_detail::MessageSenderImpl>(
            session,
            target,
            connection,
            options,
            events)}
    {
    }
    MessageSender::MessageSender(
        Session const& session,
        LinkEndpoint& endpoint,
        std::string const& target,
        Connection const& connection,
        MessageSenderOptions const& options,
        MessageSenderEvents* events)
        : m_impl{std::make_shared<
            _detail::MessageSenderImpl>(session, endpoint, target, connection, options, events)}
    {
    }

    MessageSender::MessageSender(
        Session const& session,
        std::shared_ptr<ServiceBusSasConnectionStringCredential> credential,
        std::string const& target,
        Connection const& connection,
        MessageSenderOptions const& options,
        MessageSenderEvents* events)
        : m_impl{std::make_shared<
            _detail::MessageSenderImpl>(session, credential, target, connection, options, events)}
    {
    }

    MessageSender::MessageSender(
        Session const& session,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
        std::string const& target,
        Connection const& connection,
        MessageSenderOptions const& options,
        MessageSenderEvents* events)
        : m_impl{std::make_shared<
            _detail::MessageSenderImpl>(session, credential, target, connection, options, events)}
    {
    }

    MessageSender::operator bool() const { return m_impl.operator bool(); }

    void MessageSender::Open() { m_impl->Open(); }
    void MessageSender::Close() { m_impl->Close(); }
    std::tuple<MessageSendResult, Azure::Core::Amqp::Models::Value> MessageSender::Send(
        Azure::Core::Amqp::Models::Message const& message,
        Azure::Core::Context context)
    {
      return m_impl->Send(message, context);
    }
    void MessageSender::SendAsync(
        Azure::Core::Amqp::Models::Message const& message,
        MessageSendCompleteCallback onSendComplete,
        Azure::Core::Context context)
    {
      return m_impl->SendAsync(message, onSendComplete, context);
    }
    void MessageSender::SetTrace(bool traceEnabled) { m_impl->SetTrace(traceEnabled); }

    MessageSender::~MessageSender() noexcept {}
  } // namespace _internal

  namespace _detail {

    MessageSenderImpl::MessageSenderImpl(
        Session const& session,
        std::string const& target,
        Connection const& connection,
        MessageSenderOptions const& options,
        MessageSenderEvents* events)
        : m_events{events},
          m_connection{connection}, m_session{session}, m_target{target}, m_options{options}
    {
    }

    MessageSenderImpl::MessageSenderImpl(
        Session const& session,
        LinkEndpoint& endpoint,
        std::string const& target,
        Connection const& connection,
        MessageSenderOptions const& options,
        MessageSenderEvents* events)
        : m_events{events},
          m_connection{connection}, m_session{session}, m_target{target}, m_options{options}
    {
      CreateLink(endpoint);
      m_messageSender
          = messagesender_create(*m_link, MessageSenderImpl::OnMessageSenderStateChangedFn, this);

      SetTrace(options.EnableTrace);
    }

    MessageSenderImpl::MessageSenderImpl(
        Session const& session,
        std::shared_ptr<ServiceBusSasConnectionStringCredential> credential,
        std::string const& target,
        Connection const& connection,
        MessageSenderOptions const& options,
        MessageSenderEvents* events)
        : m_events{events}, m_connection{connection}, m_session{session},
          m_connectionCredential{credential}, m_target{target}, m_options{options}

    {
    }

    MessageSenderImpl::MessageSenderImpl(
        Session const& session,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
        std::string const& target,
        Connection const& connection,
        MessageSenderOptions const& options,
        MessageSenderEvents* events)
        : m_events{events}, m_connection{connection}, m_session{session},
          m_tokenCredential{credential}, m_target{target}, m_options{options}
    {
    }

    MessageSenderImpl::~MessageSenderImpl() noexcept
    {
      // Clear the event callback before calling messagesender_destroy to short-circuit any
      // events firing after the object is destroyed.
      if (m_events)
      {
        m_events = nullptr;
      }
      if (m_claimsBasedSecurity)
      {
        m_claimsBasedSecurity->Close();
      }
      if (m_messageSender)
      {
        messagesender_destroy(m_messageSender);
        m_messageSender = nullptr;
      }
    }

    void MessageSenderImpl::CreateLink(LinkEndpoint& endpoint)
    {
      m_link = std::make_unique<_detail::Link>(
          m_session,
          endpoint,
          m_options.Name,
          SessionRole::Receiver, // This is the role of the link, not the endpoint.
          m_options.SourceAddress,
          m_target);
      PopulateLinkProperties();
    }
    void MessageSenderImpl::CreateLink()
    {
      m_link = std::make_unique<_detail::Link>(
          m_session,
          m_options.Name,
          SessionRole::Sender, // This is the role of the link, not the endpoint.
          m_options.SourceAddress,
          m_target);
      PopulateLinkProperties();
    }

    /* Populate link properties from options. */
    void MessageSenderImpl::PopulateLinkProperties()
    {
      // Populate link options from options.
      if (m_options.InitialDeliveryCount.HasValue())
      {
        m_link->SetInitialDeliveryCount(m_options.InitialDeliveryCount.Value());
      }
      if (m_options.MaxMessageSize.HasValue())
      {
        m_link->SetMaxMessageSize(m_options.MaxMessageSize.Value());
      }
      else
      {
        m_link->SetMaxMessageSize(std::numeric_limits<uint64_t>::max());
      }
      m_link->SetSenderSettleMode(m_options.SettleMode);
    }

    void MessageSenderImpl::SetTrace(bool traceEnabled)
    {
      messagesender_set_trace(m_messageSender, traceEnabled);
    }

    MessageSenderState MessageSenderStateFromLowLevel(MESSAGE_SENDER_STATE lowLevel)
    {
      switch (lowLevel)
      {
        case MESSAGE_SENDER_STATE_CLOSING:
          return MessageSenderState::Closing;
        case MESSAGE_SENDER_STATE_ERROR: // LCOV_EXCL_LINE
          return MessageSenderState::Error; // LCOV_EXCL_LINE
        case MESSAGE_SENDER_STATE_IDLE:
          return MessageSenderState::Idle;
        case MESSAGE_SENDER_STATE_INVALID: // LCOV_EXCL_LINE
          return MessageSenderState::Invalid; // LCOV_EXCL_LINE
        case MESSAGE_SENDER_STATE_OPEN:
          return MessageSenderState::Open;
        case MESSAGE_SENDER_STATE_OPENING:
          return MessageSenderState::Opening;
        default: // LCOV_EXCL_LINE
          throw std::logic_error("Unknown message receiver state."); // LCOV_EXCL_LINE
      }
    }

    void MessageSenderImpl::OnMessageSenderStateChangedFn(
        void* context,
        MESSAGE_SENDER_STATE newState,
        MESSAGE_SENDER_STATE oldState)
    {
      auto sender = static_cast<MessageSenderImpl*>(const_cast<void*>(context));
      if (sender->m_events)
      {
        sender->m_events->OnMessageSenderStateChanged(
            sender->shared_from_this(),
            MessageSenderStateFromLowLevel(newState),
            MessageSenderStateFromLowLevel(oldState));
      }
    }

    void MessageSenderImpl::Authenticate(
        CredentialType type,
        std::string const& audience,
        std::string const& token)
    {
      m_claimsBasedSecurity = std::make_unique<ClaimsBasedSecurity>(m_session, m_connection);
      if (m_claimsBasedSecurity->Open() == CbsOpenResult::Ok)
      {
        auto result = m_claimsBasedSecurity->PutToken(
            (type == CredentialType::BearerToken ? CbsTokenType::Jwt : CbsTokenType::Sas),
            audience,
            token);
      }
      else
      {
        throw std::runtime_error("Could not put Claims Based Security token."); // LCOV_EXCL_LINE
      }
    }

    void MessageSenderImpl::Open()
    {
      // If we need to authenticate with either ServiceBus or BearerToken, now is the time to do
      // it.
      if (m_connectionCredential)
      {
        auto sasCredential{std::static_pointer_cast<
            Azure::Core::Amqp::_internal::ServiceBusSasConnectionStringCredential>(
            m_connectionCredential)};
        Authenticate(
            sasCredential->GetCredentialType(),
            sasCredential->GetEndpoint() + sasCredential->GetEntityPath(),
            sasCredential->GenerateSasToken(
                std::chrono::system_clock::now() + std::chrono::minutes(60)));
      }
      else if (m_tokenCredential)
      {
        Azure::Core::Credentials::TokenRequestContext requestContext;
        requestContext.Scopes = m_options.AuthenticationScopes;
        Azure::Core::Context context;

        Authenticate(
            CredentialType::BearerToken,
            m_target,
            m_tokenCredential->GetToken(requestContext, context).Token);
      }

      if (m_link == nullptr)
      {
        CreateLink();
      }

      if (m_messageSender == nullptr)
      {
        m_messageSender
            = messagesender_create(*m_link, MessageSenderImpl::OnMessageSenderStateChangedFn, this);
      }
      if (messagesender_open(m_messageSender))
      {
        auto err = errno; // LCOV_EXCL_LINE
        throw std::runtime_error( // LCOV_EXCL_LINE
            "Could not open message sender. errno=" + std::to_string(err) + ", \"" // LCOV_EXCL_LINE
            + strerror(err) // LCOV_EXCL_LINE
            + "\"."); // LCOV_EXCL_LINE
      }
    }
    void MessageSenderImpl::Close()
    {
      if (messagesender_close(m_messageSender))
      {
        throw std::runtime_error("Could not close message sender"); // LCOV_EXCL_LINE
      }
    }

    template <typename CompleteFn> struct RewriteSendComplete
    {
      static void OnOperation(
          CompleteFn onComplete,
          MESSAGE_SEND_RESULT sendResult,
          AMQP_VALUE disposition)
      {
        MessageSendResult result{MessageSendResult::Ok};
        switch (sendResult)
        {
          case MESSAGE_SEND_RESULT_INVALID: // LCOV_EXCL_LINE
            result = MessageSendResult::Invalid; // LCOV_EXCL_LINE
            break; // LCOV_EXCL_LINE
          case MESSAGE_SEND_OK:
            result = MessageSendResult::Ok;
            break;
          case MESSAGE_SEND_CANCELLED: // LCOV_EXCL_LINE
            result = MessageSendResult::Cancelled; // LCOV_EXCL_LINE
            break; // LCOV_EXCL_LINE
          case MESSAGE_SEND_ERROR: // LCOV_EXCL_LINE
            result = MessageSendResult::Error; // LCOV_EXCL_LINE
            break; // LCOV_EXCL_LINE
          case MESSAGE_SEND_TIMEOUT: // LCOV_EXCL_LINE
            result = MessageSendResult::Timeout; // LCOV_EXCL_LINE
            break; // LCOV_EXCL_LINE
        }
        onComplete(result, disposition);
      }
    };

    void MessageSenderImpl::SendAsync(
        Azure::Core::Amqp::Models::Message const& message,
        Azure::Core::Amqp::_internal::MessageSender::MessageSendCompleteCallback onSendComplete,
        Azure::Core::Context context)
    {
      auto operation(std::make_unique<Azure::Core::Amqp::Common::_internal::CompletionOperation<
                         decltype(onSendComplete),
                         RewriteSendComplete<decltype(onSendComplete)>>>(onSendComplete));
      auto result = messagesender_send_async(
          m_messageSender,
          message,
          std::remove_pointer<decltype(operation)::element_type>::type::OnOperationFn,
          operation.release(),
          0 /*timeout*/);
      if (result == nullptr)
      {
        throw std::runtime_error("Could not send message"); // LCOV_EXCL_LINE
      }
      (void)context;
    }

    std::tuple<MessageSendResult, Azure::Core::Amqp::Models::Value> MessageSenderImpl::Send(
        Azure::Core::Amqp::Models::Message const& message,
        Azure::Core::Context context)
    {
      Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
          Azure::Core::Amqp::_internal::MessageSendResult,
          Azure::Core::Amqp::Models::Value>
          sendCompleteQueue;
      SendAsync(
          message,
          [&](Azure::Core::Amqp::_internal::MessageSendResult sendResult,
              Azure::Core::Amqp::Models::Value deliveryStatus) {
            //          std::cout << "Send Complete!" << std::endl;
            sendCompleteQueue.CompleteOperation(sendResult, deliveryStatus);
          },
          context);
      auto result = sendCompleteQueue.WaitForPolledResult(context, m_connection);
      if (result)
      {
        return std::move(*result);
      }
      throw std::runtime_error("Error sending message"); // LCOV_EXCL_LINE
    }
  } // namespace _detail
}}} // namespace Azure::Core::Amqp
