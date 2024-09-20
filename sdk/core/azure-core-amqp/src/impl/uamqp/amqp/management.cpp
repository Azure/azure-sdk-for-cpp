// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/management.hpp"

#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"
#include "private/connection_impl.hpp"
#include "private/management_impl.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <tuple>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

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
    std::unique_lock<std::mutex> lock(m_openCloseLock);
    if (m_isOpen)
    {
      throw std::runtime_error("Management object is already open.");
    }

    try
    {
      /** Authentication needs to happen *before* the links are created.
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
        messageReceiverOptions.SettleMode = _internal::ReceiverSettleMode::First;

        m_messageReceiver = std::make_shared<MessageReceiverImpl>(
            m_session, m_options.ManagementNodeName, messageReceiverOptions, this);
      }

      // Now open the message sender and receiver.
      SetState(ManagementState::Opening);
      try
      {
        auto senderResult{m_messageSender->Open(false, context)};
        if (senderResult)
        {
          Log::Stream(Logger::Level::Error)
              << "ManagementClientImpl::Open: Message sender open failed: " << senderResult;
          return _internal::ManagementOpenStatus::Error;
        }
        m_messageSenderOpen = true;
        m_messageReceiver->Open(context);
        m_messageReceiverOpen = true;
      }
      catch (Azure::Core::OperationCancelledException const& e)
      {
        Log::Stream(Logger::Level::Warning)
            << "Operation cancelled opening message sender and receiver." << e.what();
        return _internal::ManagementOpenStatus::Cancelled;
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
          m_messageSender->Close(context);
          m_messageSenderOpen = false;
          m_messageReceiver->Close(context);
          m_messageReceiverOpen = false;
        }
        else
        {
          m_isOpen = true;
        }
        return rv;
      }

      // If result is null, then it means that the context was cancelled. Close the things we opened
      // earlier (if any) and return the error.
      m_messageSender->Close({});
      m_messageSenderOpen = false;
      m_messageReceiver->Close({});
      m_messageReceiverOpen = false;
      return _internal::ManagementOpenStatus::Cancelled;
    }
    catch (...)
    {
      Log::Stream(Logger::Level::Warning) << "Exception thrown during management open.";
      // If an exception is thrown, ensure that the message sender and receiver are closed.
      if (m_messageSenderOpen)
      {
        m_messageSender->Close({});
        m_messageSenderOpen = false;
      }
      if (m_messageReceiverOpen)
      {
        m_messageReceiver->Close({});
        m_messageReceiverOpen = false;
      }
      throw;
    }
  }

  _internal::ManagementOperationResult ManagementClientImpl::ExecuteOperation(
      std::string const& operationToPerform,
      std::string const& typeOfOperation,
      std::string const& locales,
      Models::AmqpMessage messageToSend,
      Context const& context)
  {
    try
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

      // Set the message ID and remember it for later.
      auto requestId = Azure::Core::Uuid::CreateUuid().ToString();

      messageToSend.Properties.MessageId
          = static_cast<Azure::Core::Amqp::Models::AmqpValue>(requestId);
      {
        std::unique_lock<std::recursive_mutex> lock(m_messageQueuesLock);

        Log::Stream(Logger::Level::Verbose)
            << "ManagementClient::ExecuteOperation: " << requestId << ". Create Queue for request.";
        m_messageQueues.emplace(requestId, std::make_unique<ManagementOperationQueue>());
        m_sendCompleted = false;
      }

      auto sendResult = m_messageSender->Send(messageToSend, context);
      if (std::get<0>(sendResult) != _internal::MessageSendStatus::Ok)
      {
        auto sendStatus = std::get<0>(sendResult);
        const auto& sendError = std::get<1>(sendResult);
        Log::Stream(Logger::Level::Error)
            << "ManagementClient::ExecuteOperation, send failed" << sendStatus;
        _internal::ManagementOperationResult rv;
        switch (sendStatus)
        {
          case _internal::MessageSendStatus::Error:
            rv.Status = _internal::ManagementOperationStatus::Error;
            break;
          case _internal::MessageSendStatus::Cancelled:
            rv.Status = _internal::ManagementOperationStatus::Cancelled;
            break;
          case _internal::MessageSendStatus::Invalid:
            rv.Status = _internal::ManagementOperationStatus::Invalid;
            break;
          case _internal::MessageSendStatus::Timeout:
            rv.Status = _internal::ManagementOperationStatus::Error;
            break;
          case _internal::MessageSendStatus::Ok:
            AZURE_ASSERT_MSG(false, "MessageSendStatus::Ok is not a failure status.");
            break;
        }
        rv.StatusCode = 500;
        rv.Error = sendError;
        rv.Message = nullptr;
        {
          std::unique_lock<std::recursive_mutex> lock(m_messageQueuesLock);
          // Remove the queue from the map, we don't need it anymore.
          m_messageQueues.erase(requestId);
        }
        return rv;
      }

      auto result = m_messageQueues.at(requestId)->WaitForResult(context);
      if (result)
      {
        _internal::ManagementOperationResult rv;
        rv.Status = std::get<0>(*result);
        rv.StatusCode = std::get<1>(*result);
        rv.Error = std::get<2>(*result);
        rv.Message = std::get<3>(*result);

        {
          std::unique_lock<std::recursive_mutex> lock(m_messageQueuesLock);
          // Remove the queue from the map, we don't need it anymore.
          m_messageQueues.erase(requestId);
        }
        return rv;
      }
      else
      {
        throw Azure::Core::OperationCancelledException("Management operation cancelled.");
      }
    }
    catch (...)
    {
      Log::Stream(Logger::Level::Error) << "ManagementClient::ExecuteOperation: Exception thrown. "
                                           "Closing message sender and receiver.";
      if (m_messageSenderOpen)
      {
        m_messageSender->Close({});
        m_messageSenderOpen = false;
      }
      if (m_messageReceiverOpen)
      {
        m_messageReceiver->Close({});
        m_messageReceiverOpen = false;
      }
      throw;
    }
  }

  void ManagementClientImpl::SetState(ManagementState newState) { m_state = newState; }

  void ManagementClientImpl::Close(Context const& context)
  {
    std::unique_lock<std::mutex> lock(m_openCloseLock);
    Log::Stream(Logger::Level::Verbose) << "ManagementClient::Close" << std::endl;
    if (!m_isOpen)
    {
      throw std::runtime_error("Management object is not open.");
    }

    SetState(ManagementState::Closing);
    if (m_messageSender && m_messageSenderOpen)
    {
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose) << "ManagementClient::Close Sender" << std::endl;
      }
      m_messageSender->Close(context);
      m_messageSenderOpen = false;
    }
    if (m_messageReceiver && m_messageReceiverOpen)
    {
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose) << "ManagementClient::Close Receiver" << std::endl;
      }
      m_messageReceiver->Close(context);
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
      if (m_options.EnableTrace)
      {
        Log::Stream(Logger::Level::Verbose)
            << "ManagementClient::OnMessageSenderStateChanged: newState == oldState" << std::endl;
      }
      return;
    }

    if (m_options.EnableTrace)
    {
      Log::Stream(Logger::Level::Informational)
          << "ManagementClient::OnMessageSenderStateChanged: " << oldState << " -> " << newState
          << std::endl;
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

  void ManagementClientImpl::OnMessageSenderDisconnected(
      _internal::MessageSender const&,
      Models::_internal::AmqpError const& error)
  {
    if (error)
    {
      Log::Stream(Logger::Level::Warning)
          << "ManagementClient: Message sender disconnected: " << error << std::endl;
      SetState(ManagementState::Error);
      if (m_eventHandler)
      {
        m_eventHandler->OnError(error);
      }
    }
    else
    {
      Log::Stream(Logger::Level::Informational)
          << "ManagementClient: Message sender disconnected normally." << std::endl;
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
          << "ManagementClient::OnMessageReceiverStateChanged: newState == oldState" << std::endl;
      return;
    }

    if (m_options.EnableTrace)
    {
      Log::Stream(Logger::Level::Informational)
          << "ManagementClient::OnMessageReceiverStateChanged: " << oldState << " -> " << newState
          << std::endl;
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
            Log::Stream(Logger::Level::Warning) << "Message Receiver Changed State to " << newState
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
            Log::Stream(Logger::Level::Warning) << "Message Sender Changed State to " << newState
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
            Log::Stream(Logger::Level::Warning) << "Message Sender Changed State to " << newState
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
            << "Message sender state changed to " << newState
            << " when management client is in the error state, ignoring.";
        break;
    }
  }

  Models::AmqpValue ManagementClientImpl::IndicateError(
      std::string const& correlationId,
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
    if (!correlationId.empty())
    {
      // Ensure nobody else is messing with the message queues right now.
      std::unique_lock<std::recursive_mutex> lock(m_messageQueuesLock);

      // If the correlation ID is found locally, complete the operation with an error.
      if (m_messageQueues.find(correlationId) != m_messageQueues.end())
      {
        // Complete any outstanding receives with an error.
        m_messageQueues.at(correlationId)
            ->CompleteOperation(_internal::ManagementOperationStatus::Error, 500, error, nullptr);
      }
    }
    return Models::_internal::Messaging::DeliveryRejected(condition, description, {});
  }

  Models::AmqpValue ManagementClientImpl::OnMessageReceived(
      _internal::MessageReceiver const&,
      std::shared_ptr<Models::AmqpMessage> const& message)
  {
    if (!message->Properties.CorrelationId.HasValue())
    {
      return IndicateError(
          "",
          Models::_internal::AmqpErrorCondition::InternalError.ToString(),
          "Received message correlation ID not found.");
    }
    else if (message->Properties.CorrelationId.Value().GetType() != Models::AmqpValueType::String)
    {
      return IndicateError(
          "",
          Models::_internal::AmqpErrorCondition::InternalError.ToString(),
          "Received message correlation ID is not a ulong.");
    }
    std::string requestId = static_cast<std::string>(message->Properties.CorrelationId.Value());

    // Ensure nobody else is messing with the message queues right now.
    std::unique_lock<std::recursive_mutex> lock(m_messageQueuesLock);
    auto messageQueue = m_messageQueues.find(requestId);
    if (messageQueue == m_messageQueues.end())
    {
      return IndicateError(
          requestId,
          Models::_internal::AmqpErrorCondition::InternalError.ToString(),
          "Received message correlation ID does not match request ID.");
    }

    if (message->ApplicationProperties.empty())
    {
      return IndicateError(
          requestId,
          Models::_internal::AmqpErrorCondition::InternalError.ToString(),
          "Received message does not have application properties.");
    }

    auto statusCodeMap = message->ApplicationProperties.find(m_options.ExpectedStatusCodeKeyName);
    if (statusCodeMap == message->ApplicationProperties.end())
    {
      return IndicateError(
          requestId,
          Models::_internal::AmqpErrorCondition::InternalError.ToString(),
          "Received message does not have a " + m_options.ExpectedStatusCodeKeyName
              + " status code key.");
    }
    else if (statusCodeMap->second.GetType() != Models::AmqpValueType::Int)
    {
      return IndicateError(
          requestId,
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
            requestId,
            Models::_internal::AmqpErrorCondition::InternalError.ToString(),
            "Received message " + m_options.ExpectedStatusDescriptionKeyName
                + " value is not a string.");
      }
      description = static_cast<std::string>(statusDescription->second);
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
      m_messageQueues.at(requestId)->CompleteOperation(
          _internal::ManagementOperationStatus::FailedBadStatus, statusCode, messageError, message);
    }
    else
    {
      m_messageQueues.at(requestId)->CompleteOperation(
          _internal::ManagementOperationStatus::Ok, statusCode, messageError, message);
    }
    return Models::_internal::Messaging::DeliveryAccepted();
  }

  void ManagementClientImpl::OnMessageReceiverDisconnected(
      _internal::MessageReceiver const&,
      Models::_internal::AmqpError const& error)
  {
    if (error)
    {
      Log::Stream(Logger::Level::Warning) << "Message receiver disconnected: " << error;
      SetState(ManagementState::Error);
      if (m_eventHandler)
      {
        m_eventHandler->OnError(error);
      }
    }
    else
    {
      Log::Stream(Logger::Level::Informational) << "Message receiver disconnected normally.";
    }
  }

}}}} // namespace Azure::Core::Amqp::_detail
