// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <gtest/gtest.h>

#include "azure/core/amqp/common/async_operation_queue.hpp"
#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/message_receiver.hpp"
#include "azure/core/amqp/message_sender.hpp"
#include "azure/core/amqp/models/messaging_values.hpp"
#include "azure/core/amqp/network/amqp_header_detect_transport.hpp"
#include "azure/core/amqp/network/socket_listener.hpp"
#include "azure/core/amqp/network/socket_transport.hpp"
#include "azure/core/amqp/session.hpp"
#include <functional>
#include <random>

extern uint16_t FindAvailableSocket();

class TestLinks : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

using namespace Azure::Core::Amqp;
using namespace Azure::Core::Amqp::_internal;
using namespace Azure::Core::Amqp::_detail;

TEST_F(TestLinks, SimpleLink)
{

  // Create a connection
  Connection connection("amqp://localhost:5672", {});
  // Create a session
  Session session(connection, nullptr);

  {
    Link link(session, "MySession", SessionRole::Sender, "MySource", "MyTarget");
  }

  {
    // Create two sessions
    Link link1(session, "MySession", SessionRole::Sender, "Source1", "Target1");
    Link link2(session, "MySession", SessionRole::Sender, "Source2", "Target2");
  }

#if 0
  // Create a sender
  Sender sender(session, "test");

  // Create a receiver
  Receiver receiver(session, "test");

  // Create a message
  Message message;

  // Send a message
  sender.Send(message);

  // Receive a message
  receiver.Receive();

  // Close the connection
  connection.Close();
#endif
}

TEST_F(TestLinks, LinkProperties)
{ // Create a connection
  Connection connection("amqp://localhost:5672", {});
  Session session(connection, nullptr);

  {
    Link link(session, "MySession", SessionRole::Sender, "MySource", "MyTarget");

    EXPECT_EQ("MySession", link.GetName());
    EXPECT_EQ(0, link.GetInitialDeliveryCount());
    EXPECT_EQ(0, link.GetMaxMessageSize());
    EXPECT_ANY_THROW(link.GetPeerMaxMessageSize());
    EXPECT_EQ(0, link.GetReceivedMessageId());
    EXPECT_EQ(ReceiverSettleMode::First, link.GetReceiverSettleMode());
    EXPECT_EQ(SenderSettleMode::Unsettled, link.GetSenderSettleMode());
  }

  {
    Link link(session, "MySession", SessionRole::Sender, "MySource", "MyTarget");

    link.SetInitialDeliveryCount(32767);
    EXPECT_EQ(32767, link.GetInitialDeliveryCount());

    link.SetMaxMessageSize(65535);
    EXPECT_EQ(65535, link.GetMaxMessageSize());

    link.SetReceiverSettleMode(ReceiverSettleMode::Second);
    EXPECT_EQ(ReceiverSettleMode::Second, link.GetReceiverSettleMode());
    link.SetReceiverSettleMode(ReceiverSettleMode::First);
    EXPECT_EQ(ReceiverSettleMode::First, link.GetReceiverSettleMode());

    link.SetSenderSettleMode(SenderSettleMode::Settled);
    EXPECT_EQ(SenderSettleMode::Settled, link.GetSenderSettleMode());
    link.SetSenderSettleMode(SenderSettleMode::Unsettled);
    EXPECT_EQ(SenderSettleMode::Unsettled, link.GetSenderSettleMode());
    link.SetSenderSettleMode(SenderSettleMode::Mixed);
    EXPECT_EQ(SenderSettleMode::Mixed, link.GetSenderSettleMode());

    link.SetMaxLinkCredit(95);

    link.SetAttachProperties("Attach Properties");
  }

  {
    Link link(session, "MySession", SessionRole::Sender, "MySource", "MyTarget");
    Link link2(link);

    Link link3(link.GetImpl());

    EXPECT_EQ(link.GetInitialDeliveryCount(), link2.GetInitialDeliveryCount());
    EXPECT_EQ(link.GetInitialDeliveryCount(), link3.GetInitialDeliveryCount());

    // If I set the initial delivery count on one link, it should affect all the copies of that
    // link.
    link.SetInitialDeliveryCount(32767);
    EXPECT_EQ(link.GetInitialDeliveryCount(), link2.GetInitialDeliveryCount());
    EXPECT_EQ(link.GetInitialDeliveryCount(), link3.GetInitialDeliveryCount());

    EXPECT_EQ("MySource", link.GetSource());
    EXPECT_EQ("MyTarget", link.GetTarget());
  }
}

