// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Enable declaration of strerror_s.
#define __STDC_WANT_LIB_EXT1__ 1

#include "azure/core/amqp/message_receiver.hpp"

#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/connection_string_credential.hpp"
#include "azure/core/amqp/link.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "azure/core/amqp/models/messaging_values.hpp"
#include "azure/core/amqp/session.hpp"
#include "private/message_receiver_impl.hpp"

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/platform.hpp>

#include <azure_uamqp_c/message_receiver.h>

#include <iostream>
#include <memory>
#include <sstream>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Amqp::_internal;

namespace Azure { namespace Core { namespace _internal {
  void UniqueHandleHelper<MESSAGE_RECEIVER_INSTANCE_TAG>::FreeMessageReceiver(
      MESSAGE_RECEIVER_HANDLE value)
  {
    messagereceiver_destroy(value);
  }
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  MessageReceiver::~MessageReceiver() noexcept {}

  void MessageReceiver::Open(Azure::Core::Context const& context) { m_impl->Open(context); }
  void MessageReceiver::Close() { m_impl->Close(); }
  std::string MessageReceiver::GetSourceName() const { return m_impl->GetSourceName(); }
  std::pair<Azure::Nullable<Models::AmqpMessage>, Models::_internal::AmqpError>
  MessageReceiver::WaitForIncomingMessage(Azure::Core::Context const& context)
  {
    return m_impl->WaitForIncomingMessage(context);
  }
  std::string MessageReceiver::GetLinkName() const { return m_impl->GetLinkName(); }
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

    m_messageReceiver.reset(messagereceiver_create(
        *m_link, MessageReceiverImpl::OnMessageReceiverStateChangedFn, this));

    messagereceiver_set_trace(m_messageReceiver.get(), options.EnableTrace);
  }

  void MessageReceiverImpl::CreateLink(LinkEndpoint& endpoint)
  {
    // The endpoint version of CreateLink is creating a message receiver for a sender, not for a
    // receiver.
    m_link = std::make_shared<_detail::LinkImpl>(
        m_session,
        endpoint,
        m_options.Name,
        SessionRole::Sender, // This is the role of the link, not the endpoint.
        m_source,
        m_options.MessageTarget);
    PopulateLinkProperties();
  }

  void MessageReceiverImpl::CreateLink()
  {
    m_link = std::make_shared<_detail::LinkImpl>(
        m_session, m_options.Name, SessionRole::Receiver, m_source, m_options.MessageTarget);
    PopulateLinkProperties();

    m_link->SubscribeToDetachEvent([this](Models::_internal::AmqpError const& error) {
      if (m_eventHandler)
      {
        m_eventHandler->OnMessageReceiverDisconnected(error);
      }
      // Log that an error occurred.
      Log::Stream(Logger::Level::Error)
          << "Message receiver link detached: " + error.Condition.ToString() << ": "
          << error.Description;

      // Cache the error we received in the OnDetach notification so we can return it to the user
      // on the next send which fails.
      m_savedMessageError = error;
    });
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
      m_link->SetMaxMessageSize(std::numeric_limits<uint64_t>::max());
    }
    if (m_options.MaxLinkCredit != 0)
    {
      m_link->SetMaxLinkCredit(m_options.MaxLinkCredit);
    }
    m_link->SetAttachProperties(static_cast<Models::AmqpValue>(m_options.Properties));
  }

  AMQP_VALUE MessageReceiverImpl::OnMessageReceivedFn(const void* context, MESSAGE_HANDLE message)
  {
    MessageReceiverImpl* receiver = static_cast<MessageReceiverImpl*>(const_cast<void*>(context));
    // There is a window where the receiver could be closed between the time the message is
    // received by the AMQP connection and when is indicated to the MessageReceiver. Ensure that
    // the message receiver is open before attempting to process the incoming message.
    if (receiver->m_receiverOpen)
    {
      auto incomingMessage(Models::_internal::AmqpMessageFactory::FromUamqp(message));
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
      return amqpvalue_clone(rv);
    }

    return Models::_internal::Messaging::DeliveryRejected(
        Models::_internal::AmqpErrorCondition::ConnectionForced.ToString(),
        "Message Receiver is closed.");
  }

  Models::AmqpValue MessageReceiverImpl::OnMessageReceived(Models::AmqpMessage message)
  {
    m_messageQueue.CompleteOperation(message, Models::_internal::AmqpError{});
    return Models::_internal::Messaging::DeliveryAccepted();
  }

