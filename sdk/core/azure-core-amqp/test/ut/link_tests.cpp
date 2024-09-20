// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/common/async_operation_queue.hpp"
#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/models/performatives/amqp_transfer.hpp"
#include "azure/core/amqp/internal/network/amqp_header_detect_transport.hpp"
#include "azure/core/amqp/internal/network/socket_listener.hpp"
#include "azure/core/amqp/internal/session.hpp"
#include "mock_amqp_server.hpp"

#include <memory>

#include <gtest/gtest.h>

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  extern uint16_t FindAvailableSocket();
  class TestLinks : public testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
  };

  using namespace Azure::Core::Amqp;
  using namespace Azure::Core::Amqp::_internal;
  using namespace Azure::Core::Amqp::_detail;

#if !defined(AZ_PLATFORM_MAC)
  TEST_F(TestLinks, SimpleLink)
  {

    // Create a connection

    Connection connection("localhost", nullptr, {});

    // Create a session
    Session session{connection.CreateSession()};

    {
      Link link(session, "MySession", SessionRole::Sender, "MySource", "MyTarget");
    }

    {
      // Create two sessions
      Link link1(session, "MySession", SessionRole::Sender, "Source1", "Target1");
      Link link2(session, "MySession", SessionRole::Sender, "Source2", "Target2");
    }
#if ENABLE_UAMQP
    GTEST_LOG_(INFO) << LinkState::Error << LinkState::Invalid << static_cast<LinkState>(92)
                     << LinkState::HalfAttachedAttachReceived;
#endif
  }

  TEST_F(TestLinks, LinkProperties)
  { // Create a connection
    Connection connection("localhost", nullptr, {});
    Session session{connection.CreateSession()};

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

      EXPECT_ANY_THROW(link.SetReceiverSettleMode(static_cast<ReceiverSettleMode>(023)));

      link.SetSenderSettleMode(SenderSettleMode::Settled);
      EXPECT_EQ(SenderSettleMode::Settled, link.GetSenderSettleMode());
      link.SetSenderSettleMode(SenderSettleMode::Unsettled);
      EXPECT_EQ(SenderSettleMode::Unsettled, link.GetSenderSettleMode());
      link.SetSenderSettleMode(SenderSettleMode::Mixed);
      EXPECT_EQ(SenderSettleMode::Mixed, link.GetSenderSettleMode());
      EXPECT_ANY_THROW(link.SetSenderSettleMode(static_cast<SenderSettleMode>(023)));

      link.SetMaxLinkCredit(95);

      link.SetAttachProperties("Attach Properties");

      link.SetDesiredCapabilities("DesiredCapabilities");
      auto val = link.GetDesiredCapabilities();

      EXPECT_ANY_THROW(link.ResetLinkCredit(92, true));
    }

    {
      Link link(session, "MySession", SessionRole::Receiver, "MySource", "MyTarget");
      Link link2(link);

      EXPECT_EQ(link.GetInitialDeliveryCount(), link2.GetInitialDeliveryCount());

      // If I set the initial delivery count on one link, it should affect all the copies of
      // that link.
      link.SetInitialDeliveryCount(32767);
      EXPECT_EQ(link.GetInitialDeliveryCount(), link2.GetInitialDeliveryCount());

      EXPECT_EQ(Azure::Core::Amqp::Models::AmqpValue{"MySource"}, link.GetSource().GetAddress());
      EXPECT_EQ(Azure::Core::Amqp::Models::AmqpValue{"MyTarget"}, link.GetTarget().GetAddress());

      EXPECT_ANY_THROW(link.ResetLinkCredit(92, true));
    }
  }
