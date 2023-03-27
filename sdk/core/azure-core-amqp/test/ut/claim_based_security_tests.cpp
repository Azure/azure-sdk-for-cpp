// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/amqp/claims_based_security.hpp>
#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/message_receiver.hpp>
#include <azure/core/amqp/message_sender.hpp>
#include <azure/core/amqp/models/message_source.hpp>
#include <azure/core/amqp/models/message_target.hpp>
#include <azure/core/amqp/models/messaging_values.hpp>
#include <azure/core/amqp/network/amqp_header_detect_transport.hpp>
#include <azure/core/amqp/network/socket_listener.hpp>
#include <azure/core/amqp/session.hpp>

extern uint16_t FindAvailableSocket();

class TestCbs : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

using namespace Azure::Core::Amqp;
using namespace Azure::Core::_internal::Amqp;
using namespace Azure::Core::_internal::Amqp::_detail;

TEST_F(TestCbs, SimpleCbs)
{

  // Create a connection
  Connection connection("amqp://localhost:5672", nullptr, {});
  // Create a session
  Session session(connection, nullptr);

  {
    ClaimsBasedSecurity cbs(session, connection);
  }

  {
    // Create two cbs objects
    ClaimsBasedSecurity cbs1(session, connection);
    ClaimsBasedSecurity cbs2(session, connection);
  }
}

const char* ConnectionStateToString(ConnectionState state)
{
  switch (state)
  {
    case ConnectionState::Start:
      return "Start";
    case ConnectionState::HeaderReceived:
      return "HeaderReceived";
    case ConnectionState::HeaderSent:
      return "HeaderSent";
    case ConnectionState::HeaderExchanged:
      return "HeaderExchanged";
    case ConnectionState::OpenPipe:
      return "OpenPipe";
    case ConnectionState::OcPipe:
      return "OcPipe";
    case ConnectionState::OpenReceived:
      return "OpenReceived";
    case ConnectionState::OpenSent:
      return "OpenSent";
    case ConnectionState::ClosePipe:
      return "ClosePipe";
    case ConnectionState::Opened:
      return "Opened";
    case ConnectionState::CloseReceived:
      return "CloseReceived";
    case ConnectionState::CloseSent:
      return "CloseSent";
    case ConnectionState::Discarding:
      return "Discarding";
    case ConnectionState::End:
      return "End";
    case ConnectionState::Error:
      return "Error";
  }
  throw std::runtime_error("Unknown connection state");
}

const char* ReceiverStateToString(MessageReceiverState state)
{
  switch (state)
  {
    case MessageReceiverState::Invalid:
      return "Invalid";
    case MessageReceiverState::Idle:
      return "Idle";
    case MessageReceiverState::Opening:
      return "Opening";
    case MessageReceiverState::Open:
      return "Open";
    case MessageReceiverState::Closing:
      return "Closing";
    case MessageReceiverState::Error:
      return "Error";
  }
  throw std::runtime_error("Unknown receiver state");
}

const char* SenderStateToString(MessageSenderState state)
{
  // Return the stringized version of the values in the MessageSenderState enumeration
  switch (state)
  {
    case MessageSenderState::Invalid:
      return "Invalid";
    case MessageSenderState::Idle:
      return "Idle";
    case MessageSenderState::Opening:
      return "Opening";
    case MessageSenderState::Open:
      return "Open";
    case MessageSenderState::Closing:
      return "Closing";
    case MessageSenderState::Error:
      return "Error";
  }
  throw std::runtime_error("Unknown sender state");
}