  std::pair<Azure::Nullable<Models::AmqpMessage>, Models::_internal::AmqpError>
  MessageReceiverImpl::WaitForIncomingMessage(Context const& context)
  {
    if (m_eventHandler)
    {
      throw std::runtime_error("Cannot call WaitForIncomingMessage when using an event handler.");
    }

    auto result = m_messageQueue.WaitForResult(context);
    if (result)
    {
      std::pair<Azure::Nullable<Models::AmqpMessage>, Models::_internal::AmqpError> rv;
      Models::AmqpMessage message{std::move(std::get<0>(*result))};
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

  MessageReceiverImpl::~MessageReceiverImpl() noexcept
  {
    auto lock{m_session->GetConnection()->Lock()};

    // If we're registered for events, null out the event handler, so we don't get called back
    // during the destroy.
    if (m_eventHandler)
    {
      m_eventHandler = nullptr;
    }
    if (m_receiverOpen)
    {
      Close();
    }
    m_messageQueue.Clear();
  }

  MessageReceiverState MessageReceiverStateFromLowLevel(MESSAGE_RECEIVER_STATE lowLevel)
  {
    switch (lowLevel)
    {
      case MESSAGE_RECEIVER_STATE_CLOSING:
        return MessageReceiverState::Closing;
      case MESSAGE_RECEIVER_STATE_ERROR: // LCOV_EXCL_LINE
        return MessageReceiverState::Error; // LCOV_EXCL_LINE
      case MESSAGE_RECEIVER_STATE_IDLE:
        return MessageReceiverState::Idle;
      case MESSAGE_RECEIVER_STATE_INVALID: // LCOV_EXCL_LINE
        return MessageReceiverState::Invalid; // LCOV_EXCL_LINE
      case MESSAGE_RECEIVER_STATE_OPEN:
        return MessageReceiverState::Open;
      case MESSAGE_RECEIVER_STATE_OPENING:
        return MessageReceiverState::Opening;
      default: // LCOV_EXCL_LINE
        throw std::logic_error("Unknown message receiver state."); // LCOV_EXCL_LINE
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

  void MessageReceiverImpl::OnMessageReceiverStateChangedFn(
      void const* context,
      MESSAGE_RECEIVER_STATE newState,
      MESSAGE_RECEIVER_STATE oldState)
  {
    auto receiver = static_cast<MessageReceiverImpl*>(const_cast<void*>(context));

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
            << "Message receiver changed state. New: " << MESSAGE_RECEIVER_STATEStrings[newState]
            << " Old: " << MESSAGE_RECEIVER_STATEStrings[oldState];
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
  }

  void MessageReceiverImpl::Open(Azure::Core::Context const& context)
  {
    if (m_options.AuthenticationRequired)
    {
      m_session->AuthenticateIfNeeded(static_cast<std::string>(m_source.GetAddress()), context);
    }

    // Once we've authenticated the connection, establish the link and receiver.
    // We cannot do this before authenticating the client.
    if (!m_link)
    {
      CreateLink();
    }
    if (m_messageReceiver == nullptr)
    {
      m_messageReceiver.reset(messagereceiver_create(
          *m_link, MessageReceiverImpl::OnMessageReceiverStateChangedFn, this));
    }

    messagereceiver_set_trace(m_messageReceiver.get(), m_options.EnableTrace);

    if (messagereceiver_open(
            m_messageReceiver.get(), MessageReceiverImpl::OnMessageReceivedFn, this))
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
          "Could not open message receiver. errno=" + std::to_string(err) + ", \"" + buf + "\".");
      // LCOV_EXCL_STOP
    }
    m_receiverOpen = true;

    // Mark the connection as async so that we can use the async APIs.
    m_session->GetConnection()->EnableAsyncOperation(true);
  }

  void MessageReceiverImpl::Close()
  {
    if (m_receiverOpen)
    {
      m_session->GetConnection()->EnableAsyncOperation(false);

      // Clear messages from the queue.
      m_messageQueue.Clear();
      if (messagereceiver_close(m_messageReceiver.get()))
      {
        throw std::runtime_error("Could not close message receiver"); // LCOV_EXCL_LINE
      }
      m_receiverOpen = false;
    }
  }

  std::string MessageReceiverImpl::GetLinkName() const
  {
    const char* linkName;
    if (messagereceiver_get_link_name(m_messageReceiver.get(), &linkName))
    {
      throw std::runtime_error("Could not get link name");
    }
    return std::string(linkName);
  }

}}}} // namespace Azure::Core::Amqp::_detail