#if ENABLE_UAMQP
  class LinkSocketListenerEvents
      : public Azure::Core::Amqp::Network::_detail::SocketListenerEvents,
        public Azure::Core::Amqp::_internal::ConnectionEvents,
        public Azure ::Core ::Amqp ::_internal ::ConnectionEndpointEvents,
        public Azure::Core::Amqp::_internal::SessionEvents {
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> m_listeningQueue;
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> m_listeningSessionQueue;
    Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> m_receiveLinkQueue;
    std::shared_ptr<Azure::Core::Amqp::_detail::Link> m_link;
    std::unique_ptr<Azure::Core::Amqp::_internal::Session> m_session;
    std::shared_ptr<Azure::Core::Amqp::_internal::Connection> m_connection;

    virtual void OnSocketAccepted(
        std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> transport) override
    {
      GTEST_LOG_(INFO) << "OnSocketAccepted - Socket connection received.";
      auto amqpTransport{
          Azure::Core::Amqp::Network::_internal::AmqpHeaderDetectTransportFactory::Create(
              transport, nullptr)};
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      options.ContainerId = "connectionId";
      options.EnableTrace = true;
      m_connection = std::make_shared<Azure::Core::Amqp::_internal::Connection>(
          amqpTransport, options, this, this);
      m_connection->Listen();
      m_listeningQueue.CompleteOperation(true);
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
      Azure::Core::Amqp::_internal::SessionOptions sessionOptions;
      sessionOptions.InitialIncomingWindowSize = 10000;
      auto listeningSession = std::make_unique<Azure::Core::Amqp::_internal::Session>(
          connection.CreateSession(endpoint, sessionOptions, this));
      listeningSession->Begin();
      m_session = std::move(listeningSession);

      m_listeningSessionQueue.CompleteOperation(true);

      return true;
    }
    virtual void OnIOError(Azure::Core::Amqp::_internal::Connection const&) override {}
    // Inherited via Session
    virtual bool OnLinkAttached(
        Azure::Core::Amqp::_internal::Session const& session,
        Azure::Core::Amqp::_internal::LinkEndpoint& newLinkInstance,
        std::string const& name,
        Azure::Core::Amqp::_internal::SessionRole sessionRole,
        Azure::Core::Amqp::Models::AmqpValue const& source,
        Azure::Core::Amqp::Models::AmqpValue const& target,
        Azure::Core::Amqp::Models::AmqpValue const&) override
    {
      GTEST_LOG_(INFO) << "OnLinkAttached - Link attached to session.";
      std::unique_ptr<Azure::Core::Amqp::_detail::Link> newLink;
      if (sessionRole == SessionRole::Sender)
      {
        newLink = std::make_unique<Azure::Core::Amqp::_detail::Link>(
            session,
            newLinkInstance,
            name,
            Azure::Core::Amqp::_internal::SessionRole::Receiver,
            source,
            target);
      }
      else
      {
        newLink = std::make_unique<Azure::Core::Amqp::_detail::Link>(
            session,
            newLinkInstance,
            name,
            Azure::Core::Amqp::_internal::SessionRole::Sender,
            source,
            target);
      }
      m_link = std::move(newLink);
      m_receiveLinkQueue.CompleteOperation(true);

      return true;
    }

  public:
    LinkSocketListenerEvents() {}
    bool WaitForConnection(
        Azure::Core::Amqp::Network::_detail::SocketListener const& listener,
        Azure::Core::Context const& context)
    {
      auto result = m_listeningQueue.WaitForPolledResult(context, listener);
      return std::get<0>(*result);
    }
    bool WaitForSession(Azure::Core::Context const& context)
    {
      auto result = m_listeningSessionQueue.WaitForResult(context);
      return std::get<0>(*result);
    }
    bool WaitForLink(Azure::Core::Context const& context)
    {
      auto result = m_receiveLinkQueue.WaitForResult(context);
      return std::get<0>(*result);
    }

    void Cleanup()
    {
      if (m_link)
      {
        m_link.reset();
      }
      if (m_session)
      {
        m_session->End();
        m_session.reset();
      }
      if (m_connection)
      {
        m_connection->Close();
        m_connection.reset();
      }
    }
  };
