// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/common/async_operation_queue.hpp"
#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/message_receiver.hpp"
#include "azure/core/amqp/internal/message_sender.hpp"
#include "azure/core/amqp/internal/models/message_source.hpp"
#include "azure/core/amqp/internal/models/message_target.hpp"
#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/internal/network/amqp_header_detect_transport.hpp"
#include "azure/core/amqp/internal/network/socket_listener.hpp"
#include "azure/core/amqp/internal/session.hpp"

#if ENABLE_UAMQP
#undef USE_NATIVE_BROKER
#elif ENABLE_RUST_AMQP
#define USE_NATIVE_BROKER
#endif

#if !defined(USE_NATIVE_BROKER)
#include "mock_amqp_server.hpp"
#endif

#include <azure/core/internal/environment.hpp>
#include <azure/core/platform.hpp>
#include <azure/core/url.hpp>

#include <chrono>
#include <exception>
#include <functional>
#include <future>
#include <random>
#include <thread>

#include <gtest/gtest.h>

// cspell: ignore abcdabcd

using namespace Azure::Core::Amqp::_internal;

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  extern uint16_t FindAvailableSocket();

  class TestMessageSendReceive : public testing::Test {
  protected:
    void SetUp() override
    {
#if defined(USE_NATIVE_BROKER)
      auto testBrokerUrl = Azure::Core::_internal::Environment::GetVariable("TEST_BROKER_ADDRESS");
      if (testBrokerUrl.empty())
      {
        GTEST_FATAL_FAILURE_("Could not find required environment variable TEST_BROKER_ADDRESS");
      }
      Azure::Core::Url brokerUrl(testBrokerUrl);
      m_brokerEndpoint = brokerUrl;
#else
      m_brokerEndpoint
          = Azure::Core::Url("amqp://localhost:" + std::to_string(m_mockServer.GetPort()));
#endif
    }
    void TearDown() override
    { // When the test is torn down, the global state MUST be idle. If it is not,
      // something leaked.
      Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }

    std::string GetBrokerEndpoint() { return m_brokerEndpoint.GetAbsoluteUrl(); }

    std::uint16_t GetPort() { return m_brokerEndpoint.GetPort(); }

    auto CreateAmqpConnection(
        std::string const& containerId
        = testing::UnitTest::GetInstance()->current_test_info()->name(),
        bool enableTracing = false,
        Azure::Core::Context const& context = {})
    {
      ConnectionOptions options;
      options.ContainerId = containerId;
      options.EnableTrace = enableTracing;
      options.Port = GetPort();

      auto connection = Connection("localhost", nullptr, options);
#if ENABLE_RUST_AMQP
      connection.Open(context);
#endif
      return connection;
      (void)context;
    }
    auto CreateAmqpSession(Connection const& connection, Context const& context = {})
    {
      auto session = connection.CreateSession();
#if ENABLE_RUST_AMQP
      session.Begin(context);
#endif
      return session;
      (void)context;
    }

    void CloseAmqpConnection(Connection& connection, Azure::Core::Context const& context = {})
    {
#if ENABLE_RUST_AMQP
      connection.Close(context);
#endif
      (void)connection;
      (void)context;
    }
    void EndAmqpSession(Session& session, Azure::Core::Context const& context = {})
    {
#if ENABLE_RUST_AMQP
      session.End(context);
#endif
      (void)session;
      (void)context;
    }

    void StartServerListening()
    {
#if !defined(USE_NATIVE_BROKER)
      m_mockServer.StartListening();
#endif
    }

    void StopServerListening()
    {
#if !defined(USE_NATIVE_BROKER)
      m_mockServer.StopListening();
#endif
    }

#if !defined(USE_NATIVE_BROKER)
  protected:
    MessageTests::AmqpServerMock m_mockServer;
#endif
  private:
    Azure::Core::Url m_brokerEndpoint{};
  };

  using namespace Azure::Core::Amqp::_internal;
  using namespace Azure::Core::Amqp;

