// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Enable declaration of strerror_s.
#define __STDC_WANT_LIB_EXT1__ 1

#include "azure/core/amqp/internal/message_receiver.hpp"

#include "../../../models/private/message_impl.hpp"
#include "../../../models/private/value_impl.hpp"
#include "azure/core/amqp/internal/link.hpp"
#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "private/message_receiver_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/platform.hpp>

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

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

#if ENABLE_UAMQP
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

    // When creating a message receiver from a link endpoint, we don't want to enable polling on the
    // link at open time (because the Open call is made with the ConnectionLock held, resulting in a
    // deadlock.
    //
    // Instead, we'll defer the link polling until after MessageReceiver is opened and it's safe to
    // do so.
    m_deferLinkPolling = true;
  }
#elif ENABLE_RUST_AMQP
  /** Configure the MessageReceiver for receiving messages from a service instance.
   */
  MessageReceiverImpl::MessageReceiverImpl(
      std::shared_ptr<_detail::SessionImpl> session,
      Models::_internal::MessageSource const& source,
      MessageReceiverOptions const& options)
      : m_options{options}, m_source{source}, m_session{session}
  {
  }

#endif

  void MessageReceiverImpl::CreateLink(LinkEndpoint& endpoint)
  {
    PopulateLinkProperties();
    (void)endpoint;
  }

  void MessageReceiverImpl::CreateLink()
  {
    m_link = std::make_shared<_detail::LinkImpl>(
        m_session, m_options.Name, SessionRole::Receiver, m_source, m_options.MessageTarget);
    PopulateLinkProperties();
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

  std::pair<std::shared_ptr<Models::AmqpMessage>, Models::_internal::AmqpError>
  MessageReceiverImpl::WaitForIncomingMessage(Context const& context)
  {
#if ENABLE_UAMQP
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
#elif ENABLE_RUST_AMQP
    throw std::runtime_error("Not implemented");
    (void)context;
#endif
  }
  std::pair<std::shared_ptr<Models::AmqpMessage>, Models::_internal::AmqpError>
  MessageReceiverImpl::TryWaitForIncomingMessage()
  {
#if ENABLE_UAMQP
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
#elif ENABLE_RUST_AMQP
    throw std::runtime_error("Not implemented");
#endif
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

#if ENABLE_UAMQP
    // If we're registered for events, null out the event handler, so we don't get called back
    // during the destroy.
    if (m_eventHandler)
    {
      m_eventHandler = nullptr;
    }
#endif
    if (m_link)
    {
      m_link.reset();
    }
    m_messageQueue.Clear();
  }

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
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose) << "Opening message receiver. Start async";
      }
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
      }
      {
        auto lock{m_session->GetConnection()->Lock()};

        // Clear messages from the queue.
        m_messageQueue.Clear();
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
      }
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose) << "Closing message receiver. Stop async";
      }
      m_receiverOpen = false;
    }
  }

  std::string MessageReceiverImpl::GetLinkName() const
  {
    const char* linkName = "";
    return linkName;
  }

}}}} // namespace Azure::Core::Amqp::_detail
