// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <azure/core/amqp/common/async_operation_queue.hpp>
#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/message_receiver.hpp>
#include <azure/core/amqp/models/message_source.hpp>
#include <azure/core/amqp/models/message_target.hpp>
#include <azure/core/amqp/models/messaging_values.hpp>
#include <azure/core/amqp/network/amqp_header_detect_transport.hpp>
#include <azure/core/amqp/network/socket_listener.hpp>
#include <iostream>
#include <string>

using namespace Azure::Core::Amqp::_internal;
using namespace Azure::Core::Amqp;

// Convert a ConnectionState enum to a string for diagnostic purposes.
const char* ConnectionStateToString(ConnectionState const state)
{
  switch (state)
  {
    case ConnectionState::Start:
      return "Start";
    case ConnectionState::HeaderReceived:
      return "HeaderReceived";
    case ConnectionState::HeaderSent:
      return "HeaderSent";
    case ConnectionState::HeaderExchanged:
      return "HeaderExchanged";
    case ConnectionState::OpenPipe:
      return "OpenPipe";
    case ConnectionState::OcPipe:
      return "OcPipe";
    case ConnectionState::OpenReceived:
      return "OpenReceived";
    case ConnectionState::OpenSent:
      return "OpenSent";
    case ConnectionState::ClosePipe:
      return "ClosePipe";
    case ConnectionState::Opened:
      return "Opened";
    case ConnectionState::CloseReceived:
      return "CloseReceived";
    case ConnectionState::CloseSent:
      return "CloseSent";
    case ConnectionState::Discarding:
      return "Discarding";
    case ConnectionState::End:
      return "End";
    case ConnectionState::Error:
      return "Error";
    default:
      return "Unknown";
  }
}

const char* MessageStateToString(MessageReceiverState const state)
{
  switch (state)
  {
    case MessageReceiverState::Closing:
      return "Closing";
    case MessageReceiverState::Invalid:
      return "Invalid";
    case MessageReceiverState::Error:
      return "Error";
    case MessageReceiverState::Opening:
      return "Opening";
    case MessageReceiverState::Idle:
      return "Idle";
    case MessageReceiverState::Open:
      return "Open";
    default:
      throw std::logic_error("Unknown message state");
  }
}