#if !defined(AZ_PLATFORM_MAC)
  TEST_F(TestMessageSendReceive, SimpleReceiver)
  {

    // Create a connection
    auto connection{CreateAmqpConnection({})};
    // Create a session.
    auto session{CreateAmqpSession(connection, {})};

    {
      MessageReceiver receiver(session.CreateMessageReceiver("MySource", {}));
    }
    {
      MessageReceiver receiver1(session.CreateMessageReceiver("MySource", {}));
      MessageReceiver receiver2(session.CreateMessageReceiver("MySource", {}));
    }

    GTEST_LOG_(INFO) << _internal::MessageReceiverState::Invalid
                     << _internal::MessageReceiverState::Closing
                     << _internal::MessageReceiverState::Idle
                     << _internal::MessageReceiverState::Opening
                     << _internal::MessageReceiverState::Open
                     << _internal::MessageReceiverState::Error;
    GTEST_LOG_(INFO) << static_cast<_internal::MessageReceiverState>(5993);
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }
  TEST_F(TestMessageSendReceive, ReceiverProperties)
  {
    // Create a connection
    auto connection{CreateAmqpConnection({})};
    // Create a session.
    auto session{CreateAmqpSession(connection, {})};

    {
      MessageReceiverOptions options;
      options.EnableTrace = true;
      MessageReceiver receiver(session.CreateMessageReceiver("MyTarget", options));
#if ENABLE_UAMQP
      EXPECT_ANY_THROW(receiver.GetLinkName());
#endif
    }

#if ENABLE_UAMQP
    {
      auto accepted{Models::_internal::Messaging::DeliveryAccepted()};
      auto released{Models::_internal::Messaging::DeliveryReleased()};
      auto rejected{Models::_internal::Messaging::DeliveryRejected("error", "description", {})};
      auto modified{Models::_internal::Messaging::DeliveryModified(true, false, "Annotations")};
      auto received{Models::_internal::Messaging::DeliveryReceived(3, 24)};
    }
#endif
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }

  TEST_F(TestMessageSendReceive, SimpleSender)
  {
    // Create a connection
    auto connection{CreateAmqpConnection({})};
    // Create a session.
    auto session{CreateAmqpSession(connection, {})};

    {
      MessageSender sender(session.CreateMessageSender("MySource", {}));
    }
    {
      MessageSender sender1(session.CreateMessageSender("MySource", {}));
      MessageSender sender2(session.CreateMessageSender("MySource", {}));
    }
#if ENABLE_UAMQP
    GTEST_LOG_(INFO) << _internal::MessageSenderState::Invalid
                     << _internal::MessageSenderState::Closing
                     << _internal::MessageSenderState::Idle
                     << _internal::MessageSenderState::Opening
                     << _internal::MessageSenderState::Open << _internal::MessageSenderState::Error;
    GTEST_LOG_(INFO) << static_cast<_internal::MessageSenderState>(5993);
#endif
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }
  TEST_F(TestMessageSendReceive, SenderProperties)
  {
    // Create a connection
    auto connection{CreateAmqpConnection(
        testing::UnitTest::GetInstance()->current_test_info()->name(), false, {})};
    // Create a session.
    auto session{CreateAmqpSession(connection, {})};

    {
      MessageSenderOptions options;
      options.EnableTrace = true;
      MessageSender sender(session.CreateMessageSender("MySource", options));
    }
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }

  TEST_F(TestMessageSendReceive, ReceiverOpenClose)
  {
    // Create a connection
    auto connection{
        CreateAmqpConnection(testing::UnitTest::GetInstance()->current_test_info()->name(), true)};
    // Create a session.
    auto session{CreateAmqpSession(connection, {})};

    Azure::Core::Context context;

    StartServerListening();
    {
#if ENABLE_UAMQP
      class ReceiverEvents : public MessageReceiverEvents {
        virtual void OnMessageReceiverStateChanged(
            MessageReceiver const& receiver,
            MessageReceiverState newState,
            MessageReceiverState oldState) override
        {
          GTEST_LOG_(INFO) << "MessageReceiverEvents::OnMessageReceiverStateChanged: " << newState
                           << "->" << oldState;
          (void)receiver;
          (void)newState;
          (void)oldState;
        }

        Models::AmqpValue OnMessageReceived(
            Azure::Core::Amqp::_internal::MessageReceiver const&,
            std::shared_ptr<Models::AmqpMessage> const&) override
        {
          return Models::AmqpValue();
        }
        void OnMessageReceiverDisconnected(
            Azure::Core::Amqp::_internal::MessageReceiver const&,
            Azure::Core::Amqp::Models::_internal::AmqpError const& error) override
        {
          GTEST_LOG_(INFO) << "Message receiver disconnected: " << error;
        }
      };
      ReceiverEvents receiverEvents;
#endif
      MessageReceiverOptions options;
      options.Name = "Test Receiver";
      MessageReceiver receiver(session.CreateMessageReceiver(
          "MyTarget",
          options
#if ENABLE_UAMQP
          ,
          &receiverEvents
#endif
          ));

      EXPECT_NO_THROW(receiver.Open());
#if ENABLE_UAMQP
      EXPECT_EQ("Test Receiver", receiver.GetLinkName());
#endif

      receiver.Close();
    }

    StopServerListening();
    EndAmqpSession(session);
    CloseAmqpConnection(connection);

    context.Cancel();
  }

  TEST_F(TestMessageSendReceive, SenderOpenClose)
  {
    // Create a connection
    //      connectionOptions.IdleTimeout = std::chrono::minutes(5);
    auto connection{
        CreateAmqpConnection(testing::UnitTest::GetInstance()->current_test_info()->name(), true)};
    // Create a session.
    auto session{CreateAmqpSession(connection, {})};

#if !defined(USE_NATIVE_BROKER)
    class SenderLinkEndpoint : public MessageTests::MockServiceEndpoint {
    public:
      SenderLinkEndpoint(
          std::string const& name,
          MessageTests::MockServiceEndpointOptions const& options)
          : MockServiceEndpoint(name, options)
      {
      }
      virtual ~SenderLinkEndpoint() = default;

    private:
      void MessageReceived(
          std::string const& linkName,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO) << "Message received on link " << linkName << ": " << *message;
      }
    };

    MessageTests::MockServiceEndpointOptions mockServiceEndpointOptions{};
    mockServiceEndpointOptions.EnableTrace = true;
    auto senderEndpoint
        = std::make_shared<SenderLinkEndpoint>("MyTarget", mockServiceEndpointOptions);
    m_mockServer.AddServiceEndpoint(senderEndpoint);
#endif

    StartServerListening();

    {
      MessageSenderOptions options;
      options.MessageSource = "MySource";

      MessageSender sender(session.CreateMessageSender("MyTarget", options));
      EXPECT_FALSE(sender.Open());
      GTEST_LOG_(INFO) << "Close message sender.";
      EXPECT_NO_THROW(sender.Close());
      GTEST_LOG_(INFO) << "Close message sender complete";
    }
    StopServerListening();
    EXPECT_NO_THROW(EndAmqpSession(session));
    EXPECT_NO_THROW(CloseAmqpConnection(connection));
  }

  // A close that fails must leave the message receiver closed. Before the correction, the receiver
  // kept its open flag, and the destructor of the receiver stopped the process.
  // See https://github.com/Azure/azure-sdk-for-cpp/issues/7323.
  TEST_F(TestMessageSendReceive, ReceiverCloseWithCancelledContext)
  {
    auto connection{
        CreateAmqpConnection(testing::UnitTest::GetInstance()->current_test_info()->name(), true)};
    auto session{CreateAmqpSession(connection, {})};

#if !defined(USE_NATIVE_BROKER)
    // The mock server must attach the link. Without a service endpoint, the peer never sends the
    // attach, the receiver stays below the Open state, and the close returns without a look at the
    // context.
    class ReceiverLinkEndpoint : public MessageTests::MockServiceEndpoint {
    public:
      ReceiverLinkEndpoint(
          std::string const& name,
          MessageTests::MockServiceEndpointOptions const& options)
          : MockServiceEndpoint(name, options)
      {
      }
      virtual ~ReceiverLinkEndpoint() = default;

    private:
      void MessageReceived(
          std::string const& linkName,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO) << "Message received on link " << linkName << ": " << *message;
      }
    };

    MessageTests::MockServiceEndpointOptions mockServiceEndpointOptions{};
    mockServiceEndpointOptions.EnableTrace = true;
    auto receiverEndpoint
        = std::make_shared<ReceiverLinkEndpoint>("MyTarget", mockServiceEndpointOptions);
    m_mockServer.AddServiceEndpoint(receiverEndpoint);
