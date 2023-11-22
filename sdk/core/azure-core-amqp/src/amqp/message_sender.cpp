// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Enable declaration of strerror_s.
#define __STDC_WANT_LIB_EXT1__ 1

#include "../models/private/error_impl.hpp"
#include "../models/private/message_impl.hpp"
#include "../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/claims_based_security.hpp"
#include "azure/core/amqp/internal/common/completion_operation.hpp"
#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/internal/session.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
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

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  void UniqueHandleHelper<MESSAGE_SENDER_INSTANCE_TAG>::FreeMessageSender(
      MESSAGE_SENDER_HANDLE value)
  {
    messagesender_destroy(value);
  }
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  void MessageSender::Open(Context const& context) { m_impl->Open(context); }
  void MessageSender::Close() { m_impl->Close(); }
  std::tuple<MessageSendStatus, Models::_internal::AmqpError> MessageSender::Send(
      Models::AmqpMessage const& message,
      Context const& context)
  {
    return m_impl->Send(message, context);
  }

  std::uint64_t MessageSender::GetMaxMessageSize() const { return m_impl->GetMaxMessageSize(); }

  MessageSender::~MessageSender() noexcept {}
  std::ostream& operator<<(std::ostream& stream, _internal::MessageSenderState const& state)
  {
    switch (state)
    {
      case _internal::MessageSenderState::Invalid:
        stream << "Invalid";
        break;
      case _internal::MessageSenderState::Idle:
        stream << "Idle";
        break;
      case _internal::MessageSenderState::Opening:
        stream << "Opening";
        break;
      case _internal::MessageSenderState::Open:
        stream << "Open";
        break;
      case _internal::MessageSenderState::Closing:
        stream << "Closing";
        break;
      case _internal::MessageSenderState::Error:
        stream << "Error";
        break;
      default:
        throw std::runtime_error("Unknown message sender state operation type.");
    }
    return stream;
  }
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

    auto lock{m_session->GetConnection()->Lock()};
    if (m_senderOpen)
    {
      AZURE_ASSERT_MSG(m_senderOpen, "MessageSenderImpl is being destroyed while open.");
      Azure::Core::_internal::AzureNoReturnPath("MessageSenderImpl is being destroyed while open.");
    }

    if (m_messageSender)
    {
      m_messageSender.reset();
    }

    if (m_link)
    {
      // Unsubscribe from any detach events before clearing out the event handler to short-circuit
      // any events firing after the object is destroyed.
      m_link->UnsubscribeFromDetachEvent();

      m_link.reset();
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
      if (m_senderOpen)
      {
        if (m_events)
        {
          m_events->OnMessageSenderDisconnected(error);
        }
        // Log that an error occurred.
        Log::Stream(Logger::Level::Warning)
            << "Message sender link detached: " << error.Condition.ToString() << ": "
            << error.Description;

        // Cache the error we received in the OnDetach notification so we can return it to the user
        // on the next send which fails.
        m_savedMessageError = error;
      }
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
      case MESSAGE_SENDER_STATE_ERROR:
        return _internal::MessageSenderState::Error;
      case MESSAGE_SENDER_STATE_IDLE:
        return _internal::MessageSenderState::Idle;
      case MESSAGE_SENDER_STATE_INVALID:
        return _internal::MessageSenderState::Invalid;
      case MESSAGE_SENDER_STATE_OPEN:
        return _internal::MessageSenderState::Open;
      case MESSAGE_SENDER_STATE_OPENING:
        return _internal::MessageSenderState::Opening;
      default:
        throw std::logic_error("Unknown message receiver state.");
    }
  }

  std::ostream& operator<<(std::ostream& os, MESSAGE_SENDER_STATE state)
  {
    os << MessageSenderStateFromLowLevel(state) << "("
       << static_cast<std::underlying_type<MESSAGE_SENDER_STATE>::type>(state) << ")";
    return os;
  }

  void MessageSenderImpl::OnMessageSenderStateChangedFn(
      void* context,
      MESSAGE_SENDER_STATE newState,
      MESSAGE_SENDER_STATE oldState)
  {
    // We only care about the transition between states - uAMQP will sometimes set a state changed
    // to the current state.
    if (newState != oldState)
    {
      auto sender = static_cast<MessageSenderImpl*>(const_cast<void*>(context));
      sender->m_currentState = MessageSenderStateFromLowLevel(newState);
      if (sender->m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose)
            << "Message sender state changed from " << oldState << " to " << newState << ".";
      }
      if (sender->m_events)
      {
        sender->m_events->OnMessageSenderStateChanged(
            MessageSenderFactory::CreateFromInternal(sender->shared_from_this()),
            MessageSenderStateFromLowLevel(newState),
            MessageSenderStateFromLowLevel(oldState));
      }
      if (newState == MESSAGE_SENDER_STATE_ERROR)
      {
        sender->m_sendCompleteQueue.CompleteOperation(
            _internal::MessageSendStatus::Error,
            {Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::InternalError,
             "Message Sender unexpectedly entered the Error State.",
             {}});
      }
#if SENDER_SYNCHRONOUS_CLOSE

      if (oldState == MESSAGE_SENDER_STATE_CLOSING && newState == MESSAGE_SENDER_STATE_IDLE)
      {
        sender->m_closeQueue.CompleteOperation(Models::_internal::AmqpError{});
      }
#endif
    }
  }

  void MessageSenderImpl::Open(Context const& context)
  {
    if (m_options.EnableTrace)
    {
      Log::Stream(Logger::Level::Verbose)
          << "Opening message sender. Authenticate if needed with audience: " << m_target;
    }
    if (m_options.AuthenticationRequired)
    {
      // If we need to authenticate with either ServiceBus or BearerToken, now is the time to do
      // it.
      m_session->GetConnection()->AuthenticateAudience(
          m_session, static_cast<std::string>(m_target.GetAddress()), context);
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

      auto err = errno;
#if defined(AZ_PLATFORM_WINDOWS)
      char buf[256];
      strerror_s(buf, sizeof(buf), err);
#else
      std::string buf{strerror(err)};
#endif
      throw std::runtime_error(
          "Could not open message sender. errno=" + std::to_string(err) + ", \"" + buf + "\".");
    }

    // Mark the connection as async so that we can use the async APIs.
    if (m_options.EnableTrace)
    {
      Log::Stream(Logger::Level::Verbose) << "Opening message sender. Enable async operation.";
    }
    m_session->GetConnection()->EnableAsyncOperation(true);

    // Enable async on the link as well.
    Common::_detail::GlobalStateHolder::GlobalStateInstance()->AddPollable(m_link);

    m_senderOpen = true;
  }

  void MessageSenderImpl::Close()
  {
    if (m_senderOpen)
    {
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose) << "Lock for Closing message sender.";
      }

      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose) << "Closing message sender.";
        Log::Stream(Logger::Level::Verbose) << "Unsubscribe from link detach event.";
      }
      m_link->UnsubscribeFromDetachEvent();

      Common::_detail::GlobalStateHolder::GlobalStateInstance()->RemovePollable(
          m_link); // This will ensure that the link is cleaned up on the next poll()

