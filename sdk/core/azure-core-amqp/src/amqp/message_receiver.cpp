// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#include "azure/core/amqp/message_receiver.hpp"
#include "azure/core/amqp/connection_string_credential.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "azure/core/amqp/models/messaging_values.hpp"
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
      : m_eventHandler(eventHandler), m_options{options}, m_source{source}, m_session{session},
        m_connection{&connectionToPoll}, m_connectionCredential{credential}
  {
  }
  MessageReceiver::MessageReceiver(
      Session& session,
      Connection const& connectionToPoll,
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
      std::string const& source,
      MessageReceiverOptions const& options,
      MessageReceiverEvents* eventHandler)
      : m_eventHandler(eventHandler), m_options{options}, m_source{source}, m_session{session},
        m_connection{&connectionToPoll}, m_tokenCredential{credential}
  {
  }
  MessageReceiver::MessageReceiver(
      Session& session,
      std::string const& source,
      MessageReceiverOptions const& options,
      MessageReceiverEvents* eventHandler)
      : m_eventHandler(eventHandler), m_options{options}, m_source{source}, m_session{session},
        m_connection{nullptr}
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
      : m_eventHandler(eventHandler), m_options{options}, m_source{source}, m_session{session}
  {
    m_link = std::make_unique<_detail::Link>(
        session,
        linkEndpoint,
        options.Name,
        _detail::SessionRole::Sender, // This is the role of the link, not the endpoint.
        source,
        options.TargetName);

    m_messageReceiver = messagereceiver_create(
        m_link->Get(), MessageReceiver::OnMessageReceiverStateChangedFn, this);
  }

  AMQP_VALUE MessageReceiver::OnMessageReceivedFn(const void* context, MESSAGE_HANDLE message)
  {
    MessageReceiver* receiver = static_cast<MessageReceiver*>(const_cast<void*>(context));

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

  Azure::Core::Amqp::Models::Value MessageReceiver::OnMessageReceived(
      Azure::Core::Amqp::Models::Message message)
  {
    m_messageQueue.CompleteOperation(message);
    return Azure::Core::_internal::Amqp::Models::Messaging::DeliveryAccepted();
  }

  MessageReceiver::~MessageReceiver()
  {
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

  void MessageReceiver::OnMessageReceiverStateChangedFn(
      void const* context,
      MESSAGE_RECEIVER_STATE newState,
      MESSAGE_RECEIVER_STATE oldState)
  {
    auto receiver = static_cast<MessageReceiver*>(const_cast<void*>(context));

    if (receiver->m_eventHandler)
    {
      receiver->m_eventHandler->OnMessageReceiverStateChanged(
          *receiver,
          MessageReceiverStateFromLowLevel(newState),
          MessageReceiverStateFromLowLevel(oldState));
    }
    else
    {
      std::cout << "Message receiver changed state. New: "
                << MESSAGE_RECEIVER_STATEStrings[newState]
                << " Old: " << MESSAGE_RECEIVER_STATEStrings[oldState] << std::endl;
      receiver->OnStateChanged(
          MessageReceiverStateFromLowLevel(newState), MessageReceiverStateFromLowLevel(oldState));
    }
  }

  void MessageReceiver::OnStateChanged(MessageReceiverState newState, MessageReceiverState oldState)
  {
    m_stateChangeQueue.CompleteOperation(newState, oldState);
  }
  void MessageReceiver::Authenticate(
      CredentialType type,
      std::string const& audience,
      std::string const& token)
  {
    m_claimsBasedSecurity = std::make_unique<Cbs>(m_session, *m_connection);
    if (m_claimsBasedSecurity->Open() == CbsOpenResult::Ok)
    {
      auto result = m_claimsBasedSecurity->PutToken(
          (type == CredentialType::BearerToken ? CbsTokenType::Jwt : CbsTokenType::Sas),
          audience,
          token);
    }
    else
    {
      throw std::runtime_error("Could not open Claims Based Security.");
    }
  }

  void MessageReceiver::Open()
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
      m_link = std::make_unique<_detail::Link>(
          m_session,
          m_options.Name,
          _detail::SessionRole::Receiver,
          m_source,
          m_options.TargetName);
    }
    if (m_messageReceiver == nullptr)
    {
      m_messageReceiver = messagereceiver_create(
          m_link->Get(), MessageReceiver::OnMessageReceiverStateChangedFn, this);
    }

    if (messagereceiver_open(m_messageReceiver, MessageReceiver::OnMessageReceivedFn, this))
    {
      throw std::runtime_error("Could not open message receiver");
    }
  }

  void MessageReceiver::Close()
  {
    if (messagereceiver_close(m_messageReceiver))
    {
      throw std::runtime_error("Could not close message receiver");
    }
  }

  std::string MessageReceiver::GetLinkName()
  {
    const char* linkName;
    if (messagereceiver_get_link_name(m_messageReceiver, &linkName))
    {
      throw std::runtime_error("Could not get link name");
    }
    return std::string(linkName);
  }

  void MessageReceiver::SetTrace(bool traceEnabled)
  {
    messagereceiver_set_trace(m_messageReceiver, traceEnabled);
  }

}}}} // namespace Azure::Core::_internal::Amqp