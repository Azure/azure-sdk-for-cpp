// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/claims_based_security.hpp"
#include "azure/core/amqp/common/completion_operation.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "azure/core/amqp/models/messaging_values.hpp"
#include "azure/core/amqp/session.hpp"
#include "private/connection_impl.hpp"
#include "private/message_sender_impl.hpp"
#include "private/session_impl.hpp"
#include <azure/core/credentials/credentials.hpp>

#include <azure_uamqp_c/message_sender.h>
#include <memory>

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  MessageSender::MessageSender(
      Session const& session,
      std::string const& target,
      MessageSenderOptions const& options,
      MessageSenderEvents* events)
      : m_impl{
          std::make_shared<_detail::MessageSenderImpl>(session.GetImpl(), target, options, events)}
  {
  }
  MessageSender::MessageSender(
      Session const& session,
      LinkEndpoint& endpoint,
      std::string const& target,
      MessageSenderOptions const& options,
      MessageSenderEvents* events)
      : m_impl{std::make_shared<_detail::MessageSenderImpl>(
          session.GetImpl(),
          endpoint,
          target,
          options,
          events)}
  {
  }

  MessageSender::MessageSender(
      Session const& session,
      std::shared_ptr<ServiceBusSasConnectionStringCredential> credential,
      std::string const& target,
      MessageSenderOptions const& options,
      MessageSenderEvents* events)
      : m_impl{std::make_shared<_detail::MessageSenderImpl>(
          session.GetImpl(),
          credential,
          target,
          options,
          events)}
  {
  }

  MessageSender::MessageSender(
      Session const& session,
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
      std::string const& target,
      MessageSenderOptions const& options,
      MessageSenderEvents* events)
      : m_impl{std::make_shared<_detail::MessageSenderImpl>(
          session.GetImpl(),
          credential,
          target,
          options,
          events)}
  {
  }

  void MessageSender::Open(Azure::Core::Context const& context) { m_impl->Open(context); }
  void MessageSender::Close() { m_impl->Close(); }
  std::tuple<MessageSendResult, Azure::Core::Amqp::Models::AmqpValue> MessageSender::Send(
      Azure::Core::Amqp::Models::AmqpMessage const& message,
      Azure::Core::Context context)
  {
    return m_impl->Send(message, context);
  }
  void MessageSender::QueueSend(
      Azure::Core::Amqp::Models::AmqpMessage const& message,
      MessageSendCompleteCallback onSendComplete,
      Azure::Core::Context context)
  {
    return m_impl->QueueSend(message, onSendComplete, context);
  }
  void MessageSender::SetTrace(bool traceEnabled) { m_impl->SetTrace(traceEnabled); }

  MessageSender::~MessageSender() noexcept {}
}}}} // namespace Azure::Core::Amqp::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  MessageSenderImpl::MessageSenderImpl(
      std::shared_ptr<SessionImpl> session,
      std::string const& target,
      _internal::MessageSenderOptions const& options,
      _internal::MessageSenderEvents* events)
      : m_events{events}, m_session{session}, m_target{target}, m_options{options}
  {
  }

  MessageSenderImpl::MessageSenderImpl(
      std::shared_ptr<SessionImpl> session,
      _internal::LinkEndpoint& endpoint,
      std::string const& target,
      _internal::MessageSenderOptions const& options,
      _internal::MessageSenderEvents* events)
      : m_events{events}, m_session{session}, m_target{target}, m_options{options}
  {
    CreateLink(endpoint);
    m_messageSender
        = messagesender_create(*m_link, MessageSenderImpl::OnMessageSenderStateChangedFn, this);

    SetTrace(options.EnableTrace);
  }

  MessageSenderImpl::MessageSenderImpl(
      std::shared_ptr<SessionImpl> session,
      std::shared_ptr<_internal::ServiceBusSasConnectionStringCredential> credential,
      std::string const& target,
      _internal::MessageSenderOptions const& options,
      _internal::MessageSenderEvents* events)
      : m_events{events}, m_session{session},
        m_connectionCredential{credential}, m_target{target}, m_options{options}

  {
  }

  MessageSenderImpl::MessageSenderImpl(
      std::shared_ptr<SessionImpl> session,
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
      std::string const& target,
      _internal::MessageSenderOptions const& options,
      _internal::MessageSenderEvents* events)
      : m_events{events}, m_session{session},
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
      if (m_cbsOpen)
      {
        m_claimsBasedSecurity->Close();
      }
    }
    if (m_messageSender)
    {
      messagesender_destroy(m_messageSender);
      m_messageSender = nullptr;
    }
  }

  void MessageSenderImpl::CreateLink(_internal::LinkEndpoint& endpoint)
  {
    m_link = std::make_unique<_detail::Link>(
        m_session,
        endpoint,
        m_options.Name,
        _internal::SessionRole::Receiver, // This is the role of the link, not the endpoint.
        m_options.SourceAddress,
        m_target);
    PopulateLinkProperties();
  }
  void MessageSenderImpl::CreateLink()
  {
    m_link = std::make_unique<_detail::Link>(
        m_session,
        m_options.Name,
        _internal::SessionRole::Sender, // This is the role of the link, not the endpoint.
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

  _internal::MessageSenderState MessageSenderStateFromLowLevel(MESSAGE_SENDER_STATE lowLevel)
  {
    switch (lowLevel)
    {
      case MESSAGE_SENDER_STATE_CLOSING:
        return _internal::MessageSenderState::Closing;
      case MESSAGE_SENDER_STATE_ERROR: // LCOV_EXCL_LINE
        return _internal::MessageSenderState::Error; // LCOV_EXCL_LINE
      case MESSAGE_SENDER_STATE_IDLE:
        return _internal::MessageSenderState::Idle;
      case MESSAGE_SENDER_STATE_INVALID: // LCOV_EXCL_LINE
        return _internal::MessageSenderState::Invalid; // LCOV_EXCL_LINE
      case MESSAGE_SENDER_STATE_OPEN:
        return _internal::MessageSenderState::Open;
      case MESSAGE_SENDER_STATE_OPENING:
        return _internal::MessageSenderState::Opening;
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
      _internal::CredentialType type,
      std::string const& audience,
      std::string const& token,
      Azure::Core::Context const& context)
  {
    m_claimsBasedSecurity = std::make_unique<ClaimsBasedSecurityImpl>(m_session);
    if (m_claimsBasedSecurity->Open(context) == CbsOpenResult::Ok)
    {
      m_cbsOpen = true;
      auto result = m_claimsBasedSecurity->PutToken(
          (type == _internal::CredentialType::BearerToken ? CbsTokenType::Jwt : CbsTokenType::Sas),
          audience,
          token,
          context);
    }
    else
    {
      throw std::runtime_error("Could not put Claims Based Security token."); // LCOV_EXCL_LINE
    }
  }

  void MessageSenderImpl::Open(Azure::Core::Context const& context)
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
              std::chrono::system_clock::now() + std::chrono::minutes(60)),
          context);
    }
    else if (m_tokenCredential)
    {
      Azure::Core::Credentials::TokenRequestContext requestContext;
      requestContext.Scopes = m_options.AuthenticationScopes;

      Authenticate(
          _internal::CredentialType::BearerToken,
          m_target,
          m_tokenCredential->GetToken(requestContext, context).Token,
          context);
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
      _internal::MessageSendResult result{_internal::MessageSendResult::Ok};
      switch (sendResult)
      {
        case MESSAGE_SEND_RESULT_INVALID: // LCOV_EXCL_LINE
          result = _internal::MessageSendResult::Invalid; // LCOV_EXCL_LINE
          break; // LCOV_EXCL_LINE
        case MESSAGE_SEND_OK:
          result = _internal::MessageSendResult::Ok;
          break;
        case MESSAGE_SEND_CANCELLED: // LCOV_EXCL_LINE
          result = _internal::MessageSendResult::Cancelled; // LCOV_EXCL_LINE
          break; // LCOV_EXCL_LINE
        case MESSAGE_SEND_ERROR: // LCOV_EXCL_LINE
          result = _internal::MessageSendResult::Error; // LCOV_EXCL_LINE
          break; // LCOV_EXCL_LINE
        case MESSAGE_SEND_TIMEOUT: // LCOV_EXCL_LINE
          result = _internal::MessageSendResult::Timeout; // LCOV_EXCL_LINE
          break; // LCOV_EXCL_LINE
      }
      onComplete(result, disposition);
    }
  };

  void MessageSenderImpl::QueueSend(
      Azure::Core::Amqp::Models::AmqpMessage const& message,
      Azure::Core::Amqp::_internal::MessageSender::MessageSendCompleteCallback onSendComplete,
      Azure::Core::Context context)
  {
    auto operation(std::make_unique<Azure::Core::Amqp::Common::_internal::CompletionOperation<
                       decltype(onSendComplete),
                       RewriteSendComplete<decltype(onSendComplete)>>>(onSendComplete));
    auto result = messagesender_send_async(
        m_messageSender,
        Azure::Core::Amqp::Models::_internal::AmqpMessageFactory::ToUamqp(message).get(),
        std::remove_pointer<decltype(operation)::element_type>::type::OnOperationFn,
        operation.release(),
        0 /*timeout*/);
    if (result == nullptr)
    {
      throw std::runtime_error("Could not send message"); // LCOV_EXCL_LINE
    }
    (void)context;
  }

  std::tuple<_internal::MessageSendResult, Azure::Core::Amqp::Models::AmqpValue> MessageSenderImpl::
      Send(Azure::Core::Amqp::Models::AmqpMessage const& message, Azure::Core::Context context)
  {
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
        Azure::Core::Amqp::_internal::MessageSendResult,
        Azure::Core::Amqp::Models::AmqpValue>
        sendCompleteQueue;

    QueueSend(
        message,
        [&](Azure::Core::Amqp::_internal::MessageSendResult sendResult,
            Azure::Core::Amqp::Models::AmqpValue deliveryStatus) {
          sendCompleteQueue.CompleteOperation(sendResult, deliveryStatus);
        },
        context);
    auto result = sendCompleteQueue.WaitForPolledResult(context, *m_session->GetConnectionToPoll());
    if (result)
    {
      return std::move(*result);
    }
    throw std::runtime_error("Error sending message"); // LCOV_EXCL_LINE
  }
}}}} // namespace Azure::Core::Amqp::_detail
