#include <gtest/gtest.h>

#include "azure/core/amqp//message_sender.hpp"
#include "azure/core/amqp/Connection.hpp"
#include "azure/core/amqp/Session.hpp"
#include "azure/core/amqp/common/async_operation_queue.hpp"
#include "azure/core/amqp/message_receiver.hpp"
#include "azure/core/amqp/models/messaging_values.hpp"
#include "azure/core/amqp/network/amqp_header_detect_transport.hpp"
#include "azure/core/amqp/network/socket_listener.hpp"
#include "azure/core/amqp/network/socket_transport.hpp"
#include <functional>

class TestLinks : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

using namespace Azure::Core::Amqp;
using namespace Azure::Core::_internal::Amqp;
using namespace Azure::Core::_internal::Amqp::_detail;

TEST_F(TestLinks, SimpleLink)
{

  // Create a connection
  Connection connection("amqp://localhost:5672", nullptr, {});
  // Create a session
  Session session(connection, nullptr);

  {
    Link link(session, "MySession", _detail::SessionRole::Sender, "MySource", "MyTarget");
  }

  {
    // Create two sessions
    Link link1(session, "MySession", _detail::SessionRole::Sender, "Source1", "Target1");
    Link link2(session, "MySession", _detail::SessionRole::Sender, "Source2", "Target2");
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
  Connection connection("amqp://localhost:5672", nullptr, {});
  Session session(connection, nullptr);

  {
    Link link(session, "MySession", _detail::SessionRole::Sender, "MySource", "MyTarget");

    EXPECT_EQ("MySession", link.GetName());
    EXPECT_EQ(0, link.GetInitialDeliveryCount());
    EXPECT_EQ(0, link.GetMaxMessageSize());
    EXPECT_ANY_THROW(link.GetPeerMaxMessageSize());
    EXPECT_EQ(0, link.GetReceivedMessageId());
    EXPECT_EQ(ReceiverSettleMode::First, link.GetReceiverSettleMode());
    EXPECT_EQ(SenderSettleMode::Unsettled, link.GetSenderSettleMode());
  }

  {
    Link link(session, "MySession", _detail::SessionRole::Sender, "MySource", "MyTarget");

    link.SetInitialDeliveryCount(32767);
    EXPECT_EQ(32767, link.GetInitialDeliveryCount());

    link.SetMaxMessageSize(65535);
    EXPECT_EQ(65535, link.GetMaxMessageSize());

    link.SetReceiverSettleMode(ReceiverSettleMode::Second);
    EXPECT_EQ(ReceiverSettleMode::Second, link.GetReceiverSettleMode());

    link.SetSenderSettleMode(SenderSettleMode::Settled);
    EXPECT_EQ(SenderSettleMode::Settled, link.GetSenderSettleMode());

    link.SetMaxLinkCredit(95);

    link.SetAttachProperties("Attach Properties");
  }
}

class LinkSocketListenerEvents : public Azure::Core::_internal::Amqp::Network::SocketListenerEvents,
                                 public Azure::Core::_internal::Amqp::ConnectionEvents,
                                 public Azure::Core::_internal::Amqp::SessionEvents {
  Azure::Core::_internal::Amqp::Common::AsyncOperationQueue<
      std::shared_ptr<Azure::Core::_internal::Amqp::Connection>>
      m_listeningQueue;
  Common::AsyncOperationQueue<std::unique_ptr<Azure::Core::_internal::Amqp::Session>>
      m_listeningSessionQueue;
  Common::AsyncOperationQueue<std::unique_ptr<Link>> m_receiveLinkQueue;
  std::shared_ptr<Azure::Core::_internal::Amqp::Connection> m_connection;

  virtual void OnSocketAccepted(XIO_INSTANCE_TAG* xio) override
  {
    GTEST_LOG_(INFO) << "OnSocketAccepted - Socket connection received.";
    std::shared_ptr<Azure::Core::_internal::Amqp::Network::Transport> amqpTransport{
        std::make_shared<Azure::Core::_internal::Amqp::Network::AmqpHeaderTransport>(xio)};
    Azure::Core::_internal::Amqp::ConnectionOptions options;
    options.ContainerId = "connectionId";
    options.EnableTrace = true;
    options.Transport = amqpTransport;
    m_connection
        = std::make_shared<Azure::Core::_internal::Amqp::Connection>(amqpTransport, this, options);
    m_connection->Listen();
    m_listeningQueue.CompleteOperation(m_connection);
  }

  virtual void OnConnectionStateChanged(
      Azure::Core::_internal::Amqp::Connection const&,
      ConnectionState newState,
      ConnectionState oldState) override
  {
    oldState;
    newState;
  }
  virtual bool OnNewEndpoint(
      Azure::Core::_internal::Amqp::Connection const& connection,
      Azure::Core::_internal::Amqp::Endpoint& endpoint) override
  {
    GTEST_LOG_(INFO) << "OnNewEndpoint - Incoming endpoint created, create session.";
    auto listeningSession
        = std::make_unique<Azure::Core::_internal::Amqp::Session>(connection, endpoint, this);
    listeningSession->SetIncomingWindow(10000);
    listeningSession->Begin();

    m_listeningSessionQueue.CompleteOperation(std::move(listeningSession));

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
      Azure::Core::Amqp::Models::Value properties) override
  {
    GTEST_LOG_(INFO) << "OnLinkAttached - Link attached to session.";
    auto newLink = std::make_unique<Azure::Core::_internal::Amqp::_detail::Link>(
        session,
        newLinkInstance,
        name,
        Azure::Core::_internal::Amqp::_detail::SessionRole::Receiver,
        static_cast<std::string>(source),
        static_cast<std::string>(target));
    //    newLink->SetReceiverSettleMode(Azure::Core::Amqp::ReceiverSettleMode::First);
    m_receiveLinkQueue.CompleteOperation(std::move(newLink));

    return true;
  }
  virtual void OnEndpointFrameReceived(Connection const&, Azure::Core::Amqp::Models::Value, uint32_t, uint8_t*)
      override
  {
  }

public:
  LinkSocketListenerEvents() {}
  std::shared_ptr<Connection> WaitForConnection(
      Azure::Core::_internal::Amqp::Network::SocketListener const& listener)
  {
    auto result = m_listeningQueue.WaitForPolledResult(listener);
    return std::move(std::get<0>(*result));
  }
  std::unique_ptr<Session> WaitForSession()
  {
    auto result = m_listeningSessionQueue.WaitForPolledResult(*m_connection);
    return std::move(std::get<0>(*result));
  }
  std::unique_ptr<Azure::Core::_internal::Amqp::_detail::Link> WaitForLink()
  {
    auto result = m_receiveLinkQueue.WaitForPolledResult(*m_connection);
    return std::move(std::get<0>(*result));
  }
};

TEST_F(TestLinks, LinkAttachDetach)
{
  LinkSocketListenerEvents events;
  // Create a connection
  Connection connection("amqp://localhost:5672", &events, {});
  Session session(connection, nullptr);

  Network::SocketListener listener(5672, &events);
  listener.Start();
  {
    Link link(session, "MySession", SessionRole::Sender, "MySource", "MyTarget");
    link.Attach(nullptr);

    Azure::Core::Amqp::Models::Value data;
    link.Detach(false, {}, {}, data);

    //    auto listeningConnection = listener.WaitForConnection();
    //    auto listeningSession = listeningConnection->WaitForSession();
    //    auto listeningLink = listeningSession->WaitForLink();
  }
  listener.Stop();
}