#endif

    StartServerListening();
    {
#if ENABLE_UAMQP
      class ReceiverEvents : public MessageReceiverEvents {
      public:
        Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<MessageReceiverState>
            StateChangeQueue;

      private:
        void OnMessageReceiverStateChanged(
            MessageReceiver const& receiver,
            MessageReceiverState newState,
            MessageReceiverState oldState) override
        {
          GTEST_LOG_(INFO) << "Message receiver state changed: " << oldState << " -> " << newState;
          (void)receiver;
          (void)oldState;
          StateChangeQueue.CompleteOperation(newState);
        }

        Models::AmqpValue OnMessageReceived(
            MessageReceiver const&,
            std::shared_ptr<Models::AmqpMessage> const&) override
        {
          return Models::AmqpValue();
        }

        void OnMessageReceiverDisconnected(
            MessageReceiver const&,
            Models::_internal::AmqpError const& error) override
        {
          GTEST_LOG_(INFO) << "Message receiver disconnected: " << error;
        }
      };
      ReceiverEvents receiverEvents;
#endif

      MessageReceiverOptions options;
      options.Name = "Test Receiver";
      // The mock server builds its message sender from this target, so the target needs an
      // address.
      options.MessageTarget = "MyReceiverTarget";
      MessageReceiver receiver(session.CreateMessageReceiver(
          "MyTarget",
          options
#if ENABLE_UAMQP
          ,
          &receiverEvents
#endif
          ));

      EXPECT_NO_THROW(receiver.Open());

#if ENABLE_UAMQP
      // Wait until the receiver reaches the Open state. The close waits on the context only when
      // the receiver is in the Open state or in the Closing state.
      Azure::Core::Context waitContext{Azure::DateTime::clock::now() + std::chrono::seconds(30)};
      MessageReceiverState currentState{MessageReceiverState::Idle};
      while (currentState != MessageReceiverState::Open)
      {
        auto stateChange = receiverEvents.StateChangeQueue.WaitForResult(waitContext);
        ASSERT_TRUE(stateChange) << "The message receiver did not reach the Open state.";
        currentState = std::get<0>(*stateChange);
      }
#endif

      Azure::Core::Context cancelledContext;
      cancelledContext.Cancel();

#if ENABLE_UAMQP
      // The uAMQP close waits for the detach on the context. The cancelled context makes that wait
      // fail, so the close throws. The receiver must still go to the closed state, which the
      // destructor below tests.
      EXPECT_THROW(receiver.Close(cancelledContext), Azure::Core::OperationCancelledException);
#else
      // The Rust transport does not wait on the context during the close, so the close succeeds.
      EXPECT_NO_THROW(receiver.Close(cancelledContext));
#endif

      // The message receiver is destroyed here. The destructor must not stop the process.
    }

    StopServerListening();
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }

  // A close that fails must leave the message sender closed. Before the correction, the sender kept
  // its open flag, and the destructor of the sender stopped the process. The sender also kept the
  // async operation on the connection, and the destructor of the connection stopped the process.
  // See https://github.com/Azure/azure-sdk-for-cpp/issues/7323.
  TEST_F(TestMessageSendReceive, SenderCloseWithCancelledContext)
  {
    auto connection{
        CreateAmqpConnection(testing::UnitTest::GetInstance()->current_test_info()->name(), true)};
    auto session{CreateAmqpSession(connection, {})};

#if !defined(USE_NATIVE_BROKER)
    class SenderLinkEndpoint : public MessageTests::MockServiceEndpoint {
    public:
      SenderLinkEndpoint(
          std::string const& name,
          MessageTests::MockServiceEndpointOptions const& options)
          : MockServiceEndpoint(name, options)
      {
      }
      virtual ~SenderLinkEndpoint() = default;

    private:
      void MessageReceived(
          std::string const& linkName,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO) << "Message received on link " << linkName << ": " << *message;
      }
    };

    MessageTests::MockServiceEndpointOptions mockServiceEndpointOptions{};
    mockServiceEndpointOptions.EnableTrace = true;
    auto senderEndpoint
        = std::make_shared<SenderLinkEndpoint>("MyTarget", mockServiceEndpointOptions);
    m_mockServer.AddServiceEndpoint(senderEndpoint);
#endif

    StartServerListening();
    {
      MessageSenderOptions options;
      options.MessageSource = "MySource";

      MessageSender sender(session.CreateMessageSender("MyTarget", options));
      EXPECT_FALSE(sender.Open());

      Azure::Core::Context cancelledContext;
      cancelledContext.Cancel();

#if ENABLE_UAMQP
      // The uAMQP close waits for the detach on the context. The cancelled context makes that wait
      // fail, so the close throws. The sender must still go to the closed state, which the
      // destructor below tests.
      EXPECT_THROW(sender.Close(cancelledContext), Azure::Core::OperationCancelledException);
#else
      // The Rust transport does not wait on the context during the close, so the close succeeds.
      EXPECT_NO_THROW(sender.Close(cancelledContext));
#endif

      // The message sender is destroyed here. The destructor must not stop the process.
    }

    StopServerListening();
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }

#if ENABLE_UAMQP
#if !defined(USE_NATIVE_BROKER)
  TEST_F(TestMessageSendReceive, TestLocalhostVsTls)
  {
    MessageTests::AmqpServerMock mockServer(5671);

    mockServer.StartListening();

    ConnectionOptions connectionOptions;
    connectionOptions.ContainerId = ::testing::UnitTest::GetInstance()->current_test_info()->name();
    //  connectionOptions.EnableTrace = true;
    connectionOptions.Port = mockServer.GetPort();
    Connection connection("localhost", nullptr, connectionOptions);
    Session session{connection.CreateSession()};

    {
      class SenderEvents : public MessageSenderEvents {
        virtual void OnMessageSenderStateChanged(
            MessageSender const& sender,
            MessageSenderState newState,
            MessageSenderState oldState) override
        {
          GTEST_LOG_(INFO) << "MessageSenderEvents::OnMessageSenderStateChanged. OldState: "
                           << oldState << " NewState: " << newState;
          (void)sender;
        }
        virtual void OnMessageSenderDisconnected(
            MessageSender const&,
            Models::_internal::AmqpError const& error) override
        {
          GTEST_LOG_(INFO) << "MessageSenderEvents::OnMessageSenderDisconnected. Error: " << error;
        };
      };

      SenderEvents senderEvents;
      MessageSenderOptions options;
      options.Name = "sender-link";
      options.MessageSource = "ingress";
      options.SettleMode = SenderSettleMode::Settled;
      options.MaxMessageSize = 65536;
      MessageSender sender(
          session.CreateMessageSender("localhost/ingress", options, &senderEvents));

      // Opening the message sender should fail because we couldn't connect.
      EXPECT_TRUE(sender.Open());
    }
    mockServer.StopListening();
  }
#endif // !defined(USE_NATIVE_BROKER)
#endif // ENABLE_UAMQP

  TEST_F(TestMessageSendReceive, SenderSendAsync)
  {
    // Create a connection
    //      connectionOptions.IdleTimeout = std::chrono::minutes(5);
    auto connection{
        CreateAmqpConnection(testing::UnitTest::GetInstance()->current_test_info()->name(), true)};
    // Create a session.
    auto session{CreateAmqpSession(connection, {})};

#if !defined(USE_NATIVE_BROKER)
    class SenderLinkEndpoint : public MessageTests::MockServiceEndpoint {
    public:
      SenderLinkEndpoint(
          std::string const& name,
          MessageTests::MockServiceEndpointOptions const& options)
          : MockServiceEndpoint(name, options)
      {
      }
      virtual ~SenderLinkEndpoint() = default;

    private:
      void MessageReceived(
          std::string const& linkName,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO) << "Message received on link " << linkName << ": " << *message;
      }
    };

    MessageTests::MockServiceEndpointOptions mockServiceEndpointOptions{};
    mockServiceEndpointOptions.EnableTrace = true;
    auto senderEndpoint
        = std::make_shared<SenderLinkEndpoint>("localhost/ingress", mockServiceEndpointOptions);
    m_mockServer.AddServiceEndpoint(senderEndpoint);

    GTEST_LOG_(INFO) << "Test port: " << GetPort();