class SampleEvents : public ConnectionEvents,
                     public SessionEvents,
                     public MessageReceiverEvents,
                     public Network::_internal::SocketListenerEvents {
public:
  SampleEvents() {}

  std::unique_ptr<Connection> WaitForIncomingConnection(
      Network::_internal::SocketListener& listener,
      Azure::Core::Context context = {})
  {
    auto result = m_connectionQueue.WaitForPolledResult(context, listener);
    return std::move(std::get<0>(*result));
  }

  std::unique_ptr<Session> WaitForNewSession(Azure::Core::Context context = {})
  {
    auto result = m_sessionQueue.WaitForPolledResult(context, *m_connection);
    return std::move(std::get<0>(*result));
  }

  std::unique_ptr<MessageReceiver> WaitForMessageReceiver(Azure::Core::Context context = {})
  {
    auto result = m_messageReceiverQueue.WaitForPolledResult(context, *m_connection);
    return std::move(std::get<0>(*result));
  }

  // Wait for incoming messages. This method is somewhat more complicated because it
  // needs to wait on multiple waiters (both the connection and the transport).
  template <class... Waiters>
  Azure::Core::Amqp::Models::Message WaitForIncomingMessage(
      Azure::Core::Context context,
      Waiters&... waiters)
  {
    auto result = m_messageQueue.WaitForPolledResult(context, waiters...);
    return std::move(std::get<0>(*result));
  }

private:
  Connection* m_connection{};
  Common::_internal::AsyncOperationQueue<std::unique_ptr<Connection>> m_connectionQueue;
  Common::_internal::AsyncOperationQueue<std::unique_ptr<Session>> m_sessionQueue;
  Common::_internal::AsyncOperationQueue<std::unique_ptr<MessageReceiver>> m_messageReceiverQueue;
  Common::_internal::AsyncOperationQueue<Azure::Core::Amqp::Models::Message> m_messageQueue;

  virtual void OnSocketAccepted(XIO_INSTANCE_TAG* xio) override
  {
    std::cout << "OnSocketAccepted - Socket connection received." << std::endl;

    // Create an AMQP filter transport - this will filter out all incoming messages that don't have
    // an AMQP header.
    std::shared_ptr<Network::_internal::Transport> amqpTransport{
        std::make_shared<Network::_internal::AmqpHeaderTransport>(xio, nullptr)};
    ConnectionOptions options;
    //    options.IdleTimeout = std::chrono::minutes(5);
    options.ContainerId = "some";
    options.HostName = "localhost";
    auto newConnection{std::make_unique<Connection>(amqpTransport, options, this)};
    m_connection = newConnection.get();
    m_connectionQueue.CompleteOperation(std::move(newConnection));
  }

  virtual bool OnNewEndpoint(Connection const& connection, Endpoint& endpoint) override
  {
    std::unique_ptr<Session> newSession = std::make_unique<Session>(connection, endpoint, this);

    // The new session *must* call `Begin` before returning from the OnNewEndpoint callback.
    newSession->SetIncomingWindow(10000);
    newSession->Begin();
    m_sessionQueue.CompleteOperation(std::move(newSession));
    return true;
  }

  virtual void OnConnectionStateChanged(
      Connection const& connection,
      ConnectionState newState,
      ConnectionState oldState) override
  {
    std::cout << "Connection state changed. Was: " << ConnectionStateToString(oldState)
              << " now: " << ConnectionStateToString(newState) << std::endl;
    (void)connection;
  };

  virtual void OnEndpointFrameReceived(
      Connection const& connection,
      Azure::Core::Amqp::Models::Value const& value,
      uint32_t framePayloadSize,
      uint8_t* payloadBytes) override
  {
    (void)connection;
    (void)value, (void)framePayloadSize, (void)payloadBytes;
  }
  virtual bool OnLinkAttached(
      Session const& sessionForLink,
      LinkEndpoint& newLink,
      std::string const& name,
      Azure::Core::Amqp::_internal::SessionRole,
      Azure::Core::Amqp::Models::Value source,
      Azure::Core::Amqp::Models::Value target,
      Azure::Core::Amqp::Models::Value) override
  {
    Azure::Core::Amqp::Models::_internal::MessageSource messageSource(source);
    Azure::Core::Amqp::Models::_internal::MessageTarget messageTarget(target);

    MessageReceiverOptions options;
    options.SettleMode = ReceiverSettleMode::First;
    options.EnableTrace = true;
    options.Name = name;
    options.TargetAddress = static_cast<std::string>(messageTarget.GetAddress());
    auto newMessageReceiver = std::make_unique<MessageReceiver>(
        sessionForLink,
        newLink,
        static_cast<std::string>(messageSource.GetAddress()),
        options,
        this);
    newMessageReceiver->SetTrace(true);
    newMessageReceiver->Open();
    m_messageReceiverQueue.CompleteOperation(std::move(newMessageReceiver));
    return true;
  }

  virtual void OnIoError(Connection const&) override
  {
    std::cerr << "I/O error has occurred, connection is invalid." << std::endl;
  };

  // Inherited via MessageReceiverEvents
  virtual void OnMessageReceiverStateChanged(
      MessageReceiver const& receiver,
      MessageReceiverState newState,
      MessageReceiverState oldState) override
  {
    std::cout << "Message Receiver state changed. Was: " << MessageStateToString(oldState)
              << " now: " << MessageStateToString(newState) << std::endl;
    (void)receiver;
  }
  virtual Azure::Core::Amqp::Models::Value OnMessageReceived(
      Azure::Core::Amqp::Models::Message message) override
  {
    m_messageQueue.CompleteOperation(message);
    return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryAccepted();
  }
};

int main()
{
  SampleEvents sampleEvents;
  Network::_internal::SocketListener listener(5672, &sampleEvents);

  listener.Start();
  auto connection = sampleEvents.WaitForIncomingConnection(listener);
  connection->SetTrace(true);
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
