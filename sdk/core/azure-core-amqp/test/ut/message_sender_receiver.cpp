// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <gtest/gtest.h>

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

#include "mock_amqp_server.hpp"

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
  Connection connection("localhost", {});
  // Create a session
  Session session(connection, nullptr);
  //  Link link(session, "MySession", SessionRole::Receiver, "MySource", "MyTarget");

  {
    MessageReceiver receiver(session, "MySource", {}, nullptr);
  }
  {
    MessageReceiver receiver1(session, "MySource", {}, nullptr);
    MessageReceiver receiver2(session, "MySource", {}, nullptr);
  }
}
TEST_F(TestMessages, ReceiverProperties)
{ // Create a connection
  Connection connection("localhost", {});
  Session session(connection, nullptr);

  {
    MessageReceiverOptions options;
    options.EnableTrace = true;
    MessageReceiver receiver(session, "MyTarget", options);
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
  Connection connection("localhost", {});
  // Create a session
  Session session(connection, nullptr);
  //  Link link(session, "MySession", SessionRole::Sender, "MySource", "MyTarget");

  {
    MessageSender sender(session, "MySource", {}, nullptr);
  }
  {
    MessageSender sender1(session, "MySource", {}, nullptr);
    MessageSender sender2(session, "MySource", {}, nullptr);
  }
}
TEST_F(TestMessages, SenderProperties)
{ // Create a connection
  Connection connection("localhost", {});
  Session session(connection, nullptr);

  {
    MessageSenderOptions options;
    options.EnableTrace = true;
    MessageSender sender(session, "MySource", options, nullptr);
  }
}

namespace MessageTests {
#if 1

class MessageListenerEvents : public Azure::Core::Amqp::Network::_internal::SocketListenerEvents,
                              public Azure::Core::Amqp::_internal::ConnectionEvents,
                              public Azure::Core::Amqp::_internal::SessionEvents,
                              public Azure::Core::Amqp::_internal::MessageReceiverEvents {
public:
  MessageListenerEvents() = default;

  std::shared_ptr<Azure::Core::Amqp::_internal::Connection> WaitForConnection(
      Azure::Core::Amqp::Network::_internal::SocketListener const& listener,
      Azure::Core::Context const& context)
  {
    auto result = m_listeningQueue.WaitForPolledResult(context, listener);
    if (result)
    {
      return std::move(std::get<0>(*result));
    }
    return nullptr;
  }
  std::unique_ptr<Azure::Core::Amqp::_internal::Session> WaitForSession(
      Azure::Core::Context const& context)
  {
    auto result = m_listeningSessionQueue.WaitForPolledResult(context, *m_connectionToPoll);
    if (result)
    {
      return std::move(std::get<0>(*result));
    }
    return nullptr;
  }
  std::unique_ptr<MessageReceiver> WaitForReceiver(Azure::Core::Context const& context)
  {
    auto result = m_messageReceiverQueue.WaitForPolledResult(context, *m_connectionToPoll);
    if (result)
    {
      return std::move(std::get<0>(*result));
    }
    return nullptr;
  }
  Azure::Core::Amqp::Models::AmqpMessage WaitForMessage(Azure::Core::Context const& context)
  {
    auto result = m_messageQueue.WaitForPolledResult(context, *m_connectionToPoll);
    if (result)
    {
      return std::move(std::get<0>(*result));
    }
    return nullptr;
  }

private:
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
      std::shared_ptr<Azure::Core::Amqp::_internal::Connection>>
      m_listeningQueue;
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
      std::unique_ptr<Azure::Core::Amqp::_internal::Session>>
      m_listeningSessionQueue;
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<std::unique_ptr<MessageReceiver>>
      m_messageReceiverQueue;
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<Azure::Core::Amqp::Models::AmqpMessage>
      m_messageQueue;
  std::shared_ptr<Connection> m_connectionToPoll;

  // Inherited via MessageReceiver

  // Inherited via SocketListenerEvents.
  virtual void OnSocketAccepted(
      std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> transport) override
  {
    GTEST_LOG_(INFO) << "OnSocketAccepted - Socket connection received.";
    auto amqpTransport{
        Azure::Core::Amqp::Network::_internal::AmqpHeaderDetectTransportFactory::Create(
            transport, nullptr)};
    Azure::Core::Amqp::_internal::ConnectionOptions options;
    //    options.IdleTimeout = std::chrono::minutes(5);
    options.ContainerId = "some";
    options.EnableTrace = true;
    m_connectionToPoll
        = std::make_shared<Azure::Core::Amqp::_internal::Connection>(amqpTransport, options, this);
    m_connectionToPoll->Listen();
    m_listeningQueue.CompleteOperation(m_connectionToPoll);
  }

  // Inherited via ConnectionEvents
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
      Endpoint& endpoint) override
  {
    GTEST_LOG_(INFO) << "OnNewEndpoint - Incoming endpoint created, create session.";
    Azure::Core::Amqp::_internal::SessionOptions options;
    options.InitialIncomingWindowSize = 10000;
    auto listeningSession = std::make_unique<Azure::Core::Amqp::_internal::Session>(
        connection, endpoint, options, this);
    listeningSession->Begin();

    m_listeningSessionQueue.CompleteOperation(std::move(listeningSession));

    return true;
  }
  virtual void OnIOError(Azure::Core::Amqp::_internal::Connection const&) override {}

  // Inherited via SessionEvents
  virtual bool OnLinkAttached(
      Azure::Core::Amqp::_internal::Session const& session,
      Azure::Core::Amqp::_internal::LinkEndpoint& newLinkInstance,
      std::string const& name,
      Azure::Core::Amqp::_internal::SessionRole,
      Azure::Core::Amqp::Models::AmqpValue const& source,
      Azure::Core::Amqp::Models::AmqpValue const& target,
      Azure::Core::Amqp::Models::AmqpValue const& properties) override
  {
    GTEST_LOG_(INFO) << "OnLinkAttached - Link attached to session.";
    MessageReceiverOptions receiverOptions;
    Azure::Core::Amqp::Models::_internal::MessageTarget messageTarget(target);
    Azure::Core::Amqp::Models::_internal::MessageSource messageSource(source);
    receiverOptions.MessageTarget = messageTarget;
    receiverOptions.Name = name;
    receiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    receiverOptions.DynamicAddress = messageSource.GetDynamic();
    receiverOptions.EnableTrace = true;
    auto receiver = std::make_unique<MessageReceiver>(
        session,
        newLinkInstance,
        static_cast<std::string>(messageSource.GetAddress()),
        receiverOptions,
        this);
    GTEST_LOG_(INFO) << "Opening the message receiver.";
    receiver->Open();
    m_messageReceiverQueue.CompleteOperation(std::move(receiver));
    (void)properties;
    return true;
  }
  virtual Azure::Core::Amqp::Models::AmqpValue OnMessageReceived(
      Azure::Core::Amqp::_internal::MessageReceiver const&,
      Azure::Core::Amqp::Models::AmqpMessage const& message) override
  {
    GTEST_LOG_(INFO) << "Message received";
    m_messageQueue.CompleteOperation(message);

    return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryAccepted();
  }
  void OnMessageReceiverStateChanged(
      MessageReceiver const& receiver,
      MessageReceiverState newState,
      MessageReceiverState oldState) override
  {
    GTEST_LOG_(INFO) << "OnMessageReceiverStateChanged";
    (void)receiver;
    (void)newState;
    (void)oldState;
  }
};
} // namespace MessageTests
#endif