#endif

    // Set up a 30 second deadline on the receiver.
    Azure::Core::Context receiveContext
        = Azure::Core::Context{Azure::DateTime::clock::now() + std::chrono::seconds(15)};

    // Ensure that the thread is started before we start using the message sender.
    StartServerListening();

    {
#if ENABLE_UAMQP
      class SenderEvents : public MessageSenderEvents {
        virtual void OnMessageSenderStateChanged(
            MessageSender const&,
            MessageSenderState newState,
            MessageSenderState oldState) override
        {
          GTEST_LOG_(INFO) << "MessageSenderEvents::OnMessageSenderStateChanged. Old State: "
                           << oldState << " New State: " << newState;
        }
        virtual void OnMessageSenderDisconnected(
            MessageSender const&,
            Models::_internal::AmqpError const& error) override
        {
          GTEST_LOG_(INFO) << "MessageSenderEvents::OnMessageSenderDisconnected. Error: " << error;
        };
      };

      SenderEvents senderEvents;
#endif
      MessageSenderOptions options;
      options.Name = "sender-link";
      options.MessageSource = "ingress";
      options.SettleMode = SenderSettleMode::Settled;
      options.MaxMessageSize = 65536;
      MessageSender sender(session.CreateMessageSender(
          "localhost/ingress",
          options
#if ENABLE_UAMQP
          ,
          &senderEvents
#endif
          ));
      EXPECT_FALSE(sender.Open(receiveContext));

      Azure::Core::Amqp::Models::AmqpMessage message;
      message.SetBody(Azure::Core::Amqp::Models::AmqpBinaryData{'h', 'e', 'l', 'l', 'o'});

      Azure::Core::Context context;
      auto result = sender.Send(message, receiveContext);
#if ENABLE_UAMQP
      EXPECT_EQ(std::get<0>(result), MessageSendStatus::Ok);
#endif

      sender.Close();
    }
    receiveContext.Cancel();
    StopServerListening();

    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }

  TEST_F(TestMessageSendReceive, SenderSendSync)
  {
#if !defined(USE_NATIVE_BROKER)
    class SenderLinkEndpoint final : public MessageTests::MockServiceEndpoint {
    public:
      SenderLinkEndpoint(
          std::string const& name,
          MessageTests::MockServiceEndpointOptions const& options)
          : MockServiceEndpoint(name, options)
      {
      }

      virtual ~SenderLinkEndpoint() = default;

    private:
      void MessageReceived(
          std::string const& linkName,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO) << "Message received on link " << linkName << ": " << *message;
      }
    };

    MessageTests::MockServiceEndpointOptions mockServiceEndpointOptions{};
    mockServiceEndpointOptions.EnableTrace = true;
    auto senderEndpoint
        = std::make_shared<SenderLinkEndpoint>("localhost/ingress", mockServiceEndpointOptions);
    m_mockServer.AddServiceEndpoint(senderEndpoint);
#endif

    auto connection{CreateAmqpConnection({})};
    auto session{CreateAmqpSession(connection)};

    Azure::Core::Context receiveContext;

    // Ensure that the thread is started before we start using the message sender.
    StartServerListening();

    {
      MessageSenderOptions options;
      options.SettleMode = SenderSettleMode::Settled;
      options.MaxMessageSize = 65536;
      options.MessageSource = "ingress";
      options.Name = "sender-link";
      MessageSender sender(session.CreateMessageSender("localhost/ingress", options));
      EXPECT_FALSE(sender.Open());

      Azure::Core::Amqp::Models::AmqpMessage message;
      message.SetBody(Azure::Core::Amqp::Models::AmqpValue{"Hello"});

      auto result = sender.Send(message);
#if ENABLE_UAMQP
      EXPECT_EQ(std::get<0>(result), MessageSendStatus::Ok);
#endif

      sender.Close();
    }
    receiveContext.Cancel();
    StopServerListening();

    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }

#if ENABLE_UAMQP
#if !defined(USE_NATIVE_BROKER)
  TEST_F(TestMessageSendReceive, SenderLateSettlementDoesNotCompleteNextSend)
  {
    // This endpoint settles each message after the send that carried it gave up.
    class SlowSenderLinkEndpoint : public MessageTests::MockServiceEndpoint {
    public:
      SlowSenderLinkEndpoint(
          std::string const& name,
          MessageTests::MockServiceEndpointOptions const& options)
          : MockServiceEndpoint(name, options)
      {
      }

      virtual ~SlowSenderLinkEndpoint() = default;

      Azure::Core::Amqp::Models::AmqpValue OnMessageReceived(
          Azure::Core::Amqp::_internal::MessageReceiver const& receiver,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        std::this_thread::sleep_for(
            m_messagesReceived++ == 0 ? std::chrono::seconds(2) : std::chrono::seconds(5));
        return MockServiceEndpoint::OnMessageReceived(receiver, message);
      }

    private:
      void MessageReceived(
          std::string const& linkName,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO) << "Message received on link " << linkName << ": " << *message;
      }

      int m_messagesReceived{0};
    };

    MessageTests::MockServiceEndpointOptions mockServiceEndpointOptions{};
    mockServiceEndpointOptions.EnableTrace = true;
    auto senderEndpoint
        = std::make_shared<SlowSenderLinkEndpoint>("localhost/ingress", mockServiceEndpointOptions);
    m_mockServer.AddServiceEndpoint(senderEndpoint);

    auto connection{CreateAmqpConnection()};
    auto session{CreateAmqpSession(connection)};

    StartServerListening();

    {
      MessageSenderOptions options;
      options.Name = "sender-link";
      options.MessageSource = "ingress";
      // An unsettled send waits for the disposition of the server.
      options.SettleMode = SenderSettleMode::Unsettled;
      options.MaxMessageSize = 65536;
      MessageSender sender(session.CreateMessageSender("localhost/ingress", options));
      EXPECT_FALSE(sender.Open());

      Azure::Core::Amqp::Models::AmqpMessage message;
      message.SetBody(Azure::Core::Amqp::Models::AmqpValue{"Hello"});

      auto firstResult = sender.Send(
          message,
          Azure::Core::Context{Azure::DateTime::clock::now() + std::chrono::milliseconds(500)});
      EXPECT_EQ(std::get<0>(firstResult), MessageSendStatus::Cancelled);

      // The server settles the first message here, after the first send gave up.
      std::this_thread::sleep_for(std::chrono::seconds(3));

      auto secondSendStart = std::chrono::steady_clock::now();
      auto secondResult = sender.Send(
          message,
          Azure::Core::Context{Azure::DateTime::clock::now() + std::chrono::milliseconds(500)});
      auto secondSendTime = std::chrono::steady_clock::now() - secondSendStart;

      EXPECT_EQ(std::get<0>(secondResult), MessageSendStatus::Cancelled);
      EXPECT_GE(secondSendTime, std::chrono::milliseconds(400));

      // Let the second message settle, so the close finds no operation in flight.
      std::this_thread::sleep_for(std::chrono::seconds(6));

      sender.Close();
    }
    StopServerListening();

    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }
#endif // !defined(USE_NATIVE_BROKER)

#if ENABLE_UAMQP
#if !defined(USE_NATIVE_BROKER)
  TEST_F(TestMessageSendReceive, SenderCloseWhileUnsettledSendIgnoresLateDisposition)
  {
    class DelayedSenderLinkEndpoint final : public MessageTests::MockServiceEndpoint {
    public:
      DelayedSenderLinkEndpoint(
          std::string const& name,
          MessageTests::MockServiceEndpointOptions const& options)
          : MockServiceEndpoint(name, options),
            m_transferReceived{m_transferReceivedPromise.get_future()},
            m_settlementReleased{m_settlementReleasedPromise.get_future()}
      {
      }

      std::future_status WaitForTransfer(std::chrono::milliseconds timeout)
      {
        return m_transferReceived.wait_for(timeout);
      }

      void ReleaseSettlement() { m_settlementReleasedPromise.set_value(); }

      Azure::Core::Amqp::Models::AmqpValue OnMessageReceived(
          Azure::Core::Amqp::_internal::MessageReceiver const& receiver,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        m_transferReceivedPromise.set_value();
        m_settlementReleased.wait();
        return MockServiceEndpoint::OnMessageReceived(receiver, message);
      }

    private:
      void MessageReceived(
          std::string const&,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const&) override
      {
      }

      std::promise<void> m_transferReceivedPromise;
      std::future<void> m_transferReceived;
      std::promise<void> m_settlementReleasedPromise;
      std::future<void> m_settlementReleased;
    };

    MessageTests::MockServiceEndpointOptions mockServiceEndpointOptions{};
    mockServiceEndpointOptions.EnableTrace = true;
    auto senderEndpoint = std::make_shared<DelayedSenderLinkEndpoint>(
        "localhost/ingress", mockServiceEndpointOptions);
    m_mockServer.AddServiceEndpoint(senderEndpoint);

    auto connection{CreateAmqpConnection()};
    auto session{CreateAmqpSession(connection)};

    StartServerListening();

    {
      MessageSenderOptions options;
      options.Name = "sender-link";
      options.MessageSource = "ingress";
      options.SettleMode = SenderSettleMode::Unsettled;
      options.MaxMessageSize = 65536;
      MessageSender sender(session.CreateMessageSender("localhost/ingress", options));
      EXPECT_FALSE(sender.Open());

      Azure::Core::Amqp::Models::AmqpMessage message;
      message.SetBody(Azure::Core::Amqp::Models::AmqpValue{"Hello"});

      std::promise<void> sendFinishedPromise;
      auto sendFinished = sendFinishedPromise.get_future();
      std::exception_ptr sendException;
      std::thread sendWorker([&]() {
        try
        {
          (void)sender.Send(message);
        }
        catch (...)
        {
          sendException = std::current_exception();
        }
        sendFinishedPromise.set_value();
      });

      EXPECT_EQ(
          senderEndpoint->WaitForTransfer(std::chrono::seconds(5)), std::future_status::ready);

      std::promise<void> closeStartedPromise;
      auto closeStarted = closeStartedPromise.get_future();
      std::promise<void> closeFinishedPromise;
      auto closeFinished = closeFinishedPromise.get_future();
      std::exception_ptr closeException;
      std::thread closeWorker([&]() {
        closeStartedPromise.set_value();
        try
        {
          sender.Close(
              Azure::Core::Context{Azure::DateTime::clock::now() + std::chrono::seconds(5)});
        }
        catch (...)
        {
          closeException = std::current_exception();
        }
        closeFinishedPromise.set_value();
      });

      EXPECT_EQ(closeStarted.wait_for(std::chrono::seconds(5)), std::future_status::ready);

      EXPECT_EQ(sendFinished.wait_for(std::chrono::seconds(5)), std::future_status::ready);
      senderEndpoint->ReleaseSettlement();

      EXPECT_EQ(closeFinished.wait_for(std::chrono::seconds(5)), std::future_status::ready);
      sendWorker.join();
      closeWorker.join();
      EXPECT_EQ(sendException, nullptr);
      EXPECT_EQ(closeException, nullptr);
    }
    StopServerListening();

    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }
#endif // !defined(USE_NATIVE_BROKER)
#endif // ENABLE_UAMQP

  TEST_F(TestMessageSendReceive, AuthenticatedSender)
  {
#if !defined(USE_NATIVE_BROKER)
    class ReceiverServiceEndpoint : public MessageTests::MockServiceEndpoint {
    public:
      ReceiverServiceEndpoint(
          std::string const& name,
          MessageTests::MockServiceEndpointOptions const& options)
          : MockServiceEndpoint(name, options)
      {
      }

      virtual ~ReceiverServiceEndpoint() = default;

    private:
      void MessageReceived(
          std::string const& linkName,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO) << "Message received on link " << linkName << ": " << *message;
      }
    };

    MessageTests::MockServiceEndpointOptions mockServiceEndpointOptions{};
    mockServiceEndpointOptions.EnableTrace = false;
    auto serviceEndpoint
        = std::make_shared<ReceiverServiceEndpoint>("testLocation", mockServiceEndpointOptions);

    m_mockServer.AddServiceEndpoint(serviceEndpoint);
