// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/management.hpp"

#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/internal/session.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "private/connection_impl.hpp"
#include "private/management_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <azure_uamqp_c/amqp_management.h>

#include <iostream>
#include <memory>
#include <string>
#include <tuple>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _internal {

  /**
   * @brief Open the management instance.
   *
   * @returns A tuple consisting of the status code for the open and the description of the
   * status.
   */
  ManagementOpenStatus ManagementClient::Open(Context const& context)
  {
    return m_impl->Open(context);
  }

  /**
   * @brief Close the management instance.
   */
  void ManagementClient::Close() { m_impl->Close(); }

  ManagementOperationResult ManagementClient::ExecuteOperation(
      std::string const& operationToPerform,
      std::string const& typeOfOperation,
      std::string const& locales,
      Models::AmqpMessage messageToSend,
      Context const& context)
  {
    return m_impl->ExecuteOperation(
        operationToPerform, typeOfOperation, locales, messageToSend, context);
  }
}}}} // namespace Azure::Core::Amqp::_internal

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  ManagementClientImpl::ManagementClientImpl(
      std::shared_ptr<SessionImpl> session,
      std::string const& managementEntityPath,
      Azure::Core::Amqp::_internal::ManagementClientOptions const& options,
      Azure::Core::Amqp::_internal::ManagementClientEvents* managementEvents)
      : m_options{options}, m_session{session}, m_eventHandler{managementEvents},
        m_managementEntityPath{managementEntityPath}
  {
  }
  ManagementClientImpl::~ManagementClientImpl() noexcept
  {
    m_eventHandler = nullptr;
    if (m_isOpen)
    {
      AZURE_ASSERT_MSG(!m_isOpen, "Management being destroyed while open.");
      Azure::Core::_internal::AzureNoReturnPath("Management is being destroyed while open.");
    }
  }

  _internal::ManagementOpenStatus ManagementClientImpl::Open(Context const& context)
  {
    /** Authentication needs to happen *before* the management object is created.
     *
     * Note that we ONLY enable authentication if we know we're talking to the management node.
     * Other nodes require their own authentication.
     */
    if (m_options.ManagementNodeName == "$management")
    {
      m_accessToken = m_session->GetConnection()->AuthenticateAudience(
          m_session, m_managementEntityPath + "/" + m_options.ManagementNodeName, context);
    }
    {
      _internal::MessageSenderOptions messageSenderOptions;
      messageSenderOptions.EnableTrace = m_options.EnableTrace;
      messageSenderOptions.MessageSource = m_options.ManagementNodeName;
      messageSenderOptions.Name = m_options.ManagementNodeName + "-sender";
      messageSenderOptions.AuthenticationRequired = false;

      m_messageSender = std::make_shared<MessageSenderImpl>(
          m_session, m_options.ManagementNodeName, messageSenderOptions, this);
    }
    {
      _internal::MessageReceiverOptions messageReceiverOptions;
      messageReceiverOptions.EnableTrace = m_options.EnableTrace;
      messageReceiverOptions.MessageTarget = m_options.ManagementNodeName;
      messageReceiverOptions.Name = m_options.ManagementNodeName + "-receiver";
      messageReceiverOptions.AuthenticationRequired = false;

      m_messageReceiver = std::make_shared<MessageReceiverImpl>(
          m_session, m_options.ManagementNodeName, messageReceiverOptions, this);
    }

    // Now open the message sender and receiver.
    SetState(ManagementState::Opening);
    try
    {
      m_messageSender->Open(context);
      m_messageSenderOpen = true;
      m_messageReceiver->Open(context);
      m_messageReceiverOpen = true;
    }
    catch (std::runtime_error const& e)
    {
      Log::Stream(Logger::Level::Warning)
          << "Exception thrown opening message sender and receiver." << e.what();
      return _internal::ManagementOpenStatus::Error;
    }

    // And finally, wait for the message sender and receiver to finish opening before we return.
    auto result = m_openCompleteQueue.WaitForResult(context);
    if (result)
    {
      // If the message sender or receiver failed to open, we need to close them
      _internal::ManagementOpenStatus rv = std::get<0>(*result);
      if (rv != _internal::ManagementOpenStatus::Ok)
      {
        Log::Stream(Logger::Level::Warning) << "Management operation failed to open.";
        m_messageSender->Close();
        m_messageSenderOpen = false;
        m_messageReceiver->Close();
        m_messageReceiverOpen = false;
      }
      else
      {
        m_isOpen = true;
      }
      return rv;
    }
    // If result is null, then it means that the context was cancelled.
    return _internal::ManagementOpenStatus::Cancelled;
  }

  _internal::ManagementOperationResult ManagementClientImpl::ExecuteOperation(
      std::string const& operationToPerform,
      std::string const& typeOfOperation,
      std::string const& locales,
      Models::AmqpMessage messageToSend,
      Context const& context)
  {
    // If the connection is authenticated, include the token in the message.
    if (!m_accessToken.Token.empty())
    {
      messageToSend.ApplicationProperties["security_token"]
          = Models::AmqpValue{m_accessToken.Token};
    }
    messageToSend.ApplicationProperties.emplace("operation", operationToPerform);
    messageToSend.ApplicationProperties.emplace("type", typeOfOperation);
    if (!locales.empty())
    {
      messageToSend.ApplicationProperties.emplace("locales", locales);
    }
    messageToSend.Properties.MessageId = m_nextMessageId;
    m_expectedMessageId = m_nextMessageId;
    m_sendCompleted = false;
    m_nextMessageId++;

    auto sendResult = m_messageSender->Send(messageToSend, context);
    if (std::get<0>(sendResult) != _internal::MessageSendStatus::Ok)
    {
      _internal::ManagementOperationResult rv;
      rv.Status = _internal::ManagementOperationStatus::Error;
      rv.StatusCode = 500;
      rv.Error = std::get<1>(sendResult);
      rv.Message = nullptr;
      return rv;
    }
    auto result = m_messageQueue.WaitForResult(context);
    if (result)
    {
      _internal::ManagementOperationResult rv;
      rv.Status = std::get<0>(*result);
      rv.StatusCode = std::get<1>(*result);
      rv.Error = std::get<2>(*result);
      rv.Message = std::get<3>(*result);
      return rv;
    }
    else
    {
      throw Azure::Core::OperationCancelledException("Management operation cancelled.");
    }
  }

  void ManagementClientImpl::SetState(ManagementState newState) { m_state = newState; }

  void ManagementClientImpl::Close()
  {
    SetState(ManagementState::Closing);
    if (m_messageSender && m_messageSenderOpen)
    {
      m_messageSender->Close();
      m_messageSenderOpen = false;
    }
    if (m_messageReceiver && m_messageReceiverOpen)
    {
      m_messageReceiver->Close();
      m_messageReceiverOpen = false;
    }
    m_isOpen = false;
  }

  void ManagementClientImpl::OnMessageSenderStateChanged(
      _internal::MessageSender const&,
      _internal::MessageSenderState newState,
      _internal::MessageSenderState oldState)
  {
    if (newState == oldState)
    {
      Log::Stream(Logger::Level::Verbose)
          << "OnMessageSenderStateChanged: newState == oldState" << std::endl;
      return;
    }

    if (m_options.EnableTrace)
    {
      Log::Stream(Logger::Level::Informational)
          << "OnMessageSenderStateChanged: " << oldState << " -> " << newState << std::endl;
    }

    switch (m_state)
    {
      case ManagementState::Opening:
        switch (newState)
        {
            // If the message sender is opening, we don't need to do anything.
          case _internal::MessageSenderState::Opening:
            break;
            // If the message sender is open, remember it. If the message receiver is also
            // open, complete the outstanding open.
          case _internal::MessageSenderState::Open:
            m_messageSenderOpen = true;
            if (m_messageReceiverOpen)
            {
              SetState(ManagementState::Open);
              m_openCompleteQueue.CompleteOperation(_internal::ManagementOpenStatus::Ok);
            }
            break;
            // If the message sender is transitioning to an error or state other than open,
            // it's an error.
          default:
          case _internal::MessageSenderState::Idle:
          case _internal::MessageSenderState::Closing:
          case _internal::MessageSenderState::Error:
            Log::Stream(Logger::Level::Warning) << "Message Sender Changed State to " << newState
                                                << " while management client is opening";
            SetState(ManagementState::Closing);
            m_openCompleteQueue.CompleteOperation(_internal::ManagementOpenStatus::Error);
            break;
        }
        break;
      case ManagementState::Open:
        switch (newState)
        {
            // If the message sender goes to a non-open state, it's an error.
          default:
          case _internal::MessageSenderState::Idle:
          case _internal::MessageSenderState::Closing:
          case _internal::MessageSenderState::Error:
            Log::Stream(Logger::Level::Warning) << "Message Sender Changed State to " << newState
                                                << " while management client is open";
            SetState(ManagementState::Closing);
            if (m_eventHandler)
            {
              m_eventHandler->OnError(Models::_internal::AmqpError{});
            }
            break;
            // Ignore message sender open changes.
          case _internal::MessageSenderState::Open:
            break;
        }
        break;
      case ManagementState::Closing:
        switch (newState)
        {
            // If the message sender goes to a non-open state, it's an error.
          default:
          case _internal::MessageSenderState::Open:
          case _internal::MessageSenderState::Opening:
          case _internal::MessageSenderState::Error:
            Log::Stream(Logger::Level::Warning) << "Message Sender Changed State to " << newState
                                                << " while management client is closing";
            SetState(ManagementState::Closing);
            if (m_eventHandler)
            {
              m_eventHandler->OnError(Models::_internal::AmqpError{});
            }
            break;
          // Ignore message sender closing or idle state changes if we're closing.
          case _internal::MessageSenderState::Idle:
          case _internal::MessageSenderState::Closing:
            break;
        }
        break;
      case ManagementState::Idle:
      case ManagementState::Error:
        Log::Stream(Logger::Level::Warning)
            << "Message sender state changed to " << newState
            << " when management client is in the error state, ignoring.";
        break;
    }
  }

  void ManagementClientImpl::OnMessageSenderDisconnected(Models::_internal::AmqpError const& error)
  {
    Log::Stream(Logger::Level::Warning) << "Message sender disconnected: " << error << std::endl;
    SetState(ManagementState::Error);
    if (m_eventHandler)
    {
      m_eventHandler->OnError(error);
    }
  }

  void ManagementClientImpl::OnMessageReceiverStateChanged(
      _internal::MessageReceiver const&,
      _internal::MessageReceiverState newState,
      _internal::MessageReceiverState oldState)
  {
    if (newState == oldState)
    {
      Log::Stream(Logger::Level::Verbose)
          << "OnMessageReceiverStateChanged: newState == oldState" << std::endl;
      return;
    }

    if (m_options.EnableTrace)
    {
      Log::Stream(Logger::Level::Informational)
          << "OnMessageReceiverStateChanged: " << oldState << " -> " << newState << std::endl;
    }

    switch (m_state)
    {
      case ManagementState::Opening:
        switch (newState)
        {
            // If the message sender is opening, we don't need to do anything.
          case _internal::MessageReceiverState::Opening:
            break;
            // If the message receiver is open, remember it. If the message sender is also
            // open, complete the outstanding open.
          case _internal::MessageReceiverState::Open:
            m_messageReceiverOpen = true;
            if (m_messageSenderOpen)
            {
              SetState(ManagementState::Open);
              m_openCompleteQueue.CompleteOperation(_internal::ManagementOpenStatus::Ok);
            }
            break;
            // If the message receiver is transitioning to an error or state other than open,
            // it's an error.
          default:
          case _internal::MessageReceiverState::Idle:
          case _internal::MessageReceiverState::Closing:
          case _internal::MessageReceiverState::Error:
            Log::Stream(Logger::Level::Warning)
                << "Message Receiver Changed State to "
                << static_cast<std::underlying_type<decltype(newState)>::type>(newState)
                << " while management client is opening";
            SetState(ManagementState::Closing);
            m_openCompleteQueue.CompleteOperation(_internal::ManagementOpenStatus::Error);
            break;
        }
        break;
      case ManagementState::Open:
        switch (newState)
        {
            // If the message sender goes to a non-open state, it's an error.
          default:
          case _internal::MessageReceiverState::Idle:
          case _internal::MessageReceiverState::Closing:
          case _internal::MessageReceiverState::Error:
            Log::Stream(Logger::Level::Warning)
                << "Message Sender Changed State to "
                << static_cast<std::underlying_type<decltype(newState)>::type>(newState)
                << " while management client is open";
            SetState(ManagementState::Closing);
            if (m_eventHandler)
            {
              m_eventHandler->OnError(Models::_internal::AmqpError{});
            }
            break;
            // Ignore message sender open changes.
          case _internal::MessageReceiverState::Open:
            break;
        }
        break;
      case ManagementState::Closing:
        switch (newState)
        {
            // If the message sender goes to a non-open state, it's an error.
          default:
          case _internal::MessageReceiverState::Open:
          case _internal::MessageReceiverState::Opening:
          case _internal::MessageReceiverState::Error:
            Log::Stream(Logger::Level::Warning)
                << "Message Sender Changed State to "
                << static_cast<std::underlying_type<decltype(newState)>::type>(newState)
                << " while management client is closing";
            SetState(ManagementState::Closing);
            if (m_eventHandler)
            {
              m_eventHandler->OnError(Models::_internal::AmqpError{});
            }
            break;
            // Ignore message sender closing or idle state changes.
          case _internal::MessageReceiverState::Idle:
          case _internal::MessageReceiverState::Closing:
            break;
        }
        break;
      case ManagementState::Idle:
      case ManagementState::Error:
        Log::Stream(Logger::Level::Warning)
            << "Message sender state changed to "
            << static_cast<std::underlying_type<decltype(newState)>::type>(newState)
            << " when management client is in the error state, ignoring.";
        break;
    }
  }

  Models::AmqpValue ManagementClientImpl::IndicateError(
      std::string const& condition,
      std::string const& description)
  {
    Models::_internal::AmqpError error;
    error.Condition = Models::_internal::AmqpErrorCondition(condition);
    error.Description = "Message Delivery Rejected: " + description;

    Log::Stream(Logger::Level::Warning)
        << "Indicate Management Error: " << condition << " - " << description;
    if (m_eventHandler)
    {
      // Let external callers know that the error was triggered.
      m_eventHandler->OnError(error);
    }

    // Complete any outstanding receives with an error.
    m_messageQueue.CompleteOperation(
        _internal::ManagementOperationStatus::Error, 500, error, nullptr);

    return Models::_internal::Messaging::DeliveryRejected(condition, description, {});
  }

  Models::AmqpValue ManagementClientImpl::OnMessageReceived(
      _internal::MessageReceiver const&,
      std::shared_ptr<Models::AmqpMessage> const& message)
  {
    if (message->ApplicationProperties.empty())
    {
      return IndicateError(
          Models::_internal::AmqpErrorCondition::InternalError.ToString(),
          "Received message does not have application properties.");
    }
    if (!message->Properties.CorrelationId.HasValue())
    {
      return IndicateError(
          Models::_internal::AmqpErrorCondition::InternalError.ToString(),
          "Received message correlation ID not found.");
    }
    else if (message->Properties.CorrelationId.Value().GetType() != Models::AmqpValueType::Ulong)
    {
      return IndicateError(
          Models::_internal::AmqpErrorCondition::InternalError.ToString(),
          "Received message correlation ID is not a ulong.");
    }
    uint64_t correlationId = message->Properties.CorrelationId.Value();

    auto statusCodeMap = message->ApplicationProperties.find(m_options.ExpectedStatusCodeKeyName);
    if (statusCodeMap == message->ApplicationProperties.end())
    {
      return IndicateError(
          Models::_internal::AmqpErrorCondition::InternalError.ToString(),
          "Received message does not have a " + m_options.ExpectedStatusCodeKeyName
              + " status code key.");
    }
    else if (statusCodeMap->second.GetType() != Models::AmqpValueType::Int)
    {
      return IndicateError(
          Models::_internal::AmqpErrorCondition::InternalError.ToString(),
          "Received message " + m_options.ExpectedStatusCodeKeyName + " value is not an int.");
    }
    int32_t statusCode = statusCodeMap->second;

    // If the message has a status description, remember it.
    auto statusDescription
        = message->ApplicationProperties.find(m_options.ExpectedStatusDescriptionKeyName);
    std::string description;
    if (statusDescription != message->ApplicationProperties.end())
    {
      if (statusDescription->second.GetType() != Models::AmqpValueType::String)
      {
        return IndicateError(
            Models::_internal::AmqpErrorCondition::InternalError.ToString(),
            "Received message " + m_options.ExpectedStatusDescriptionKeyName
                + " value is not a string.");
      }
      description = static_cast<std::string>(statusDescription->second);
    }

    if (correlationId != m_expectedMessageId)
    {
      return IndicateError(
          Models::_internal::AmqpErrorCondition::InternalError.ToString(),
          "Received message correlation ID does not match request ID.");
    }
    if (!m_sendCompleted)
    {
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Informational) << "Received message before send completed.";
      }
    }

    Models::_internal::AmqpError messageError;
    messageError.Description = description;
    messageError.Condition = Models::_internal::AmqpErrorCondition::NotAllowed;

    // AMQP management statusCode values are [RFC
    // 2616](https://www.rfc-editor.org/rfc/rfc2616#section-6.1.1) status codes.
    if ((statusCode < 200) || (statusCode > 299))
    {
      m_messageQueue.CompleteOperation(
          _internal::ManagementOperationStatus::FailedBadStatus, statusCode, messageError, message);
    }
    else
    {
      m_messageQueue.CompleteOperation(
          _internal::ManagementOperationStatus::Ok, statusCode, messageError, message);
    }
    return Models::_internal::Messaging::DeliveryAccepted();
  }

  void ManagementClientImpl::OnMessageReceiverDisconnected(
      Models::_internal::AmqpError const& error)
  {
    Log::Stream(Logger::Level::Warning) << "Message receiver disconnected: " << error << std::endl;
    SetState(ManagementState::Error);
    if (m_eventHandler)
    {
      m_eventHandler->OnError(error);
    }
  }

}}}} // namespace Azure::Core::Amqp::_detail