TEST_F(TestMessages, ReceiverOpenClose)
{
  uint16_t testPort = FindAvailableSocket();

  GTEST_LOG_(INFO) << "Test port: " << testPort;

  MessageTests::MessageListenerEvents events;
  ConnectionOptions connectionOptions;
  //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
  connectionOptions.EnableTrace = true;
  connectionOptions.Port = testPort;
  Connection connection("localhost", connectionOptions);
  Session session(connection, nullptr);

  Azure::Core::Amqp::Network::_internal::SocketListener listener(testPort, &events);

  Azure::Core::Context context;

  // Ensure that the thread is started before we start using the message sender.
  std::mutex threadRunningMutex;
  std::condition_variable threadStarted;
  bool running = false;

  std::thread listenerThread([&]() {
    listener.Start();
    running = true;
    threadStarted.notify_one();

    auto listeningConnection = events.WaitForConnection(listener, context);

    listener.Stop();
  });

  std::unique_lock<std::mutex> waitForThreadStart(threadRunningMutex);
  threadStarted.wait(waitForThreadStart, [&running]() { return running == true; });
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
    };

    ReceiverEvents receiverEvents;
    MessageReceiverOptions options;
    options.Name = "Test Receiver";
    MessageReceiver receiver(session, "MyTarget", options, &receiverEvents);

    EXPECT_NO_THROW(receiver.Open());
    EXPECT_EQ("Test Receiver", receiver.GetLinkName());

    receiver.Close();
  }

  context.Cancel();

  listenerThread.join();
}