#endif // !defined(USE_NATIVE_BROKER)

    auto sasCredential = std::make_shared<ServiceBusSasConnectionStringCredential>(
        "Endpoint=amqp://localhost:" + std::to_string(GetPort())
        + "/;SharedAccessKeyName=MyTestKey;SharedAccessKey=abcdabcd;EntityPath=testLocation");

    auto connection{CreateAmqpConnection()};
    auto session{CreateAmqpSession(connection)};

    StartServerListening();

    MessageSenderOptions senderOptions;
    senderOptions.Name = "sender-link";
    senderOptions.EnableTrace = true;
    senderOptions.MessageSource = "ingress";
    senderOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
    senderOptions.MaxMessageSize = 65536;
    MessageSender sender(
        session.CreateMessageSender(sasCredential->GetEntityPath(), senderOptions, nullptr));

    EXPECT_FALSE(sender.Open());

    Azure::Core::Amqp::Models::AmqpMessage message;
    message.SetBody(Azure::Core::Amqp::Models::AmqpValue{"Hello"});
    EXPECT_EQ(MessageSendStatus::Ok, std::get<0>(sender.Send(message)));

    sender.Close();

    StopServerListening();
  }
#endif

  TEST_F(TestMessageSendReceive, AuthenticatedSenderAzureToken)
  {
    class AzureTokenCredential : public Azure::Core::Credentials::TokenCredential {
      Azure::Core::Credentials::AccessToken GetToken(
          const Azure::Core::Credentials::TokenRequestContext& requestContext,
          const Azure::Core::Context& context) const override
      {
        Azure::Core::Credentials::AccessToken rv;
        rv.Token = "ThisIsAJwt.WithABogusBody.AndSignature";
        rv.ExpiresOn = std::chrono::system_clock::now();
        (void)requestContext;
        (void)context;
        return rv;
      }

    public:
      AzureTokenCredential() : Azure::Core::Credentials::TokenCredential("Testing") {}
    };
#if !defined(USE_NATIVE_BROKER)

    class ReceiverServiceEndpoint : public MessageTests::MockServiceEndpoint {
    public:
      ReceiverServiceEndpoint(
          std::string const& name,
          MessageTests::MockServiceEndpointOptions const& options)
          : MockServiceEndpoint(name, options)
      {
      }
      virtual ~ReceiverServiceEndpoint() = default;

    private:
      void MessageReceived(
          std::string const& linkName,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO) << "Message received on link " << linkName << ": " << *message;
      }
    };
#endif
    auto tokenCredential = std::make_shared<AzureTokenCredential>();
    std::string hostName = "localhost";
    std::string entityPath = "testLocation";

    std::string endpoint
        = "amqp://" + hostName + ":" + std::to_string(GetPort()) + "/" + entityPath;

#if !defined(USE_NATIVE_BROKER)
    MessageTests::MockServiceEndpointOptions mockServiceEndpointOptions{};
    mockServiceEndpointOptions.EnableTrace = false;
    auto serviceEndpoint
        = std::make_shared<ReceiverServiceEndpoint>(endpoint, mockServiceEndpointOptions);
    m_mockServer.AddServiceEndpoint(serviceEndpoint);
#endif

    auto connection{CreateAmqpConnection({})};
    auto session{CreateAmqpSession(connection, {})};

    StartServerListening();
    MessageSenderOptions senderOptions;
    senderOptions.Name = "sender-link";
    senderOptions.MessageSource = "ingress";
    senderOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
    senderOptions.MaxMessageSize = 65536;
    senderOptions.Name = "sender-link";
    MessageSender sender(session.CreateMessageSender(endpoint, senderOptions));
    EXPECT_FALSE(sender.Open());

    Azure::Core::Amqp::Models::AmqpMessage message;
    message.SetBody(Azure::Core::Amqp::Models::AmqpValue{"Hello"});
    auto result = sender.Send(message);
#if ENABLE_UAMQP
    EXPECT_EQ(MessageSendStatus::Ok, std::get<0>(result));
#endif

    sender.Close();

    StopServerListening();
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }

  TEST_F(TestMessageSendReceive, AuthenticatedReceiver)
  {
    std::string brokerEndpoint = GetBrokerEndpoint() + "/testLocation";
    std::string senderEndpoint
        = GetBrokerEndpoint() + "/" + testing::UnitTest::GetInstance()->current_test_case()->name();

#if !defined(USE_NATIVE_BROKER)
    class ReceiverServiceEndpoint : public MessageTests::MockServiceEndpoint {
    public:
      ReceiverServiceEndpoint(
          std::string const& name,
          MessageTests::MockServiceEndpointOptions const& options)
          : MockServiceEndpoint(name, options)
      {
      }
      virtual ~ReceiverServiceEndpoint() = default;

      void ShouldSendMessage(bool shouldSend, std::string messageId)
      {
        m_shouldSendMessage = shouldSend;
        m_messageId = messageId;
      }
      void SetSenderNodeName(std::string const& senderNodeName)
      {
        GTEST_LOG_(INFO) << "Sender Node Name: " << senderNodeName;
        m_senderNodeName = senderNodeName;
      }

    private:
      mutable bool m_shouldSendMessage{false};
      std::string m_senderNodeName;
      std::string m_messageId;

      void Poll() const override
      {
        if (m_shouldSendMessage && HasMessageSender())
        {
          GTEST_LOG_(INFO) << "Sending message to client." + m_senderNodeName;
          Azure::Core::Amqp::Models::AmqpMessage sendMessage;
          if (!m_messageId.empty())
          {
            sendMessage.Properties.MessageId
                = static_cast<Azure::Core::Amqp::Models::AmqpValue>(m_messageId);
          }
          sendMessage.SetBody(Azure::Core::Amqp::Models::AmqpValue{"This is a message body."});
          if (HasMessageSender())
          {
            GTEST_LOG_(INFO) << "Sent, resetting should send.";
            m_shouldSendMessage = false;
            EXPECT_EQ(MessageSendStatus::Ok, std::get<0>(GetMessageSender().Send(sendMessage)));
          }
          else
          {
            GTEST_LOG_(INFO) << "No sender, skipping";
          }
        }
      }

      void MessageReceived(
          std::string const& linkName,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO) << "Message received on link " << linkName << ": " << *message;
      }
    };
    auto serviceEndpoint = std::make_shared<ReceiverServiceEndpoint>(
        brokerEndpoint, MessageTests::MockServiceEndpointOptions{});
    m_mockServer.AddServiceEndpoint(serviceEndpoint);
#endif

    ConnectionOptions connectionOptions;

    //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
    auto connection{CreateAmqpConnection({})};
    auto session{CreateAmqpSession(connection)};
#if !defined(USE_NATIVE_BROKER)
    serviceEndpoint->SetSenderNodeName(senderEndpoint);
    StartServerListening();