class CbsServerMock : public Azure::Core::_internal::Amqp::Network::SocketListenerEvents,
                      public Azure::Core::_internal::Amqp::ConnectionEvents,
                      public Azure::Core::_internal::Amqp::SessionEvents,
                      public Azure::Core::_internal::Amqp::MessageReceiverEvents,
                      public Azure::Core::_internal::Amqp::MessageSenderEvents {
public:
  CbsServerMock() { m_testPort = FindAvailableSocket(); }
  void Poll()
  {
    if (m_connection)
    {
      m_connection->Poll();
    }
  }

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
  std::unique_ptr<Models::Message> WaitForMessage(Azure::Core::Context context = {})
  {
    auto result = m_messageQueue.WaitForPolledResult(context, *m_connection);
    if (result)
    {
      return std::move(std::get<0>(*result));
    }
    else
    {
      return nullptr;
    }
  }

  uint16_t GetPort() const { return m_testPort; }

  void StartListening()
  {
    // Start the mock AMQP server which will be used to receive the connect open.
    // Ensure that the thread is started before we start using the message sender.
    std::mutex threadRunningMutex;
    std::condition_variable threadStarted;
    bool running = false;

    Azure::Core::Context listenerContext;
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
        GTEST_LOG_(INFO) << "Wait for incoming message.";
        auto message = WaitForMessage(m_listenerContext);
        if (!message)
        {
          GTEST_LOG_(INFO) << "No message, canceling thread";
        }
        else
        {
          GTEST_LOG_(INFO) << "Received message: " << *message;
          auto applicationProperties = message->GetApplicationProperties();
          if (applicationProperties.GetType() == Models::AmqpValueType::Described)
          {
            auto descriptor = applicationProperties.GetDescriptor();
            auto value = applicationProperties.GetDescribedValue();

            auto operation = value.GetMapValue("operation");
            auto type = value.GetMapValue("type");
            auto name = value.GetMapValue("name");
            // If we're processing a put-token message, then we should get a "type" and "name"
            // value.
            EXPECT_EQ(operation.GetType(), Models::AmqpValueType::String);
            if (static_cast<std::string>(operation) == "put-token")
            {
              EXPECT_EQ(type.GetType(), Models::AmqpValueType::String);
              EXPECT_EQ(name.GetType(), Models::AmqpValueType::String);
              // The body of a put-token operation MUST be an AMQP Value.
              EXPECT_EQ(message->GetBodyType(), Models::MessageBodyType::Value);

              // Respond to the operation.
              Models::Message response;
              Models::Properties responseProperties;

              // Management specification section 3.2: The correlation-id of the response message
              // MUST be the correlation-id from the request message (if present), else the
              // message-id from the request message.
              auto requestCorrelationId = message->GetProperties().GetCorrelationId();
              if (requestCorrelationId.IsNull())
              {
                requestCorrelationId = message->GetProperties().GetMessageId();
              }
              responseProperties.SetCorrelationId(requestCorrelationId);
              response.SetProperties(responseProperties);

              // Populate the response application properties.

              auto propertyMap = Models::Value::CreateMap();
              propertyMap.SetMapValue("status-code", 200);
              propertyMap.SetMapValue("status-description", "OK-put");

              // Create a descriptor to hold the property map and set it as the response's
              // application properties.
              response.SetApplicationProperties(propertyMap);

              // Set the response body type to an empty AMQP value.
              if (m_listenerContext.IsCancelled())
              {
                break;
              }
              response.SetBodyAmqpValue(Models::Value());
              m_messageSender->Send(response, m_listenerContext);
            }
            else if (static_cast<std::string>(operation) == "delete-token")
            {
              Models::Message response;
              Models::Properties responseProperties;

              // Management specification section 3.2: The correlation-id of the response message
              // MUST be the correlation-id from the request message (if present), else the
              // message-id from the request message.
              auto requestCorrelationId = message->GetProperties().GetCorrelationId();
              if (requestCorrelationId.IsNull())
              {
                requestCorrelationId = message->GetProperties().GetMessageId();
              }
              responseProperties.SetCorrelationId(requestCorrelationId);
              response.SetProperties(responseProperties);

              auto propertyMap = Models::Value::CreateMap();
              propertyMap.SetMapValue("status-code", 200);
              propertyMap.SetMapValue("status-description", "OK-delete");

              // Create a descriptor to hold the property map and set it as the response's
              // application properties.
              response.SetApplicationProperties(propertyMap);
              // Set the response body type to an empty AMQP value.
              if (m_listenerContext.IsCancelled())
              {
                break;
              }

              response.SetBodyAmqpValue(Models::Value());

              m_messageSender->Send(response, m_listenerContext);
            }
            else
            {
              EXPECT_TRUE(false) << "Unknown operation: " << operation;
            }
          }
          else
          {
            EXPECT_TRUE(false) << "Unknown message type: "
                               << static_cast<int>(applicationProperties.GetType());
          }
        }
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
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<std::unique_ptr<Models::Message>>
      m_messageQueue;
  std::unique_ptr<Azure::Core::_internal::Amqp::MessageReceiver> m_messageReceiver;
  std::unique_ptr<Azure::Core::_internal::Amqp::MessageSender> m_messageSender;
  std::thread m_serverThread;
  uint16_t m_testPort;
  Azure::Core::Context m_listenerContext; // Used to cancel the listener if necessary.

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
      ConnectionState newState,
      ConnectionState oldState) override
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

    // If the incoming role is receiver, then we want to create a sender to talk to it.
    // Similarly, if the incoming role is sender, we want to create a receiver to receive from it.
    if (role == SessionRole::Receiver)
    {
      MessageSenderOptions senderOptions;
      senderOptions.EnableTrace = true;
      senderOptions.Name = name;
      senderOptions.SourceAddress = static_cast<std::string>(msgSource.GetAddress());
      senderOptions.InitialDeliveryCount = 0;
      std::string targetAddress = static_cast<std::string>(msgTarget.GetAddress());
      m_messageSender = std::make_unique<MessageSender>(
          session, newLinkInstance, targetAddress, *m_connection, senderOptions, this);
      m_messageSender->Open();
      m_messageSenderQueue.CompleteOperation(true);
    }
    else if (role == SessionRole::Sender)
    {
      MessageReceiverOptions receiverOptions;
      receiverOptions.EnableTrace = true;
      receiverOptions.Name = name;
      receiverOptions.TargetAddress = static_cast<std::string>(msgTarget.GetAddress());
      receiverOptions.InitialDeliveryCount = 0;
      std::string sourceAddress = static_cast<std::string>(msgSource.GetAddress());
      m_messageReceiver = std::make_unique<MessageReceiver>(
          session, newLinkInstance, sourceAddress, receiverOptions, this);
      m_messageReceiver->Open();
      m_messageReceiverQueue.CompleteOperation(true);
    }
    return true;
  }

  virtual void OnEndpointFrameReceived(
      Connection const&,
      Azure::Core::Amqp::Models::Value const&,
      uint32_t,
      uint8_t*) override
  {
  }

  // Inherited via MessageReceiverEvents
  void OnMessageReceiverStateChanged(
      MessageReceiver const&,
      MessageReceiverState newState,
      MessageReceiverState oldState) override
  {
    GTEST_LOG_(INFO) << "Message Receiver State changed. Old state: "
                     << ReceiverStateToString(oldState)
                     << " New state: " << ReceiverStateToString(newState);
  }
  Azure::Core::Amqp::Models::Value OnMessageReceived(
      Azure::Core::Amqp::Models::Message message) override
  {
    GTEST_LOG_(INFO) << "Received a message " << message;
    m_messageQueue.CompleteOperation(std::make_unique<Models::Message>(message));
    return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryAccepted();
  }

  // Inherited via MessageSenderEvents
  void OnMessageSenderStateChanged(
      MessageSender const&,
      MessageSenderState newState,
      MessageSenderState oldState) override
  {
    GTEST_LOG_(INFO) << "Message Sender State changed. Old state: " << SenderStateToString(oldState)
                     << " New state: " << SenderStateToString(newState);
  }
};

