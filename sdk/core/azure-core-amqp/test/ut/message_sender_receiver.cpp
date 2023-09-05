// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/common/async_operation_queue.hpp"
#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/message_receiver.hpp"
#include "azure/core/amqp/message_sender.hpp"
#include "azure/core/amqp/models/message_source.hpp"
#include "azure/core/amqp/models/message_target.hpp"
#include "azure/core/amqp/models/messaging_values.hpp"
#include "azure/core/amqp/network/amqp_header_detect_transport.hpp"
#include "azure/core/amqp/network/socket_listener.hpp"
#include "azure/core/amqp/session.hpp"
#include "mock_amqp_server.hpp"

#include <azure/core/platform.hpp>

#include <functional>
#include <random>

#include <gtest/gtest.h>

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  extern uint16_t FindAvailableSocket();

  class TestMessages : public testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
  };

  using namespace Azure::Core::Amqp::_internal;
  using namespace Azure::Core::Amqp;

#if !defined(AZ_PLATFORM_MAC)
  TEST_F(TestMessages, SimpleReceiver)
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
  }
  TEST_F(TestMessages, ReceiverProperties)
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
      auto rejected{Models::_internal::Messaging::DeliveryRejected("error", "description")};
      auto modified{Models::_internal::Messaging::DeliveryModified(true, false, "Annotations")};
      auto received{Models::_internal::Messaging::DeliveryReceived(3, 24)};
    }
  }

  TEST_F(TestMessages, SimpleSender)
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
  }
  TEST_F(TestMessages, SenderProperties)
  { // Create a connection
    Connection connection("localhost", nullptr, {});
    Session session{connection.CreateSession()};

    {
      MessageSenderOptions options;
      options.EnableTrace = true;
      MessageSender sender(session.CreateMessageSender("MySource", options, nullptr));
    }
  }

  TEST_F(TestMessages, ReceiverOpenClose)
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
          GTEST_LOG_(INFO) << "MessageReceiverEvents::OnMessageReceiverSTateChanged.";
          (void)receiver;
          (void)newState;
          (void)oldState;
        }

        Models::AmqpValue OnMessageReceived(
            Azure::Core::Amqp::_internal::MessageReceiver const&,
            Models::AmqpMessage const&) override
        {
          return Models::AmqpValue();
        }
        void OnMessageReceiverDisconnected(
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

  TEST_F(TestMessages, SenderOpenClose)
  {
    uint16_t testPort = FindAvailableSocket();
    GTEST_LOG_(INFO) << "Test port: " << testPort;
    ConnectionOptions connectionOptions;
    connectionOptions.IdleTimeout = std::chrono::minutes(5);
    connectionOptions.Port = testPort;
    connectionOptions.ContainerId = ::testing::UnitTest::GetInstance()->current_test_info()->name();

    Connection connection("localhost", nullptr, connectionOptions);
    Session session{connection.CreateSession()};

    Azure::Core::Amqp::Network::_internal::SocketListener listener(testPort, nullptr);
    EXPECT_NO_THROW(listener.Start());
    {
      MessageSenderOptions options;
      options.MessageSource = "MySource";

      MessageSender sender(session.CreateMessageSender("MyTarget", options, nullptr));
      sender.Open();
      sender.Close();
    }
    listener.Stop();
  }

  TEST_F(TestMessages, TestLocalhostVsTls)
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
                           << std::to_string(static_cast<uint32_t>(oldState))
                           << " NewState: " << std::to_string(static_cast<uint32_t>(newState));
          (void)sender;
        }
        virtual void OnMessageSenderDisconnected(Models::_internal::AmqpError const& error) override
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
      EXPECT_NO_THROW(sender.Open());

      Azure::Core::Amqp::Models::AmqpMessage message;
      message.SetBody(Azure::Core::Amqp::Models::AmqpBinaryData{'h', 'e', 'l', 'l', 'o'});

      Azure::Core::Context context;
      try
      {
        auto result = sender.Send(message, context);

        // Because we're trying to use TLS to connect to a non-TLS port, we should get an error
        // sending the message.
        EXPECT_EQ(std::get<0>(result), MessageSendStatus::Error);
      }
      catch (Azure::Core::OperationCancelledException const&)
      {
        GTEST_LOG_(INFO) << "Operation cancelled.";
      }
      catch (std::runtime_error const& ex)
      {
        // The WaitForPolledResult call can throw an exception if the connection enters the
        // "End" state.
        GTEST_LOG_(INFO) << "Exception: " << ex.what();
      }
      sender.Close();
    }
    mockServer.StopListening();
  }

  TEST_F(TestMessages, SenderSendAsync)
  {
    MessageTests::AmqpServerMock mockServer{};

    GTEST_LOG_(INFO) << "Test port: " << mockServer.GetPort();

    ConnectionOptions connectionOptions;
    //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
    connectionOptions.ContainerId = "some";
    //  connectionOptions.EnableTrace = true;
    connectionOptions.Port = mockServer.GetPort();
    Connection connection("localhost", nullptr, connectionOptions);
    Session session{connection.CreateSession()};

    // Set up a 30 second deadline on the receiver.
    Azure::Core::Context receiveContext = Azure::Core::Context::ApplicationContext.WithDeadline(
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
        virtual void OnMessageSenderDisconnected(Models::_internal::AmqpError const& error) override
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
      EXPECT_NO_THROW(sender.Open());

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

  TEST_F(TestMessages, SenderSendSync)
  {
    MessageTests::AmqpServerMock mockServer{};
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
      EXPECT_NO_THROW(sender.Open());

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

  TEST_F(TestMessages, AuthenticatedSender)
  {
    MessageTests::AmqpServerMock server;

    auto sasCredential = std::make_shared<ServiceBusSasConnectionStringCredential>(
        "Endpoint=amqp://localhost:" + std::to_string(server.GetPort())
        + "/;SharedAccessKeyName=MyTestKey;SharedAccessKey=abcdabcd;EntityPath=testLocation");

    ConnectionOptions connectionOptions;

    //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
    connectionOptions.ContainerId = testing::UnitTest::GetInstance()->current_test_info()->name();
    connectionOptions.Port = server.GetPort();
    Connection connection("localhost", sasCredential, connectionOptions);
    Session session{connection.CreateSession()};

    server.StartListening();

    MessageSenderOptions senderOptions;
    senderOptions.Name = "sender-link";
    senderOptions.MessageSource = "ingress";
    senderOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
    senderOptions.MaxMessageSize = 65536;
    MessageSender sender(
        session.CreateMessageSender(sasCredential->GetEntityPath(), senderOptions, nullptr));

    sender.Open();

    Azure::Core::Amqp::Models::AmqpMessage message;
    message.SetBody(Azure::Core::Amqp::Models::AmqpValue{"Hello"});
    sender.Send(message);

    sender.Close();
    server.StopListening();
  }

  TEST_F(TestMessages, AuthenticatedSenderAzureToken)
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

    MessageTests::AmqpServerMock server;

    auto tokenCredential = std::make_shared<AzureTokenCredential>();
    std::string hostName = "localhost";
    std::string entityPath = "testLocation";
    uint16_t port = server.GetPort();
    std::string endpoint = "amqp://" + hostName + ":" + std::to_string(port) + "/" + entityPath;

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
    sender.Open();

    Azure::Core::Amqp::Models::AmqpMessage message;
    message.SetBody(Azure::Core::Amqp::Models::AmqpValue{"Hello"});
    sender.Send(message);

    sender.Close();
    server.StopListening();
  }

  TEST_F(TestMessages, AuthenticatedReceiver)
  {
    class ReceiverMock : public MessageTests::AmqpServerMock {
    public:
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
        if (m_shouldSendMessage
            && m_linkMessageQueues.find(m_senderNodeName) != m_linkMessageQueues.end())
        {
          GTEST_LOG_(INFO) << "Sending message to client." + m_senderNodeName;
          Azure::Core::Amqp::Models::AmqpMessage sendMessage;
          sendMessage.SetBody(Azure::Core::Amqp::Models::AmqpValue{"This is a message body."});
          if (m_linkMessageQueues.at(m_senderNodeName).LinkSender)
          {
            m_linkMessageQueues.at(m_senderNodeName).LinkSender->Send(sendMessage);
            m_shouldSendMessage = false;
          }
        }
      }
    };

    ReceiverMock server;

    auto sasCredential = std::make_shared<ServiceBusSasConnectionStringCredential>(
        "Endpoint=amqp://localhost:" + std::to_string(server.GetPort())
        + "/;SharedAccessKeyName=MyTestKey;SharedAccessKey=abcdabcd;EntityPath=testLocation");

    ConnectionOptions connectionOptions;

    //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
    connectionOptions.ContainerId = testing::UnitTest::GetInstance()->current_test_info()->name();
    connectionOptions.Port = server.GetPort();
    Connection connection("localhost", sasCredential, connectionOptions);
    Session session{connection.CreateSession()};

    server.SetSenderNodeName(sasCredential->GetEndpoint() + sasCredential->GetEntityPath());
    server.StartListening();

    MessageReceiverOptions receiverOptions;
    receiverOptions.Name = "receiver-link";
    receiverOptions.MessageTarget = "egress";
    receiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    receiverOptions.MaxMessageSize = 65536;
    receiverOptions.Name = "receiver-link";
    receiverOptions.EnableTrace = true;
    MessageReceiver receiver(session.CreateMessageReceiver(
        sasCredential->GetEndpoint() + sasCredential->GetEntityPath(), receiverOptions, nullptr));

    receiver.Open();

    // Send a message.
    {
      server.ShouldSendMessage(true);
      auto message = receiver.WaitForIncomingMessage();
      ASSERT_TRUE(message.first.HasValue());
      ASSERT_FALSE(message.second);
      EXPECT_EQ(
          static_cast<std::string>(message.first.Value().GetBodyAsAmqpValue()),
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

  TEST_F(TestMessages, AuthenticatedReceiverAzureToken)
  {
    class ReceiverMock : public MessageTests::AmqpServerMock {
    public:
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
        if (m_shouldSendMessage
            && m_linkMessageQueues.find(m_senderNodeName) != m_linkMessageQueues.end())
        {
          Azure::Core::Amqp::Models::AmqpMessage sendMessage;
          sendMessage.SetBody(Azure::Core::Amqp::Models::AmqpValue{"This is a message body."});
          if (m_linkMessageQueues.at(m_senderNodeName).LinkSender)
          {
            m_linkMessageQueues.at(m_senderNodeName).LinkSender->Send(sendMessage);
            m_shouldSendMessage = false;
          }
        }
      }
    };

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

    ReceiverMock server;

    auto tokenCredential = std::make_shared<AzureTokenCredential>();
    std::string hostName = "localhost";
    std::string entityPath = "testLocation";
    uint16_t port = server.GetPort();
    std::string endpoint = "amqp://" + hostName + ":" + std::to_string(port) + "/" + entityPath;

    ConnectionOptions connectionOptions;

    connectionOptions.IdleTimeout = std::chrono::minutes(5);
    connectionOptions.ContainerId = "some";
    connectionOptions.Port = port;
    Connection connection(hostName, tokenCredential, connectionOptions);
    Session session{connection.CreateSession()};

    server.SetSenderNodeName(endpoint);
    server.StartListening();

    MessageReceiverOptions receiverOptions;
    receiverOptions.Name = "receiver-link";
    receiverOptions.MessageTarget = "egress";
    receiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    receiverOptions.MaxMessageSize = 65536;
    receiverOptions.Name = "receiver-link";
    MessageReceiver receiver(session.CreateMessageReceiver(endpoint, receiverOptions, nullptr));

    receiver.Open();

    // Send a message.
    {
      server.ShouldSendMessage(true);
      auto message = receiver.WaitForIncomingMessage();
      ASSERT_TRUE(message.first.HasValue());
      ASSERT_FALSE(message.second);
      EXPECT_EQ(
          static_cast<std::string>(message.first.Value().GetBodyAsAmqpValue()),
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
#endif // !defined(AZ_PLATFORM_MAC)
}}}} // namespace Azure::Core::Amqp::Tests
