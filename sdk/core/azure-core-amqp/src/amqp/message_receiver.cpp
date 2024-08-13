// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Enable declaration of strerror_s.
#define __STDC_WANT_LIB_EXT1__ 1

#include "azure/core/amqp/internal/message_receiver.hpp"

#include "../models/private/message_impl.hpp"
#include "../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/link.hpp"
#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "private/message_receiver_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/platform.hpp>

#if ENABLE_UAMQP
#include <azure_uamqp_c/message_receiver.h>
#endif

#include <memory>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Amqp::_internal;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  void UniqueHandleHelper<MESSAGE_RECEIVER_INSTANCE_TAG>::FreeMessageReceiver(
      MESSAGE_RECEIVER_HANDLE value)
  {
    messagereceiver_destroy(value);
  }
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  std::ostream& operator<<(std::ostream& stream, ReceiverSettleMode settleMode)
  {
    switch (settleMode)
    {
      case ReceiverSettleMode::First:
        stream << "First";
        break;
      case ReceiverSettleMode::Second:
        stream << "Second";
        break;
    }
    return stream;
  }

  MessageReceiver::~MessageReceiver() noexcept {}

  void MessageReceiver::Open(Context const& context)
  {
    if (m_impl)
    {
      m_impl->Open(context);
    }
    else
    {
      AZURE_ASSERT_FALSE("MessageReceiver::Open called on moved message receiver.");
    }
  }
  void MessageReceiver::Close(Context const& context)
  {
    if (m_impl)
    {
      m_impl->Close(context);
    }
  }
  std::string MessageReceiver::GetSourceName() const { return m_impl->GetSourceName(); }
  std::pair<std::shared_ptr<const Models::AmqpMessage>, Models::_internal::AmqpError>
  MessageReceiver::WaitForIncomingMessage(Azure::Core::Context const& context)
  {
    if (m_impl)
    {
      return m_impl->WaitForIncomingMessage(context);
    }
    else
    {
      AZURE_ASSERT_FALSE(
          "MessageReceiver::WaitForIncomingMessage called on moved message receiver.");
      Azure::Core::_internal::AzureNoReturnPath(
          "MessageReceiver::WaitForIncomingMessage called on moved message receiver.");
    }
  }

  std::pair<std::shared_ptr<const Models::AmqpMessage>, Models::_internal::AmqpError>
  MessageReceiver::TryWaitForIncomingMessage()
  {
    if (m_impl)
    {
      return m_impl->TryWaitForIncomingMessage();
    }
    else
    {
      AZURE_ASSERT_FALSE(
          "MessageReceiver::TryWaitForIncomingMessage called on moved message receiver.");
      Azure::Core::_internal::AzureNoReturnPath(
          "MessageReceiver::TryWaitForIncomingMessage called on moved message receiver.");
    }
  }

  std::string MessageReceiver::GetLinkName() const { return m_impl->GetLinkName(); }
  std::ostream& operator<<(std::ostream& stream, _internal::MessageReceiverState state)
  {
    switch (state)
    {
      case _internal::MessageReceiverState::Invalid:
        stream << "Invalid";
        break;
      case _internal::MessageReceiverState::Idle:
        stream << "Idle";
        break;
      case _internal::MessageReceiverState::Opening:
        stream << "Opening";
        break;
      case _internal::MessageReceiverState::Open:
        stream << "Open";
        break;
      case _internal::MessageReceiverState::Closing:
        stream << "Closing";
        break;
      case _internal::MessageReceiverState::Error:
        stream << "Error";
        break;
    }
    return stream;
  }

#if defined(_azure_TESTING_BUILD)
  void MessageReceiver::EnableLinkPolling()
  {
    if (m_impl)
    {
      m_impl->EnableLinkPolling();
    }
    else
    {
      AZURE_ASSERT_FALSE("MessageReceiver::EnableLinkPolling called on moved message receiver.");
    }
  }
