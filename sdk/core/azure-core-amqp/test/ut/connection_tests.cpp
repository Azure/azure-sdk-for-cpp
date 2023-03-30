// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <gtest/gtest.h>

#include "azure/core/amqp/common/async_operation_queue.hpp"
#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/message_receiver.hpp"
#include "azure/core/amqp/models/messaging_values.hpp"
#include "azure/core/amqp/network/amqp_header_detect_transport.hpp"
#include "azure/core/amqp/network/socket_listener.hpp"
#include "azure/core/amqp/network/socket_transport.hpp"
#include "azure/core/amqp/session.hpp"
#include <azure/core/context.hpp>
#include <functional>
#include <random>

extern uint16_t FindAvailableSocket();

class TestConnections : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestConnections, SimpleConnection)
{
  {
    // Create a connection
    Azure::Core::_internal::Amqp::Connection connection("amqp://localhost:5672", nullptr, {});

    Azure::Core::_internal::Amqp::ConnectionOptions options;
    options.SaslCredentials
        = std::make_shared<Azure::Core::_internal::Amqp::SaslPlainConnectionStringCredential>(
            "Endpoint=sb://testHost.net/"
            ";SharedAccessKeyName=SomeName;SharedAccessKey=SomeKey;EntityPath=testhub");
    EXPECT_ANY_THROW(Azure::Core::_internal::Amqp::Connection connection2(
        "amqp://localhost:5672", nullptr, options));
  }
  {
    // Create a connection
    Azure::Core::_internal::Amqp::Connection connection("amqps://localhost:5671", nullptr, {});
  }
  {
    Azure::Core::_internal::Amqp::ConnectionOptions options;
    auto socketTransport{std::make_shared<Azure::Core::_internal::Amqp::Network::SocketTransport>(
        "localhost", 5672)};

    Azure::Core::_internal::Amqp::Connection connection(socketTransport, nullptr, options);
  }
  {
    Azure::Core::_internal::Amqp::ConnectionOptions options;
    auto socketTransport{std::make_shared<Azure::Core::_internal::Amqp::Network::SocketTransport>(
        "localhost", 5672)};
    options.SaslCredentials
        = std::make_shared<Azure::Core::_internal::Amqp::SaslPlainConnectionStringCredential>(
            "Endpoint=sb://testHost.net/"
            ";SharedAccessKeyName=SomeName;SharedAccessKey=SomeKey;EntityPath=testhub");

    EXPECT_ANY_THROW(
        Azure::Core::_internal::Amqp::Connection connection(socketTransport, nullptr, options));
  }

#if 0
  // Create a session
  Azure::Core::_internal::Amqp::Session session(connection);

  // Create a sender
  Azure::Core::Amqp::Sender sender(session, "test");

  // Create a receiver
  Azure::Core::Amqp::Receiver receiver(session, "test");

  // Create a message
  Azure::Core::Amqp::Message message;

  // Send a message
  sender.Send(message);

  // Receive a message
  receiver.Receive();

  // Close the connection
  connection.Close();
#endif
}

TEST_F(TestConnections, ConnectionAttributes)
{
  Azure::Core::_internal::Amqp::Connection connection("amqp://localhost:5672", nullptr, {});

  {
    auto idleTimeout = connection.GetIdleTimeout();
    (void)idleTimeout;
    EXPECT_NO_THROW(connection.SetIdleTimeout(std::chrono::milliseconds(1532)));
    EXPECT_EQ(std::chrono::milliseconds(1532), connection.GetIdleTimeout());
  }
  {
    auto maxFrameSize = connection.GetMaxFrameSize();
    EXPECT_NO_THROW(connection.SetMaxFrameSize(1024 * 64));
    EXPECT_EQ(1024 * 64, connection.GetMaxFrameSize());
    (void)maxFrameSize;

    EXPECT_NO_THROW(
        connection.GetRemoteMaxFrameSize()); // Likely doesn't work unless there's a remote.
  }

  {
    auto maxChannel = connection.GetMaxChannel();
    EXPECT_NO_THROW(connection.SetMaxChannel(128));
    EXPECT_EQ(128, connection.GetMaxChannel());
    (void)maxChannel;
  }

  {
    // Ratio must be a number between 0 and 1.
    EXPECT_NO_THROW(connection.SetRemoteIdleTimeoutEmptyFrameSendRatio(0.5));
  }

  {
    EXPECT_NO_THROW(connection.SetProperties("32.95"));
    EXPECT_EQ(std::string("32.95"), static_cast<std::string>(connection.GetProperties()));
  }
}