#if SENDER_SYNCHRONOUS_CLOSE
      bool shouldWaitForClose = m_currentState == _internal::MessageSenderState::Closing
          || m_currentState == _internal::MessageSenderState::Open;
#endif

      m_session->GetConnection()->EnableAsyncOperation(false);

      auto lock{m_session->GetConnection()->Lock()};

      if (messagesender_close(m_messageSender.get()))
      {
        throw std::runtime_error("Could not close message sender");
      }

#if SENDER_SYNCHRONOUS_CLOSE
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose)
            << "Wait for sender detach to complete. Current state: " << m_currentState;
      }

      // The message sender (and it's underlying link) is in the half open state. Wait until the
      // link has fully closed.
      if (shouldWaitForClose && false)
      {
        lock.unlock();

        auto result = m_closeQueue.WaitForResult(context);
        if (!result)
        {
          throw Azure::Core::OperationCancelledException(
              "Message sender close operation cancelled.");
        }
        if (std::get<0>(*result))
        {
          throw std::runtime_error("Error closing message sender");
        }
      }
#endif

      m_senderOpen = false;
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
        case MESSAGE_SEND_RESULT_INVALID:
          result = _internal::MessageSendStatus::Invalid;
          break;
        case MESSAGE_SEND_OK:
          result = _internal::MessageSendStatus::Ok;
          break;
        case MESSAGE_SEND_CANCELLED:
          result = _internal::MessageSendStatus::Cancelled;
          break;
        case MESSAGE_SEND_ERROR:
          result = _internal::MessageSendStatus::Error;
          break;
        case MESSAGE_SEND_TIMEOUT:
          result = _internal::MessageSendStatus::Timeout;
          break;
      }
      // Reference disposition so that we don't over-release when the AmqpValue passed to OnComplete
      // is destroyed.
      onComplete(
          result,
          Models::_detail::AmqpValueFactory::FromUamqp(
              Models::_detail::UniqueAmqpValueHandle{amqpvalue_clone(disposition)}));
    }
  };

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
        Models::_detail::AmqpMessageFactory::ToUamqp(message).get(),
        std::remove_pointer<decltype(operation)::element_type>::type::OnOperationFn,
        operation.release(),
        0 /*timeout*/);
    if (result == nullptr)
    {
      throw std::runtime_error("Could not send message");
    }
    (void)context;
  }

  std::tuple<_internal::MessageSendStatus, Models::_internal::AmqpError> MessageSenderImpl::Send(
      Models::AmqpMessage const& message,
      Context const& context)
  {
    {
      auto lock{m_session->GetConnection()->Lock()};

      QueueSendInternal(
          message,
          [this](
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
                if (!amqpvalue_get_error(
                        Models::_detail::AmqpValueFactory::ToUamqp(firstState), &errorHandle))
                {
                  Models::_detail::UniqueAmqpErrorHandle uniqueError{
                      errorHandle}; // This will free the error handle when it goes out of scope.
                  error = Models::_detail::AmqpErrorFactory::FromUamqp(errorHandle);
                }
              }
            }
            else
            {
              // If we successfully sent the message, then whatever saved error should be cleared,
              // it's no longer valid.
              m_savedMessageError = Models::_internal::AmqpError();
            }
            m_sendCompleteQueue.CompleteOperation(sendResult, error);
          },
          context);
    }
    auto result = m_sendCompleteQueue.WaitForResult(context);
    if (result)
    {
      return std::move(*result);
    }
    throw std::runtime_error("Error sending message");
  }
}}}} // namespace Azure::Core::Amqp::_detail