#endif

}}}} // namespace Azure::Core::Amqp::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  /** Configure the MessageReceiver for receiving messages from a service instance.
   */
  MessageReceiverImpl::MessageReceiverImpl(
      std::shared_ptr<_detail::SessionImpl> session,
      Models::_internal::MessageSource const& source,
      MessageReceiverOptions const& options,
      MessageReceiverEvents* eventHandler)
      : m_options{options}, m_source{source}, m_session{session}, m_eventHandler(eventHandler)
  {
  }

  /** Configure the MessageReceiverImpl for receiving messages from a network listener.
   */
  MessageReceiverImpl::MessageReceiverImpl(
      std::shared_ptr<_detail::SessionImpl> session,
      LinkEndpoint& linkEndpoint,
      Models::_internal::MessageSource const& source,
      MessageReceiverOptions const& options,
      MessageReceiverEvents* eventHandler)
      : m_options{options}, m_source{source}, m_session{session}, m_eventHandler(eventHandler)
  {
    CreateLink(linkEndpoint);
#if ENABLE_UAMQP
    m_messageReceiver.reset(messagereceiver_create(
        *m_link, MessageReceiverImpl::OnMessageReceiverStateChangedFn, this));

    messagereceiver_set_trace(m_messageReceiver.get(), options.EnableTrace);
#endif

    // When creating a message receiver from a link endpoint, we don't want to enable polling on the
    // link at open time (because the Open call is made with the ConnectionLock held, resulting in a
    // deadlock.
    //
    // Instead, we'll defer the link polling until after MessageReceiver is opened and it's safe to
    // do so.
    m_deferLinkPolling = true;
  }

  void MessageReceiverImpl::CreateLink(LinkEndpoint& endpoint)
  {
#if ENABLE_UAMQP
    // The endpoint version of CreateLink is creating a message receiver for a sender, not for a
    // receiver.
    m_link = std::make_shared<_detail::LinkImpl>(
        m_session,
        endpoint,
        m_options.Name,
        SessionRole::Sender, // This is the role of the link, not the endpoint.
        m_source,
        m_options.MessageTarget,
        nullptr);
#endif
    PopulateLinkProperties();
#if ENABLE_UAMQP
    Log::Stream(Logger::Level::Verbose)
        << "MessageReceiver: Subscribe to link detach on:" << m_link->GetUnderlyingLink();

    m_link->SubscribeToDetachEvent(
        [this](Models::_internal::AmqpError const& error) { OnLinkDetached(error); });
#else
    (void)endpoint;
#endif
  }

  void MessageReceiverImpl::CreateLink()
  {
    m_link = std::make_shared<_detail::LinkImpl>(
        m_session,
        m_options.Name,
        SessionRole::Receiver,
        m_source,
        m_options.MessageTarget,
        nullptr);
    PopulateLinkProperties();

#if ENABLE_UAMQP
    Log::Stream(Logger::Level::Verbose)
        << "MessageReceiver: Subscribe to link detach on:" << m_link->GetUnderlyingLink();

    m_link->SubscribeToDetachEvent(
        [this](Models::_internal::AmqpError const& error) { OnLinkDetached(error); });
#endif
  }

  void MessageReceiverImpl::PopulateLinkProperties()
  {
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
    if (m_options.MaxLinkCredit != 0)
    {
      m_link->SetMaxLinkCredit(m_options.MaxLinkCredit);
    }
    m_link->SetAttachProperties(m_options.Properties.AsAmqpValue());
  }

#if ENABLE_UAMQP
  AMQP_VALUE MessageReceiverImpl::OnMessageReceivedFn(const void* context, MESSAGE_HANDLE message)
  {
    MessageReceiverImpl* receiver = static_cast<MessageReceiverImpl*>(const_cast<void*>(context));
    // There is a window where the receiver could be closed between the time the message is
    // received by the AMQP connection and when is indicated to the MessageReceiver. Ensure that
    // the message receiver is open before attempting to process the incoming message.
    if (receiver->m_receiverOpen)
    {
      auto incomingMessage(Models::_detail::AmqpMessageFactory::FromUamqp(message));
      Models::AmqpValue rv;
      if (receiver->m_eventHandler)
      {
        rv = receiver->m_eventHandler->OnMessageReceived(
            MessageReceiverFactory::CreateFromInternal(receiver->shared_from_this()),
            incomingMessage);
      }
      else
      {
        rv = receiver->OnMessageReceived(incomingMessage);
      }
      return amqpvalue_clone(Models::_detail::AmqpValueFactory::ToImplementation(rv));
    }

    return amqpvalue_clone(
        Models::_detail::AmqpValueFactory::ToImplementation(Models::_internal::Messaging::DeliveryRejected(
            Models::_internal::AmqpErrorCondition::ConnectionForced.ToString(),
            "Message Receiver is closed.",
            {})));
  }

  Models::AmqpValue MessageReceiverImpl::OnMessageReceived(
      std::shared_ptr<Models::AmqpMessage> const& message)
  {
    m_messageQueue.CompleteOperation(message, Models::_internal::AmqpError{});
    return Models::_internal::Messaging::DeliveryAccepted();
  }

  void MessageReceiverImpl::OnLinkDetached(Models::_internal::AmqpError const& error)
  {
    if (m_receiverOpen)
    {
      if (m_eventHandler)
      {
        m_eventHandler->OnMessageReceiverDisconnected(
            MessageReceiverFactory::CreateFromInternal(shared_from_this()), error);
      }

      // Log that an error occurred.
      Log::Stream(Logger::Level::Warning)
          << "Message receiver link detached: " + error.Condition.ToString() << ": "
          << error.Description;

      // Cache the error we received in the OnDetach notification so we can return it to the
      // user on the next receive which fails.
      m_savedMessageError = error;

      // When we've received a link detached, we can complete the close.
      m_closeQueue.CompleteOperation(error);
    }
  }
