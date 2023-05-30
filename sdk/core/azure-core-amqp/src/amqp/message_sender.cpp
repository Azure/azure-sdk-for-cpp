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
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <azure_uamqp_c/message_sender.h>

#include <memory>

using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Diagnostics::_internal;

void Azure::Core::_internal::UniqueHandleHelper<MESSAGE_SENDER_INSTANCE_TAG>::FreeMessageSender(
    MESSAGE_SENDER_HANDLE value)
{
  messagesender_destroy(value);
}

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  MessageSender::MessageSender(
      Session const& session,
      Models::_internal::MessageTarget const& target,
      MessageSenderOptions const& options,
      MessageSenderEvents* events)
      : m_impl{std::make_shared<_detail::MessageSenderImpl>(
          _detail::SessionFactory::GetImpl(session),
          target,
          options,
          events)}
  {
  }
  MessageSender::MessageSender(
      Session const& session,
      LinkEndpoint& endpoint,
      Models::_internal::MessageTarget const& target,
      MessageSenderOptions const& options,
      MessageSenderEvents* events)
      : m_impl{std::make_shared<_detail::MessageSenderImpl>(
          _detail::SessionFactory::GetImpl(session),
          endpoint,
          target,
          options,
          events)}
  {
  }

  void MessageSender::Open(Context const& context) { m_impl->Open(context); }
  void MessageSender::Close() { m_impl->Close(); }
  std::tuple<MessageSendStatus, Models::AmqpValue> MessageSender::Send(
      Models::AmqpMessage const& message,
      Context const& context)
  {
    return m_impl->Send(message, context);
  }
  void MessageSender::QueueSend(
      Models::AmqpMessage const& message,
      MessageSendCompleteCallback onSendComplete,
      Context const& context)
  {
    return m_impl->QueueSend(message, onSendComplete, context);
  }

  MessageSender::~MessageSender() noexcept {}
}}}} // namespace Azure::Core::Amqp::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  MessageSenderImpl::MessageSenderImpl(
      std::shared_ptr<SessionImpl> session,
      Models::_internal::MessageTarget const& target,
      _internal::MessageSenderOptions const& options,
      _internal::MessageSenderEvents* events)
      : m_events{events}, m_session{session}, m_target{target}, m_options{options}
  {
  }

  MessageSenderImpl::MessageSenderImpl(
      std::shared_ptr<SessionImpl> session,
      _internal::LinkEndpoint& endpoint,
      Models::_internal::MessageTarget const& target,
      _internal::MessageSenderOptions const& options,
      _internal::MessageSenderEvents* events)
      : m_events{events}, m_session{session}, m_target{target}, m_options{options}
  {
    CreateLink(endpoint);
    m_messageSender.reset(
        messagesender_create(*m_link, MessageSenderImpl::OnMessageSenderStateChangedFn, this));

    messagesender_set_trace(m_messageSender.get(), m_options.EnableTrace);
  }

  MessageSenderImpl::~MessageSenderImpl() noexcept
  {
    // Clear the event callback before calling messagesender_destroy to short-circuit any
    // events firing after the object is destroyed.
    if (m_events)
    {
      m_events = nullptr;
    }
  }

  void MessageSenderImpl::CreateLink(_internal::LinkEndpoint& endpoint)
  {
    m_link = std::make_shared<_detail::LinkImpl>(
        m_session,
        endpoint,
        m_options.Name,
        _internal::SessionRole::Receiver, // This is the role of the link, not the endpoint.
        m_options.MessageSource,
        m_target);
    PopulateLinkProperties();
  }
  void MessageSenderImpl::CreateLink()
  {
    m_link = std::make_shared<_detail::LinkImpl>(
        m_session,
        m_options.Name,
        _internal::SessionRole::Sender, // This is the role of the link, not the endpoint.
        m_options.MessageSource,
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
          MessageSenderFactory::CreateFromInternal(sender->shared_from_this()),
          MessageSenderStateFromLowLevel(newState),
          MessageSenderStateFromLowLevel(oldState));
    }
  }

  void MessageSenderImpl::Open(Context const& context)
  {
    // If we need to authenticate with either ServiceBus or BearerToken, now is the time to do
    // it.
    m_session->AuthenticateIfNeeded(static_cast<std::string>(m_target.GetAddress()), context);

    if (m_link == nullptr)
    {
      CreateLink();
    }

    if (m_messageSender == nullptr)
    {
      m_messageSender.reset(
          messagesender_create(*m_link, MessageSenderImpl::OnMessageSenderStateChangedFn, this));
    }
    if (messagesender_open(m_messageSender.get()))
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
    if (messagesender_close(m_messageSender.get()))
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
      _internal::MessageSendStatus result{_internal::MessageSendStatus::Ok};
      switch (sendResult)
      {
        case MESSAGE_SEND_RESULT_INVALID: // LCOV_EXCL_LINE
          result = _internal::MessageSendStatus::Invalid; // LCOV_EXCL_LINE
          break; // LCOV_EXCL_LINE
        case MESSAGE_SEND_OK:
          result = _internal::MessageSendStatus::Ok;
          break;
        case MESSAGE_SEND_CANCELLED: // LCOV_EXCL_LINE
          result = _internal::MessageSendStatus::Cancelled; // LCOV_EXCL_LINE
          break; // LCOV_EXCL_LINE
        case MESSAGE_SEND_ERROR: // LCOV_EXCL_LINE
          result = _internal::MessageSendStatus::Error; // LCOV_EXCL_LINE
          break; // LCOV_EXCL_LINE
        case MESSAGE_SEND_TIMEOUT: // LCOV_EXCL_LINE
          result = _internal::MessageSendStatus::Timeout; // LCOV_EXCL_LINE
          break; // LCOV_EXCL_LINE
      }
      onComplete(result, disposition);
    }
  };

  void MessageSenderImpl::QueueSend(
      Models::AmqpMessage const& message,
      Azure::Core::Amqp::_internal::MessageSender::MessageSendCompleteCallback onSendComplete,
      Context const& context)
  {
    auto operation(std::make_unique<Azure::Core::Amqp::Common::_internal::CompletionOperation<
                       decltype(onSendComplete),
                       RewriteSendComplete<decltype(onSendComplete)>>>(onSendComplete));
    auto result = messagesender_send_async(
        m_messageSender.get(),
        Models::_internal::AmqpMessageFactory::ToUamqp(message).get(),
        std::remove_pointer<decltype(operation)::element_type>::type::OnOperationFn,
        operation.release(),
        0 /*timeout*/);
    if (result == nullptr)
    {
      throw std::runtime_error("Could not send message"); // LCOV_EXCL_LINE
    }
    (void)context;
  }

  std::tuple<_internal::MessageSendStatus, Models::AmqpValue> MessageSenderImpl::Send(
      Models::AmqpMessage const& message,
      Context const& context)
  {
    Azure::Core::Amqp::Common::_internal::
        AsyncOperationQueue<Azure::Core::Amqp::_internal::MessageSendStatus, Models::AmqpValue>
            sendCompleteQueue;

    QueueSend(
        message,
        [&](Azure::Core::Amqp::_internal::MessageSendStatus sendResult,
            Models::AmqpValue deliveryStatus) {
          sendCompleteQueue.CompleteOperation(sendResult, deliveryStatus);
        },
        context);
    auto result = sendCompleteQueue.WaitForPolledResult(context, *m_session->GetConnection());
    if (result)
    {
      return std::move(*result);
    }
    throw std::runtime_error("Error sending message"); // LCOV_EXCL_LINE
  }
}}}} // namespace Azure::Core::Amqp::_detail
