// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Enable declaration of strerror_s.
#define __STDC_WANT_LIB_EXT1__ 1

#include "../models/private/error_impl.hpp"
#include "../models/private/message_impl.hpp"
#include "../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/common/completion_operation.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "private/connection_impl.hpp"
#include "private/message_sender_impl.hpp"
#include "private/session_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/platform.hpp>

#if ENABLE_UAMQP
#include <azure_uamqp_c/message_sender.h>
#endif

#include <memory>

using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Diagnostics::_internal;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  void UniqueHandleHelper<MESSAGE_SENDER_INSTANCE_TAG>::FreeMessageSender(
      MESSAGE_SENDER_HANDLE value)
  {
    messagesender_destroy(value);
  }
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _internal {
  std::ostream& operator<<(std::ostream& stream, SenderSettleMode settleMode)
  {
    switch (settleMode)
    {
      case SenderSettleMode::Settled:
        stream << "Settled";
        break;
      case SenderSettleMode::Unsettled:
        stream << "Unsettled";
        break;
      case SenderSettleMode::Mixed:
        stream << "Mixed";
        break;
    }
    return stream;
  }

  Models::_internal::AmqpError MessageSender::Open(Context const& context)
  {
    return m_impl->Open(false, context);
  }

  Models::_internal::AmqpError MessageSender::HalfOpen(Context const& context)
  {
    return m_impl->Open(true, context);
  }

  void MessageSender::Close(Context const& context) { m_impl->Close(context); }

  std::tuple<MessageSendStatus, Models::_internal::AmqpError> MessageSender::Send(
      Models::AmqpMessage const& message,
      Context const& context)
  {
    return m_impl->Send(message, context);
  }

  std::uint64_t MessageSender::GetMaxMessageSize() const { return m_impl->GetMaxMessageSize(); }
  std::string MessageSender::GetLinkName() const { return m_impl->GetLinkName(); }
  MessageSender::~MessageSender() noexcept {}
  std::ostream& operator<<(std::ostream& stream, _internal::MessageSenderState state)
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
    }
    return stream;
  }

  std::ostream& operator<<(std::ostream& stream, _internal::MessageSendStatus status)
  {
    switch (status)
    {
      case _internal::MessageSendStatus::Invalid:
        stream << "Invalid";
        break;
      case _internal::MessageSendStatus::Cancelled:
        stream << "Cancelled";
        break;
      case _internal::MessageSendStatus::Error:
        stream << "Error";
        break;
      case _internal::MessageSendStatus::Ok:
        stream << "Ok";
        break;
      case _internal::MessageSendStatus::Timeout:
        stream << "Timeout";
        break;
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
#if ENABLE_UAMQP
    m_messageSender.reset(
        messagesender_create(*m_link, MessageSenderImpl::OnMessageSenderStateChangedFn, this));

    messagesender_set_trace(m_messageSender.get(), m_options.EnableTrace);
#endif
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

    if (m_link)
    {
      // Unsubscribe from any detach events before clearing out the event handler to short-circuit
      // any events firing after the object is destroyed.
      m_link->UnsubscribeFromDetachEvent();

      m_link.reset();
    }
#if ENABLE_UAMQP
    if (m_messageSender)
    {
      m_messageSender.reset();
    }
#endif
  }

  void MessageSenderImpl::CreateLink(_internal::LinkEndpoint& endpoint)
  {
    m_link = std::make_shared<_detail::LinkImpl>(
        m_session,
        endpoint,
        m_options.Name,
        _internal::SessionRole::Receiver, // This is the role of the link, not the endpoint.
        m_options.MessageSource,
        m_target,
        nullptr);
    PopulateLinkProperties();

    m_link->SubscribeToDetachEvent(
        [this](Models::_internal::AmqpError const& error) { OnLinkDetached(error); });
  }

  void MessageSenderImpl::CreateLink()
  {
    m_link = std::make_shared<_detail::LinkImpl>(
        m_session,
        m_options.Name,
        _internal::SessionRole::Sender, // This is the role of the link, not the endpoint.
        m_options.MessageSource,
        m_target,
        nullptr);
    PopulateLinkProperties();

    m_link->SubscribeToDetachEvent(
        [this](Models::_internal::AmqpError const& error) { OnLinkDetached(error); });
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
      m_link->SetMaxMessageSize((std::numeric_limits<uint64_t>::max)());
    }
    if (m_options.MaxLinkCredits != 0)
    {
      m_link->SetMaxLinkCredit(m_options.MaxLinkCredits);
    }
    m_link->SetSenderSettleMode(m_options.SettleMode);
  }

  std::uint64_t MessageSenderImpl::GetMaxMessageSize() const
  {
    if (!m_senderOpen)
    {
      throw std::runtime_error("Message sender is not open.");
    }
    // Get the max message size from the link (which is the max frame size for the link
    // endpoint) and the peer (which is the max frame size for the other end of the connection).
    //
    auto linkSize{m_link->GetMaxMessageSize()};
    auto peerSize{m_link->GetPeerMaxMessageSize()};

    // Return the smaller of the two values
    return (std::min)(linkSize, peerSize);
  }