TEST_F(TestCbs, CbsOpen)
{
  {
    CbsServerMock mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), nullptr, {});
    Session session(connection, nullptr);
    {
      ClaimsBasedSecurity cbs(session, connection);
      GTEST_LOG_(INFO) << "Expected failure for Open because no listener." << mockServer.GetPort();

      EXPECT_EQ(CbsOpenResult::Error, cbs.Open());
    }
  }
  {
    CbsServerMock mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), nullptr, {});
    Session session(connection, nullptr);

    mockServer.StartListening();

    {
      ClaimsBasedSecurity cbs(session, connection);
      cbs.SetTrace(true);
      EXPECT_EQ(CbsOpenResult::Ok, cbs.Open());
      GTEST_LOG_(INFO) << "Open Completed.";

      cbs.Close();
    }
    mockServer.StopListening();
  }
}

TEST_F(TestCbs, CbsOpenAndPut)
{
  {
    CbsServerMock mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), nullptr, {});
    Session session(connection, nullptr);

    mockServer.StartListening();

    {
      ClaimsBasedSecurity cbs(session, connection);
      cbs.SetTrace(true);

      EXPECT_EQ(CbsOpenResult::Ok, cbs.Open());
      GTEST_LOG_(INFO) << "Open Completed.";

      auto putResult = cbs.PutToken(
          Azure::Core::_internal::Amqp::CbsTokenType::Sas, "of one", "stringizedToken");
      EXPECT_EQ(CbsOperationResult::Ok, std::get<0>(putResult));
      EXPECT_EQ("OK-put", std::get<2>(putResult));

      cbs.Close();
    }

    mockServer.StopListening();
  }
}
