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
#include <functional>
#include <random>

#include "mock_amqp_server.hpp"

extern uint16_t FindAvailableSocket();

class TestMessages : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

using namespace Azure::Core::_internal::Amqp;
using namespace Azure::Core::Amqp;

TEST_F(TestMessages, SimpleReceiver)
{

  // Create a connection
  Connection connection("amqp://localhost:5672", nullptr, {});
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

TEST_F(TestMessages, SimpleSender)
{

  // Create a connection
  Connection connection("amqp://localhost:5672", nullptr, {});
  // Create a session
  Session session(connection, nullptr);
  //  Link link(session, "MySession", SessionRole::Sender, "MySource", "MyTarget");

  {
    MessageSender sender(session, "MySource", connection, {}, nullptr);
  }
  {
    MessageSender sender1(session, "MySource", connection, {}, nullptr);
    MessageSender sender2(session, "MySource", connection, {}, nullptr);
  }
}

TEST_F(TestMessages, ReceiverProperties)
{ // Create a connection
  Connection connection("amqp://localhost:5672", nullptr, {});
  Session session(connection, nullptr);

  {
    MessageReceiver receiver(session, "MyTarget", {});
  }
}

namespace MessageTests {
#if 1

class MessageListenerEvents : public Azure::Core::_internal::Amqp::Network::SocketListenerEvents,
                              public Azure::Core::_internal::Amqp::ConnectionEvents,
                              public Azure::Core::_internal::Amqp::SessionEvents,
                              public Azure::Core::_internal::Amqp::MessageReceiverEvents {
public:
  MessageListenerEvents() = default;

  std::shared_ptr<Azure::Core::_internal::Amqp::Connection> WaitForConnection(
      Azure::Core::_internal::Amqp::Network::SocketListener const& listener,
      Azure::Core::Context context)
  {
    auto result = m_listeningQueue.WaitForPolledResult(context, listener);
    if (result)
    {
      return std::move(std::get<0>(*result));
    }
    return nullptr;
  }
  std::unique_ptr<Azure::Core::_internal::Amqp::Session> WaitForSession(
      Azure::Core::Context context)
  {
    auto result = m_listeningSessionQueue.WaitForPolledResult(context, *m_connectionToPoll);
    if (result)
    {
      return std::move(std::get<0>(*result));
    }
    return nullptr;
  }
  std::unique_ptr<MessageReceiver> WaitForReceiver(Azure::Core::Context context)
  {
    auto result = m_messageReceiverQueue.WaitForPolledResult(context, *m_connectionToPoll);
    if (result)
    {
      return std::move(std::get<0>(*result));
    }
    return nullptr;
  }
  Azure::Core::Amqp::Models::Message WaitForMessage(Azure::Core::Context context)
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
      std::shared_ptr<Azure::Core::_internal::Amqp::Connection>>
      m_listeningQueue;
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
      std::unique_ptr<Azure::Core::_internal::Amqp::Session>>
      m_listeningSessionQueue;
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<std::unique_ptr<MessageReceiver>>
      m_messageReceiverQueue;
  Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<Azure::Core::Amqp::Models::Message>
      m_messageQueue;
  std::shared_ptr<Connection> m_connectionToPoll;

  // Inherited via MessageReceiver

  // Inherited via SocketListenerEvents.
  virtual void OnSocketAccepted(XIO_INSTANCE_TAG* xio) override
  {
    GTEST_LOG_(INFO) << "OnSocketAccepted - Socket connection received.";
    std::shared_ptr<Azure::Core::_internal::Amqp::Network::Transport> amqpTransport{
        std::make_shared<Azure::Core::_internal::Amqp::Network::AmqpHeaderTransport>(xio, nullptr)};
    Azure::Core::_internal::Amqp::ConnectionOptions options;
    //    options.IdleTimeout = std::chrono::minutes(5);
    options.ContainerId = "some";
    options.EnableTrace = true;
    m_connectionToPoll
        = std::make_shared<Azure::Core::_internal::Amqp::Connection>(amqpTransport, this, options);
    m_connectionToPoll->Listen();
    m_listeningQueue.CompleteOperation(m_connectionToPoll);
  }

  // Inherited via ConnectionEvents
  virtual void OnConnectionStateChanged(
      Azure::Core::_internal::Amqp::Connection const&,
      ConnectionState newState,
      ConnectionState oldState) override
  {
    (void)oldState;
    (void)newState;
  }
  virtual bool OnNewEndpoint(
      Azure::Core::_internal::Amqp::Connection const& connection,
      Endpoint& endpoint) override
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
  virtual void OnEndpointFrameReceived(
      Connection const&,
      Azure::Core::Amqp::Models::Value const&,
      uint32_t,
      uint8_t*) override
  {
  }

  // Inherited via SessionEvents
  virtual bool OnLinkAttached(
      Azure::Core::_internal::Amqp::Session const& session,
      Azure::Core::_internal::Amqp::LinkEndpoint& newLinkInstance,
      std::string const& name,
      Azure::Core::_internal::Amqp::SessionRole,
      Azure::Core::Amqp::Models::Value source,
      Azure::Core::Amqp::Models::Value target,
      Azure::Core::Amqp::Models::Value properties) override
  {
    GTEST_LOG_(INFO) << "OnLinkAttached - Link attached to session.";
    MessageReceiverOptions receiverOptions;
    Azure::Core::Amqp::Models::_internal::MessageTarget messageTarget(target);
    Azure::Core::Amqp::Models::_internal::MessageSource messageSource(source);
    receiverOptions.TargetAddress = static_cast<std::string>(messageTarget.GetAddress());
    receiverOptions.Name = name;
    receiverOptions.SettleMode = Azure::Core::_internal::Amqp::ReceiverSettleMode::First;
    receiverOptions.DynamicAddress = messageSource.GetDynamic();
    auto receiver = std::make_unique<MessageReceiver>(
        session,
        newLinkInstance,
        static_cast<std::string>(messageSource.GetAddress()),
        receiverOptions,
        this);
    receiver->SetTrace(true);
    GTEST_LOG_(INFO) << "Opening the message receiver.";
    receiver->Open();
    m_messageReceiverQueue.CompleteOperation(std::move(receiver));
    (void)properties;
    return true;
  }
  virtual Azure::Core::Amqp::Models::Value OnMessageReceived(
      Azure::Core::Amqp::Models::Message message) override
  {
    GTEST_LOG_(INFO) << "Message received";
    m_messageQueue.CompleteOperation(std::move(message));

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
  Connection connection("amqp://localhost:" + std::to_string(testPort), &events, connectionOptions);
  connection.SetTrace(true);
  Session session(connection, nullptr);

  Azure::Core::_internal::Amqp::Network::SocketListener listener(testPort, &events);

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

      Models::Value OnMessageReceived(Models::Message) override { return Models::Value(); }
    };

    ReceiverEvents receiverEvents;
    MessageReceiver receiver(session, "MyTarget", {}, &receiverEvents);

    EXPECT_NO_THROW(receiver.Open());
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
  //  connectionOptions.IdleTimeout = std::chrono::minutes(5);

  Connection connection("amqp://localhost:" + std::to_string(testPort), nullptr, connectionOptions);
  Session session(connection, nullptr);
  //  Link link(session, "MySession", SessionRole::Receiver, "MySource", "MyTarget");

  Azure::Core::_internal::Amqp::Network::SocketListener listener(testPort, nullptr);
  EXPECT_NO_THROW(listener.Start());
  {
    MessageSenderOptions options;
    options.SourceAddress = "MySource";

    MessageSender sender(session, "MyTarget", connection, options, nullptr);
    sender.Open();
    sender.Close();
  }
  connection.Close("Test complete", "", Models::Value());
  listener.Stop();
}

TEST_F(TestMessages, SenderSendAsync)
{
  uint16_t testPort = FindAvailableSocket();

  GTEST_LOG_(INFO) << "Test port: " << testPort;

  ConnectionOptions connectionOptions;
  //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
  connectionOptions.ContainerId = "some";
  //  connectionOptions.EnableTrace = true;
  Connection connection("amqp://localhost:" + std::to_string(testPort), nullptr, connectionOptions);
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
      Azure::Core::_internal::Amqp::Network::SocketListener listener(testPort, &events);
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
    options.SourceAddress = "ingress";
    options.SettleMode = SenderSettleMode::Settled;
    options.MaxMessageSize = 65536;
    MessageSender sender(session, "localhost/ingress", connection, options, &senderEvents);
    EXPECT_NO_THROW(sender.Open());

    uint8_t messageBody[] = "hello";

    Azure::Core::Amqp::Models::Message message;
    message.AddBodyAmqpData({messageBody, sizeof(messageBody)});

    Azure::Core::Context context;
    Azure::Core::Amqp::Common::_internal::
        AsyncOperationQueue<MessageSendResult, Azure::Core::Amqp::Models::Value>
            sendCompleteQueue;
    sender.SendAsync(
        message,
        [&](MessageSendResult sendResult, Azure::Core::Amqp::Models::Value deliveryStatus) {
          GTEST_LOG_(INFO) << "Send Complete!";
          sendCompleteQueue.CompleteOperation(sendResult, deliveryStatus);
        });
    auto result = sendCompleteQueue.WaitForPolledResult(context, connection);
    EXPECT_EQ(std::get<0>(*result), MessageSendResult::Ok);

    sender.Close();
  }
  receiveContext.Cancel();
  listenerThread.join();
  connection.Close("", "", Models::Value());
}

TEST_F(TestMessages, SenderSendSync)
{
  ConnectionOptions connectionOptions;

  uint16_t testPort = FindAvailableSocket();
  GTEST_LOG_(INFO) << "Test port: " << testPort;

  //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
  connectionOptions.ContainerId = "some";
  Connection connection("amqp://localhost:" + std::to_string(testPort), nullptr, connectionOptions);
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
      Azure::Core::_internal::Amqp::Network::SocketListener listener(testPort, &events);
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
    options.SourceAddress = "ingress";
    options.Name = "sender-link";
    MessageSender sender(session, "localhost/ingress", connection, options, nullptr);
    EXPECT_NO_THROW(sender.Open());

    uint8_t messageBody[] = "hello";

    Azure::Core::Amqp::Models::Message message;
    message.AddBodyAmqpData({messageBody, sizeof(messageBody)});

    Azure::Core::Amqp::Common::_internal::
        AsyncOperationQueue<MessageSendResult, Azure::Core::Amqp::Models::Value>
            sendCompleteQueue;
    auto result = sender.Send(message);
    EXPECT_EQ(std::get<0>(result), MessageSendResult::Ok);

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
  connectionOptions.HostName = sasCredential->GetHostName();
  connectionOptions.Port = sasCredential->GetPort();
  Connection connection(sasCredential->GetTransport(), nullptr, connectionOptions);
  Session session(connection, nullptr);

  server.StartListening();

  MessageSenderOptions senderOptions;
  senderOptions.Name = "sender-link";
  senderOptions.SourceAddress = "ingress";
  senderOptions.SettleMode = Azure::Core::_internal::Amqp::SenderSettleMode::Settled;
  senderOptions.MaxMessageSize = 65536;
  senderOptions.Name = "sender-link";
  MessageSender sender(
      session,
      sasCredential,
      sasCredential->GetEndpoint() + sasCredential->GetEntityPath(),
      connection,
      senderOptions,
      nullptr);

  sender.Open();
  uint8_t messageBody[] = "hello";

  Azure::Core::Amqp::Models::Message message;
  message.AddBodyAmqpData({messageBody, sizeof(messageBody)});
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
  connectionOptions.HostName = hostName;
  connectionOptions.Port = port;
  Connection connection(endpoint, nullptr, connectionOptions);
  Session session(connection, nullptr);

  server.StartListening();
  MessageSenderOptions senderOptions;
  senderOptions.Name = "sender-link";
  senderOptions.SourceAddress = "ingress";
  senderOptions.SettleMode = Azure::Core::_internal::Amqp::SenderSettleMode::Settled;
  senderOptions.MaxMessageSize = 65536;
  senderOptions.Name = "sender-link";
  MessageSender sender(session, tokenCredential, endpoint, connection, senderOptions, nullptr);
  EXPECT_TRUE(sender);
  sender.Open();
  uint8_t messageBody[] = "hello";

  Azure::Core::Amqp::Models::Message message;
  message.AddBodyAmqpData({messageBody, sizeof(messageBody)});
  sender.Send(message);

  sender.Close();
  server.StopListening();
}

TEST_F(TestMessages, AuthenticatedReceiver)
{
  class ReceiverMock : public MessageTests::AmqpServerMock {
  public:
    void ShouldSendMessage(bool shouldSend) { m_shouldSendMessage = shouldSend; }

  private:
    mutable bool m_shouldSendMessage{false};

    void Poll() const override
    {
      if (m_shouldSendMessage)
      {
        Azure::Core::Amqp::Models::Message sendMessage;
        sendMessage.SetBodyAmqpValue("This is a message body.");
        if (m_messageSender)
        {
          m_messageSender->Send(sendMessage);
          m_shouldSendMessage = false;
        }
      }
    }
  };

  ReceiverMock server;

  auto sasCredential = std::make_shared<ServiceBusSasConnectionStringCredential>(
      "Endpoint=amqp://localhost:" + std::to_string(server.GetPort())
      + ";SharedAccessKeyName=MyTestKey;SharedAccessKey=abcdabcd;EntityPath=testLocation");

  ConnectionOptions connectionOptions;

  //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
  connectionOptions.ContainerId = "some";
  connectionOptions.HostName = sasCredential->GetHostName();
  connectionOptions.Port = sasCredential->GetPort();
  Connection connection(sasCredential->GetTransport(), nullptr, connectionOptions);
  Session session(connection, nullptr);

  server.StartListening();

  MessageReceiverOptions receiverOptions;
  receiverOptions.Name = "receiver-link";
  receiverOptions.TargetAddress = "egress";
  receiverOptions.SettleMode = Azure::Core::_internal::Amqp::ReceiverSettleMode::First;
  receiverOptions.MaxMessageSize = 65536;
  receiverOptions.Name = "receiver-link";
  MessageReceiver receiver(
      session,
      connection,
      sasCredential,
      sasCredential->GetEndpoint() + sasCredential->GetEntityPath(),
      receiverOptions,
      nullptr);
  EXPECT_TRUE(receiver);

  receiver.Open();

  // Send a message.
  {
    server.ShouldSendMessage(true);
    auto message = receiver.WaitForIncomingMessage(connection);
    EXPECT_TRUE(message);
    EXPECT_EQ(static_cast<std::string>(message.GetBodyAmqpValue()), "This is a message body.");
  }

  {
    Azure::Core::Context receiveContext;
    receiveContext.Cancel();
    auto message = receiver.WaitForIncomingMessage(connection, receiveContext);
    EXPECT_FALSE(message);
  }
  receiver.Close();
  server.StopListening();
}

TEST_F(TestMessages, AuthenticatedReceiverAzureToken)
{
  class ReceiverMock : public MessageTests::AmqpServerMock {
  public:
    void ShouldSendMessage(bool shouldSend) { m_shouldSendMessage = shouldSend; }

  private:
    mutable bool m_shouldSendMessage{false};

    void Poll() const override
    {
      if (m_shouldSendMessage)
      {
        Azure::Core::Amqp::Models::Message sendMessage;
        sendMessage.SetBodyAmqpValue("This is a message body.");
        if (m_messageSender)
        {
          m_messageSender->Send(sendMessage);
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
  };

  ReceiverMock server;

  auto tokenCredential = std::make_shared<AzureTokenCredential>();
  std::string hostName = "localhost";
  std::string entityPath = "testLocation";
  uint16_t port = server.GetPort();
  std::string endpoint = "amqp://" + hostName + ":" + std::to_string(port) + "/" + entityPath;

  ConnectionOptions connectionOptions;

  //  connectionOptions.IdleTimeout = std::chrono::minutes(5);
  connectionOptions.ContainerId = "some";
  connectionOptions.HostName = hostName;
  connectionOptions.Port = port;
  Connection connection(endpoint, nullptr, connectionOptions);
  Session session(connection, nullptr);

  server.StartListening();

  MessageReceiverOptions receiverOptions;
  receiverOptions.Name = "receiver-link";
  receiverOptions.TargetAddress = "egress";
  receiverOptions.SettleMode = Azure::Core::_internal::Amqp::ReceiverSettleMode::First;
  receiverOptions.MaxMessageSize = 65536;
  receiverOptions.Name = "receiver-link";
  MessageReceiver receiver(
      session, connection, tokenCredential, endpoint, receiverOptions, nullptr);

  receiver.Open();

  // Send a message.
  {
    server.ShouldSendMessage(true);
    auto message = receiver.WaitForIncomingMessage(connection);
    EXPECT_TRUE(message);
    EXPECT_EQ(static_cast<std::string>(message.GetBodyAmqpValue()), "This is a message body.");
  }

  {
    Azure::Core::Context receiveContext;
    receiveContext.Cancel();
    auto message = receiver.WaitForIncomingMessage(connection, receiveContext);
    EXPECT_FALSE(message);
  }
  receiver.Close();
  server.StopListening();
}