TEST_F(TestConnections, ConnectionOpenClose)
{
  class TestListener : public Azure::Core::_internal::Amqp::Network::SocketListenerEvents {
  public:
    std::shared_ptr<Azure::Core::_internal::Amqp::Network::Transport> WaitForResult(
        Azure::Core::_internal::Amqp::Network::SocketListener const& listener,
        Azure::Core::Context context = {})
    {
      auto result = m_listenerQueue.WaitForPolledResult(context, listener);
      return std::get<0>(*result);
    }

  private:
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
        std::shared_ptr<Azure::Core::_internal::Amqp::Network::Transport>>
        m_listenerQueue;

    virtual void OnSocketAccepted(XIO_INSTANCE_TAG* xio)
    {
      // Capture the XIO into a transport so it won't leak.
      m_listenerQueue.CompleteOperation(
          std::make_shared<Azure::Core::_internal::Amqp::Network::Transport>(xio, nullptr));
    }
  };

  {
    // Ensure someone is listening on the connection for when we call connection.Open.

    uint16_t testPort = FindAvailableSocket();

    GTEST_LOG_(INFO) << "Test listener using port: " << testPort;

    TestListener listenerEvents;
    Azure::Core::_internal::Amqp::Network::SocketListener listener(testPort, &listenerEvents);
    EXPECT_NO_THROW(listener.Start());

    // Create a connection
    Azure::Core::_internal::Amqp::Connection connection(
        "amqp://localhost:" + std::to_string(testPort), nullptr, {});

    // Open the connection
    connection.Open();

    // Ensure that we got an OnComplete callback.
    auto transport = listenerEvents.WaitForResult(listener);

    // Now we can close the connection.
    connection.Close("xxx", "yyy", {});
    listener.Stop();
  }

  {
    Azure::Core::_internal::Amqp::ConnectionOptions options;
    options.HostName = "localhost";
    options.Port = 5671;
    //    std::shared_ptr<Azure::Core::_internal::Amqp::Network::SocketTransport> sockets
    //        =
    //        std::make_shared<Azure::Core::_internal::Amqp::Network::SocketTransport>("localhost",
    //        5671);
    Azure::Core::_internal::Amqp::Connection connection("amqp://localhost:5671", nullptr, options);
  }
}

class TestSocketListenerEvents : public Azure::Core::_internal::Amqp::SessionEvents,
                                 public Azure::Core::_internal::Amqp::ConnectionEvents,
                                 public Azure::Core::_internal::Amqp::Network::SocketListenerEvents,
                                 public Azure::Core::_internal::Amqp::MessageReceiverEvents {
public:
  std::unique_ptr<Azure::Core::_internal::Amqp::Connection> WaitForListener(
      Azure::Core::_internal::Amqp::Network::SocketListener const& listener,
      Azure::Core::Context context = {})
  {
    auto result = m_listeningQueue.WaitForPolledResult(context, listener);
    return std::move(std::get<0>(*result));
  }

private:
  std::unique_ptr<Azure::Core::_internal::Amqp::MessageReceiver> m_messageReceiver;
  //  std::unique_ptr<Azure::Core::Amqp::Link> m_link;

  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
      std::unique_ptr<Azure::Core::_internal::Amqp::Connection>>
      m_listeningQueue;
  virtual void OnSocketAccepted(XIO_INSTANCE_TAG* xio) override
  {
    std::shared_ptr<Azure::Core::_internal::Amqp::Network::Transport> amqpTransport{
        std::make_shared<Azure::Core::_internal::Amqp::Network::AmqpHeaderTransport>(xio, nullptr)};
    Azure::Core::_internal::Amqp::ConnectionOptions options;
    options.ContainerId = "containerId";
    options.HostName = "localhost";
    auto newConnection{
        std::make_unique<Azure::Core::_internal::Amqp::Connection>(amqpTransport, this, options)};
    newConnection->SetTrace(true);
    newConnection->Listen();
    m_listeningQueue.CompleteOperation(std::move(newConnection));
  }
  // Inherited via Session
  virtual bool OnLinkAttached(
      Azure::Core::_internal::Amqp::Session const& session,
      Azure::Core::_internal::Amqp::LinkEndpoint& newLinkInstance,
      std::string const& name,
      Azure::Core::_internal::Amqp::SessionRole,
      Azure::Core::Amqp::Models::Value source,
      Azure::Core::Amqp::Models::Value target,
      Azure::Core::Amqp::Models::Value) override
  {
    Azure::Core::_internal::Amqp::MessageReceiverOptions receiverOptions;
    receiverOptions.Name = name;
    receiverOptions.TargetAddress = static_cast<std::string>(target);
    receiverOptions.SettleMode = Azure::Core::_internal::Amqp::ReceiverSettleMode::First;

    m_messageReceiver = std::make_unique<Azure::Core::_internal::Amqp::MessageReceiver>(
        session, newLinkInstance, static_cast<std::string>(source), receiverOptions);

    m_messageReceiver->Open();

    return true;
  }
  // Inherited via ConnectionCallback
  virtual void OnConnectionStateChanged(
      Azure::Core::_internal::Amqp::Connection const&,
      Azure::Core::_internal::Amqp::ConnectionState newState,
      Azure::Core::_internal::Amqp::ConnectionState oldState) override
  {
    (void)oldState;
    (void)newState;
  }
  virtual bool OnNewEndpoint(
      Azure::Core::_internal::Amqp::Connection const& connection,
      Azure::Core::_internal::Amqp::Endpoint& endpoint) override
  {
    m_listeningSession
        = std::make_unique<Azure::Core::_internal::Amqp::Session>(connection, endpoint, this);
    m_listeningSession->SetIncomingWindow(10000);
    m_listeningSession->Begin();

    return true;
  }
  virtual void OnIoError(Azure::Core::_internal::Amqp::Connection const&) override {}
  virtual void OnEndpointFrameReceived(
      Azure::Core::_internal::Amqp::Connection const& connection,
      Azure::Core::Amqp::Models::Value const& value,
      uint32_t framePayloadSize,
      uint8_t* payloadBytes) override
  {
    (void)connection, (void)value, (void)framePayloadSize, (void)payloadBytes;
  }
  // Inherited via MessageReceiver
  virtual Azure::Core::Amqp::Models::Value OnMessageReceived(
      Azure::Core::Amqp::Models::Message) override
  {
    GTEST_LOG_(INFO) << "Message received";

    return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryAccepted();
  }
  virtual void OnMessageReceiverStateChanged(
      Azure::Core::_internal::Amqp::MessageReceiver const& receiver,
      Azure::Core::_internal::Amqp::MessageReceiverState newState,
      Azure::Core::_internal::Amqp::MessageReceiverState oldState) override
  {
    (void)receiver;
    (void)newState;
    (void)oldState;
  }

  std::unique_ptr<Azure::Core::_internal::Amqp::Session> m_listeningSession;
};

