// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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

namespace Azure { namespace Core { namespace _internal {
  void UniqueHandleHelper<MESSAGE_SENDER_INSTANCE_TAG>::FreeMessageSender(
      MESSAGE_SENDER_HANDLE value)
  {
    messagesender_destroy(value);
  }
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  void MessageSender::Open(Context const& context) { m_impl->Open(context); }
  void MessageSender::Close() { m_impl->Close(); }
  std::tuple<MessageSendStatus, Models::_internal::AmqpError> MessageSender::Send(
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

  std::uint64_t MessageSender::GetMaxMessageSize() const { return m_impl->GetMaxMessageSize(); }

  MessageSender::~MessageSender() noexcept {}
}}}} // namespace Azure::Core::Amqp::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  MessageSenderImpl::MessageSenderImpl(
      std::shared_ptr<_detail::SessionImpl> session,
      Models::_internal::MessageTarget const& target,
      _internal::MessageSenderOptions const& options,
      _internal::MessageSenderEvents* events)
      : m_events{events}, m_session{session}, m_target{target}, m_options{options}
  {
  }

  MessageSenderImpl::MessageSenderImpl(
      std::shared_ptr<_detail::SessionImpl> session,
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
    // Clear the event callback before calling messagesender_destroy to short-circuit any state
    // change events firing after the object is destroyed.
    if (m_events)
    {
      m_events = nullptr;
    }

    if (m_isOpen)
    {
      Close();
    }

    auto lock{m_session->GetConnection()->Lock()};
    if (m_link)
    {
      // Unsubscribe from any detach events before clearing out the event handler to short-circuit
      // any events firing after the object is destroyed.
      m_link->UnsubscribeFromDetachEvent();
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

      // Cache the error we received in the OnDetach notification so we can return it to the user
      // on the next send which fails.
      m_savedMessageError = error;
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
    if (m_options.MaxLinkCredits != 0)
    {
      m_link->SetMaxLinkCredit(m_options.MaxLinkCredits);
    }
    m_link->SetSenderSettleMode(m_options.SettleMode);
  }

  std::uint64_t MessageSenderImpl::GetMaxMessageSize() const { return m_link->GetMaxMessageSize(); }

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
    Log::Stream(Logger::Level::Verbose) << "Opening message sender. Authenticate if needed.";
    if (m_options.AuthenticationRequired)
    {
      // If we need to authenticate with either ServiceBus or BearerToken, now is the time to do
      // it.
      m_session->AuthenticateIfNeeded(static_cast<std::string>(m_target.GetAddress()), context);
    }

    auto lock{m_session->GetConnection()->Lock()};
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

    // Mark the connection as async so that we can use the async APIs.
    Log::Stream(Logger::Level::Verbose) << "Opening message sender. Enable async operation.";
    m_session->GetConnection()->EnableAsyncOperation(true);
    m_isOpen = true;
  }

  void MessageSenderImpl::Close()
  {
    if (m_isOpen)
    {
      Log::Stream(Logger::Level::Verbose) << "Lock for Closing message sender.";
      auto lock{m_session->GetConnection()->Lock()};
      Log::Stream(Logger::Level::Verbose) << "Closing message sender.";
      m_session->GetConnection()->EnableAsyncOperation(false);
      if (messagesender_close(m_messageSender.get()))
      {
        throw std::runtime_error("Could not close message sender"); // LCOV_EXCL_LINE
      }
      m_isOpen = false;
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
    auto lock{m_session->GetConnection()->Lock()};
    QueueSendInternal(message, onSendComplete, context);
  }

  void MessageSenderImpl::QueueSendInternal(
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
  std::tuple<_internal::MessageSendStatus, Models::_internal::AmqpError> MessageSenderImpl::Send(
      Models::AmqpMessage const& message,
      Context const& context)
  {
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
        Azure::Core::Amqp::_internal::MessageSendStatus,
        Models::_internal::AmqpError>
        sendCompleteQueue;
    {
      auto lock{m_session->GetConnection()->Lock()};

      QueueSendInternal(
          message,
          [&sendCompleteQueue, this](
              Azure::Core::Amqp::_internal::MessageSendStatus sendResult,
              Models::AmqpValue deliveryStatus) {
            Models::_internal::AmqpError error;

            // If the send failed. then we need to return the error. If the send completed because
            // of an error, it's possible that the deliveryStatus provided is null. In that case,
            // we use the cached saved error because it is highly likely to be better than
            // nothing.
            if (sendResult != _internal::MessageSendStatus::Ok)
            {
              if (deliveryStatus.IsNull())
              {
                error = m_savedMessageError;
              }
              else
              {
                if (deliveryStatus.GetType() != Models::AmqpValueType::List)
                {
                  throw std::runtime_error("Delivery status is not a list");
                }
                auto deliveryStatusAsList{deliveryStatus.AsList()};
                if (deliveryStatusAsList.size() != 1)
                {
                  throw std::runtime_error("Delivery Status list is not of size 1");
                }
                Models::AmqpValue firstState{deliveryStatusAsList[0]};
                ERROR_HANDLE errorHandle;
                if (!amqpvalue_get_error(firstState, &errorHandle))
                {
                  Models::_internal::UniqueAmqpErrorHandle uniqueError{
                      errorHandle}; // This will free the error handle when it goes out of scope.
                  error = Models::_internal::AmqpErrorFactory::FromUamqp(errorHandle);
                }
              }
            }
            else
            {
              // If we successfully sent the message, then whatever saved error should be cleared,
              // it's no longer valid.
              m_savedMessageError = Models::_internal::AmqpError();
            }
            sendCompleteQueue.CompleteOperation(sendResult, error);
          },
          context);
    }
    auto result = sendCompleteQueue.WaitForResult(context);
    if (result)
    {
      return std::move(*result);
    }
    throw std::runtime_error("Error sending message"); // LCOV_EXCL_LINE
  }
}}}} // namespace Azure::Core::Amqp::_detail
