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
            m_session, m_options.ManagementNodeName, messageSenderOptions);
      }
      {
        _internal::MessageReceiverOptions messageReceiverOptions;
        messageReceiverOptions.EnableTrace = m_options.EnableTrace;
        messageReceiverOptions.MessageTarget = m_options.ManagementNodeName;
        messageReceiverOptions.Name = m_options.ManagementNodeName + "-receiver";
        messageReceiverOptions.AuthenticationRequired = false;
        messageReceiverOptions.SettleMode = _internal::ReceiverSettleMode::First;

        m_messageReceiver = std::make_shared<MessageReceiverImpl>(
            m_session, m_options.ManagementNodeName, messageReceiverOptions);
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
    throw std::runtime_error("Not implemented");
    (void)operationToPerform;
    (void)typeOfOperation;
    (void)locales;
    (void)messageToSend;
    (void)context;
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

}}}} // namespace Azure::Core::Amqp::_detail
