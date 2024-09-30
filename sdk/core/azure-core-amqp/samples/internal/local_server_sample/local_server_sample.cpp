// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/amqp/internal/common/async_operation_queue.hpp>
#include <azure/core/amqp/internal/connection.hpp>
#include <azure/core/amqp/internal/message_receiver.hpp>
#include <azure/core/amqp/internal/models/message_source.hpp>
#include <azure/core/amqp/internal/models/message_target.hpp>
#include <azure/core/amqp/internal/models/messaging_values.hpp>
#include <azure/core/amqp/internal/network/amqp_header_detect_transport.hpp>
#include <azure/core/amqp/internal/network/socket_listener.hpp>

#include <iostream>
#include <memory>
#include <string>

using namespace Azure::Core::Amqp::_internal;
using namespace Azure::Core::Amqp;
namespace LocalServerSample {

class SampleEvents : public ConnectionEvents,
                     public ConnectionEndpointEvents,
                     public SessionEvents,
                     public MessageReceiverEvents,
                     public Network::_detail::SocketListenerEvents {
public:
  SampleEvents() {}

  std::unique_ptr<Connection> WaitForIncomingConnection(
      Network::_detail::SocketListener& listener,
      Azure::Core::Context const& context = {})
  {
    auto result = m_connectionQueue.WaitForPolledResult(context, listener);
    return std::move(std::get<0>(*result));
  }

  Session WaitForNewSession(Azure::Core::Context const& context = {})
  {
    auto result = m_sessionQueue.WaitForPolledResult(context, *m_connection);
    return std::move(std::get<0>(*result));
  }

  std::unique_ptr<MessageReceiver> WaitForMessageReceiver(Azure::Core::Context const& context = {})
  {
    auto result = m_messageReceiverQueue.WaitForPolledResult(context, *m_connection);
    return std::move(std::get<0>(*result));
  }

  // Wait for incoming messages. This method is somewhat more complicated because it
  // needs to wait on multiple waiters (both the connection and the transport).
  template <class... Waiters>
  std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> WaitForIncomingMessage(
      Azure::Core::Context const& context,
      Waiters&... waiters)
  {
    auto result = m_messageQueue.WaitForPolledResult(context, waiters...);
    return std::move(std::get<0>(*result));
  }

private:
  Connection* m_connection{};
  Common::_internal::AsyncOperationQueue<std::unique_ptr<Connection>> m_connectionQueue;
  Common::_internal::AsyncOperationQueue<Session> m_sessionQueue;
  Common::_internal::AsyncOperationQueue<std::unique_ptr<MessageReceiver>> m_messageReceiverQueue;
  Common::_internal::AsyncOperationQueue<std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage>>
      m_messageQueue;

  virtual void OnSocketAccepted(std::shared_ptr<Network::_internal::Transport> transport) override
  {
    std::cout << "OnSocketAccepted - Socket connection received." << std::endl;

    // Create an AMQP filter transport - this will filter out all incoming messages that don't have
    // an AMQP header.
    auto amqpTransport{
        Network::_internal::AmqpHeaderDetectTransportFactory::Create(transport, nullptr)};
    ConnectionOptions options;
    options.ContainerId = "some";
    options.EnableTrace = true;
    auto newConnection{std::make_unique<Connection>(amqpTransport, options, this, this)};
    m_connection = newConnection.get();
    m_connectionQueue.CompleteOperation(std::move(newConnection));
  }

  virtual bool OnNewEndpoint(Connection const& connection, Endpoint& endpoint) override
  {
    SessionOptions sessionOptions;
    sessionOptions.InitialIncomingWindowSize = 10000;

    auto newSession = connection.CreateSession(endpoint, sessionOptions, this);

    // The new session *must* call `Begin` before returning from the OnNewEndpoint callback.
    newSession.Begin({});
    m_sessionQueue.CompleteOperation(std::move(newSession));
    return true;
  }

  virtual void OnConnectionStateChanged(
      Connection const& connection,
      ConnectionState newState,
      ConnectionState oldState) override
  {
    std::cout << "Connection state changed. Was: " << oldState << " now: " << newState << std::endl;
    (void)connection;
  };

  virtual bool OnLinkAttached(
      Session const& sessionForLink,
      LinkEndpoint& newLink,
      std::string const& name,
      Azure::Core::Amqp::_internal::SessionRole,
      Azure::Core::Amqp::Models::AmqpValue const& source,
      Azure::Core::Amqp::Models::AmqpValue const& target,
      Azure::Core::Amqp::Models::AmqpValue const&) override
  {
    Azure::Core::Amqp::Models::_internal::MessageSource messageSource(source);
    Azure::Core::Amqp::Models::_internal::MessageTarget messageTarget(target);

    MessageReceiverOptions options;
    options.SettleMode = ReceiverSettleMode::First;
    options.EnableTrace = true;
    options.Name = name;
    options.MessageTarget = messageTarget;
    auto newMessageReceiver
        = std::make_unique<MessageReceiver>(sessionForLink.CreateMessageReceiver(
            newLink, static_cast<std::string>(messageSource.GetAddress()), options, this));
    newMessageReceiver->Open();
    m_messageReceiverQueue.CompleteOperation(std::move(newMessageReceiver));
    return true;
  }

  virtual void OnIOError(Connection const&) override
  {
    std::cerr << "I/O error has occurred, connection is invalid." << std::endl;
  };

  // Inherited via MessageReceiverEvents
  virtual void OnMessageReceiverStateChanged(
      MessageReceiver const& receiver,
      MessageReceiverState newState,
      MessageReceiverState oldState) override
  {
    std::cout << "Message Receiver state changed. Was: " << oldState << " now: " << newState
              << std::endl;
    (void)receiver;
  }
  virtual Azure::Core::Amqp::Models::AmqpValue OnMessageReceived(
      Azure::Core::Amqp::_internal::MessageReceiver const&,
      std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
  {
    m_messageQueue.CompleteOperation(message);
    return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryAccepted();
  }
  virtual void OnMessageReceiverDisconnected(
      MessageReceiver const&,
      Models::_internal::AmqpError const& error) override
  {
    std::cerr << "Message receiver error: " << error << std::endl;
  }
};

// Because the Connection.Listen method is private, we need to put the meat of the sample in a
// method other than "main".
int LocalServerSampleMain()
{
  SampleEvents sampleEvents;
  // Configure a socket listener on the AMQP port (5672).
  Network::_detail::SocketListener listener(5672, &sampleEvents);

  listener.Start();
  auto connection = sampleEvents.WaitForIncomingConnection(listener);
  connection->Listen();

  auto session = sampleEvents.WaitForNewSession();

  auto receiver = sampleEvents.WaitForMessageReceiver();

  while (true)
  {
    auto message = sampleEvents.WaitForIncomingMessage({}, listener, *connection);
    std::cout << "Received message." << message << std::endl;
  }

  return 0;
}
} // namespace LocalServerSample
int main() { LocalServerSample::LocalServerSampleMain(); }
