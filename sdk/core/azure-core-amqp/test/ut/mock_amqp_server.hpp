// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include <azure/core/amqp/claims_based_security.hpp>
#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/message_receiver.hpp>
#include <azure/core/amqp/message_sender.hpp>
#include <azure/core/amqp/models/amqp_message.hpp>
#include <azure/core/amqp/models/amqp_protocol.hpp>
#include <azure/core/amqp/models/message_source.hpp>
#include <azure/core/amqp/models/message_target.hpp>
#include <azure/core/amqp/models/messaging_values.hpp>
#include <azure/core/amqp/network/amqp_header_detect_transport.hpp>
#include <azure/core/amqp/network/socket_listener.hpp>
#include <azure/core/amqp/session.hpp>
#include <gtest/gtest.h>

extern uint16_t FindAvailableSocket();
namespace MessageTests {

class AmqpServerMock : public Azure::Core::Amqp::Network::_internal::SocketListenerEvents,
                       public Azure::Core::Amqp::_internal::ConnectionEvents,
                       public Azure::Core::Amqp::_internal::SessionEvents,
                       public Azure::Core::Amqp::_internal::MessageReceiverEvents,
                       public Azure::Core::Amqp::_internal::MessageSenderEvents {
public:
  struct MessageLinkComponents
  {
    std::unique_ptr<Azure::Core::Amqp::_internal::MessageSender> LinkSender;
    std::unique_ptr<Azure::Core::Amqp::_internal::MessageReceiver> LinkReceiver;
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
        std::unique_ptr<Azure::Core::Amqp::Models::AmqpMessage>>
        MessageQueue;
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> MessageReceiverPresentQueue;
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> MessageSenderPresentQueue;
  };

  AmqpServerMock() { m_testPort = FindAvailableSocket(); }
  virtual void Poll() const {}

  bool WaitForConnection(
      Azure::Core::Amqp::Network::_internal::SocketListener const& listener,
      Azure::Core::Context context = {})
  {
    auto result = m_connectionQueue.WaitForPolledResult(context, listener);
    return result != nullptr;
  }
  bool WaitForMessageReceiver(std::string const& nodeName, Azure::Core::Context context = {})
  {
    auto result = m_linkMessageQueues[nodeName].MessageReceiverPresentQueue.WaitForPolledResult(
        context, *this, *m_connection);
    return result != nullptr;
  }
  bool WaitForMessageSender(std::string const& nodeName, Azure::Core::Context context = {})
  {
    auto result = m_linkMessageQueues[nodeName].MessageSenderPresentQueue.WaitForPolledResult(
        context, *this, *m_connection);
    return result != nullptr;
  }
  std::unique_ptr<Azure::Core::Amqp::Models::AmqpMessage> WaitForMessage(
      std::string const& nodeName)
  {
    // Poll for completion on both the mock server and the connection, that ensures that
    // we can implement unsolicited sends from the Poll function.
    auto result = m_linkMessageQueues[nodeName].MessageQueue.WaitForPolledResult(
        m_listenerContext, *m_connection, *this);
    if (result)
    {
      return std::move(std::get<0>(*result));
    }
    else
    {
      return nullptr;
    }
  }

  /** @brief Override for non CBS message receive operations which allows a specialization to
   * customize the behavior for received messages.
   */
  virtual void MessageReceived(
      std::string const&,
      MessageLinkComponents const&,
      Azure::Core::Amqp::Models::AmqpMessage const&) const {};

  virtual void MessageLoop(std::string const& nodeName, MessageLinkComponents const& linkComponents)
  {
    GTEST_LOG_(INFO) << "Wait for incoming message.";
    auto message = WaitForMessage(nodeName);
    if (!message)
    {
      GTEST_LOG_(INFO) << "No message, canceling thread";
    }
    else
    {
      GTEST_LOG_(INFO) << "Received message: " << *message;
      if (nodeName == "$cbs" && IsCbsMessage(*message))
      {
        ProcessCbsMessage(linkComponents, *message);
      }
      else
      {
        MessageReceived(nodeName, linkComponents, *message);
      }
    }
  };

  uint16_t GetPort() const { return m_testPort; }
  Azure::Core::Context& GetListenerContext() { return m_listenerContext; }

  void StartListening()
  {
    // Start the mock AMQP server which will be used to receive the connect open.
    // Ensure that the thread is started before we start using the message sender.
    std::mutex threadRunningMutex;
    std::condition_variable threadStarted;
    bool running = false;

    //    Azure::Core::Context listenerContext;
    m_serverThread = std::thread([this, &threadStarted, &running]() {
      Azure::Core::Amqp::Network::_internal::SocketListener listener(GetPort(), this);
      GTEST_LOG_(INFO) << "Start test listener on port " << GetPort();
      listener.Start();
      GTEST_LOG_(INFO) << "listener started";
      running = true;
      threadStarted.notify_one();

      GTEST_LOG_(INFO) << "Wait for connection on listener.";
      if (!WaitForConnection(listener, m_listenerContext))
      {
        GTEST_LOG_(INFO) << "Cancelling thread.";
        return;
      }
      while (!m_listenerContext.IsCancelled())
      {
        std::this_thread::yield();
        m_connection->Poll();
        for (const auto& val : m_linkMessageQueues)
        {
          if (!val.second.LinkReceiver)
          {
            GTEST_LOG_(INFO) << "Wait for message receiver for " << val.first;

            if (!WaitForMessageReceiver(val.first, m_listenerContext))
            {
              GTEST_LOG_(INFO) << "Cancelling thread.";
              return;
            }
          }
          if (!val.second.LinkSender)
          {
            GTEST_LOG_(INFO) << "Wait for message sender for " << val.first;
            if (!WaitForMessageSender(val.first, m_listenerContext))
            {
              GTEST_LOG_(INFO) << "Cancelling thread.";
              return;
            }
          }
          MessageLoop(val.first, val.second);
        }
      }
      listener.Stop();
    });

    // Wait until our running thread is actually listening before we return.
    GTEST_LOG_(INFO) << "Wait 10 seconds for listener to start.";
    std::unique_lock<std::mutex> waitForThreadStart(threadRunningMutex);
    threadStarted.wait_until(
        waitForThreadStart,
        std::chrono::system_clock::now() + std::chrono::seconds(10),
        [&running]() { return running == true; });
    GTEST_LOG_(INFO) << "Listener running.";
  }

  void StopListening()
  {
    GTEST_LOG_(INFO) << "Stop listening";
    // Cancel the listener context, which will cause any WaitForXxx calls to exit.

    m_listenerContext.Cancel();
    m_serverThread.join();
    for (auto& val : m_linkMessageQueues)
    {
      if (val.second.LinkSender)
      {
        val.second.LinkSender.reset();
      }
      if (val.second.LinkReceiver)
      {
        val.second.LinkReceiver.reset();
      }
    }
    if (m_session)
    {
      m_session.reset();
    }
    if (m_connection)
    {
      m_connection.reset();
    }
  }

  void ForceCbsError(bool forceError) { m_forceCbsError = forceError; }

private:
  std::shared_ptr<Azure::Core::Amqp::_internal::Connection> m_connection;
  std::shared_ptr<Azure::Core::Amqp::_internal::Session> m_session;

  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> m_connectionQueue;

  std::thread m_serverThread;
  std::uint16_t m_testPort;
  bool m_forceCbsError{false};

protected:
  // For each incoming message source, we create a queue of messages intended for that message
  // source.
  //
  // Each message queue is keyed by the message-id.
  std::map<std::string, MessageLinkComponents> m_linkMessageQueues;
  Azure::Core::Context m_listenerContext; // Used to cancel the listener if necessary.

  bool IsCbsMessage(Azure::Core::Amqp::Models::AmqpMessage const& message)
  {
    if (!message.ApplicationProperties.empty())
    {
      Azure::Core::Amqp::Models::AmqpValue operation
          = message.ApplicationProperties.at("operation");
      Azure::Core::Amqp::Models::AmqpValue type = message.ApplicationProperties.at("type");

      // If we're processing a put-token message, then we should get a "type" and "name"
      // value.
      EXPECT_EQ(operation.GetType(), Azure::Core::Amqp::Models::AmqpValueType::String);
      if (static_cast<std::string>(operation) == "put-token")
      {
        return true;
      }
      else if (static_cast<std::string>(operation) == "delete-token")
      {
        return true;
      }
    }
    return false;
  }

  void ProcessCbsMessage(
      MessageLinkComponents const& linkComponents,
      Azure::Core::Amqp::Models::AmqpMessage const& message)
  {
    Azure::Core::Amqp::Models::AmqpValue operation = message.ApplicationProperties.at("operation");
    Azure::Core::Amqp::Models::AmqpValue type = message.ApplicationProperties.at("type");
    Azure::Core::Amqp::Models::AmqpValue name = message.ApplicationProperties.at("name");
    // If we're processing a put-token message, then we should get a "type" and "name"
    // value.
    EXPECT_EQ(operation.GetType(), Azure::Core::Amqp::Models::AmqpValueType::String);
    if (static_cast<std::string>(operation) == "put-token")
    {
      EXPECT_EQ(type.GetType(), Azure::Core::Amqp::Models::AmqpValueType::String);
      EXPECT_EQ(name.GetType(), Azure::Core::Amqp::Models::AmqpValueType::String);
      // The body of a put-token operation MUST be an AMQP AmqpValue.
      EXPECT_EQ(message.BodyType, Azure::Core::Amqp::Models::MessageBodyType::Value);

      // Respond to the operation.
      Azure::Core::Amqp::Models::AmqpMessage response;
      Azure::Core::Amqp::Models::MessageProperties responseProperties;

      // Management specification section 3.2: The correlation-id of the response message
      // MUST be the correlation-id from the request message (if present), else the
      // message-id from the request message.
      auto requestCorrelationId = message.Properties.CorrelationId;
      if (!message.Properties.CorrelationId.HasValue())
      {
        requestCorrelationId = message.Properties.MessageId.Value();
      }
      response.Properties.CorrelationId = requestCorrelationId;

      // Populate the response application properties.

      if (m_forceCbsError)
      {
        response.ApplicationProperties["status-code"] = 500;
        response.ApplicationProperties["status-description"] = "Internal Server Error";
      }
      else
      {
        response.ApplicationProperties["status-code"] = 200;
        response.ApplicationProperties["status-description"] = "OK-put";
      }

      response.SetBody(Azure::Core::Amqp::Models::AmqpValue());

      // Set the response body type to an empty AMQP value.
      if (m_listenerContext.IsCancelled())
      {
        return;
      }
      try
      {
        linkComponents.LinkSender->QueueSend(response, nullptr, m_listenerContext);
      }
      catch (std::exception& ex)
      {
        GTEST_LOG_(INFO) << "Exception thrown sending CBS response: " << ex.what();
        return;
      }
    }
    else if (static_cast<std::string>(operation) == "delete-token")
    {
      Azure::Core::Amqp::Models::AmqpMessage response;
      Azure::Core::Amqp::Models::MessageProperties responseProperties;

      // Management specification section 3.2: The correlation-id of the response message
      // MUST be the correlation-id from the request message (if present), else the
      // message-id from the request message.
      auto requestCorrelationId = message.Properties.CorrelationId;
      if (!message.Properties.CorrelationId.HasValue())
      {
        requestCorrelationId = message.Properties.MessageId;
      }
      response.Properties.CorrelationId = requestCorrelationId;
      response.ApplicationProperties["status-code"] = 200;
      response.ApplicationProperties["status-description"] = "OK-delete";

      response.SetBody(Azure::Core::Amqp::Models::AmqpValue());

      // Set the response body type to an empty AMQP value.
      if (m_listenerContext.IsCancelled())
      {
        return;
      }

      linkComponents.LinkSender->QueueSend(response, nullptr, m_listenerContext);
    }
  }

  virtual void OnSocketAccepted(
      std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> transport) override
  {
    GTEST_LOG_(INFO) << "OnSocketAccepted - Socket connection received.";
    std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> amqpTransport{
        std::make_shared<Azure::Core::Amqp::Network::_internal::AmqpHeaderDetectTransport>(
            transport, nullptr)};
    Azure::Core::Amqp::_internal::ConnectionOptions options;
    options.ContainerId = "connectionId";
    options.EnableTrace = true;
    options.Transport = amqpTransport;
    m_connection
        = std::make_shared<Azure::Core::Amqp::_internal::Connection>(amqpTransport, options, this);
    m_connection->Listen();
    m_connectionQueue.CompleteOperation(true);
  }

  virtual void OnConnectionStateChanged(
      Azure::Core::Amqp::_internal::Connection const&,
      Azure::Core::Amqp::_internal::ConnectionState newState,
      Azure::Core::Amqp::_internal::ConnectionState oldState) override
  {
    GTEST_LOG_(INFO) << "Connection State changed. Old state: " << ConnectionStateToString(oldState)
                     << " New state: " << ConnectionStateToString(newState);
  }
  virtual bool OnNewEndpoint(
      Azure::Core::Amqp::_internal::Connection const& connection,
      Azure::Core::Amqp::_internal::Endpoint& endpoint) override
  {
    GTEST_LOG_(INFO) << "OnNewEndpoint - Incoming endpoint created, create session.";
    m_session = std::make_unique<Azure::Core::Amqp::_internal::Session>(connection, endpoint, this);
    m_session->SetIncomingWindow(10000);
    m_session->Begin();
    return true;
  }
  virtual void OnIoError(Azure::Core::Amqp::_internal::Connection const&) override {}

  // Inherited via Session
  virtual bool OnLinkAttached(
      Azure::Core::Amqp::_internal::Session const& session,
      Azure::Core::Amqp::_internal::LinkEndpoint& newLinkInstance,
      std::string const& name,
      Azure::Core::Amqp::_internal::SessionRole role,
      Azure::Core::Amqp::Models::AmqpValue source,
      Azure::Core::Amqp::Models::AmqpValue target,
      Azure::Core::Amqp::Models::AmqpValue) override
  {
    Azure::Core::Amqp::Models::_internal::MessageSource msgSource(source);
    Azure::Core::Amqp::Models::_internal::MessageTarget msgTarget(target);

    GTEST_LOG_(INFO) << "OnLinkAttached. Source: " << msgSource << " Target: " << msgTarget;

    // If the incoming role is receiver, then we want to create a sender to talk to it.
    // Similarly, if the incoming role is sender, we want to create a receiver to receive from it.
    if (role == Azure::Core::Amqp::_internal::SessionRole::Receiver)
    {
      Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;
      senderOptions.EnableTrace = true;
      senderOptions.Name = name;
      senderOptions.SourceAddress = static_cast<std::string>(msgSource.GetAddress());
      senderOptions.InitialDeliveryCount = 0;
      std::string targetAddress = static_cast<std::string>(msgTarget.GetAddress());
      MessageLinkComponents& linkComponents = m_linkMessageQueues[senderOptions.SourceAddress];

      if (!linkComponents.LinkSender)
      {
        linkComponents.LinkSender = std::make_unique<Azure::Core::Amqp::_internal::MessageSender>(
            session, newLinkInstance, targetAddress, senderOptions, this);
        linkComponents.LinkSender->Open();
        linkComponents.MessageSenderPresentQueue.CompleteOperation(true);
      }
    }
    else if (role == Azure::Core::Amqp::_internal::SessionRole::Sender)
    {
      Azure::Core::Amqp::_internal::MessageReceiverOptions receiverOptions;
      receiverOptions.EnableTrace = true;
      receiverOptions.Name = name;
      receiverOptions.TargetAddress = static_cast<std::string>(msgTarget.GetAddress());
      receiverOptions.InitialDeliveryCount = 0;
      std::string sourceAddress = static_cast<std::string>(msgSource.GetAddress());
      MessageLinkComponents& linkComponents = m_linkMessageQueues[receiverOptions.TargetAddress];
      if (!linkComponents.LinkReceiver)
      {
        linkComponents.LinkReceiver
            = std::make_unique<Azure::Core::Amqp::_internal::MessageReceiver>(
                session, newLinkInstance, sourceAddress, receiverOptions, this);
        linkComponents.LinkReceiver->Open();
        linkComponents.MessageReceiverPresentQueue.CompleteOperation(true);
      }
    }
    return true;
  }

  virtual void OnEndpointFrameReceived(
      Azure::Core::Amqp::_internal::Connection const&,
      Azure::Core::Amqp::Models::AmqpValue const&,
      uint32_t,
      uint8_t*) override
  {
  }

  // Inherited via MessageReceiverEvents
  void OnMessageReceiverStateChanged(
      Azure::Core::Amqp::_internal::MessageReceiver const&,
      Azure::Core::Amqp::_internal::MessageReceiverState newState,
      Azure::Core::Amqp::_internal::MessageReceiverState oldState) override
  {
    GTEST_LOG_(INFO) << "Message Receiver State changed. Old state: "
                     << ReceiverStateToString(oldState)
                     << " New state: " << ReceiverStateToString(newState);
  }
  Azure::Core::Amqp::Models::AmqpValue OnMessageReceived(
      Azure::Core::Amqp::_internal::MessageReceiver const& receiver,
      Azure::Core::Amqp::Models::AmqpMessage const& message) override
  {
    GTEST_LOG_(INFO) << "Received a message " << message;
    m_linkMessageQueues[receiver.GetSourceName()].MessageQueue.CompleteOperation(
        std::make_unique<Azure::Core::Amqp::Models::AmqpMessage>(message));
    return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryAccepted();
  }

  // Inherited via MessageSenderEvents
  void OnMessageSenderStateChanged(
      Azure::Core::Amqp::_internal::MessageSender const&,
      Azure::Core::Amqp::_internal::MessageSenderState newState,
      Azure::Core::Amqp::_internal::MessageSenderState oldState) override
  {
    GTEST_LOG_(INFO) << "Message Sender State changed. Old state: " << SenderStateToString(oldState)
                     << " New state: " << SenderStateToString(newState);
  }

  const char* ConnectionStateToString(Azure::Core::Amqp::_internal::ConnectionState state)
  {
    switch (state)
    {
      case Azure::Core::Amqp::_internal::ConnectionState::Start:
        return "Start";
      case Azure::Core::Amqp::_internal::ConnectionState::HeaderReceived:
        return "HeaderReceived";
      case Azure::Core::Amqp::_internal::ConnectionState::HeaderSent:
        return "HeaderSent";
      case Azure::Core::Amqp::_internal::ConnectionState::HeaderExchanged:
        return "HeaderExchanged";
      case Azure::Core::Amqp::_internal::ConnectionState::OpenPipe:
        return "OpenPipe";
      case Azure::Core::Amqp::_internal::ConnectionState::OcPipe:
        return "OcPipe";
      case Azure::Core::Amqp::_internal::ConnectionState::OpenReceived:
        return "OpenReceived";
      case Azure::Core::Amqp::_internal::ConnectionState::OpenSent:
        return "OpenSent";
      case Azure::Core::Amqp::_internal::ConnectionState::ClosePipe:
        return "ClosePipe";
      case Azure::Core::Amqp::_internal::ConnectionState::Opened:
        return "Opened";
      case Azure::Core::Amqp::_internal::ConnectionState::CloseReceived:
        return "CloseReceived";
      case Azure::Core::Amqp::_internal::ConnectionState::CloseSent:
        return "CloseSent";
      case Azure::Core::Amqp::_internal::ConnectionState::Discarding:
        return "Discarding";
      case Azure::Core::Amqp::_internal::ConnectionState::End:
        return "End";
      case Azure::Core::Amqp::_internal::ConnectionState::Error:
        return "Error";
    }
    throw std::runtime_error("Unknown connection state");
  }

  const char* ReceiverStateToString(Azure::Core::Amqp::_internal::MessageReceiverState state)
  {
    switch (state)
    {
      case Azure::Core::Amqp::_internal::MessageReceiverState::Invalid:
        return "Invalid";
      case Azure::Core::Amqp::_internal::MessageReceiverState::Idle:
        return "Idle";
      case Azure::Core::Amqp::_internal::MessageReceiverState::Opening:
        return "Opening";
      case Azure::Core::Amqp::_internal::MessageReceiverState::Open:
        return "Open";
      case Azure::Core::Amqp::_internal::MessageReceiverState::Closing:
        return "Closing";
      case Azure::Core::Amqp::_internal::MessageReceiverState::Error:
        return "Error";
    }
    throw std::runtime_error("Unknown receiver state");
  }

  const char* SenderStateToString(Azure::Core::Amqp::_internal::MessageSenderState state)
  {
    // Return the stringized version of the values in the MessageSenderState enumeration
    switch (state)
    {
      case Azure::Core::Amqp::_internal::MessageSenderState::Invalid:
        return "Invalid";
      case Azure::Core::Amqp::_internal::MessageSenderState::Idle:
        return "Idle";
      case Azure::Core::Amqp::_internal::MessageSenderState::Opening:
        return "Opening";
      case Azure::Core::Amqp::_internal::MessageSenderState::Open:
        return "Open";
      case Azure::Core::Amqp::_internal::MessageSenderState::Closing:
        return "Closing";
      case Azure::Core::Amqp::_internal::MessageSenderState::Error:
        return "Error";
    }
    throw std::runtime_error("Unknown sender state");
  }
};
} // namespace MessageTests