#endif

  std::pair<std::shared_ptr<Models::AmqpMessage>, Models::_internal::AmqpError>
  MessageReceiverImpl::WaitForIncomingMessage(Context const& context)
  {
    if (m_eventHandler)
    {
      throw std::runtime_error("Cannot call WaitForIncomingMessage when using an event handler.");
    }

    auto result = m_messageQueue.WaitForResult(context);
    if (result)
    {
      std::pair<std::shared_ptr<Models::AmqpMessage>, Models::_internal::AmqpError> rv;
      std::shared_ptr<Models::AmqpMessage> message{std::move(std::get<0>(*result))};
      if (message)
      {
        rv.first = std::move(message);
      }
      rv.second = std::move(std::get<1>(*result));
      return rv;
    }
    else
    {
      throw Azure::Core::OperationCancelledException("Receive Operation was cancelled.");
    }
  }
  std::pair<std::shared_ptr<Models::AmqpMessage>, Models::_internal::AmqpError>
  MessageReceiverImpl::TryWaitForIncomingMessage()
  {
    if (m_eventHandler)
    {
      throw std::runtime_error("Cannot call WaitForIncomingMessage when using an event handler.");
    }

    auto result = m_messageQueue.TryWaitForResult();
    if (result)
    {
      std::pair<std::shared_ptr<Models::AmqpMessage>, Models::_internal::AmqpError> rv;
      std::shared_ptr<Models::AmqpMessage> message{std::move(std::get<0>(*result))};
      if (message)
      {
        rv.first = std::move(message);
      }
      rv.second = std::move(std::get<1>(*result));
      return rv;
    }
    else
    {
      // There is no data available, let the caller know that there's nothing happening here.
      return {};
    }
  }

  void MessageReceiverImpl::EnableLinkPolling()
  {
    std::unique_lock<std::mutex> lock{m_mutableState};
    if (!m_linkPollingEnabled)
    {
      Common::_detail::GlobalStateHolder::GlobalStateInstance()->AddPollable(m_link);
      m_linkPollingEnabled = true;
    }
  }

  MessageReceiverImpl::~MessageReceiverImpl() noexcept
  {
    auto lock{m_session->GetConnection()->Lock()};

    if (m_receiverOpen)
    {
      AZURE_ASSERT_MSG(m_receiverOpen, "MessageReceiverImpl is being destroyed while open.");
      Azure::Core::_internal::AzureNoReturnPath(
          "MessageReceiverImpl is being destroyed while open.");
    }

    // If we're registered for events, null out the event handler, so we don't get called back
    // during the destroy.
    if (m_eventHandler)
    {
      m_eventHandler = nullptr;
    }
#if ENABLE_UAMQP
    if (m_messageReceiver)
    {
      m_messageReceiver.reset();
    }
#endif
    if (m_link)
    {
      m_link.reset();
    }
    m_messageQueue.Clear();
  }

