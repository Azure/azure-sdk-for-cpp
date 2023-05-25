// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/common/async_operation_queue.hpp"
#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/message_receiver.hpp"
#include "azure/core/amqp/models/amqp_protocol.hpp"
#include "azure/core/amqp/models/messaging_values.hpp"
#include "azure/core/amqp/network/amqp_header_detect_transport.hpp"
#include "azure/core/amqp/network/socket_listener.hpp"
#include "azure/core/amqp/network/socket_transport.hpp"
#include "azure/core/amqp/session.hpp"
#include <azure/core/context.hpp>
#include <azure/core/platform.hpp>
#include <functional>
#include <gtest/gtest.h>
#include <random>

extern uint16_t FindAvailableSocket();

class TestConnections : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

#if !defined(AZ_PLATFORM_MAC)
TEST_F(TestConnections, SimpleConnection)
{
  {
    // Create a connection
    Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
    connectOptions.Port = Azure::Core::Amqp::_internal::AmqpPort;

    Azure::Core::Amqp::_internal::Connection connection("localhost", {});
  }
  {
    // Create a connection
    Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
    connectOptions.Port = Azure::Core::Amqp::_internal::AmqpPort;

    Azure::Core::Amqp::_internal::Connection connection("localhost", connectOptions);
  }
  {
    Azure::Core::Amqp::_internal::ConnectionOptions options;
    auto socketTransport{Azure::Core::Amqp::Network::_internal::SocketTransportFactory::Create(
        "localhost", Azure::Core::Amqp::_internal::AmqpPort)};

    Azure::Core::Amqp::_internal::Connection connection(socketTransport, options);
  }

#if 0
  // Create a session
  Azure::Core::Amqp::_internal::Session session(connection);

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
  {
    Azure::Core::Amqp::_internal::ConnectionOptions options;
    options.IdleTimeout = std::chrono::milliseconds(1532);

    Azure::Core::Amqp::_internal::Connection connection("localhost", options);

    EXPECT_EQ(connection.GetPort(), 5671);
    EXPECT_EQ(connection.GetHost(), "localhost");

    auto idleTimeout = connection.GetIdleTimeout();
    (void)idleTimeout;
    EXPECT_EQ(std::chrono::milliseconds(1532), connection.GetIdleTimeout());
  }
  {
    Azure::Core::Amqp::_internal::ConnectionOptions options;
    options.MaxFrameSize = 1024 * 64;
    options.Port = Azure::Core::Amqp::_internal::AmqpPort;
    Azure::Core::Amqp::_internal::Connection connection("localhost", options);
    EXPECT_EQ(connection.GetPort(), 5672);
    EXPECT_EQ(connection.GetHost(), "localhost");

    auto maxFrameSize = connection.GetMaxFrameSize();
    (void)maxFrameSize;
    EXPECT_EQ(1024 * 64, connection.GetMaxFrameSize());

    EXPECT_NO_THROW(
        connection.GetRemoteMaxFrameSize()); // Likely doesn't work unless there's a remote.
  }

  {
    Azure::Core::Amqp::_internal::ConnectionOptions options;
    options.MaxChannelCount = 128;
    options.Port = Azure::Core::Amqp::_internal::AmqpPort;

    Azure::Core::Amqp::_internal::Connection connection("localhost", options);
    EXPECT_EQ(connection.GetPort(), 5672);
    EXPECT_EQ(connection.GetHost(), "localhost");

    auto maxChannel = connection.GetMaxChannel();
    EXPECT_EQ(128, connection.GetMaxChannel());
    (void)maxChannel;
  }

  {
    Azure::Core::Amqp::_internal::ConnectionOptions options;
    options.MaxChannelCount = 128;

    Azure::Core::Amqp::_internal::Connection connection("localhost", options);
    // Ratio must be a number between 0 and 1.
    EXPECT_NO_THROW(connection.SetIdleEmptyFrameSendPercentage(0.5));
  }

  {
    Azure::Core::Amqp::_internal::ConnectionOptions options;
    options.MaxChannelCount = 128;
    options.Properties["test"] = "test";

    Azure::Core::Amqp::_internal::Connection connection("localhost", options);
    EXPECT_EQ(Azure::Core::Amqp::Models::AmqpValue{"test"}, connection.GetProperties()["test"]);
  }
}

