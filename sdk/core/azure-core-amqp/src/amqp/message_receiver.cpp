// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/message_receiver.hpp"
#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/connection_string_credential.hpp"
#include "azure/core/amqp/link.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "azure/core/amqp/models/messaging_values.hpp"
#include "azure/core/amqp/private/message_receiver_impl.hpp"
#include "azure/core/amqp/session.hpp"
#include <azure/core/credentials/credentials.hpp>
// #include <azure_uamqp_c/link.h>
#include <azure_uamqp_c/message_receiver.h>
#include <iostream>
#include <memory>

namespace Azure { namespace Core { namespace _internal { namespace Amqp {
  /** Configure the MessageReceiver for receiving messages from a service instance.
   */
  MessageReceiver::MessageReceiver(
      Session& session,
      Connection const& connectionToPoll,
      std::shared_ptr<ConnectionStringCredential> credential,
      std::string const& source,
      MessageReceiverOptions const& options,
      MessageReceiverEvents* eventHandler)
      : m_impl{std::make_shared<_detail::MessageReceiverImpl>(
          session,
          connectionToPoll,
          credential,
          source,
          options,
          eventHandler)}
  {
  }
  MessageReceiver::MessageReceiver(
      Session& session,
      Connection const& connectionToPoll,
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
      std::string const& source,
      MessageReceiverOptions const& options,
      MessageReceiverEvents* eventHandler)
      : m_impl{std::make_shared<_detail::MessageReceiverImpl>(
          session,
          connectionToPoll,
          credential,
          source,
          options,
          eventHandler)}
  {
  }
  MessageReceiver::MessageReceiver(
      Session& session,
      std::string const& source,
      MessageReceiverOptions const& options,
      MessageReceiverEvents* eventHandler)
      : m_impl{
          std::make_shared<_detail::MessageReceiverImpl>(session, source, options, eventHandler)}
  {
  }

  /** Configure the MessageReceiver for receiving messages from a network listener.
   */
  MessageReceiver::MessageReceiver(
      Session const& session,
      LinkEndpoint& linkEndpoint,
      std::string const& source,
      MessageReceiverOptions const& options,
      MessageReceiverEvents* eventHandler)
      : m_impl{std::make_shared<_detail::MessageReceiverImpl>(
          session,
          linkEndpoint,
          source,
          options,
          eventHandler)}
  {
  }

  MessageReceiver::~MessageReceiver() noexcept {}

  MessageReceiver::operator bool() const { return m_impl.operator bool(); }

  void MessageReceiver::Open() { m_impl->Open(); }
  void MessageReceiver::Close() { m_impl->Close(); }
  void MessageReceiver::SetTrace(bool traceEnabled) { m_impl->SetTrace(traceEnabled); }
  Azure::Core::Amqp::Models::Message MessageReceiver::WaitForIncomingMessage(
      Connection& connection,
      Azure::Core::Context context)
  {
    return m_impl->WaitForIncomingMessage(context, connection);
  }
  std::string MessageReceiver::GetLinkName() const { return m_impl->GetLinkName(); }
  namespace _detail {

    /** Configure the MessageReceiver for receiving messages from a service instance.
     */
    MessageReceiverImpl::MessageReceiverImpl(
        Session& session,
        Connection const& connectionToPoll,
        std::shared_ptr<ConnectionStringCredential> credential,
        std::string const& source,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* eventHandler)
        : m_options{options}, m_source{source}, m_session{session}, m_connection{&connectionToPoll},
          m_connectionCredential{credential}, m_eventHandler(eventHandler)
    {
    }
    MessageReceiverImpl::MessageReceiverImpl(
        Session& session,
        Connection const& connectionToPoll,
        std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
        std::string const& source,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* eventHandler)
        : m_options{options}, m_source{source}, m_session{session}, m_connection{&connectionToPoll},
          m_tokenCredential{credential}, m_eventHandler(eventHandler)
    {
    }
    MessageReceiverImpl::MessageReceiverImpl(
        Session& session,
        std::string const& source,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* eventHandler)
        : m_options{options}, m_source{source}, m_session{session}, m_connection{nullptr},
          m_eventHandler(eventHandler)
    {
    }

    /** Configure the MessageReceiverImpl for receiving messages from a network listener.
     */
    MessageReceiverImpl::MessageReceiverImpl(
        Session const& session,
        LinkEndpoint& linkEndpoint,
        std::string const& source,
        MessageReceiverOptions const& options,
        MessageReceiverEvents* eventHandler)
        : m_options{options}, m_source{source}, m_session{session}, m_eventHandler(eventHandler)
    {
      CreateLink(linkEndpoint);

      m_messageReceiver = messagereceiver_create(
          *m_link, MessageReceiverImpl::OnMessageReceiverStateChangedFn, this);
    }

    void MessageReceiverImpl::CreateLink(LinkEndpoint& endpoint)
    {
      // The endpoint version of CreateLink is creating a message receiver for a sender, not for a
      // receiver.
      m_link = std::make_unique<_detail::Link>(
          m_session,
          endpoint,
          m_options.Name,
          SessionRole::Sender, // This is the role of the link, not the endpoint.
          m_source,
          m_options.TargetAddress);
      PopulateLinkProperties();
    }

