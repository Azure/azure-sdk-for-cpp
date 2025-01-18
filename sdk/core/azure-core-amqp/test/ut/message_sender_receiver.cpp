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

#include <functional>
#include <random>

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

  private:
    Azure::Core::Url m_brokerEndpoint{};
#if !defined(USE_NATIVE_BROKER)
    MessageTests::AmqpServerMock m_mockServer;
#endif
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

      void ShouldSendMessage(bool shouldSend) { m_shouldSendMessage = shouldSend; }
      void SetSenderNodeName(std::string const& senderNodeName)
      {
        m_senderNodeName = senderNodeName;
      }

    private:
      mutable bool m_shouldSendMessage{false};
      std::string m_senderNodeName;

      void Poll() const override
      {
        if (m_shouldSendMessage && HasMessageSender())
        {
          GTEST_LOG_(INFO) << "Sending message to client." + m_senderNodeName;
          Azure::Core::Amqp::Models::AmqpMessage sendMessage;
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
        "amqp://localhost:" + std::to_string(m_mockServer.GetPort()) + "/testLocation",
        MessageTests::MockServiceEndpointOptions{});
    m_mockServer.AddServiceEndpoint(serviceEndpoint);
#endif

    //    auto sasCredential = std::make_shared<ServiceBusSasConnectionStringCredential>(
    //        "Endpoint=amqp://localhost:" + std::to_string(GetPort())
    //        + "/;SharedAccessKeyName=MyTestKey;SharedAccessKey=abcdabcd;EntityPath=testLocation");

    ConnectionOptions connectionOptions;

    //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
    auto connection{CreateAmqpConnection({})};
    auto session{CreateAmqpSession(connection)};
#if !defined(USE_NATIVE_BROKER)
    serviceEndpoint->SetSenderNodeName(
        sasCredential->GetEndpoint() + sasCredential->GetEntityPath());
    StartServerListening();
#endif

    GTEST_LOG_(INFO) << "Create receiver on testLocation";

    MessageReceiverOptions receiverOptions;
    receiverOptions.Name = "receiver-link";
    receiverOptions.MessageTarget = "egress";
    receiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    receiverOptions.MaxMessageSize = 65536;
    receiverOptions.MaxLinkCredit = 500; // We allow at most 500 messages to be received.
    receiverOptions.Name = "receiver-link";
    receiverOptions.EnableTrace = true;
    MessageReceiver receiver(session.CreateMessageReceiver(
        GetBrokerEndpoint() + testing::UnitTest::GetInstance()->current_test_case()->name(),
        receiverOptions));

    receiver.Open();

    // Send a message.
    {
#if !defined(USE_NATIVE_BROKER)
      serviceEndpoint->ShouldSendMessage(true);
#else
      std::string messageId = "Message from line " + std::to_string(__LINE__);
      {
        MessageSender sender(session.CreateMessageSender(
            GetBrokerEndpoint() + testing::UnitTest::GetInstance()->current_test_case()->name(),
            {}));
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
        GTEST_LOG_(INFO) << "Found an incoming message, espected no message." << *result.first;
      }
      EXPECT_FALSE(result.first);
    }

    {
      GTEST_LOG_(INFO) << "Trigger message send for polling.";
#if !defined(USE_NATIVE_BROKER)
      serviceEndpoint->ShouldSendMessage(true);
      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
#else
      std::string messageId = "Message from line " + std::to_string(__LINE__);

      {
        MessageSender sender(session.CreateMessageSender(
            GetBrokerEndpoint() + testing::UnitTest::GetInstance()->current_test_case()->name(),
            {}));
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

      void ShouldSendMessage(bool shouldSend) { m_shouldSendMessage = shouldSend; }
      void SetSenderNodeName(std::string const& senderNodeName)
      {
        m_senderNodeName = senderNodeName;
      }

    private:
      mutable bool m_shouldSendMessage{false};
      std::string m_senderNodeName;

      void Poll() const override
      {
        if (m_shouldSendMessage && HasMessageSender())
        {
          GTEST_LOG_(INFO) << "Sending message to client." + m_senderNodeName;
          Azure::Core::Amqp::Models::AmqpMessage sendMessage;
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
        "amqp://localhost:" + std::to_string(m_mockServer.GetPort()) + "/testLocation",
        MessageTests::MockServiceEndpointOptions{});

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
    std::string entityPath = "testLocation";
    std::string endpoint
        = GetBrokerEndpoint() + testing::UnitTest::GetInstance()->current_test_case()->name();

    auto connection{CreateAmqpConnection()};
    auto session{CreateAmqpSession(connection)};

#if !defined(USE_NATIVE_BROKER)
    serviceEndpoint->SetSenderNodeName(endpoint);
    StartServerListening();
#endif

    MessageReceiverOptions receiverOptions;
    receiverOptions.Name = "receiver-link";
    receiverOptions.MessageTarget = "egress";
    receiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    receiverOptions.MaxMessageSize = 65536;
    receiverOptions.Name = "receiver-link";
    MessageReceiver receiver(session.CreateMessageReceiver(endpoint, receiverOptions));

    receiver.Open();

    // Receive a message with a 15 second timeout. It shouldn't throw.
    {
      Azure::Core::Context receiveContext{
          std::chrono::system_clock::now() + std::chrono::seconds(15)};

#if !defined(USE_NATIVE_BROKER)
      // Tell the server it should send a message in the polling loop.
      serviceEndpoint->ShouldSendMessage(true);
#else
      {
        MessageSender sender(session.CreateMessageSender(
            GetBrokerEndpoint() + testing::UnitTest::GetInstance()->current_test_case()->name(),
            {}));
        ASSERT_FALSE(sender.Open());
        Azure::Core::Amqp::Models::AmqpMessage sendMessage;
        sendMessage.Properties.MessageId
            = Azure::Core::Amqp::Models::AmqpValue("Message from line " + std::to_string(__LINE__));
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

      void ShouldSendMessage(bool shouldSend) { m_shouldSendMessage = shouldSend; }
      void SetSenderNodeName(std::string const& senderNodeName)
      {
        m_senderNodeName = senderNodeName;
      }

    private:
      mutable bool m_shouldSendMessage{false};
      std::string m_senderNodeName;

      void Poll() const override
      {
        if (m_shouldSendMessage && HasMessageSender())
        {
          GTEST_LOG_(INFO) << "Sending message to client." + m_senderNodeName;
          Azure::Core::Amqp::Models::AmqpMessage sendMessage;
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
        "amqp://localhost:" + std::to_string(m_mockServer.GetPort()) + "/testLocation",
        MessageTests::MockServiceEndpointOptions{});
    m_mockServer.AddServiceEndpoint(serviceEndpoint);
#endif
    std::uint16_t serverPort = GetPort();

    auto sasCredential = std::make_shared<ServiceBusSasConnectionStringCredential>(
        "Endpoint=amqp://localhost:" + std::to_string(serverPort)
        + "/;SharedAccessKeyName=MyTestKey;SharedAccessKey=abcdabcd;EntityPath="
        + testing::UnitTest::GetInstance()->current_test_case()->name());

    ConnectionOptions connectionOptions;

    //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
    auto connection{CreateAmqpConnection()};
    auto session{CreateAmqpSession(connection)};

#if !defined(USE_NATIVE_BROKER)
    serviceEndpoint->SetSenderNodeName(
        sasCredential->GetEndpoint() + sasCredential->GetEntityPath());
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
    MessageReceiver receiver(session.CreateMessageReceiver(
        GetBrokerEndpoint() + testing::UnitTest::GetInstance()->current_test_case()->name(),
        receiverOptions));

    receiver.Open();

    {
      auto result = receiver.TryWaitForIncomingMessage();
      EXPECT_FALSE(result.first);
    }

    {
      GTEST_LOG_(INFO) << "Trigger message send for polling.";
      std::string messageId = "Message from line " + std::to_string(__LINE__);
#if !defined(USE_NATIVE_BROKER)
      serviceEndpoint->ShouldSendMessage(true);
#else
      {
        MessageSender sender(session.CreateMessageSender(
            GetBrokerEndpoint() + testing::UnitTest::GetInstance()->current_test_case()->name(),
            {}));
        ASSERT_FALSE(sender.Open());
        Azure::Core::Amqp::Models::AmqpMessage sendMessage;
        sendMessage.Properties.MessageId = Azure::Core::Amqp::Models::AmqpValue(messageId);
        sendMessage.SetBody(Azure::Core::Amqp::Models::AmqpValue{"This is a message body."});
        EXPECT_FALSE(sender.Send(sendMessage));
        sender.Close();
      }

#endif

      GTEST_LOG_(INFO) << "Polling AMQP broker for 5 seconds looking for an incoming message.";
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
      EXPECT_TRUE(message);
      EXPECT_EQ(messageId, static_cast<std::string>(message->Properties.MessageId));
    }
    receiver.Close();

    StopServerListening();
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }

#endif // !defined(AZ_PLATFORM_MAC)
}}}} // namespace Azure::Core::Amqp::Tests
