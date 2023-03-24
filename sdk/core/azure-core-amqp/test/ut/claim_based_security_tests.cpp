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
    ClaimBasedSecurity cbs(session, connection);
  }

  {
    // Create two cbs objects
    ClaimBasedSecurity cbs1(session, connection);
    ClaimBasedSecurity cbs2(session, connection);
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
  std::shared_ptr<Azure::Core::_internal::Amqp::Connection> m_connection;
  std::shared_ptr<Azure::Core::_internal::Amqp::Session> m_session;

  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> m_messageReceiverQueue;
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> m_messageSenderQueue;
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> m_connectionQueue;
  Azure::Core::_internal::Amqp::MessageReceiver m_messageReceiver;
  Azure::Core::_internal::Amqp::MessageSender m_messageSender;

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
    GTEST_LOG_(INFO) << "OnLinkAttached - Link attached to session. Source: " << source
                     << " Target: " << target;
    Azure::Core::Amqp::Models::_internal::MessageSource msgSource(source);
    Azure::Core::Amqp::Models::_internal::MessageTarget msgTarget(target);

    // If the incoming role is receiver, then we want to create a sender to talk to it.
    // Similarly, if the incoming role is sender, we want to create a receiver to receive from it.
    if (role == SessionRole::Receiver)
    {
      GTEST_LOG_(INFO) << "Message Sender link attached.";
      MessageSenderOptions senderOptions;
      senderOptions.EnableTrace = true;
      senderOptions.Name = name;
      senderOptions.SourceAddress = static_cast<std::string>(msgSource.GetAddress());
      senderOptions.InitialDeliveryCount = 0;
      std::string targetAddress = static_cast<std::string>(msgTarget.GetAddress());
      m_messageSender = MessageSender(
          session, newLinkInstance, targetAddress, *m_connection, senderOptions, this);
      m_messageSender.Open();
      m_messageSenderQueue.CompleteOperation(true);
    }
    else if (role == SessionRole::Sender)
    {
      GTEST_LOG_(INFO) << "Message Receiver link attached.";
      MessageReceiverOptions receiverOptions;
      receiverOptions.EnableTrace = true;
      receiverOptions.Name = name;
      receiverOptions.TargetAddress = static_cast<std::string>(msgTarget.GetAddress());
      receiverOptions.InitialDeliveryCount = 0;
      std::string sourceAddress = static_cast<std::string>(msgSource.GetAddress());
      m_messageReceiver
          = MessageReceiver(session, newLinkInstance, sourceAddress, receiverOptions, this);
      m_messageReceiver.Open();
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
    // Assume we're going to accept the message delivery.
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

public:
  CbsServerMock() {}
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
  std::shared_ptr<Azure::Core::_internal::Amqp::Connection> GetConnection() { return m_connection; }
  Azure::Core::_internal::Amqp::MessageReceiver& GetMessageReceiver() { return m_messageReceiver; }
  Azure::Core::_internal::Amqp::MessageSender& GetMessageSender() { return m_messageSender; }
};

TEST_F(TestCbs, CbsOpen)
{
  auto testPort = FindAvailableSocket();
  {
    Connection connection("amqp://localhost:" + std::to_string(testPort), nullptr, {});
    Session session(connection, nullptr);
    {
      ClaimBasedSecurity cbs(session, connection);
      GTEST_LOG_(INFO) << "Expected failure for Open because no listener." << testPort;

      EXPECT_EQ(CbsOpenResult::Error, cbs.Open());
    }
  }
  {
    Connection connection("amqp://localhost:" + std::to_string(testPort), nullptr, {});
    Session session(connection, nullptr);
    // Start the mock AMQP server which will be used to receive the connect open.
    // Ensure that the thread is started before we start using the message sender.
    std::mutex threadRunningMutex;
    std::condition_variable threadStarted;
    bool running = false;

    Azure::Core::Context listenerContext;
    std::thread listenerThread([&]() {
      CbsServerMock mockServer;
      Azure::Core::_internal::Amqp::Network::SocketListener listener(testPort, &mockServer);
      GTEST_LOG_(INFO) << "Start test listener on port " << testPort;
      listener.Start();
      running = true;
      threadStarted.notify_one();

      if (!mockServer.WaitForConnection(listener, listenerContext))
      {
        GTEST_LOG_(INFO) << "Cancelling thread.";
        return;
      }
      GTEST_LOG_(INFO) << "Wait for message receiver.";
      if (!mockServer.WaitForMessageReceiver(listenerContext))
      {
        GTEST_LOG_(INFO) << "Cancelling thread.";
        return;
      }
      if (!mockServer.WaitForMessageSender(listenerContext))
      {
        GTEST_LOG_(INFO) << "Cancelling thread.";
        return;
      }

      GTEST_LOG_(INFO) << "Wait for incoming message.";
      auto message = mockServer.GetMessageReceiver().WaitForIncomingMessage(
          *mockServer.GetConnection(), listenerContext);
      if (!message)
      {
        GTEST_LOG_(INFO) << "Canceling thread";
      }
      else
      {
        GTEST_LOG_(INFO) << "Received message: " << message;
      }
      mockServer.GetMessageSender().Close();
      mockServer.GetMessageReceiver().Close();
      listener.Stop();
    });

    GTEST_LOG_(INFO) << "Wait for listener to start.";
    std::unique_lock<std::mutex> waitForThreadStart(threadRunningMutex);
    threadStarted.wait(waitForThreadStart, [&running]() { return running == true; });
    GTEST_LOG_(INFO) << "Listener running, attempt CBS.";

    {
      ClaimBasedSecurity cbs(session, connection);
      cbs.SetTrace(true);

      EXPECT_EQ(CbsOpenResult::Ok, cbs.Open());
      GTEST_LOG_(INFO) << "Open Completed.";
    }
    listenerContext.Cancel();

    // Wait for the listener thread to exit.
    listenerThread.join();
  }
}