    void MessageReceiverImpl::CreateLink()
    {
      m_link = std::make_unique<_detail::Link>(
          m_session, m_options.Name, SessionRole::Receiver, m_source, m_options.TargetAddress);
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
        m_link->SetMaxMessageSize(std::numeric_limits<uint64_t>::max());
      }
    }

    AMQP_VALUE MessageReceiverImpl::OnMessageReceivedFn(const void* context, MESSAGE_HANDLE message)
    {
      MessageReceiverImpl* receiver = static_cast<MessageReceiverImpl*>(const_cast<void*>(context));

      Azure::Core::Amqp::Models::Message incomingMessage(message);
      Azure::Core::Amqp::Models::Value rv;
      if (receiver->m_eventHandler)
      {
        rv = receiver->m_eventHandler->OnMessageReceived(incomingMessage);
      }
      else
      {
        rv = receiver->OnMessageReceived(incomingMessage);
      }
      return amqpvalue_clone(rv);
    }

    Azure::Core::Amqp::Models::Value MessageReceiverImpl::OnMessageReceived(
        Azure::Core::Amqp::Models::Message message)
    {
      m_messageQueue.CompleteOperation(message);
      return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryAccepted();
    }

    MessageReceiverImpl::~MessageReceiverImpl() noexcept
    {
      // If we're registered for events, null out the event handler, so we don't get called back
      // during the destroy.
      if (m_eventHandler)
      {
        m_eventHandler = nullptr;
      }
      if (m_claimsBasedSecurity)
      {
        m_claimsBasedSecurity->Close();
      }
      if (m_messageReceiver)
      {
        messagereceiver_destroy(m_messageReceiver);
        m_messageReceiver = nullptr;
      }
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
            receiver->shared_from_this(),
            MessageReceiverStateFromLowLevel(newState),
            MessageReceiverStateFromLowLevel(oldState));
      }
      else
      {
        std::cout << "Message receiver changed state. New: "
                  << MESSAGE_RECEIVER_STATEStrings[newState]
                  << " Old: " << MESSAGE_RECEIVER_STATEStrings[oldState] << std::endl;
      }
    }

    void MessageReceiverImpl::Authenticate(
        CredentialType type,
        std::string const& audience,
        std::string const& token)
    {
      m_claimsBasedSecurity = std::make_unique<ClaimsBasedSecurity>(m_session, *m_connection);
      if (m_claimsBasedSecurity->Open() == CbsOpenResult::Ok)
      {
        auto result = m_claimsBasedSecurity->PutToken(
            (type == CredentialType::BearerToken ? CbsTokenType::Jwt : CbsTokenType::Sas),
            audience,
            token);
      }
      else
      {
        throw std::runtime_error("Could not open Claims Based Security."); // LCOV_EXCL_LINE
      }
    }

    void MessageReceiverImpl::Open()
    {
      // If we need to authenticate with either ServiceBus or BearerToken, now is the time to do it.
      if (m_connectionCredential)
      {
        auto sasCredential{std::static_pointer_cast<
            Azure::Core::_internal::Amqp::ServiceBusSasConnectionStringCredential>(
            m_connectionCredential)};
        Authenticate(
            CredentialType::ServiceBusSas,
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
            m_source,
            m_tokenCredential->GetToken(requestContext, context).Token);
      }

      // Once we've authenticated the connection, establish the link and receiver.
      // We cannot do this before authenticating the client.
      if (!m_link)
      {
        CreateLink();
      }
      if (m_messageReceiver == nullptr)
      {
        m_messageReceiver = messagereceiver_create(
            *m_link, MessageReceiverImpl::OnMessageReceiverStateChangedFn, this);
      }

      if (messagereceiver_open(m_messageReceiver, MessageReceiverImpl::OnMessageReceivedFn, this))
      {
        auto err = errno; // LCOV_EXCL_LINE
        throw std::runtime_error( // LCOV_EXCL_LINE
            "Could not open message receiver. errno=" + std::to_string(err) // LCOV_EXCL_LINE
            + ", \"" // LCOV_EXCL_LINE
            + strerror(err) // LCOV_EXCL_LINE
            + "\"."); // LCOV_EXCL_LINE
      }
    }

    void MessageReceiverImpl::Close()
    {
      if (messagereceiver_close(m_messageReceiver))
      {
        throw std::runtime_error("Could not close message receiver"); // LCOV_EXCL_LINE
      }
    }

    std::string MessageReceiverImpl::GetLinkName() const
    {
      const char* linkName;
      if (messagereceiver_get_link_name(m_messageReceiver, &linkName))
      {
        throw std::runtime_error("Could not get link name");
      }
      return std::string(linkName);
    }

    void MessageReceiverImpl::SetTrace(bool traceEnabled)
    {
      messagereceiver_set_trace(m_messageReceiver, traceEnabled);
    }
  } // namespace _detail
}}}} // namespace Azure::Core::_internal::Amqp