#if ENABLE_UAMQP
  MessageReceiverState MessageReceiverStateFromLowLevel(MESSAGE_RECEIVER_STATE lowLevel)
  {
    switch (lowLevel)
    {
      case MESSAGE_RECEIVER_STATE_CLOSING:
        return MessageReceiverState::Closing;
      case MESSAGE_RECEIVER_STATE_ERROR:
        return MessageReceiverState::Error;
      case MESSAGE_RECEIVER_STATE_IDLE:
        return MessageReceiverState::Idle;
      case MESSAGE_RECEIVER_STATE_INVALID:
        return MessageReceiverState::Invalid;
      case MESSAGE_RECEIVER_STATE_OPEN:
        return MessageReceiverState::Open;
      case MESSAGE_RECEIVER_STATE_OPENING:
        return MessageReceiverState::Opening;
      default:
        throw std::logic_error("Unknown message receiver state.");
    }
  }

  const char* MESSAGE_RECEIVER_STATEStrings[] = {
      "MESSAGE_RECEIVER_STATE_INVALID",
      "MESSAGE_RECEIVER_STATE_IDLE",
      "MESSAGE_RECEIVER_STATE_OPENING",
      "MESSAGE_RECEIVER_STATE_OPEN",
      "MESSAGE_RECEIVER_STATE_CLOSING",
      "MESSAGE_RECEIVER_STATE_ERROR",
  };

  std::ostream& operator<<(std::ostream& stream, MESSAGE_RECEIVER_STATE state)
  {
    if (state < sizeof(MESSAGE_RECEIVER_STATEStrings) / sizeof(MESSAGE_RECEIVER_STATEStrings[0]))
    {
      stream << MESSAGE_RECEIVER_STATEStrings[static_cast<int>(state)];
    }
    else
    {
      stream << "Unknown MESSAGE_RECEIVER_STATE value: " << state;
    }
    return stream;
  }

  void MessageReceiverImpl::OnMessageReceiverStateChangedFn(
      void const* context,
      MESSAGE_RECEIVER_STATE newState,
      MESSAGE_RECEIVER_STATE oldState)
  {
    auto receiver = static_cast<MessageReceiverImpl*>(const_cast<void*>(context));
    receiver->m_currentState = MessageReceiverStateFromLowLevel(newState);

    if (receiver->m_options.EnableTrace)
    {
      Log::Stream(Logger::Level::Verbose)
          << "Message receiver state change " << oldState << " -> " << newState;
    }
    // If the message receiver isn't open, or if it's in the process of being destroyed, ignore
    // this notification.
    if (receiver->m_receiverOpen)
    {
      if (receiver->m_eventHandler)
      {
        receiver->m_eventHandler->OnMessageReceiverStateChanged(
            MessageReceiverFactory::CreateFromInternal(receiver->shared_from_this()),
            MessageReceiverStateFromLowLevel(newState),
            MessageReceiverStateFromLowLevel(oldState));
      }
      else
      {
        if (receiver->m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose)
              << "Message receiver changed state. Old: " << oldState << " -> New: " << newState;
        }
      }

      // If we are transitioning to the error state, we want to stick a response on the incoming
      // queue indicating an error occurred.
      if (newState == MESSAGE_RECEIVER_STATE_ERROR && oldState != MESSAGE_RECEIVER_STATE_ERROR)
      {
        if (receiver->m_savedMessageError)
        {
          receiver->m_messageQueue.CompleteOperation(nullptr, receiver->m_savedMessageError);
        }
        else
        {
          Models::_internal::AmqpError error;
          error.Condition = Models::_internal::AmqpErrorCondition::InternalError;
          error.Description = "Message receiver has transitioned to the error state.";
          receiver->m_messageQueue.CompleteOperation(nullptr, error);
        }
      }

      // When we transition from the closing to idle state, we can return from the close
      // operation.
      if (oldState == MESSAGE_RECEIVER_STATE_CLOSING && newState == MESSAGE_RECEIVER_STATE_IDLE)
      {
        Log::Stream(Logger::Level::Informational)
            << "Message receiver state changed from closing to idle. Receiver closed.";
        receiver->m_closeQueue.CompleteOperation(Models::_internal::AmqpError{});
      }
    }
  }
