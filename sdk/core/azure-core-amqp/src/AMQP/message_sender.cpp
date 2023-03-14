#include "azure/core/amqp/message_sender.hpp"
#include "azure/core/amqp/claims_based_security.hpp"
#include "azure/core/amqp/common/completion_operation.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "azure/core/amqp/models/messaging_values.hpp"
#include "azure/core/amqp/session.hpp"
#include <azure/core/credentials/credentials.hpp>
#include <azure_uamqp_c/message_sender.h>
#include <memory>

namespace Azure { namespace Core { namespace _internal { namespace Amqp {

  MessageSender::MessageSender(
      Session const& session,
      std::string const& target,
      Connection const& connection,
      MessageSenderOptions const& options)
      : m_session{session}, m_connection{connection}, m_options{options}, m_target{target}
  {
  }

  MessageSender::MessageSender(
      Session const& session,
      std::shared_ptr<ServiceBusSasConnectionStringCredential> credential,
      std::string const& target,
      Connection const& connection,
      MessageSenderOptions const& options)
      : m_session{session}, m_connection{connection}, m_options{options}, m_target{target},
        m_connectionCredential{credential}
  {
  }

  MessageSender::MessageSender(
      Session const& session,
      std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential,
      std::string const& target,
      Connection const& connection,
      MessageSenderOptions const& options)
      : m_session{session}, m_connection{connection}, m_options{options}, m_target{target},
        m_tokenCredential{credential}
  {
  }

  MessageSender::~MessageSender()
  {
    if (m_messageSender)
    {
      messagesender_destroy(m_messageSender);
      m_messageSender = nullptr;
    }
  }

  MessageSenderState MessageSenderStateFromLowLevel(MESSAGE_SENDER_STATE lowLevel)
  {
    switch (lowLevel)
    {
      case MESSAGE_SENDER_STATE_CLOSING:
        return MessageSenderState::Closing;
      case MESSAGE_SENDER_STATE_ERROR:
        return MessageSenderState::Error;
      case MESSAGE_SENDER_STATE_IDLE:
        return MessageSenderState::Idle;
      case MESSAGE_SENDER_STATE_INVALID:
        return MessageSenderState::Invalid;
      case MESSAGE_SENDER_STATE_OPEN:
        return MessageSenderState::Open;
      case MESSAGE_SENDER_STATE_OPENING:
        return MessageSenderState::Opening;
      default:
        throw std::logic_error("Unknown message receiver state.");
    }
  }

  void MessageSender::OnMessageSenderStateChangedFn(
      void* context,
      MESSAGE_SENDER_STATE newState,
      MESSAGE_SENDER_STATE oldState)
  {
    auto receiver = static_cast<MessageSender*>(const_cast<void*>(context));
    receiver->OnStateChanged(
        MessageSenderStateFromLowLevel(newState), MessageSenderStateFromLowLevel(oldState));
  }

  void MessageSender::OnStateChanged(MessageSenderState newState, MessageSenderState oldState)
  {
    m_stateChangeQueue.CompleteOperation(newState, oldState);
  }

  void MessageSender::Authenticate(
      CredentialType type,
      std::string const& audience,
      std::string const& token)
  {
    m_claimsBasedSecurity = std::make_unique<Cbs>(m_session, m_connection);
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

  void MessageSender::Open()
  {
    // If we need to authenticate with either ServiceBus or BearerToken, now is the time to do it.
    if (m_connectionCredential)
    {
      auto sasCredential{std::static_pointer_cast<
          Azure::Core::_internal::Amqp::ServiceBusSasConnectionStringCredential>(
          m_connectionCredential)};
      Authenticate(
          sasCredential->GetCredentialType(),
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
          m_target,
          m_tokenCredential->GetToken(requestContext, context).Token);
    }

    // We cannot create the link until after we've authenticated the connection because the
    // OnNewLink notification will call into unattached links.
    m_link = std::make_unique<_detail::Link>(
        m_session, m_options.Name, _detail::SessionRole::Sender, m_options.SourceAddress, m_target);
    m_link->SetMaxMessageSize(m_options.MaxMessageSize);
    m_link->SetSenderSettleMode(m_options.SenderSettleMode);

    m_messageSender
        = messagesender_create(m_link->Get(), MessageSender::OnMessageSenderStateChangedFn, this);

    if (messagesender_open(m_messageSender))
    {
      throw std::runtime_error("Could not open message sender");
    }
  }

  void MessageSender::Close()
  {
    if (messagesender_close(m_messageSender))
    {
      throw std::runtime_error("Could not close message sender");
    }
  }

  template <typename CompleteFn> struct WrapSendComplete
  {
    static void OnOperation(
        CompleteFn onComplete,
        MESSAGE_SEND_RESULT sendResult,
        AMQP_VALUE disposition)
    {
      MessageSendResult result{MessageSendResult::Ok};
      switch (sendResult)
      {
        case MESSAGE_SEND_OK:
          result = MessageSendResult::Ok;
          break;
        case MESSAGE_SEND_CANCELLED:
          result = MessageSendResult::Cancelled;
          break;
        case MESSAGE_SEND_ERROR:
          result = MessageSendResult::Error;
          break;
        case MESSAGE_SEND_TIMEOUT:
          result = MessageSendResult::Timeout;
          break;
      }
      onComplete(result, disposition);
    }
  };

  void MessageSender::SendAsync(
      Azure::Core::Amqp::Models::Message const& message,
      MessageSendCompleteCallback onSendComplete)
  {
    auto operation(std::make_unique<Azure::Core::_internal::Amqp::Common::CompletionOperation<
                       decltype(onSendComplete),
                       WrapSendComplete<decltype(onSendComplete)>>>(onSendComplete));
    auto result = messagesender_send_async(
        m_messageSender,
        message,
        std::remove_pointer<decltype(operation)::element_type>::type::OnOperationFn,
        operation.release(),
        0 /*timeout*/);
    if (result == nullptr)
    {
      throw std::runtime_error("Could not send message");
    }
  }

  std::tuple<MessageSendResult, Azure::Core::Amqp::Models::Value> MessageSender::Send(
      Azure::Core::Amqp::Models::Message const& message)
  {
    Azure::Core::_internal::Amqp::Common::AsyncOperationQueue<
        Azure::Core::_internal::Amqp::MessageSendResult,
        Azure::Core::Amqp::Models::Value>
        sendCompleteQueue;
    SendAsync(
        message,
        [&](Azure::Core::_internal::Amqp::MessageSendResult sendResult,
            Azure::Core::Amqp::Models::Value deliveryStatus) {
          //          std::cout << "Send Complete!" << std::endl;
          sendCompleteQueue.CompleteOperation(sendResult, deliveryStatus);
        });
    //    auto result = sendCompleteQueue.WaitForResult();
    auto result = sendCompleteQueue.WaitForPolledResult(m_connection);
    return std::move(*result);
  }

}}}} // namespace Azure::Core::_internal::Amqp