TEST_F(TestConnections, ConnectionListenClose)
{
  // Ensure someone is listening on the connection for when we call connection.Open.
  TestSocketListenerEvents listenerEvents;
  Azure::Core::_internal::Amqp::Network::SocketListener listener(5672, &listenerEvents);

  listener.Start();

  {
    // Create a connection
    Azure::Core::_internal::Amqp::Connection connection("amqp://localhost:5672", nullptr, {});

    // Open the connection
    connection.Open();

    //    // Ensure that we got an OnComplete callback.
    auto incomingConnection = listenerEvents.WaitForListener(listener);

    // Now we can close the connection.
    connection.Close("", "yyy", {});

    incomingConnection->Close({}, {}, {});
  }

  {
    Azure::Core::_internal::Amqp::ConnectionOptions options;
    options.HostName = "localhost";
    options.Port = 5671;
    Azure::Core::_internal::Amqp::Connection connection("amqp://localhost:5671", nullptr, options);
  }

  listener.Stop();
}
#if 0
TEST_F(TestConnections, ConnectionSendSimpleMessage)
{
  TestSocketListenerEvents listenerEvents;
  Azure::Core::_internal::Amqp::Network::SocketListener listener(5672, &listenerEvents);
  listener.Start();
  // Create a connection
  Azure::Core::_internal::Amqp::Connection connection("amqp://localhost:5672", nullptr, {});

  Azure::Core::_internal::Amqp::Session session(connection, nullptr);
  session.SetIncomingWindow(std::numeric_limits<int32_t>::max());
  session.SetOutgoingWindow(655536);

  Azure::Core::Amqp::Link link(
      session,
      "sender-link",
      Azure::Core::_internal::Amqp::SessionRole::Sender,
      "ingress",
      "localhost/ingress");

  link.SetMaxMessageSize(65536);

  uint8_t messageBody[] = "hello";

  Azure::Core::Amqp::Models::Message message;
  message.AddBodyAmqpData({messageBody, sizeof(messageBody)});

  connection.Open();

  // We have to explicitly call "Stop" on the listener, we cannot wait on the destructor to shut it
  // down.
  listener.Stop();
}
#endif

#if 0
TEST_F(TestConnections, CreateSessionFromConnection)
{
  // Create a connection
  Azure::Core::_internal::Amqp::Connection connection{Azure::Core::_internal::Amqp::Connection::Create(
      "amqp://localhost:5672", {})};

  // Create a session
  Azure::Core::_internal::Amqp::SessionOptions sessionOptions;
  std::unique_ptr<Azure::Core::_internal::Amqp::Session> session{connection.CreateSession(sessionOptions)};
  //  Azure::Core::_internal::Amqp::Session session(connection);
}
#endif
