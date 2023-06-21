// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

// Enable declaration of strerror_s.
#define __STDC_WANT_LIB_EXT1__ 1

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
#include <azure/core/platform.hpp>

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

  void MessageSender::Open(Context const& context) { m_impl->Open(context); }
  void MessageSender::Close() { m_impl->Close(); }
  MessageSender::SendResult MessageSender::Send(
      Models::AmqpMessage const& message,
      Context const& context)
  {
    return m_impl->Send(message, context);
  }
  Common::_internal::QueuedOperation<MessageSender::SendResult> MessageSender::QueueSend(
      Models::AmqpMessage const& message)
  {
    return m_impl->QueueSend(message);
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
    if (m_link)
    {
      // Unsubscribe from any detach events before clearing out the event handler to short-circuit
      // any events firing after the object is destroyed.
      m_link->UnsubscribeFromDetachEvent();
    }
    // Clear the event callback before calling messagesender_destroy to short-circuit any state
    // change events firing after the object is destroyed.
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

    m_link->SubscribeToDetachEvent([this](Models::_internal::AmqpError const& error) {
      if (m_events)
      {
        m_events->OnMessageSenderDisconnected(error);
      }
      // Log that an error occurred.
      Log::Stream(Logger::Level::Error)
          << "Message sender link detached: " << error.Condition.ToString() << ": "
          << error.Description;

      // Cache the error we received in the OnDetach notification so we can return it to the user on
      // the next send which fails.
      m_savedMessageError = Models::_internal::AmqpErrorFactory::ToAmqp(error);
    });
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
    if (m_options.AuthenticationRequired)
    {
      // If we need to authenticate with either ServiceBus or BearerToken, now is the time to do
      // it.
      m_session->AuthenticateIfNeeded(static_cast<std::string>(m_target.GetAddress()), context);
    }

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
      // LCOV_EXCL_START
      auto err = errno;
#if defined(AZ_PLATFORM_WINDOWS)
      char buf[256];
      strerror_s(buf, sizeof(buf), err);
#else
      std::string buf{strerror(err)};
#endif
      throw std::runtime_error(
          "Could not open message sender. errno=" + std::to_string(err) + ", \"" + buf + "\".");
      // LCOV_EXCL_STOP
    }
  }
  void MessageSenderImpl::Close()
  {
    if (messagesender_close(m_messageSender.get()))
    {
      throw std::runtime_error("Could not close message sender"); // LCOV_EXCL_LINE
    }
  }

}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _detail {

  /*
   * Specialization of the WaitForOperationResult function for the MessageSender.
   */
  template <>
  template <>
  Amqp::_internal::MessageSender::SendResult
  QueuedOperationImpl<Amqp::_internal::MessageSender::SendResult>::WaitForOperationResult<
      Amqp::_detail::ConnectionImpl>(
      Context const& context,
      Amqp::_detail::ConnectionImpl& connection)
  {
    m_waitForOperationResultCalled = true;
    // Remember the context so we can use it in the Poll function.
    m_context = &context;
    auto result = m_queue.WaitForPolledResult(context, connection, *this);
    // Since we are no longer polling, we can clear the context.
    m_context = nullptr;
    if (result)
    {
      return std::move(std::get<0>(*result));
    }
    throw std::runtime_error("Error sending message");
  }

  template <> void QueuedOperationImpl<Amqp::_internal::MessageSender::SendResult>::Cancel() const
  {
    if (m_operation)
    {
      async_operation_cancel(m_operation);
    }
  }

  template <> void QueuedOperationImpl<Amqp::_internal::MessageSender::SendResult>::Poll() const
  {
    if (m_context && m_context->IsCancelled())
    {
      this->Cancel();
    }
  }

  template <>
  _internal::QueuedOperation<Amqp::_internal::MessageSender::SendResult>
  QueuedOperationFactory::CreateQueuedOperation<Amqp::_internal::MessageSender::SendResult>(
      std::shared_ptr<QueuedOperationImpl<Amqp::_internal::MessageSender::SendResult>> impl)
  {
    return Azure::Core::Amqp::Common::_internal::QueuedOperation<
        Amqp::_internal::MessageSender::SendResult>(impl);
  }

}}}}} // namespace Azure::Core::Amqp::Common::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _internal {
  /*
   * Called from MessageSenderImpl::Send() to send a message. This method will block until the
   * send completes.
   */
  template <>
  template <>
  Amqp::_internal::MessageSender::SendResult
  QueuedOperation<Amqp::_internal::MessageSender::SendResult>::WaitForOperationResult<
      Amqp::_detail::ConnectionImpl>(
      Context const& context,
      Amqp::_detail::ConnectionImpl& connection)
  {
    return m_impl->WaitForOperationResult(context, connection);
  }

  template <> void QueuedOperation<Amqp::_internal::MessageSender::SendResult>::Cancel()
  {
    m_impl->Cancel();
  }
  /*
   * Called by callers of MessageSender::QueueSend() after the message was queued. This method
   * will block until the send completes.
   */
  template <>
  template <>
  Amqp::_internal::MessageSender::SendResult
  QueuedOperation<Amqp::_internal::MessageSender::SendResult>::WaitForOperationResult<
      Amqp::_internal::Connection>(Context const& context, Amqp::_internal::Connection& connection)
  {
    return m_impl->WaitForOperationResult<Amqp::_detail::ConnectionImpl>(
        context, *Amqp::_detail::ConnectionFactory::GetImpl(connection));
  }
}}}}} // namespace Azure::Core::Amqp::Common::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  void MessageSenderImpl::MessageSenderQueuedOperation::OnSendCompleteFn(
      void* context,
      MESSAGE_SEND_RESULT sendResult,
      AMQP_VALUE disposition)
  {
    MessageSenderQueuedOperation* thisPtr = static_cast<MessageSenderQueuedOperation*>(context);
    _internal::MessageSendStatus result{_internal::MessageSendStatus::Ok};
    switch (sendResult)
    {
      case MESSAGE_SEND_RESULT_INVALID: // LCOV_EXCL_LINE
        result = _internal::MessageSendStatus::Invalid; // LCOV_EXCL_LINE
        break; // LCOV_EXCL_LINE
      case MESSAGE_SEND_OK:
        result = _internal::MessageSendStatus::Ok;
        break;
      case MESSAGE_SEND_CANCELLED:
        result = _internal::MessageSendStatus::Cancelled;
        break; // LCOV_EXCL_LINE
      case MESSAGE_SEND_ERROR: // LCOV_EXCL_LINE
        result = _internal::MessageSendStatus::Error; // LCOV_EXCL_LINE
        break; // LCOV_EXCL_LINE
      case MESSAGE_SEND_TIMEOUT: // LCOV_EXCL_LINE
        result = _internal::MessageSendStatus::Timeout; // LCOV_EXCL_LINE
        break; // LCOV_EXCL_LINE
    }
    thisPtr->OnSendComplete(result, disposition);
  }

  void MessageSenderImpl::MessageSenderQueuedOperation::OnSendComplete(
      _internal::MessageSendStatus status,
      Models::AmqpValue disposition)
  {
    Log::Stream(Logger::Level::Informational)
        << "OnSendComplete. Send Status: " << static_cast<int>(status)
        << " Disposition: " << disposition << std::endl;

    // If the send failed. then we need to return the error. If the send completed because
    // of an error, it's possible that the deliveryStatus provided is null. In that case,
    // we use the cached saved error because it is highly likely to be better than
    // nothing.
    if (status != _internal::MessageSendStatus::Ok)
    {
      if (disposition.IsNull())
      {
        disposition = m_senderImpl->GetAndResetSavedMessageError();
      }
    }
    else
    {
      // If we successfully sent the message, then whatever saved error should be cleared,
      // it's no longer valid.
      m_senderImpl->GetAndResetSavedMessageError();
    }
    // At this point, the underlying operation can no longer be cancelled.
    m_operation = nullptr;
    m_queue.CompleteOperation(std::make_tuple(status, disposition));
  }

  MessageSenderImpl::MessageSenderQueuedOperation::~MessageSenderQueuedOperation()
  {
    // If we previously have not waited for the operation to complete, block in the destructor until
    // the operation completes.
    if (!HasWaited())
    {
      // Block until the operation completes.
      m_queue.WaitForPolledResult({}, *m_senderImpl->m_session->GetConnection());
    }
  }

  Common::_internal::QueuedOperation<_internal::MessageSender::SendResult>
  MessageSenderImpl::QueueSend(Models::AmqpMessage const& message)
  {
    std::shared_ptr<MessageSenderQueuedOperation> operation{
        std::make_shared<MessageSenderQueuedOperation>(shared_from_this())};

    ASYNC_OPERATION_HANDLE result = messagesender_send_async(
        m_messageSender.get(),
        Models::_internal::AmqpMessageFactory::ToUamqp(message).get(),
        MessageSenderQueuedOperation::OnSendCompleteFn,
        operation.get(),
        0 /*timeout*/);
    if (result == nullptr)
    {
      throw std::runtime_error("Could not send message"); // LCOV_EXCL_LINE
    }
    operation->SetAsyncOperation(result);

    // Return a public wrapper to the queued operation for the caller to use.
    return Common::_detail::QueuedOperationFactory::CreateQueuedOperation(
        std::static_pointer_cast<
            Common::_detail::QueuedOperationImpl<_internal::MessageSender::SendResult>>(operation));
  }

  _internal::MessageSender::SendResult MessageSenderImpl::Send(
      Models::AmqpMessage const& message,
      Context const& context)
  {
    // Queue the current message for sending and then wait until the queued message completes.
    auto deferredResult = QueueSend(message);
    auto returnValue = deferredResult.WaitForOperationResult(context, *m_session->GetConnection());
    return returnValue;
  }

}}}} // namespace Azure::Core::Amqp::_detail