#if ENABLE_UAMQP
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
        if (oldState == MESSAGE_SENDER_STATE_OPENING)
        {
          sender->m_openQueue.CompleteOperation(Models::_internal::AmqpError{
              Models::_internal::AmqpErrorCondition::InternalError,
              "Message Sender entered the Error State.",
              {}});
        }
        else
        {
          sender->m_sendCompleteQueue.CompleteOperation(
              _internal::MessageSendStatus::Error,
              {Azure::Core::Amqp::Models::_internal::AmqpErrorCondition::InternalError,
               "Message Sender unexpectedly entered the Error State.",
               {}});
        }
      }

      // If we're transitioning from Opening to Open, we're done with the open operation.
      if (oldState == MESSAGE_SENDER_STATE_OPENING && newState == MESSAGE_SENDER_STATE_OPEN)
      {
        sender->m_openQueue.CompleteOperation(Models::_internal::AmqpError{});
      }
      if (oldState == MESSAGE_SENDER_STATE_CLOSING && newState == MESSAGE_SENDER_STATE_IDLE)
      {
        sender->m_closeQueue.CompleteOperation(Models::_internal::AmqpError{});
      }
    }
  }
#endif

  Models::_internal::AmqpError MessageSenderImpl::Open(bool halfOpen, Context const& context)
  {
    Models::_internal::AmqpError rv;
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
    if (m_senderOpen)
    {
      throw std::runtime_error("Message sender is already open.");
    }
    {
      auto lock{m_session->GetConnection()->Lock()};
      if (m_link == nullptr)
      {
        CreateLink();
      }
#if ENABLE_UAMQP
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
#endif
      // Mark the connection as async so that we can use the async APIs.
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose) << "Opening message sender. Enable async operation.";
      }
      m_session->GetConnection()->EnableAsyncOperation(true);

      // Enable async on the link as well.
      Common::_detail::GlobalStateHolder::GlobalStateInstance()->AddPollable(m_link);
    }
    if (!halfOpen)
    {
      auto result = m_openQueue.WaitForResult(context);
      if (!result || std::get<0>(*result))
      {
        if (m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose) << "Opening message sender. Enable async operation.";
        }
        m_session->GetConnection()->EnableAsyncOperation(false);

        // Clean up from changes made earlier in the open, since the open was not successful.
        auto lock{m_session->GetConnection()->Lock()};

        m_link->UnsubscribeFromDetachEvent();

        Common::_detail::GlobalStateHolder::GlobalStateInstance()->RemovePollable(
            m_link); // This will ensure that the link is cleaned up on the next poll()
#if ENABLE_UAMQP
        messagesender_close(m_messageSender.get());
        m_link.reset();
        m_messageSender.reset();
#endif
        if (!result)
        {
          throw Azure::Core::OperationCancelledException(
              "Message sender open operation cancelled.");
        }
        else
        {
          rv = std::move(std::get<0>(*result));
        }
      }
    }
    // If the open was successful, then we're in the open state.
    if (!rv)
    {
      m_senderOpen = true;
    }
    return rv;
  }

  void MessageSenderImpl::Close(Context const& context)
  {
    if (m_senderOpen)
    {
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose) << "Closing message sender.";
      }

      Common::_detail::GlobalStateHolder::GlobalStateInstance()->RemovePollable(
          m_link); // This will ensure that the link is cleaned up on the next poll()

      bool shouldWaitForClose = m_currentState == _internal::MessageSenderState::Closing
          || m_currentState == _internal::MessageSenderState::Open;

      {
        if (m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose) << "Lock for Closing message sender.";
        }

        auto lock{m_session->GetConnection()->Lock()};