TEST_F(TestMessages, SenderOpenClose)
{
  uint16_t testPort = FindAvailableSocket();
  GTEST_LOG_(INFO) << "Test port: " << testPort;
  ConnectionOptions connectionOptions;
  connectionOptions.IdleTimeout = std::chrono::minutes(5);
  connectionOptions.Port = testPort;

  Connection connection("localhost", connectionOptions);
  Session session(connection, nullptr);
  //  Link link(session, "MySession", SessionRole::Receiver, "MySource", "MyTarget");

  Azure::Core::Amqp::Network::_internal::SocketListener listener(testPort, nullptr);
  EXPECT_NO_THROW(listener.Start());
  {
    MessageSenderOptions options;
    options.MessageSource = "MySource";

    MessageSender sender(session, "MyTarget", options, nullptr);
    sender.Open();
    sender.Close();
  }
  connection.Close("Test complete", "", Models::AmqpValue());
  listener.Stop();
}

TEST_F(TestMessages, TestLocalhostVsTls)
{
  MessageTests::AmqpServerMock mockServer(5671);

  mockServer.StartListening();

  ConnectionOptions connectionOptions;
  //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
  connectionOptions.ContainerId = "some";
  //  connectionOptions.EnableTrace = true;
  connectionOptions.Port = mockServer.GetPort();
  Connection connection("localhost", connectionOptions);
  Session session(connection, nullptr);

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
    };

    SenderEvents senderEvents;
    MessageSenderOptions options;
    options.Name = "sender-link";
    options.MessageSource = "ingress";
    options.SettleMode = SenderSettleMode::Settled;
    options.MaxMessageSize = 65536;
    MessageSender sender(session, "localhost/ingress", options, &senderEvents);
    EXPECT_NO_THROW(sender.Open());

    Azure::Core::Amqp::Models::AmqpMessage message;
    message.SetBody(Azure::Core::Amqp::Models::AmqpBinaryData{'h', 'e', 'l', 'l', 'o'});

    Azure::Core::Context context;
    Azure::Core::Amqp::Common::_internal::
        AsyncOperationQueue<MessageSendStatus, Azure::Core::Amqp::Models::AmqpValue>
            sendCompleteQueue;
    sender.QueueSend(
        message,
        [&](MessageSendStatus sendResult, Azure::Core::Amqp::Models::AmqpValue deliveryStatus) {
          GTEST_LOG_(INFO) << "Send Complete!";
          sendCompleteQueue.CompleteOperation(sendResult, deliveryStatus);
        });
    try
    {

      auto result = sendCompleteQueue.WaitForPolledResult(context, connection);
      // Because we're trying to use TLS to connect to a non-TLS port, we should get an error
      // sending the message.
      EXPECT_EQ(std::get<0>(*result), MessageSendStatus::Error);
    }
    catch (Azure::Core::OperationCancelledException const&)
    {
      GTEST_LOG_(INFO) << "Operation cancelled.";
    }
    catch (std::runtime_error const& ex)
    {
      // The WaitForPolledResult call can throw an exception if the connection enters the "End"
      // state.
      GTEST_LOG_(INFO) << "Exception: " << ex.what();
    }
    sender.Close();
  }
  connection.Close("", "", Models::AmqpValue());
  mockServer.StopListening();
}