class LinkSocketListenerEvents : public Azure::Core::Amqp::Network::_internal::SocketListenerEvents,
                                 public Azure::Core::Amqp::_internal::ConnectionEvents,
                                 public Azure::Core::Amqp::_internal::SessionEvents {
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
      std::shared_ptr<Azure::Core::Amqp::_internal::Connection>>
      m_listeningQueue;
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
      std::unique_ptr<Azure::Core::Amqp::_internal::Session>>
      m_listeningSessionQueue;
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<std::unique_ptr<Link>>
      m_receiveLinkQueue;
  std::shared_ptr<Azure::Core::Amqp::_internal::Connection> m_connection;

  virtual void OnSocketAccepted(XIO_INSTANCE_TAG* xio) override
  {
    GTEST_LOG_(INFO) << "OnSocketAccepted - Socket connection received.";
    std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> amqpTransport{
        std::make_shared<Azure::Core::Amqp::Network::_internal::AmqpHeaderTransport>(xio, nullptr)};
    Azure::Core::Amqp::_internal::ConnectionOptions options;
    options.ContainerId = "connectionId";
    options.EnableTrace = true;
    options.Transport = amqpTransport;
    m_connection
        = std::make_shared<Azure::Core::Amqp::_internal::Connection>(amqpTransport, options, this);
    m_connection->Listen();
    m_listeningQueue.CompleteOperation(m_connection);
  }

  virtual void OnConnectionStateChanged(
      Azure::Core::Amqp::_internal::Connection const&,
      ConnectionState newState,
      ConnectionState oldState) override
  {
    (void)oldState;
    (void)newState;
  }
  virtual bool OnNewEndpoint(
      Azure::Core::Amqp::_internal::Connection const& connection,
      Azure::Core::Amqp::_internal::Endpoint& endpoint) override
  {
    GTEST_LOG_(INFO) << "OnNewEndpoint - Incoming endpoint created, create session.";
    auto listeningSession
        = std::make_unique<Azure::Core::Amqp::_internal::Session>(connection, endpoint, this);
    listeningSession->SetIncomingWindow(10000);
    listeningSession->Begin();

    m_listeningSessionQueue.CompleteOperation(std::move(listeningSession));

    return true;
  }
  virtual void OnIoError(Azure::Core::Amqp::_internal::Connection const&) override {}
  // Inherited via Session
  virtual bool OnLinkAttached(
      Azure::Core::Amqp::_internal::Session const& session,
      Azure::Core::Amqp::_internal::LinkEndpoint& newLinkInstance,
      std::string const& name,
      Azure::Core::Amqp::_internal::SessionRole,
      Azure::Core::Amqp::Models::AmqpValue source,
      Azure::Core::Amqp::Models::AmqpValue target,
      Azure::Core::Amqp::Models::AmqpValue) override
  {
    GTEST_LOG_(INFO) << "OnLinkAttached - Link attached to session.";
    auto newLink = std::make_unique<Azure::Core::Amqp::_detail::Link>(
        session,
        newLinkInstance,
        name,
        Azure::Core::Amqp::_internal::SessionRole::Receiver,
        static_cast<std::string>(source),
        static_cast<std::string>(target));
    //    newLink->SetReceiverSettleMode(Azure::Core::Amqp::ReceiverSettleMode::First);
    m_receiveLinkQueue.CompleteOperation(std::move(newLink));

    return true;
  }
  virtual void OnEndpointFrameReceived(
      Connection const&,
      Azure::Core::Amqp::Models::AmqpValue const&,
      uint32_t,
      uint8_t*) override
  {
  }

public:
  LinkSocketListenerEvents() {}
  std::shared_ptr<Connection> WaitForConnection(
      Azure::Core::Amqp::Network::_internal::SocketListener const& listener,
      Azure::Core::Context context)
  {
    auto result = m_listeningQueue.WaitForPolledResult(context, listener);
    return std::move(std::get<0>(*result));
  }
  std::unique_ptr<Session> WaitForSession(Azure::Core::Context context)
  {
    auto result = m_listeningSessionQueue.WaitForPolledResult(context, *m_connection);
    return std::move(std::get<0>(*result));
  }
  std::unique_ptr<Azure::Core::Amqp::_detail::Link> WaitForLink(Azure::Core::Context context)
  {
    auto result = m_receiveLinkQueue.WaitForPolledResult(context, *m_connection);
    return std::move(std::get<0>(*result));
  }
};

TEST_F(TestLinks, LinkAttachDetach)
{
  LinkSocketListenerEvents events;

  uint16_t testPort = FindAvailableSocket();
  GTEST_LOG_(INFO) << "Test port: " << testPort;
  // Create a connection
  Connection connection("amqp://localhost:" + std::to_string(testPort), {}, &events);
  Session session(connection, nullptr);

  Network::_internal::SocketListener listener(testPort, &events);

  EXPECT_NO_THROW(listener.Start());
  {
    Link link(session, "MySession", SessionRole::Sender, "MySource", "MyTarget");
    link.Attach();

    Azure::Core::Amqp::Models::AmqpValue data;
    link.Detach(false, {}, {}, data);

    //    auto listeningConnection = listener.WaitForConnection();
    //    auto listeningSession = listeningConnection->WaitForSession();
    //    auto listeningLink = listeningSession->WaitForLink();
  }
  connection.Close("Test complete", "Completed", Models::AmqpValue());
  listener.Stop();
}
