// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/amqp/claims_based_security.hpp>
#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/message_receiver.hpp>
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
    Cbs cbs(session, connection);
  }

  {
    // Create two cbs objects
    Cbs cbs1(session, connection);
    Cbs cbs2(session, connection);
  }
}

const char* ConnectionStateToString(ConnectionState state)
{
  switch (state)
  {
    case ConnectionState::Start:
      return "Start";
    case ConnectionState::HeaderReceived:
      return "HeaderReceivd";
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
};

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

class CbsServerMock : public Azure::Core::_internal::Amqp::Network::SocketListenerEvents,
                      public Azure::Core::_internal::Amqp::ConnectionEvents,
                      public Azure::Core::_internal::Amqp::SessionEvents,
                      public Azure::Core::_internal::Amqp::MessageReceiverEvents {
  std::shared_ptr<Azure::Core::_internal::Amqp::Connection> m_connection;
  std::shared_ptr<Azure::Core::_internal::Amqp::Session> m_session;
  std::shared_ptr<Azure::Core::_internal::Amqp::MessageReceiver> m_messageReceiver;

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
      //      Azure::Core::_internal::Amqp::SessionRole role,
      Azure::Core::Amqp::Models::Value source,
      Azure::Core::Amqp::Models::Value target,
      Azure::Core::Amqp::Models::Value) override
  {
    GTEST_LOG_(INFO) << "OnLinkAttached - Link attached to session.";
    Azure::Core::Amqp::Models::_internal::MessageSource msgSource(source);
    Azure::Core::Amqp::Models::_internal::MessageTarget msgTarget(target);

    MessageReceiverOptions receiverOptions;
    receiverOptions.EnableTrace = true;
    receiverOptions.Name = name;
    receiverOptions.TargetName = msgTarget.GetAddress();
    std::string sourceAddress = static_cast<std::string>(msgSource.GetAddress());
    m_messageReceiver = std::make_shared<MessageReceiver>(
        session, newLinkInstance, sourceAddress, receiverOptions, this);

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

public:
  CbsServerMock() {}
};

TEST_F(TestCbs, CbsOpen)
{
  auto testPort = FindAvailableSocket();

  Connection connection("amqp://localhost:" + std::to_string(testPort), nullptr, {});
  Session session(connection, nullptr);
  {
    Cbs cbs(session, connection);

    EXPECT_EQ(CbsOpenResult::Error, cbs.Open());
  }

  // Start the mock AMQP server which will be used to receive the connect open.
  CbsServerMock mockServer;
  Azure::Core::_internal::Amqp::Network::SocketListener listener(testPort, &mockServer);
  {
    Cbs cbs(session, connection);

    EXPECT_EQ(CbsOpenResult::Ok, cbs.Open());
  }
}