TEST_F(TestMessages, SenderSendAsync)
{
  uint16_t testPort = FindAvailableSocket();

  GTEST_LOG_(INFO) << "Test port: " << testPort;

  ConnectionOptions connectionOptions;
  //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
  connectionOptions.ContainerId = "some";
  //  connectionOptions.EnableTrace = true;
  connectionOptions.Port = testPort;
  Connection connection("localhost", connectionOptions);
  Session session(connection, nullptr);

  // Set up a 30 second deadline on the receiver.
  Azure::Core::Context receiveContext = Azure::Core::Context::ApplicationContext.WithDeadline(
      Azure::DateTime::clock::now() + std::chrono::seconds(15));

  // Ensure that the thread is started before we start using the message sender.
  std::mutex threadRunningMutex;
  std::condition_variable threadStarted;
  bool running = false;

  std::thread listenerThread([&]() {
    try
    {

      MessageTests::MessageListenerEvents events;
      Azure::Core::Amqp::Network::_internal::SocketListener listener(testPort, &events);
      ASSERT_NO_THROW(listener.Start());

      running = true;
      threadStarted.notify_one();

      auto listeningConnection = events.WaitForConnection(listener, receiveContext);
      auto listeningSession = events.WaitForSession(receiveContext);
      auto messageReceiver = events.WaitForReceiver(receiveContext);
      GTEST_LOG_(INFO) << "Message receiver opened, waiting for incoming message.";

      auto message = events.WaitForMessage(receiveContext);
      GTEST_LOG_(INFO) << "Received incoming message!!";
      listener.Stop();
    }
    catch (std::exception const& ex)
    {
      GTEST_LOG_(INFO) << std::string("Exception thrown in listener thread. ") + ex.what();
    }
  });

  std::unique_lock<std::mutex> waitForThreadStart(threadRunningMutex);
  threadStarted.wait(waitForThreadStart, [&running]() { return running == true; });

  {
    class SenderEvents : public MessageSenderEvents {
      virtual void OnMessageSenderStateChanged(
          MessageSender const& sender,
          MessageSenderState newState,
          MessageSenderState oldState) override
      {
        GTEST_LOG_(INFO) << "MessageSenderEvents::OnMessageSenderSTateChanged.";
        (void)sender;
        (void)newState;
        (void)oldState;
      }
    };

    SenderEvents senderEvents;
    MessageSenderOptions options;
    options.Name = "sender-link";
    options.MessageSource = "ingress";
    options.SettleMode = SenderSettleMode::Settled;
    options.MaxMessageSize = 65536;
    MessageSender sender(session, "localhost/ingress", options, &senderEvents);
    EXPECT_NO_THROW(sender.Open());

    Azure::Core::Amqp::Models::AmqpMessage message;
    message.SetBody(Azure::Core::Amqp::Models::AmqpBinaryData{'h', 'e', 'l', 'l', 'o'});

    Azure::Core::Context context;
    Azure::Core::Amqp::Common::_internal::
        AsyncOperationQueue<MessageSendStatus, Azure::Core::Amqp::Models::AmqpValue>
            sendCompleteQueue;
    sender.QueueSend(
        message,
        [&](MessageSendStatus sendResult, Azure::Core::Amqp::Models::AmqpValue deliveryStatus) {
          GTEST_LOG_(INFO) << "Send Complete!";
          sendCompleteQueue.CompleteOperation(sendResult, deliveryStatus);
        });
    auto result = sendCompleteQueue.WaitForPolledResult(context, connection);
    EXPECT_EQ(std::get<0>(*result), MessageSendStatus::Ok);

    sender.Close();
  }
  receiveContext.Cancel();
  listenerThread.join();
  connection.Close("", "", Models::AmqpValue());
}

