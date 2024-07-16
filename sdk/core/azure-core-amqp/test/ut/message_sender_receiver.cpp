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
#include "mock_amqp_server.hpp"

#include <azure/core/platform.hpp>

#include <functional>
#include <random>

#include <gtest/gtest.h>

// cspell: ignore abcdabcd

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  extern uint16_t FindAvailableSocket();

  class TestMessageSendReceive : public testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override
    { // When the test is torn down, the global state MUST be idle. If it is not, something leaked.
      Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }
  };

  using namespace Azure::Core::Amqp::_internal;
  using namespace Azure::Core::Amqp;

#if !defined(AZ_PLATFORM_MAC)
  TEST_F(TestMessageSendReceive, SimpleReceiver)
  {

    // Create a connection
    Connection connection("localhost", nullptr, {});
    // Create a session
    Session session{connection.CreateSession()};

    {
      MessageReceiver receiver(session.CreateMessageReceiver("MySource", {}, nullptr));
    }
    {
      MessageReceiver receiver1(session.CreateMessageReceiver("MySource", {}, nullptr));
      MessageReceiver receiver2(session.CreateMessageReceiver("MySource", {}, nullptr));
    }

    GTEST_LOG_(INFO) << _internal::MessageReceiverState::Invalid
                     << _internal::MessageReceiverState::Closing
                     << _internal::MessageReceiverState::Idle
                     << _internal::MessageReceiverState::Opening
                     << _internal::MessageReceiverState::Open
                     << _internal::MessageReceiverState::Error;
    GTEST_LOG_(INFO) << static_cast<_internal::MessageReceiverState>(5993);
  }
  TEST_F(TestMessageSendReceive, ReceiverProperties)
  { // Create a connection
    Connection connection("localhost", nullptr, {});
    Session session{connection.CreateSession()};

    {
      MessageReceiverOptions options;
      options.EnableTrace = true;
      MessageReceiver receiver(session.CreateMessageReceiver("MyTarget", options));
      EXPECT_ANY_THROW(receiver.GetLinkName());
    }

    {
      auto accepted{Models::_internal::Messaging::DeliveryAccepted()};
      auto released{Models::_internal::Messaging::DeliveryReleased()};
      auto rejected{Models::_internal::Messaging::DeliveryRejected("error", "description", {})};
      auto modified{Models::_internal::Messaging::DeliveryModified(true, false, "Annotations")};
      auto received{Models::_internal::Messaging::DeliveryReceived(3, 24)};
    }
  }

  TEST_F(TestMessageSendReceive, SimpleSender)
  {

    // Create a connection
    Connection connection("localhost", nullptr, {});
    // Create a session
    Session session{connection.CreateSession()};

    {
      MessageSender sender(session.CreateMessageSender("MySource", {}, nullptr));
    }
    {
      MessageSender sender1(session.CreateMessageSender("MySource", {}, nullptr));
      MessageSender sender2(session.CreateMessageSender("MySource", {}, nullptr));
    }
    GTEST_LOG_(INFO) << _internal::MessageSenderState::Invalid
                     << _internal::MessageSenderState::Closing
                     << _internal::MessageSenderState::Idle
                     << _internal::MessageSenderState::Opening
                     << _internal::MessageSenderState::Open << _internal::MessageSenderState::Error;
    GTEST_LOG_(INFO) << static_cast<_internal::MessageSenderState>(5993);
  }
  TEST_F(TestMessageSendReceive, SenderProperties)
  { // Create a connection
    Connection connection("localhost", nullptr, {});
    Session session{connection.CreateSession()};

    {
      MessageSenderOptions options;
      options.EnableTrace = true;
      MessageSender sender(session.CreateMessageSender("MySource", options, nullptr));
    }
  }

  TEST_F(TestMessageSendReceive, ReceiverOpenClose)
  {
    MessageTests::AmqpServerMock mockServer;

    ConnectionOptions connectionOptions;
    //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
    connectionOptions.EnableTrace = true;
    connectionOptions.Port = mockServer.GetPort();
    connectionOptions.ContainerId = testing::UnitTest::GetInstance()->current_test_info()->name();
    Connection connection("localhost", nullptr, connectionOptions);
    Session session{connection.CreateSession()};

    mockServer.StartListening();

    Azure::Core::Context context;

    {
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
      MessageReceiverOptions options;
      options.Name = "Test Receiver";
      MessageReceiver receiver(session.CreateMessageReceiver("MyTarget", options, &receiverEvents));

      EXPECT_NO_THROW(receiver.Open());
      EXPECT_EQ("Test Receiver", receiver.GetLinkName());

      receiver.Close();
    }

    mockServer.StopListening();

    context.Cancel();
  }

  TEST_F(TestMessageSendReceive, SenderOpenClose)
  {
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
    MessageTests::AmqpServerMock mockServer{};
    mockServer.AddServiceEndpoint(senderEndpoint);

    mockServer.StartListening();

    ConnectionOptions connectionOptions;
    connectionOptions.IdleTimeout = std::chrono::minutes(5);
    connectionOptions.Port = mockServer.GetPort();
    connectionOptions.EnableTrace = true;
    connectionOptions.ContainerId = ::testing::UnitTest::GetInstance()->current_test_info()->name();

    Connection connection("localhost", nullptr, connectionOptions);
    Session session{connection.CreateSession()};

    {
      MessageSenderOptions options;
      options.MessageSource = "MySource";

      MessageSender sender(session.CreateMessageSender("MyTarget", options, nullptr));
      EXPECT_FALSE(sender.Open());
      sender.Close();
    }
    mockServer.StopListening();
  }

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

  TEST_F(TestMessageSendReceive, SenderSendAsync)
  {
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
    MessageTests::AmqpServerMock mockServer{};
    mockServer.AddServiceEndpoint(senderEndpoint);

    GTEST_LOG_(INFO) << "Test port: " << mockServer.GetPort();

    ConnectionOptions connectionOptions;
    //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
    connectionOptions.ContainerId = "some";
    //  connectionOptions.EnableTrace = true;
    connectionOptions.Port = mockServer.GetPort();
    Connection connection("localhost", nullptr, connectionOptions);
    Session session{connection.CreateSession()};

    // Set up a 30 second deadline on the receiver.
    Azure::Core::Context receiveContext = Azure::Core::Context::CreateWithDeadline(
        Azure::DateTime::clock::now() + std::chrono::seconds(15));

    // Ensure that the thread is started before we start using the message sender.
    mockServer.StartListening();

    {
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
      MessageSenderOptions options;
      options.Name = "sender-link";
      options.MessageSource = "ingress";
      options.SettleMode = SenderSettleMode::Settled;
      options.MaxMessageSize = 65536;
      MessageSender sender(
          session.CreateMessageSender("localhost/ingress", options, &senderEvents));
      EXPECT_FALSE(sender.Open());

      Azure::Core::Amqp::Models::AmqpMessage message;
      message.SetBody(Azure::Core::Amqp::Models::AmqpBinaryData{'h', 'e', 'l', 'l', 'o'});

      Azure::Core::Context context;
      auto result = sender.Send(message);
      EXPECT_EQ(std::get<0>(result), MessageSendStatus::Ok);

      sender.Close();
    }
    receiveContext.Cancel();
    mockServer.StopListening();
  }

  TEST_F(TestMessageSendReceive, SenderSendSync)
  {
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
    MessageTests::AmqpServerMock mockServer{};
    mockServer.AddServiceEndpoint(senderEndpoint);

    ConnectionOptions connectionOptions;

    GTEST_LOG_(INFO) << "Test port: " << mockServer.GetPort();

    //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
    connectionOptions.ContainerId = testing::UnitTest::GetInstance()->current_test_case()->name();
    connectionOptions.Port = mockServer.GetPort();
    Connection connection("localhost", nullptr, connectionOptions);
    Session session{connection.CreateSession()};

    Azure::Core::Context receiveContext;

    // Ensure that the thread is started before we start using the message sender.
    mockServer.StartListening();

    {
      MessageSenderOptions options;
      options.SettleMode = SenderSettleMode::Settled;
      options.MaxMessageSize = 65536;
      options.MessageSource = "ingress";
      options.Name = "sender-link";
      MessageSender sender(session.CreateMessageSender("localhost/ingress", options, nullptr));
      EXPECT_FALSE(sender.Open());

      Azure::Core::Amqp::Models::AmqpMessage message;
      message.SetBody(Azure::Core::Amqp::Models::AmqpValue{"Hello"});

      Azure::Core::Amqp::Common::_internal::
          AsyncOperationQueue<MessageSendStatus, Azure::Core::Amqp::Models::AmqpValue>
              sendCompleteQueue;
      auto result = sender.Send(message);
      EXPECT_EQ(std::get<0>(result), MessageSendStatus::Ok);

      sender.Close();
    }
    receiveContext.Cancel();
    mockServer.StopListening();
  }

  TEST_F(TestMessageSendReceive, AuthenticatedSender)
  {
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
    MessageTests::AmqpServerMock server;

    MessageTests::MockServiceEndpointOptions mockServiceEndpointOptions{};
    mockServiceEndpointOptions.EnableTrace = false;
    auto serviceEndpoint
        = std::make_shared<ReceiverServiceEndpoint>("testLocation", mockServiceEndpointOptions);

    server.AddServiceEndpoint(serviceEndpoint);

    auto sasCredential = std::make_shared<ServiceBusSasConnectionStringCredential>(
        "Endpoint=amqp://localhost:" + std::to_string(server.GetPort())
        + "/;SharedAccessKeyName=MyTestKey;SharedAccessKey=abcdabcd;EntityPath=testLocation");

    ConnectionOptions connectionOptions;

    //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
    connectionOptions.ContainerId = testing::UnitTest::GetInstance()->current_test_info()->name();
    connectionOptions.Port = server.GetPort();
    connectionOptions.EnableTrace = true;
    Connection connection("localhost", sasCredential, connectionOptions);
    Session session{connection.CreateSession()};

    server.StartListening();

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
    server.StopListening();
  }

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
    MessageTests::AmqpServerMock server;

    auto tokenCredential = std::make_shared<AzureTokenCredential>();
    std::string hostName = "localhost";
    std::string entityPath = "testLocation";
    uint16_t port = server.GetPort();
    std::string endpoint = "amqp://" + hostName + ":" + std::to_string(port) + "/" + entityPath;

    MessageTests::MockServiceEndpointOptions mockServiceEndpointOptions{};
    mockServiceEndpointOptions.EnableTrace = false;
    auto serviceEndpoint
        = std::make_shared<ReceiverServiceEndpoint>(endpoint, mockServiceEndpointOptions);
    server.AddServiceEndpoint(serviceEndpoint);

    ConnectionOptions connectionOptions;

    //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
    connectionOptions.ContainerId = "some";
    connectionOptions.Port = server.GetPort();
    Connection connection(hostName, tokenCredential, connectionOptions);
    Session session{connection.CreateSession()};

    server.StartListening();
    MessageSenderOptions senderOptions;
    senderOptions.Name = "sender-link";
    senderOptions.MessageSource = "ingress";
    senderOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
    senderOptions.MaxMessageSize = 65536;
    senderOptions.Name = "sender-link";
    MessageSender sender(session.CreateMessageSender(endpoint, senderOptions, nullptr));
    EXPECT_FALSE(sender.Open());

    Azure::Core::Amqp::Models::AmqpMessage message;
    message.SetBody(Azure::Core::Amqp::Models::AmqpValue{"Hello"});
    EXPECT_EQ(MessageSendStatus::Ok, std::get<0>(sender.Send(message)));

    sender.Close();
    server.StopListening();
  }

  TEST_F(TestMessageSendReceive, AuthenticatedReceiver)
  {
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

    MessageTests::AmqpServerMock server;
    auto serviceEndpoint = std::make_shared<ReceiverServiceEndpoint>(
        "amqp://localhost:" + std::to_string(server.GetPort()) + "/testLocation",
        MessageTests::MockServiceEndpointOptions{});
    server.AddServiceEndpoint(serviceEndpoint);

    auto sasCredential = std::make_shared<ServiceBusSasConnectionStringCredential>(
        "Endpoint=amqp://localhost:" + std::to_string(server.GetPort())
        + "/;SharedAccessKeyName=MyTestKey;SharedAccessKey=abcdabcd;EntityPath=testLocation");

    ConnectionOptions connectionOptions;

    //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
    connectionOptions.ContainerId = testing::UnitTest::GetInstance()->current_test_info()->name();
    connectionOptions.Port = server.GetPort();
    Connection connection("localhost", sasCredential, connectionOptions);
    Session session{connection.CreateSession()};

    serviceEndpoint->SetSenderNodeName(
        sasCredential->GetEndpoint() + sasCredential->GetEntityPath());
    server.StartListening();

    MessageReceiverOptions receiverOptions;
    receiverOptions.Name = "receiver-link";
    receiverOptions.MessageTarget = "egress";
    receiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    receiverOptions.MaxMessageSize = 65536;
    receiverOptions.MaxLinkCredit = 500; // We allow at most 500 messages to be received.
    receiverOptions.Name = "receiver-link";
    receiverOptions.EnableTrace = true;
    MessageReceiver receiver(session.CreateMessageReceiver(
        sasCredential->GetEndpoint() + sasCredential->GetEntityPath(), receiverOptions, nullptr));

    receiver.Open();

    // Send a message.
    {
      serviceEndpoint->ShouldSendMessage(true);
      auto message = receiver.WaitForIncomingMessage();
      GTEST_LOG_(INFO) << "Received message.";
      ASSERT_TRUE(message.first);
      ASSERT_FALSE(message.second);
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
      EXPECT_FALSE(result.first);
    }

    {
      GTEST_LOG_(INFO) << "Trigger message send for polling.";
      serviceEndpoint->ShouldSendMessage(true);
      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
      GTEST_LOG_(INFO) << "Message should have been sent and processed.";
      auto result = receiver.TryWaitForIncomingMessage();
      EXPECT_TRUE(result.first);
    }
    receiver.Close();
    server.StopListening();
  }

  TEST_F(TestMessageSendReceive, AuthenticatedReceiverAzureToken)
  {
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

    MessageTests::AmqpServerMock server;
    auto serviceEndpoint = std::make_shared<ReceiverServiceEndpoint>(
        "amqp://localhost:" + std::to_string(server.GetPort()) + "/testLocation",
        MessageTests::MockServiceEndpointOptions{});

    server.AddServiceEndpoint(serviceEndpoint);

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
    std::string hostName = "localhost";
    std::string entityPath = "testLocation";
    uint16_t port = server.GetPort();
    std::string endpoint = "amqp://" + hostName + ":" + std::to_string(port) + "/" + entityPath;

    ConnectionOptions connectionOptions;

    connectionOptions.IdleTimeout = std::chrono::minutes(5);
    connectionOptions.ContainerId = testing::UnitTest::GetInstance()->current_test_case()->name();
    connectionOptions.Port = port;
    Connection connection(hostName, tokenCredential, connectionOptions);
    Session session{connection.CreateSession()};

    serviceEndpoint->SetSenderNodeName(endpoint);
    server.StartListening();

    MessageReceiverOptions receiverOptions;
    receiverOptions.Name = "receiver-link";
    receiverOptions.MessageTarget = "egress";
    receiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    receiverOptions.MaxMessageSize = 65536;
    receiverOptions.Name = "receiver-link";
    MessageReceiver receiver(session.CreateMessageReceiver(endpoint, receiverOptions, nullptr));

    receiver.Open();

    // Receive a message with a 15 second timeout. It shouldn't throw.
    {
      Azure::Core::Context receiveContext = Azure::Core::Context::CreateWithDeadline(
          std::chrono::system_clock::now() + std::chrono::seconds(15));

      // Tell the server it should send a message in the polling loop.
      serviceEndpoint->ShouldSendMessage(true);
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
    server.StopListening();
  }

  TEST_F(TestMessageSendReceive, AuthenticatedReceiverTryReceive)
  {
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

    MessageTests::AmqpServerMock server;
    auto serviceEndpoint = std::make_shared<ReceiverServiceEndpoint>(
        "amqp://localhost:" + std::to_string(server.GetPort()) + "/testLocation",
        MessageTests::MockServiceEndpointOptions{});
    server.AddServiceEndpoint(serviceEndpoint);

    auto sasCredential = std::make_shared<ServiceBusSasConnectionStringCredential>(
        "Endpoint=amqp://localhost:" + std::to_string(server.GetPort())
        + "/;SharedAccessKeyName=MyTestKey;SharedAccessKey=abcdabcd;EntityPath=testLocation");

    ConnectionOptions connectionOptions;

    //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
    connectionOptions.ContainerId = testing::UnitTest::GetInstance()->current_test_info()->name();
    connectionOptions.Port = server.GetPort();
    Connection connection("localhost", sasCredential, connectionOptions);
    Session session{connection.CreateSession()};

    serviceEndpoint->SetSenderNodeName(
        sasCredential->GetEndpoint() + sasCredential->GetEntityPath());
    server.StartListening();

    MessageReceiverOptions receiverOptions;
    receiverOptions.Name = "receiver-link";
    receiverOptions.MessageTarget = "egress";
    receiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    receiverOptions.MaxMessageSize = 65536;
    receiverOptions.MaxLinkCredit = 500; // We allow at most 500 messages to be received.
    receiverOptions.Name = "receiver-link";
    receiverOptions.EnableTrace = true;
    MessageReceiver receiver(session.CreateMessageReceiver(
        sasCredential->GetEndpoint() + sasCredential->GetEntityPath(), receiverOptions, nullptr));

    receiver.Open();

    {
      auto result = receiver.TryWaitForIncomingMessage();
      EXPECT_FALSE(result.first);
    }

    {
      GTEST_LOG_(INFO) << "Trigger message send for polling.";
      serviceEndpoint->ShouldSendMessage(true);
      std::this_thread::sleep_for(std::chrono::seconds(2));
      GTEST_LOG_(INFO) << "Message should have been sent and processed.";
      auto result = receiver.TryWaitForIncomingMessage();
      EXPECT_TRUE(result.first);
    }
    receiver.Close();
    server.StopListening();
  }

#endif // !defined(AZ_PLATFORM_MAC)
}}}} // namespace Azure::Core::Amqp::Tests