TEST_F(TestConnections, ConnectionOpenClose)
{
  class TestListener : public Azure::Core::Amqp::Network::_internal::SocketListenerEvents {
  public:
    std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> WaitForResult(
        Azure::Core::Amqp::Network::_internal::SocketListener const& listener,
        Azure::Core::Context const& context = {})
    {
      GTEST_LOG_(INFO) << "Waiting for listener to accept connection.";
      auto result = m_listenerQueue.WaitForPolledResult(context, listener);
      return std::get<0>(*result);
    }

  private:
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
        std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport>>
        m_listenerQueue;

    virtual void OnSocketAccepted(
        std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> transport)
    {
      GTEST_LOG_(INFO) << "Socket for listener accepted connection.";
      m_listenerQueue.CompleteOperation(transport);
    }
  };

  {
    // Ensure someone is listening on the connection for when we call connection.Open.

    uint16_t testPort = FindAvailableSocket();

    GTEST_LOG_(INFO) << "Test listener using port: " << testPort;

    TestListener listenerEvents;
    Azure::Core::Amqp::Network::_internal::SocketListener listener(testPort, &listenerEvents);
    EXPECT_NO_THROW(listener.Start());

    // Create a connection
    Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
    connectOptions.Port = testPort;
    Azure::Core::Amqp::_internal::Connection connection("localhost", connectOptions);

    // Open the connection
    connection.Open();

    // Ensure that we got an OnComplete callback within 5 seconds.
    auto transport = listenerEvents.WaitForResult(
        listener,
        Azure::Core::Context::ApplicationContext.WithDeadline(
            std::chrono::system_clock::now() + std::chrono::seconds(5)));

    // Now we can close the connection.
    connection.Close("xxx", "yyy", {});
    listener.Stop();
  }

  {
    Azure::Core::Amqp::_internal::ConnectionOptions options;
    Azure::Core::Amqp::_internal::Connection connection("localhost", options);
  }
}

class TestSocketListenerEvents : public Azure::Core::Amqp::_internal::SessionEvents,
                                 public Azure::Core::Amqp::_internal::ConnectionEvents,
                                 public Azure::Core::Amqp::Network::_internal::SocketListenerEvents,
                                 public Azure::Core::Amqp::_internal::MessageReceiverEvents {
public:
  std::unique_ptr<Azure::Core::Amqp::_internal::Connection> WaitForListener(
      Azure::Core::Amqp::Network::_internal::SocketListener const& listener,
      Azure::Core::Context const& context = {})
  {
    auto result = m_listeningQueue.WaitForPolledResult(context, listener);
    return std::move(std::get<0>(*result));
  }

private:
  std::unique_ptr<Azure::Core::Amqp::_internal::MessageReceiver> m_messageReceiver;
  //  std::unique_ptr<Azure::Core::Amqp::Link> m_link;

  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
      std::unique_ptr<Azure::Core::Amqp::_internal::Connection>>
      m_listeningQueue;
  virtual void OnSocketAccepted(
      std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> transport) override
  {
    auto amqpTransport{
        Azure::Core::Amqp::Network::_internal::AmqpHeaderDetectTransportFactory::Create(
            transport, nullptr)};
    Azure::Core::Amqp::_internal::ConnectionOptions options;
    options.ContainerId = "containerId";
    options.EnableTrace = true;
    auto newConnection{
        std::make_unique<Azure::Core::Amqp::_internal::Connection>(amqpTransport, options, this)};
    newConnection->Listen();
    m_listeningQueue.CompleteOperation(std::move(newConnection));
  }
  // Inherited via Session
  virtual bool OnLinkAttached(
      Azure::Core::Amqp::_internal::Session const& session,
      Azure::Core::Amqp::_internal::LinkEndpoint& newLinkInstance,
      std::string const& name,
      Azure::Core::Amqp::_internal::SessionRole,
      Azure::Core::Amqp::Models::AmqpValue const& source,
      Azure::Core::Amqp::Models::AmqpValue const& target,
      Azure::Core::Amqp::Models::AmqpValue const&) override
  {
    Azure::Core::Amqp::_internal::MessageReceiverOptions receiverOptions;
    receiverOptions.Name = name;
    receiverOptions.MessageTarget = static_cast<std::string>(target);
    receiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;

    m_messageReceiver = std::make_unique<Azure::Core::Amqp::_internal::MessageReceiver>(
        session, newLinkInstance, static_cast<std::string>(source), receiverOptions);

    m_messageReceiver->Open();

    return true;
  }
  // Inherited via ConnectionCallback
  virtual void OnConnectionStateChanged(
      Azure::Core::Amqp::_internal::Connection const&,
      Azure::Core::Amqp::_internal::ConnectionState newState,
      Azure::Core::Amqp::_internal::ConnectionState oldState) override
  {
    (void)oldState;
    (void)newState;
  }
  virtual bool OnNewEndpoint(
      Azure::Core::Amqp::_internal::Connection const& connection,
      Azure::Core::Amqp::_internal::Endpoint& endpoint) override
  {
    Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
    sessionOptions.InitialIncomingWindowSize = 10000;
    m_listeningSession = std::make_unique<Azure::Core::Amqp::_internal::Session>(
        connection, endpoint, sessionOptions, this);

    m_listeningSession->Begin();

    return true;
  }
  virtual void OnIoError(Azure::Core::Amqp::_internal::Connection const&) override {}
  // Inherited via MessageReceiver
  virtual Azure::Core::Amqp::Models::AmqpValue OnMessageReceived(
      Azure::Core::Amqp::_internal::MessageReceiver const&,
      Azure::Core::Amqp::Models::AmqpMessage const&) override
  {
    GTEST_LOG_(INFO) << "Message received";

    return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryAccepted();
  }
  virtual void OnMessageReceiverStateChanged(
      Azure::Core::Amqp::_internal::MessageReceiver const& receiver,
      Azure::Core::Amqp::_internal::MessageReceiverState newState,
      Azure::Core::Amqp::_internal::MessageReceiverState oldState) override
  {
    (void)receiver;
    (void)newState;
    (void)oldState;
  }

  std::unique_ptr<Azure::Core::Amqp::_internal::Session> m_listeningSession;
};