TEST_F(TestMessages, SenderSendSync)
{
  ConnectionOptions connectionOptions;

  uint16_t testPort = FindAvailableSocket();
  GTEST_LOG_(INFO) << "Test port: " << testPort;

  //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
  connectionOptions.ContainerId = "some";
  connectionOptions.Port = testPort;
  Connection connection("localhost", connectionOptions);
  Session session(connection, nullptr);

  Azure::Core::Context receiveContext;

  // Ensure that the thread is started before we start using the message sender.
  std::mutex threadRunningMutex;
  std::condition_variable threadStarted;
  bool running = false;

  std::thread listenerThread([&]() {
    try
    {

      MessageTests::MessageListenerEvents events;
      Azure::Core::Amqp::Network::_internal::SocketListener listener(testPort, &events);
      EXPECT_NO_THROW(listener.Start());

      running = true;
      threadStarted.notify_one();

      auto listeningConnection = events.WaitForConnection(listener, receiveContext);
      auto listeningSession = events.WaitForSession(receiveContext);
      auto messageReceiver = events.WaitForReceiver(receiveContext);
      GTEST_LOG_(INFO) << "Message receiver opened, waiting for incoming message.";

      auto message = events.WaitForMessage(receiveContext);
      GTEST_LOG_(INFO) << "Received incoming message!!";

      listener.Stop();
    }
    catch (std::exception const& ex)
    {
      GTEST_LOG_(ERROR) << std::string("Exception thrown in listener thread. ") + ex.what();
    }
  });

  // Block waiting until the listening thread has called listener.Start() to ensure that we don't
  // race with the listener startup.
  std::unique_lock<std::mutex> waitForThreadStart(threadRunningMutex);
  threadStarted.wait(waitForThreadStart, [&running]() { return running == true; });

  {
    MessageSenderOptions options;
    options.SettleMode = SenderSettleMode::Settled;
    options.MaxMessageSize = 65536;
    options.MessageSource = "ingress";
    options.Name = "sender-link";
    MessageSender sender(session, "localhost/ingress", options, nullptr);
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
  listenerThread.join();
}

TEST_F(TestMessages, AuthenticatedSender)
{
  MessageTests::AmqpServerMock server;

  auto sasCredential = std::make_shared<ServiceBusSasConnectionStringCredential>(
      "Endpoint=amqp://localhost:" + std::to_string(server.GetPort())
      + "/;SharedAccessKeyName=MyTestKey;SharedAccessKey=abcdabcd;EntityPath=testLocation");

  ConnectionOptions connectionOptions;

  //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
  connectionOptions.ContainerId = "some";
  connectionOptions.Port = server.GetPort();
  Connection connection("localhost", connectionOptions);
  Session session(connection, sasCredential);

  server.StartListening();

  MessageSenderOptions senderOptions;
  senderOptions.Name = "sender-link";
  senderOptions.MessageSource = "ingress";
  senderOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
  senderOptions.MaxMessageSize = 65536;
  senderOptions.Name = "sender-link";
  MessageSender sender(
      session,
      sasCredential->GetEndpoint() + sasCredential->GetEntityPath(),
      senderOptions,
      nullptr);

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
  Connection connection(hostName, connectionOptions);
  Session session(connection, tokenCredential);

  server.StartListening();
  MessageSenderOptions senderOptions;
  senderOptions.Name = "sender-link";
  senderOptions.MessageSource = "ingress";
  senderOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
  senderOptions.MaxMessageSize = 65536;
  senderOptions.Name = "sender-link";
  MessageSender sender(session, endpoint, senderOptions, nullptr);
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
    void SetSenderNodeName(std::string const& senderNodeName) { m_senderNodeName = senderNodeName; }

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
  connectionOptions.ContainerId = "some";
  connectionOptions.Port = server.GetPort();
  Connection connection("localhost", connectionOptions);
  Session session(connection, sasCredential);

  server.SetSenderNodeName(sasCredential->GetEndpoint() + sasCredential->GetEntityPath());
  server.StartListening();

  MessageReceiverOptions receiverOptions;
  receiverOptions.Name = "receiver-link";
  receiverOptions.MessageTarget = "egress";
  receiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
  receiverOptions.MaxMessageSize = 65536;
  receiverOptions.Name = "receiver-link";
  receiverOptions.EnableTrace = true;
  MessageReceiver receiver(
      session,
      sasCredential->GetEndpoint() + sasCredential->GetEntityPath(),
      receiverOptions,
      nullptr);
  EXPECT_TRUE(receiver);

  receiver.Open();

  // Send a message.
  {
    server.ShouldSendMessage(true);
    auto message = receiver.WaitForIncomingMessage();
    EXPECT_EQ(static_cast<std::string>(message.GetBodyAsAmqpValue()), "This is a message body.");
  }

  {
    Azure::Core::Context receiveContext;
    receiveContext.Cancel();
    auto message = receiver.WaitForIncomingMessage(receiveContext);
    (void)message;
  }
  receiver.Close();
  server.StopListening();
}

TEST_F(TestMessages, AuthenticatedReceiverAzureToken)
{
  class ReceiverMock : public MessageTests::AmqpServerMock {
  public:
    void ShouldSendMessage(bool shouldSend) { m_shouldSendMessage = shouldSend; }
    void SetSenderNodeName(std::string const& senderNodeName) { m_senderNodeName = senderNodeName; }

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
  Connection connection(hostName, connectionOptions);
  Session session(connection, tokenCredential);

  server.SetSenderNodeName(endpoint);
  server.StartListening();

  MessageReceiverOptions receiverOptions;
  receiverOptions.Name = "receiver-link";
  receiverOptions.MessageTarget = "egress";
  receiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
  receiverOptions.MaxMessageSize = 65536;
  receiverOptions.Name = "receiver-link";
  MessageReceiver receiver(session, endpoint, receiverOptions, nullptr);

  receiver.Open();

  // Send a message.
  {
    server.ShouldSendMessage(true);
    auto message = receiver.WaitForIncomingMessage();
    EXPECT_EQ(static_cast<std::string>(message.GetBodyAsAmqpValue()), "This is a message body.");
  }

  {
    Azure::Core::Context receiveContext;
    receiveContext.Cancel();
    auto message = receiver.WaitForIncomingMessage(receiveContext);
    //    EXPECT_FALSE(message);
    (void)message;
  }
  receiver.Close();
  server.StopListening();
}
#endif // !defined(AZ_PLATFORM_MAC)