#endif

    GTEST_LOG_(INFO) << "Create receiver on " << senderEndpoint;

    MessageReceiverOptions receiverOptions;
    receiverOptions.Name = "receiver-link";
    receiverOptions.MessageTarget = "egress";
    receiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    receiverOptions.MaxMessageSize = 65536;
    receiverOptions.MaxLinkCredit = 500; // We allow at most 500 messages to be received.
    receiverOptions.EnableTrace = true;
    MessageReceiver receiver(session.CreateMessageReceiver(brokerEndpoint, receiverOptions));

    receiver.Open();

    // Send a message.
    {
      std::string messageId = "Message from line " + std::to_string(__LINE__);
#if !defined(USE_NATIVE_BROKER)
      serviceEndpoint->ShouldSendMessage(true, messageId);
#else
      {
        MessageSender sender(session.CreateMessageSender(brokerEndpoint, {}));
        ASSERT_FALSE(sender.Open());
        Azure::Core::Amqp::Models::AmqpMessage sendMessage;
        sendMessage.Properties.MessageId = Azure::Core::Amqp::Models::AmqpValue(messageId);
        sendMessage.SetBody(Azure::Core::Amqp::Models::AmqpValue{"This is a message body."});
        EXPECT_FALSE(sender.Send(sendMessage));
        sender.Close();
      }
#endif
      auto message = receiver.WaitForIncomingMessage();
      GTEST_LOG_(INFO) << "Received message." << *message.first;
      ASSERT_TRUE(message.first);
      ASSERT_FALSE(message.second);
      ASSERT_EQ(
          message.first->Properties.MessageId.GetType(),
          Azure::Core::Amqp::Models::AmqpValueType::String);
      EXPECT_EQ(static_cast<std::string>(message.first->Properties.MessageId), messageId);
      EXPECT_EQ(
          static_cast<std::string>(message.first->GetBodyAsAmqpValue()), "This is a message body.");
    }

    {
      Azure::Core::Context receiveContext;
      receiveContext.Cancel();
      EXPECT_THROW(
          auto message = receiver.WaitForIncomingMessage(receiveContext),
          Azure::Core::OperationCancelledException);
    }

    {
      auto result = receiver.TryWaitForIncomingMessage();
      if (result.first)
      {
        GTEST_LOG_(INFO) << "Found an incoming message, expected no message." << *result.first;
      }
      EXPECT_FALSE(result.first);
    }

    {
      GTEST_LOG_(INFO) << "Trigger message send for polling.";
      std::string messageId = "Message from line " + std::to_string(__LINE__);
#if !defined(USE_NATIVE_BROKER)
      serviceEndpoint->ShouldSendMessage(true, messageId);
      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
#else

      {
        MessageSender sender(session.CreateMessageSender(brokerEndpoint, {}));
        ASSERT_FALSE(sender.Open());
        Azure::Core::Amqp::Models::AmqpMessage sendMessage;
        sendMessage.Properties.MessageId = Azure::Core::Amqp::Models::AmqpValue(messageId);
        sendMessage.SetBody(Azure::Core::Amqp::Models::AmqpValue{"This is a message body."});
        EXPECT_FALSE(sender.Send(sendMessage));
        sender.Close();
      }
#endif
      GTEST_LOG_(INFO) << "Message should have been sent and processed.";
      auto result = receiver.TryWaitForIncomingMessage();
      EXPECT_TRUE(result.first);
      EXPECT_EQ(messageId, static_cast<std::string>(result.first->Properties.MessageId));
    }
    receiver.Close();

    StopServerListening();
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }

  TEST_F(TestMessageSendReceive, AuthenticatedReceiverAzureToken)
  {
    std::string brokerEndpoint
        = GetBrokerEndpoint() + "/" + testing::UnitTest::GetInstance()->current_test_case()->name();

#if !defined(USE_NATIVE_BROKER)
    class ReceiverServiceEndpoint : public MessageTests::MockServiceEndpoint {
    public:
      ReceiverServiceEndpoint(
          std::string const& name,
          MessageTests::MockServiceEndpointOptions const& options)
          : MockServiceEndpoint(name, options)
      {
      }
      virtual ~ReceiverServiceEndpoint() = default;

      void ShouldSendMessage(bool shouldSend, Azure::Core::Amqp::Models::AmqpValue messageId)
      {
        m_shouldSendMessage = shouldSend;
        m_messageId = messageId;
      }
      void SetSenderNodeName(std::string const& senderNodeName)
      {
        m_senderNodeName = senderNodeName;
      }

    private:
      mutable bool m_shouldSendMessage{false};
      Azure::Core::Amqp::Models::AmqpValue m_messageId;
      std::string m_senderNodeName;

      void Poll() const override
      {
        if (m_shouldSendMessage && HasMessageSender())
        {
          GTEST_LOG_(INFO) << "Sending message to client." + m_senderNodeName;
          Azure::Core::Amqp::Models::AmqpMessage sendMessage;
          sendMessage.Properties.MessageId = m_messageId;
          sendMessage.SetBody(Azure::Core::Amqp::Models::AmqpValue{"This is a message body."});
          if (HasMessageSender())
          {
            GTEST_LOG_(INFO) << "Sent, resetting should send.";
            m_shouldSendMessage = false;
            EXPECT_EQ(MessageSendStatus::Ok, std::get<0>(GetMessageSender().Send(sendMessage)));
          }
          else
          {
            GTEST_LOG_(INFO) << "No sender, skipping";
          }
        }
      }
      void MessageReceived(
          std::string const& linkName,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO) << "Message received on link " << linkName << ": " << *message;
      }
    };

    auto serviceEndpoint = std::make_shared<ReceiverServiceEndpoint>(
        brokerEndpoint, MessageTests::MockServiceEndpointOptions{});

    m_mockServer.AddServiceEndpoint(serviceEndpoint);
#endif

    class AzureTokenCredential : public Azure::Core::Credentials::TokenCredential {
      Azure::Core::Credentials::AccessToken GetToken(
          const Azure::Core::Credentials::TokenRequestContext& requestContext,
          const Azure::Core::Context& context) const override
      {
        Azure::Core::Credentials::AccessToken rv;
        rv.Token = "ThisIsAJwt.WithABogusBody.AndSignature";
        rv.ExpiresOn = std::chrono::system_clock::now();
        (void)requestContext;
        (void)context;
        return rv;
      }

    public:
      AzureTokenCredential() : Azure::Core::Credentials::TokenCredential("Testing") {}
    };

    auto tokenCredential = std::make_shared<AzureTokenCredential>();

    auto connection{CreateAmqpConnection()};
    auto session{CreateAmqpSession(connection)};

#if !defined(USE_NATIVE_BROKER)
    serviceEndpoint->SetSenderNodeName("receiver-link");
    StartServerListening();