TEST_F(TestConnections, ConnectionListenClose)
{
  // Ensure someone is listening on the connection for when we call connection.Open.
  TestSocketListenerEvents listenerEvents;
  Azure::Core::Amqp::Network::_internal::SocketListener listener(
      Azure::Core::Amqp::_internal::AmqpPort, &listenerEvents);

  listener.Start();

  {
    // Create a connection
    Azure::Core::Amqp::_internal::ConnectionOptions connectOptions;
    connectOptions.Port = Azure::Core::Amqp::_internal::AmqpPort;
    Azure::Core::Amqp::_internal::Connection connection("localhost", connectOptions);
    // Open the connection
    connection.Open();

    //    // Ensure that we got an OnComplete callback.
    auto incomingConnection = listenerEvents.WaitForListener(listener);

    // Now we can close the connection.
    connection.Close("", "yyy", {});

    incomingConnection->Close({}, {}, {});
  }

  {
    Azure::Core::Amqp::_internal::ConnectionOptions options;
    options.Port = Azure::Core::Amqp::_internal::AmqpPort;
    Azure::Core::Amqp::_internal::Connection connection("localhost", options);
  }

  listener.Stop();
}
#endif // !defined(AZ_PLATFORM_MAC)

#if 0
TEST_F(TestConnections, ConnectionSendSimpleMessage)
{
  TestSocketListenerEvents listenerEvents;
  Azure::Core::Amqp::_internal::Network::SocketListener listener(AmqpPort, &listenerEvents);
  listener.Start();
  // Create a connection
  Azure::Core::Amqp::_internal::Connection connection("amqp://localhost:5672", {});

  Azure::Core::Amqp::_internal::Session session(connection, nullptr);
  session.SetIncomingWindow(std::numeric_limits<int32_t>::max());
  session.SetOutgoingWindow(655536);

  Azure::Core::Amqp::Link link(
      session,
      "sender-link",
      Azure::Core::Amqp::_internal::SessionRole::Sender,
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
  Azure::Core::Amqp::_internal::Connection connection{Azure::Core::Amqp::_internal::Connection::Create(
      "amqp://localhost:AmqpPort", {})};

  // Create a session
  Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
  std::unique_ptr<Azure::Core::Amqp::_internal::Session> session{connection.CreateSession(sessionOptions)};
  //  Azure::Core::Amqp::_internal::Session session(connection);
}
#endif
