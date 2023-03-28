// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#pragma once

#include <azure/core/amqp/claims_based_security.hpp>
#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/message_receiver.hpp>
#include <azure/core/amqp/message_sender.hpp>
#include <azure/core/amqp/models/amqp_message.hpp>
#include <azure/core/amqp/session.hpp>

extern uint16_t FindAvailableSocket();
namespace MessageTests {

// AMQP ApplicationProperties descriptor (AMQP Specification 1.0 section 3.2.5)
// http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-application-properties
constexpr uint64_t AmqpApplicationPropertiesDescriptor = 116;

class AmqpServerMock : public Azure::Core::_internal::Amqp::Network::SocketListenerEvents,
                       public Azure::Core::_internal::Amqp::ConnectionEvents,
                       public Azure::Core::_internal::Amqp::SessionEvents,
                       public Azure::Core::_internal::Amqp::MessageReceiverEvents,
                       public Azure::Core::_internal::Amqp::MessageSenderEvents {
public:
  AmqpServerMock() { m_testPort = FindAvailableSocket(); }
  virtual void Poll() const {}

  bool WaitForConnection(
      Azure::Core::_internal::Amqp::Network::SocketListener const& listener,
      Azure::Core::Context context = {})
  {
    auto result = m_connectionQueue.WaitForPolledResult(context, listener);
    return result != nullptr;
  }
  bool WaitForMessageReceiver(Azure::Core::Context context = {})
  {
    auto result = m_messageReceiverQueue.WaitForPolledResult(context, *m_connection);
    return result != nullptr;
  }
  bool WaitForMessageSender(Azure::Core::Context context = {})
  {
    auto result = m_messageSenderQueue.WaitForPolledResult(context, *m_connection);
    return result != nullptr;
  }
  std::unique_ptr<Azure::Core::Amqp::Models::Message> WaitForMessage()
  {
    auto result = m_messageQueue.WaitForPolledResult(m_listenerContext, *m_connection, *this);
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
   * customize the link behavior.
   *  @param message The message received.
   *  @return The value to send back to the sender.
   */
  virtual Azure::Core::Amqp::Models::Value MessageReceived(Azure::Core::Amqp::Models::Message)
  {
    return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryAccepted();
  };
  virtual void MessageLoop()
  {
    GTEST_LOG_(INFO) << "Wait for incoming message.";
    auto message = WaitForMessage();
    if (!message)
    {
      GTEST_LOG_(INFO) << "No message, canceling thread";
    }
    else
    {
      GTEST_LOG_(INFO) << "Received message: " << *message;
      if (IsCbsMessage(*message))
      {
        ProcessCbsMessage(*message);
      }
      else
      {
        OnMessageReceived(*message);
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
      Azure::Core::_internal::Amqp::Network::SocketListener listener(GetPort(), this);
      GTEST_LOG_(INFO) << "Start test listener on port " << GetPort();
      listener.Start();
      running = true;
      threadStarted.notify_one();

      if (!WaitForConnection(listener, m_listenerContext))
      {
        GTEST_LOG_(INFO) << "Cancelling thread.";
        return;
      }
      GTEST_LOG_(INFO) << "Wait for message receiver.";
      if (!WaitForMessageReceiver(m_listenerContext))
      {
        GTEST_LOG_(INFO) << "Cancelling thread.";
        return;
      }
      if (!WaitForMessageSender(m_listenerContext))
      {
        GTEST_LOG_(INFO) << "Cancelling thread.";
        return;
      }
      while (!m_listenerContext.IsCancelled())
      {
        MessageLoop();
      }
      listener.Stop();
    });

    // Wait until our running thread is actually listening before we return.
    GTEST_LOG_(INFO) << "Wait for listener to start.";
    std::unique_lock<std::mutex> waitForThreadStart(threadRunningMutex);
    threadStarted.wait(waitForThreadStart, [&running]() { return running == true; });
    GTEST_LOG_(INFO) << "Listener running.";
  }

  void StopListening()
  { // Cancel the listener context, which will cause any WaitForXxx calls to exit.

    m_listenerContext.Cancel();
    m_serverThread.join();

    if (m_messageSender)
    {
      m_messageSender.reset();
    }
    if (m_messageReceiver)
    {
      m_messageReceiver.reset();
    }
    if (m_cbsMessageSender)
    {
      m_cbsMessageSender.reset();
    }
    if (m_cbsMessageReceiver)
    {
      m_cbsMessageReceiver.reset();
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

private:
  std::shared_ptr<Azure::Core::_internal::Amqp::Connection> m_connection;
  std::shared_ptr<Azure::Core::_internal::Amqp::Session> m_session;

  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> m_messageReceiverQueue;
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> m_messageSenderQueue;
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> m_connectionQueue;
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
      std::unique_ptr<Azure::Core::Amqp::Models::Message>>
      m_messageQueue;

  std::unique_ptr<Azure::Core::_internal::Amqp::MessageSender> m_cbsMessageSender;
  std::unique_ptr<Azure::Core::_internal::Amqp::MessageReceiver> m_cbsMessageReceiver;

  std::unique_ptr<Azure::Core::_internal::Amqp::MessageReceiver> m_messageReceiver;

  std::thread m_serverThread;
  uint16_t m_testPort;

protected:
  std::unique_ptr<Azure::Core::_internal::Amqp::MessageSender> m_messageSender;
  Azure::Core::Context m_listenerContext; // Used to cancel the listener if necessary.

  bool IsCbsMessage(Azure::Core::Amqp::Models::Message const& message)
  {
    auto applicationProperties = message.GetApplicationProperties();
    if (applicationProperties.GetType() == Azure::Core::Amqp::Models::AmqpValueType::Described)
    {
      auto descriptor = applicationProperties.GetDescriptor();
      auto value = applicationProperties.GetDescribedValue();
      if (static_cast<uint64_t>(descriptor) != 116)
      {
        return false;
      }

      auto operation = value.GetMapValue("operation");
      auto type = value.GetMapValue("type");
      auto name = value.GetMapValue("name");
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

  void ProcessCbsMessage(Azure::Core::Amqp::Models::Message const& message)
  {
    auto applicationProperties = message.GetApplicationProperties();
    auto value = applicationProperties.GetDescribedValue();

    auto operation = value.GetMapValue("operation");
    auto type = value.GetMapValue("type");
    auto name = value.GetMapValue("name");
    // If we're processing a put-token message, then we should get a "type" and "name"
    // value.
    EXPECT_EQ(operation.GetType(), Azure::Core::Amqp::Models::AmqpValueType::String);
    if (static_cast<std::string>(operation) == "put-token")
    {
      EXPECT_EQ(type.GetType(), Azure::Core::Amqp::Models::AmqpValueType::String);
      EXPECT_EQ(name.GetType(), Azure::Core::Amqp::Models::AmqpValueType::String);
      // The body of a put-token operation MUST be an AMQP Value.
      EXPECT_EQ(message.GetBodyType(), Azure::Core::Amqp::Models::MessageBodyType::Value);

      // Respond to the operation.
      Azure::Core::Amqp::Models::Message response;
      Azure::Core::Amqp::Models::Properties responseProperties;

      // Management specification section 3.2: The correlation-id of the response message
      // MUST be the correlation-id from the request message (if present), else the
      // message-id from the request message.
      auto requestCorrelationId = message.GetProperties().GetCorrelationId();
      if (requestCorrelationId.IsNull())
      {
        requestCorrelationId = message.GetProperties().GetMessageId();
      }
      responseProperties.SetCorrelationId(requestCorrelationId);
      response.SetProperties(responseProperties);

      // Populate the response application properties.

      auto propertyMap = Azure::Core::Amqp::Models::Value::CreateMap();
      propertyMap.SetMapValue("status-code", 200);
      propertyMap.SetMapValue("status-description", "OK-put");

      // Create a descriptor to hold the property map and set it as the response's
      // application properties.
      response.SetApplicationProperties(propertyMap);

      // Set the response body type to an empty AMQP value.
      if (m_listenerContext.IsCancelled())
      {
        return;
      }
      response.SetBodyAmqpValue(Azure::Core::Amqp::Models::Value());
      m_cbsMessageSender->Send(response, m_listenerContext);
    }
    else if (static_cast<std::string>(operation) == "delete-token")
    {
      Azure::Core::Amqp::Models::Message response;
      Azure::Core::Amqp::Models::Properties responseProperties;

      // Management specification section 3.2: The correlation-id of the response message
      // MUST be the correlation-id from the request message (if present), else the
      // message-id from the request message.
      auto requestCorrelationId = message.GetProperties().GetCorrelationId();
      if (requestCorrelationId.IsNull())
      {
        requestCorrelationId = message.GetProperties().GetMessageId();
      }
      responseProperties.SetCorrelationId(requestCorrelationId);
      response.SetProperties(responseProperties);

      auto propertyMap = Azure::Core::Amqp::Models::Value::CreateMap();
      propertyMap.SetMapValue("status-code", 200);
      propertyMap.SetMapValue("status-description", "OK-delete");

      // Create a descriptor to hold the property map and set it as the response's
      // application properties.
      response.SetApplicationProperties(propertyMap);
      // Set the response body type to an empty AMQP value.
      if (m_listenerContext.IsCancelled())
      {
        return;
      }

      response.SetBodyAmqpValue(Azure::Core::Amqp::Models::Value());

      m_cbsMessageSender->Send(response, m_listenerContext);
    }
  }

  virtual void OnSocketAccepted(XIO_INSTANCE_TAG* xio) override
  {
    GTEST_LOG_(INFO) << "OnSocketAccepted - Socket connection received.";
    std::shared_ptr<Azure::Core::_internal::Amqp::Network::Transport> amqpTransport{
        std::make_shared<Azure::Core::_internal::Amqp::Network::AmqpHeaderTransport>(xio, nullptr)};
    Azure::Core::_internal::Amqp::ConnectionOptions options;
    options.ContainerId = "connectionId";
    options.EnableTrace = true;
    options.Transport = amqpTransport;
    m_connection
        = std::make_shared<Azure::Core::_internal::Amqp::Connection>(amqpTransport, this, options);
    m_connection->Listen();
    m_connectionQueue.CompleteOperation(true);
  }

  virtual void OnConnectionStateChanged(
      Azure::Core::_internal::Amqp::Connection const&,
      Azure::Core::_internal::Amqp::ConnectionState newState,
      Azure::Core::_internal::Amqp::ConnectionState oldState) override
  {
    GTEST_LOG_(INFO) << "Connection State changed. Old state: " << ConnectionStateToString(oldState)
                     << " New state: " << ConnectionStateToString(newState);
  }
  virtual bool OnNewEndpoint(
      Azure::Core::_internal::Amqp::Connection const& connection,
      Azure::Core::_internal::Amqp::Endpoint& endpoint) override
  {
    GTEST_LOG_(INFO) << "OnNewEndpoint - Incoming endpoint created, create session.";
    m_session = std::make_unique<Azure::Core::_internal::Amqp::Session>(connection, endpoint, this);
    m_session->SetIncomingWindow(10000);
    m_session->Begin();
    return true;
  }
  virtual void OnIoError(Azure::Core::_internal::Amqp::Connection const&) override {}

  // Inherited via Session
  virtual bool OnLinkAttached(
      Azure::Core::_internal::Amqp::Session const& session,
      Azure::Core::_internal::Amqp::LinkEndpoint& newLinkInstance,
      std::string const& name,
      Azure::Core::_internal::Amqp::SessionRole role,
      Azure::Core::Amqp::Models::Value source,
      Azure::Core::Amqp::Models::Value target,
      Azure::Core::Amqp::Models::Value) override
  {
    Azure::Core::Amqp::Models::_internal::MessageSource msgSource(source);
    Azure::Core::Amqp::Models::_internal::MessageTarget msgTarget(target);

    GTEST_LOG_(INFO) << "OnLinkAttached. Source: " << msgSource << " Target: " << msgTarget;

    // If the incoming role is receiver, then we want to create a sender to talk to it.
    // Similarly, if the incoming role is sender, we want to create a receiver to receive from it.
    if (role == Azure::Core::_internal::Amqp::SessionRole::Receiver)
    {
      Azure::Core::_internal::Amqp::MessageSenderOptions senderOptions;
      senderOptions.EnableTrace = true;
      senderOptions.Name = name;
      senderOptions.SourceAddress = static_cast<std::string>(msgSource.GetAddress());
      senderOptions.InitialDeliveryCount = 0;
      std::string targetAddress = static_cast<std::string>(msgTarget.GetAddress());
      if (senderOptions.SourceAddress == "$cbs")
      {
        m_cbsMessageSender = std::make_unique<Azure::Core::_internal::Amqp::MessageSender>(
            session, newLinkInstance, targetAddress, *m_connection, senderOptions, this);
        m_cbsMessageSender->Open();
        m_messageSenderQueue.CompleteOperation(true);
      }
      else
      {
        m_messageSender = std::make_unique<Azure::Core::_internal::Amqp::MessageSender>(
            session, newLinkInstance, targetAddress, *m_connection, senderOptions, this);
        m_messageSender->Open();
        m_messageSenderQueue.CompleteOperation(true);
      }
    }
    else if (role == Azure::Core::_internal::Amqp::SessionRole::Sender)
    {
      Azure::Core::_internal::Amqp::MessageReceiverOptions receiverOptions;
      receiverOptions.EnableTrace = true;
      receiverOptions.Name = name;
      receiverOptions.TargetAddress = static_cast<std::string>(msgTarget.GetAddress());
      receiverOptions.InitialDeliveryCount = 0;
      std::string sourceAddress = static_cast<std::string>(msgSource.GetAddress());
      if (receiverOptions.TargetAddress == "$cbs")
      {
        m_cbsMessageReceiver = std::make_unique<Azure::Core::_internal::Amqp::MessageReceiver>(
            session, newLinkInstance, sourceAddress, receiverOptions, this);
        m_cbsMessageReceiver->Open();
        m_messageReceiverQueue.CompleteOperation(true);
      }
      else
      {
        m_messageReceiver = std::make_unique<Azure::Core::_internal::Amqp::MessageReceiver>(
            session, newLinkInstance, sourceAddress, receiverOptions, this);
        m_messageReceiver->Open();
        m_messageReceiverQueue.CompleteOperation(true);
      }
    }
    return true;
  }

  virtual void OnEndpointFrameReceived(
      Azure::Core::_internal::Amqp::Connection const&,
      Azure::Core::Amqp::Models::Value const&,
      uint32_t,
      uint8_t*) override
  {
  }

  // Inherited via MessageReceiverEvents
  void OnMessageReceiverStateChanged(
      Azure::Core::_internal::Amqp::MessageReceiver const&,
      Azure::Core::_internal::Amqp::MessageReceiverState newState,
      Azure::Core::_internal::Amqp::MessageReceiverState oldState) override
  {
    GTEST_LOG_(INFO) << "Message Receiver State changed. Old state: "
                     << ReceiverStateToString(oldState)
                     << " New state: " << ReceiverStateToString(newState);
  }
  Azure::Core::Amqp::Models::Value OnMessageReceived(
      Azure::Core::Amqp::Models::Message message) override
  {
    GTEST_LOG_(INFO) << "Received a message " << message;
    if (IsCbsMessage(message))
    {
    }
    m_messageQueue.CompleteOperation(std::make_unique<Azure::Core::Amqp::Models::Message>(message));
    return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryAccepted();
  }

  // Inherited via MessageSenderEvents
  void OnMessageSenderStateChanged(
      Azure::Core::_internal::Amqp::MessageSender const&,
      Azure::Core::_internal::Amqp::MessageSenderState newState,
      Azure::Core::_internal::Amqp::MessageSenderState oldState) override
  {
    GTEST_LOG_(INFO) << "Message Sender State changed. Old state: " << SenderStateToString(oldState)
                     << " New state: " << SenderStateToString(newState);
  }

  const char* ConnectionStateToString(Azure::Core::_internal::Amqp::ConnectionState state)
  {
    switch (state)
    {
      case Azure::Core::_internal::Amqp::ConnectionState::Start:
        return "Start";
      case Azure::Core::_internal::Amqp::ConnectionState::HeaderReceived:
        return "HeaderReceived";
      case Azure::Core::_internal::Amqp::ConnectionState::HeaderSent:
        return "HeaderSent";
      case Azure::Core::_internal::Amqp::ConnectionState::HeaderExchanged:
        return "HeaderExchanged";
      case Azure::Core::_internal::Amqp::ConnectionState::OpenPipe:
        return "OpenPipe";
      case Azure::Core::_internal::Amqp::ConnectionState::OcPipe:
        return "OcPipe";
      case Azure::Core::_internal::Amqp::ConnectionState::OpenReceived:
        return "OpenReceived";
      case Azure::Core::_internal::Amqp::ConnectionState::OpenSent:
        return "OpenSent";
      case Azure::Core::_internal::Amqp::ConnectionState::ClosePipe:
        return "ClosePipe";
      case Azure::Core::_internal::Amqp::ConnectionState::Opened:
        return "Opened";
      case Azure::Core::_internal::Amqp::ConnectionState::CloseReceived:
        return "CloseReceived";
      case Azure::Core::_internal::Amqp::ConnectionState::CloseSent:
        return "CloseSent";
      case Azure::Core::_internal::Amqp::ConnectionState::Discarding:
        return "Discarding";
      case Azure::Core::_internal::Amqp::ConnectionState::End:
        return "End";
      case Azure::Core::_internal::Amqp::ConnectionState::Error:
        return "Error";
    }
    throw std::runtime_error("Unknown connection state");
  }

  const char* ReceiverStateToString(Azure::Core::_internal::Amqp::MessageReceiverState state)
  {
    switch (state)
    {
      case Azure::Core::_internal::Amqp::MessageReceiverState::Invalid:
        return "Invalid";
      case Azure::Core::_internal::Amqp::MessageReceiverState::Idle:
        return "Idle";
      case Azure::Core::_internal::Amqp::MessageReceiverState::Opening:
        return "Opening";
      case Azure::Core::_internal::Amqp::MessageReceiverState::Open:
        return "Open";
      case Azure::Core::_internal::Amqp::MessageReceiverState::Closing:
        return "Closing";
      case Azure::Core::_internal::Amqp::MessageReceiverState::Error:
        return "Error";
    }
    throw std::runtime_error("Unknown receiver state");
  }

  const char* SenderStateToString(Azure::Core::_internal::Amqp::MessageSenderState state)
  {
    // Return the stringized version of the values in the MessageSenderState enumeration
    switch (state)
    {
      case Azure::Core::_internal::Amqp::MessageSenderState::Invalid:
        return "Invalid";
      case Azure::Core::_internal::Amqp::MessageSenderState::Idle:
        return "Idle";
      case Azure::Core::_internal::Amqp::MessageSenderState::Opening:
        return "Opening";
      case Azure::Core::_internal::Amqp::MessageSenderState::Open:
        return "Open";
      case Azure::Core::_internal::Amqp::MessageSenderState::Closing:
        return "Closing";
      case Azure::Core::_internal::Amqp::MessageSenderState::Error:
        return "Error";
    }
    throw std::runtime_error("Unknown sender state");
  }
};
} // namespace MessageTests