#endif

    MessageReceiverOptions receiverOptions;
    receiverOptions.MessageTarget = "egress";
    receiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    receiverOptions.MaxMessageSize = 65536;
    receiverOptions.Name = "receiver-link";
    MessageReceiver receiver(session.CreateMessageReceiver(brokerEndpoint, receiverOptions));

    receiver.Open();

    // Receive a message with a 15 second timeout. It shouldn't throw.
    {
      Azure::Core::Context receiveContext{
          std::chrono::system_clock::now() + std::chrono::seconds(15)};
      auto messageId
          = Azure::Core::Amqp::Models::AmqpValue("Message from line " + std::to_string(__LINE__));
#if !defined(USE_NATIVE_BROKER)
      // Tell the server it should send a message in the polling loop.
      serviceEndpoint->ShouldSendMessage(true, messageId);
#else
      {
        MessageSender sender(session.CreateMessageSender(brokerEndpoint, {}));
        ASSERT_FALSE(sender.Open());
        Azure::Core::Amqp::Models::AmqpMessage sendMessage;
        sendMessage.Properties.MessageId = messageId;
        sendMessage.SetBody(Azure::Core::Amqp::Models::AmqpValue{"This is a message body."});
        EXPECT_FALSE(sender.Send(sendMessage));
        sender.Close();
      }
#endif
      GTEST_LOG_(INFO) << "Waiting for message to be received.";
      std::pair<std::shared_ptr<const Azure::Core::Amqp::Models::AmqpMessage>, bool> response;
      ASSERT_NO_THROW(response = receiver.WaitForIncomingMessage(receiveContext));
      ASSERT_TRUE(response.first);
      ASSERT_FALSE(response.second);
      EXPECT_EQ(
          static_cast<std::string>(response.first->GetBodyAsAmqpValue()),
          "This is a message body.");
    }

    {
      Azure::Core::Context receiveContext;
      receiveContext.Cancel();
      EXPECT_THROW(
          auto message = receiver.WaitForIncomingMessage(receiveContext),
          Azure::Core::OperationCancelledException);
    }
    receiver.Close();
    StopServerListening();
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }

  TEST_F(TestMessageSendReceive, AuthenticatedReceiverTryReceive)
  {
    std::string brokerEndpoint
        = GetBrokerEndpoint() + testing::UnitTest::GetInstance()->current_test_info()->name();
#if !defined(USE_NATIVE_BROKER)
    class ReceiverServiceEndpoint final : public MessageTests::MockServiceEndpoint {
    public:
      ReceiverServiceEndpoint(
          std::string const& name,
          MessageTests::MockServiceEndpointOptions const& options)
          : MockServiceEndpoint(name, options)
      {
      }
      virtual ~ReceiverServiceEndpoint() = default;

      void ShouldSendMessage(bool shouldSend, Azure::Core::Amqp::Models::AmqpValue messageId)
      {
        m_shouldSendMessage = shouldSend;
        m_messageId = messageId;
      }
      void SetSenderNodeName(std::string const& senderNodeName)
      {
        m_senderNodeName = senderNodeName;
      }

    private:
      mutable bool m_shouldSendMessage{false};
      std::string m_senderNodeName;
      Azure::Core::Amqp::Models::AmqpValue m_messageId;

      void Poll() const override
      {
        if (m_shouldSendMessage && HasMessageSender())
        {
          GTEST_LOG_(INFO) << "Sending message to client." + m_senderNodeName;
          Azure::Core::Amqp::Models::AmqpMessage sendMessage;
          sendMessage.Properties.MessageId = m_messageId;
          sendMessage.SetBody(Azure::Core::Amqp::Models::AmqpValue{"This is a message body."});
          if (HasMessageSender())
          {
            GTEST_LOG_(INFO) << "Sent, resetting should send.";
            m_shouldSendMessage = false;
            EXPECT_EQ(MessageSendStatus::Ok, std::get<0>(GetMessageSender().Send(sendMessage)));
          }
          else
          {
            GTEST_LOG_(INFO) << "No sender, skipping";
          }
        }
      }
      void MessageReceived(
          std::string const& linkName,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO) << "Message received on link " << linkName << ": " << *message;
      }
    };

    auto serviceEndpoint = std::make_shared<ReceiverServiceEndpoint>(
        brokerEndpoint, MessageTests::MockServiceEndpointOptions{});
    m_mockServer.AddServiceEndpoint(serviceEndpoint);
#endif
    ConnectionOptions connectionOptions;

    //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
    auto connection{CreateAmqpConnection()};
    auto session{CreateAmqpSession(connection)};

#if !defined(USE_NATIVE_BROKER)
    serviceEndpoint->SetSenderNodeName("receiverLink");
    StartServerListening();
#endif

    MessageReceiverOptions receiverOptions;
    receiverOptions.Name = "receiver-link";
    receiverOptions.MessageTarget = "egress";
    receiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    receiverOptions.MaxMessageSize = 65536;
    receiverOptions.MaxLinkCredit = 500; // We allow at most 500 messages to be received.
    receiverOptions.Name = "receiver-link";
    receiverOptions.EnableTrace = true;
    MessageReceiver receiver(session.CreateMessageReceiver(brokerEndpoint, receiverOptions));

    receiver.Open();

    {
      auto result = receiver.TryWaitForIncomingMessage();
      EXPECT_FALSE(result.first);
    }

    {
      GTEST_LOG_(INFO) << "Trigger message send for polling.";
      std::string messageId = "Message from line " + std::to_string(__LINE__);
#if !defined(USE_NATIVE_BROKER)
      serviceEndpoint->ShouldSendMessage(
          true, static_cast<Azure::Core::Amqp::Models::AmqpValue>(messageId));
#else
      {
        MessageSender sender(session.CreateMessageSender(brokerEndpoint, {}));
        ASSERT_FALSE(sender.Open());
        Azure::Core::Amqp::Models::AmqpMessage sendMessage;
        sendMessage.Properties.MessageId = Azure::Core::Amqp::Models::AmqpValue(messageId);
        sendMessage.SetBody(Azure::Core::Amqp::Models::AmqpValue{"This is a message body."});
        EXPECT_FALSE(sender.Send(sendMessage));
        sender.Close();
      }

#endif

      GTEST_LOG_(INFO) << "Polling AMQP broker for 10 seconds looking for an incoming message.";
      auto timeout = std::chrono::system_clock::now() + std::chrono::seconds(10);
      std::shared_ptr<const Azure::Core::Amqp::Models::AmqpMessage> message;
      do
      {
        GTEST_LOG_(INFO) << "Check for message..";
        auto result = receiver.TryWaitForIncomingMessage();
        if (result.first)
        {
          GTEST_LOG_(INFO) << "Found an incoming message." << *result.first;
          message = result.first;
          break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

      } while (std::chrono::system_clock::now() < timeout);
      ASSERT_TRUE(message);
      EXPECT_EQ(messageId, static_cast<std::string>(message->Properties.MessageId));
    }
    receiver.Close();

    StopServerListening();
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }

#endif // !defined(AZ_PLATFORM_MAC)
}}}} // namespace Azure::Core::Amqp::Tests