#endif

  TEST_F(TestLinks, LinkAttachDetach)
  {
#if ENABLE_UAMQP
    LinkSocketListenerEvents events;

    uint16_t testPort = FindAvailableSocket();
    GTEST_LOG_(INFO) << "Test port: " << testPort;
    // Create a connection
    ConnectionOptions connectionOptions;
    connectionOptions.Port = testPort;
    Connection connection("localhost", nullptr, connectionOptions, &events);
    Session session{connection.CreateSession()};

    Network::_detail::SocketListener listener(testPort, &events);

    EXPECT_NO_THROW(listener.Start());
    {
      Link link(session, "MySession", SessionRole::Sender, "MySource", "MyTarget");
      link.Attach();

      EXPECT_TRUE(events.WaitForConnection(listener, {}));
      EXPECT_TRUE(events.WaitForSession({}));

      EXPECT_TRUE(events.WaitForLink({}));
      link.Detach(false, {}, {}, {});
    }
    events.Cleanup();
    listener.Stop();
#else
    EXPECT_TRUE(false);
#endif
  }

  TEST_F(TestLinks, LinkAttachDetachMultipleOneSession)
  {
#if ENABLE_UAMQP
    class MySessionListener final : public MessageTests::MockServiceEndpoint {
    public:
      MySessionListener(MessageTests::MockServiceEndpointOptions const& options)
          : MockServiceEndpoint("MyTarget", options)
      {
      }
      void MessageReceived(
          std::string const& linkName,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO) << "Message received on link " << linkName << ": " << *message;
      }
    };

    MessageTests::AmqpServerMock server;
    auto sessionListener{
        std::make_shared<MySessionListener>(MessageTests::MockServiceEndpointOptions{})};
    server.AddServiceEndpoint(sessionListener);
    server.EnableTrace(false);

    // Create a connection
    ConnectionOptions connectionOptions;
    connectionOptions.Port = server.GetPort();
    connectionOptions.EnableTrace = true;
    Connection connection("localhost", nullptr, connectionOptions);
    server.StartListening();

    Session session{connection.CreateSession()};

    class ClientLinkEvents : public Azure::Core::Amqp::_detail::LinkEvents {
    public:
      LinkState WaitForLink(Azure::Core::Context const& context)
      {
        auto result = m_linkStateQueue.WaitForResult(context);
        if (!result)
        {
          throw Azure::Core::OperationCancelledException("Canceled link wait.");
        }
        return std::get<0>(*result);
      }

      void WaitForLinkState(LinkState state, Azure::Core::Context const& context)
      {
        LinkState result;
        do
        {
          result = WaitForLink(context);
          GTEST_LOG_(INFO) << "Link state changed to: " << result;
        } while (result != state);
      }

    private:
      void OnLinkFlowOn(Link const& link) override
      {
        GTEST_LOG_(INFO) << "Link Flow On on link " << link.GetName();
      }
      Models::AmqpValue OnTransferReceived(
          Link const& link,
          Azure::Core::Amqp::Models::_internal::Performatives::AmqpTransfer transfer,
          uint32_t payloadSize,
          const unsigned char*) override
      {
        GTEST_LOG_(INFO) << "OnTransferReceived(" << link.GetName() << "). Transfer : " << transfer
                         << "Payload size: " << payloadSize;
        return Azure::Core::Amqp::Models::AmqpValue{};
      }
      void OnLinkStateChanged(
          Azure::Core::Amqp::_detail::Link const& link,
          LinkState newLinkState,
          LinkState previousLinkState) override
      {
        GTEST_LOG_(INFO) << "Link " << link.GetName()
                         << ", State Changed. OldState: " << previousLinkState
                         << " NewState: " << newLinkState;
        m_linkStateQueue.CompleteOperation(newLinkState);
      }

      Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<LinkState> m_linkStateQueue;
    };
    Azure::Core::Context timeoutContext
        = Azure::Core::Context{Azure::DateTime::clock::now() + std::chrono::seconds(60)};
    Link keepAliveLink{
        session, "KeepConnectionAlive", SessionRole::Receiver, "MyTarget", "TestReceiver"};
    keepAliveLink.Attach();

    {
      ClientLinkEvents linkEvents;
      Link link(session, "MySession", SessionRole::Sender, "MySource", "MyTarget", &linkEvents);
      link.Attach();

      // Iterate until the state changes to Attached.
      linkEvents.WaitForLinkState(LinkState::Attached, timeoutContext);

      Models::AmqpMessage message;
      message.SetBody("Hello");

      link.Transfer(Models::AmqpMessage::Serialize(message), timeoutContext);

      Azure::Core::Amqp::Models::AmqpValue data;

      link.Detach(true, {}, {}, data);
      // Iterate until the state changes to Detached.
      linkEvents.WaitForLinkState(LinkState::Detached, timeoutContext);
    }

    {
      ClientLinkEvents linkEvents;
      Link link(session, "MySession2", SessionRole::Sender, "MySource", "MyTarget", &linkEvents);

      link.Attach();

      // Iterate until the state changes to Attached.
      linkEvents.WaitForLinkState(LinkState::Attached, timeoutContext);

      Azure::Core::Amqp::Models::AmqpValue data;
      link.Detach(true, {}, {}, data);

      // Iterate until the state changes to Detached.
      linkEvents.WaitForLinkState(LinkState::Detached, timeoutContext);
    }

    {
      constexpr const size_t linkCount = 20;

      std::vector<Link> links;
      std::vector<std::unique_ptr<ClientLinkEvents>> linkEvents;
      for (size_t i = 0; i < linkCount; i += 1)
      {
        // Create linkCount links on the session.
        linkEvents.push_back(std::make_unique<ClientLinkEvents>());
        links.push_back(Link{
            session,
            "MySession " + std::to_string(i),
            SessionRole::Sender,
            "MySource",
            "MyTarget",
            linkEvents.back().get()});
      }
      for (size_t i = 0; i < linkCount; i += 1)
      {
        links[i].Attach();
        // Iterate until the state changes to Attached.
        linkEvents[i]->WaitForLinkState(LinkState::Attached, timeoutContext);
      }

      for (size_t i = 0; i < linkCount; i += 1)
      {
        links[i].Detach(true, "", "", Models::AmqpValue{});
        // Iterate until the state changes to Detached.
        linkEvents[i]->WaitForLinkState(LinkState::Detached, timeoutContext);
      }
    }

    keepAliveLink.Detach(true, "", "", {});

    server.StopListening();
#else
    EXPECT_TRUE(false);
#endif
  }
#endif // defined(AZ_PLATFORM_MAC)
}}}} // namespace Azure::Core::Amqp::Tests