#endif

  void MessageReceiverImpl::Open(Azure::Core::Context const& context)
  {
    if (m_options.AuthenticationRequired)
    {
      m_session->GetConnection()->AuthenticateAudience(
          m_session, static_cast<std::string>(m_source.GetAddress()), context);
    }

    {
      auto lock{m_session->GetConnection()->Lock()};

      // Once we've authenticated the connection, establish the link and receiver.
      // We cannot do this before authenticating the client.
      if (!m_link)
      {
        CreateLink();
      }
#if ENABLE_UAMQP
      if (m_messageReceiver == nullptr)
      {
        m_messageReceiver.reset(messagereceiver_create(
            *m_link, MessageReceiverImpl::OnMessageReceiverStateChangedFn, this));
      }

      messagereceiver_set_trace(m_messageReceiver.get(), m_options.EnableTrace);

      if (messagereceiver_open(
              m_messageReceiver.get(), MessageReceiverImpl::OnMessageReceivedFn, this))
      {

        auto err = errno;
#if defined(AZ_PLATFORM_WINDOWS)
        char buf[256];
        strerror_s(buf, sizeof(buf), err);
#else
        std::string buf{strerror(err)};
#endif
        throw std::runtime_error(
            "Could not open message receiver. errno=" + std::to_string(err) + ", \"" + buf + "\".");
      }
      m_receiverOpen = true;
#endif

      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose) << "Opening message receiver. Start async";
      }

      // Mark the connection as async so that we can use the async APIs.
      m_session->GetConnection()->EnableAsyncOperation(true);
    }

    // And add the link to the list of pollable items.
    //
    // Note that you *cannot* hold any connection or link locks when calling AddPollable. This is
    // because the the AddPollable function attempts to lock the pollable and the RemovePollable
    // function blocks until any pollables have completed while holding the pollable lock.
    //
    // This can result in a deadlock because the polling thread is also going to acquire the
    // connection lock resulting in a deadlock.
    // If we're not deferring link polling, enable the async operation on the connection.
    if (!m_deferLinkPolling)
    {
      EnableLinkPolling();
    }
  }

  void MessageReceiverImpl::Close(Context const& context)
  {
    if (m_receiverOpen)
    {
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose) << "Lock for Closing message receiver.";
      }

      AZURE_ASSERT(m_link);

      bool shouldWaitForClose = m_currentState == _internal::MessageReceiverState::Closing
          || m_currentState == _internal::MessageReceiverState::Open;

      {
        std::unique_lock<std::mutex> lock{m_mutableState};
        if (m_linkPollingEnabled)
        {
          Common::_detail::GlobalStateHolder::GlobalStateInstance()->RemovePollable(
              m_link); // This will ensure that the link is cleaned up on the next poll()
          m_linkPollingEnabled = false;
        }
      }
      {
        auto lock{m_session->GetConnection()->Lock()};

        // Clear messages from the queue.
        m_messageQueue.Clear();
#if ENABLE_UAMQP
        if (messagereceiver_close(m_messageReceiver.get()))
        {
          throw std::runtime_error("Could not close message receiver");
        }
#endif
      }

      // Release the lock so that the polling thread can make forward progress delivering the
      // detach notification.
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose)
            << "Wait for receiver detach to complete. Current state: " << m_currentState;
      }

      if (shouldWaitForClose)
      {
        // At this point, the underlying link is in the "half closed" state.
        // We need to wait for the link to be fully closed before we can destroy it.
        auto closeResult = m_closeQueue.WaitForResult(context);
        if (!closeResult)
        {
          throw Azure::Core::OperationCancelledException(
              "MessageReceiver close operation was cancelled.");
        }
      }
      {
        auto lock{m_session->GetConnection()->Lock()};

        // We've received the close, we don't care about the detach event any more.
#if ENABLE_UAMQP
        if (m_options.EnableTrace)
        {
          Log::Stream(Logger::Level::Verbose)
              << "Receiver unsubscribe from link detach event on " << m_link->GetUnderlyingLink();
        }
        m_link->UnsubscribeFromDetachEvent();

        // Now that the connection is closed, the link is no longer needed. This will free the link
        m_link.reset();
#endif
      }
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose) << "Closing message receiver. Stop async";
      }
      m_session->GetConnection()->EnableAsyncOperation(false);

      m_receiverOpen = false;
    }
  }

  std::string MessageReceiverImpl::GetLinkName() const
  {
    const char* linkName = "";
#if ENABLE_UAMQP
    if (messagereceiver_get_link_name(m_messageReceiver.get(), &linkName))
    {
      throw std::runtime_error("Could not get link name");
    }
#endif
    return linkName;
  }

}}}} // namespace Azure::Core::Amqp::_detail