#if ENABLE_UAMQP
        if (messagesender_close(m_messageSender.get()))
        {
          throw std::runtime_error("Could not close message sender");
        }
#endif
      }
      // The message sender (and it's underlying link) is in the half open state. Wait until the
      // link has fully closed.
      if (shouldWaitForClose)
      {
        if (m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose)
              << "Wait for sender detach to complete. Current state: " << m_currentState;
        }

        auto result = m_closeQueue.WaitForResult(context);
        if (!result)
        {
          throw Azure::Core::OperationCancelledException(
              "Message sender close operation cancelled.");
        }
        if (std::get<0>(*result))
        {
          auto rv = std::move(std::get<0>(*result));
          if (rv)
          {
            throw std::runtime_error(
                "Message sender close operation failed: " + rv.Condition.ToString()
                + " description: " + rv.Description);
          }
        }
      }

      {
        auto lock{m_session->GetConnection()->Lock()};

        if (m_options.EnableTrace)

#if ENABLE_UAMQP
        {
          Log::Stream(Logger::Level::Verbose)
              << "Sender Unsubscribe from link detach event. Link instance: "
              << m_link->GetUnderlyingLink();
        }
#endif
        m_link->UnsubscribeFromDetachEvent();

        // Now that the connection is closed, the link is no longer needed. This will free the link
        m_link.reset();
      }
      m_session->GetConnection()->EnableAsyncOperation(false);

      m_senderOpen = false;
    }
  }

  void MessageSenderImpl::OnLinkDetached(Models::_internal::AmqpError const& error)
  {
    if (m_senderOpen)
    {
      if (m_events)
      {
        m_events->OnMessageSenderDisconnected(
            MessageSenderFactory::CreateFromInternal(shared_from_this()), error);
      }

      if (m_options.EnableTrace)
      {
        // Log that an error occurred.
        Log::Stream(Logger::Level::Warning) << "Message sender link detached: " << error;
      }

      // Cache the error we received in the OnDetach notification so we can return it to the user
      // on the next send which fails.
      m_savedMessageError = error;

      // When we've received a link detached, we can complete the close.
      m_closeQueue.CompleteOperation(error);
      m_openQueue.CompleteOperation(error);
    }
  }

#if ENABLE_UAMQP
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
          Models::_detail::AmqpValueFactory::FromImplementation(
              Models::_detail::UniqueAmqpValueHandle{amqpvalue_clone(disposition)}));
    }
  };
#endif

  void MessageSenderImpl::QueueSendInternal(
      Models::AmqpMessage const& message,
      Azure::Core::Amqp::_internal::MessageSender::MessageSendCompleteCallback onSendComplete,
      Context const& context)
  {
    // If the context is canceled, don't queue the operation.
    // Note that normally this would be handled via uAMQP's async operation cancellation, but if the
    // remote node sends an incoming frame, the async operation completion handler will be called
    // twice, which results in a double free of the underlying operation.
    if (!context.IsCancelled())
    {
#if ENABLE_UAMQP
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
#else
      (void)message;
      (void)onSendComplete;
      (void)context;
      throw std::runtime_error("Send operation is not supported.");
#endif
    }
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
#if ENABLE_UAMQP
                ERROR_HANDLE errorHandle;
                if (!amqpvalue_get_error(
                        Models::_detail::AmqpValueFactory::ToImplementation(firstState), &errorHandle))
                {
                  Models::_detail::UniqueAmqpErrorHandle uniqueError{
                      errorHandle}; // This will free the error handle when it goes out of scope.
                  error = Models::_detail::AmqpErrorFactory::FromImplementation(errorHandle);
                }
#endif
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
    else
    {
      Models::_internal::AmqpError error{
          Models::_internal::AmqpErrorCondition::OperationCancelled,
          "Message send operation cancelled.",
          {}};
      return std::make_tuple(_internal::MessageSendStatus::Cancelled, error);
    }
  }

  std::string MessageSenderImpl::GetLinkName() const { return m_link->GetName(); }

}}}} // namespace Azure::Core::Amqp::_detail
