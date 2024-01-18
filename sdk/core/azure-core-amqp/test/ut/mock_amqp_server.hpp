// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include "azure/core/amqp/internal/models/amqp_error.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"

#include <azure/core/amqp/internal/claims_based_security.hpp>
#include <azure/core/amqp/internal/connection.hpp>
#include <azure/core/amqp/internal/link.hpp>
#include <azure/core/amqp/internal/message_receiver.hpp>
#include <azure/core/amqp/internal/message_sender.hpp>
#include <azure/core/amqp/internal/models/amqp_protocol.hpp>
#include <azure/core/amqp/internal/models/message_source.hpp>
#include <azure/core/amqp/internal/models/message_target.hpp>
#include <azure/core/amqp/internal/models/messaging_values.hpp>
#include <azure/core/amqp/internal/network/amqp_header_detect_transport.hpp>
#include <azure/core/amqp/internal/network/socket_listener.hpp>
#include <azure/core/amqp/internal/session.hpp>

#include <memory>

#include <gtest/gtest.h>

#define NEW_MOCK_SERVER 1

namespace Azure { namespace Core { namespace Amqp { namespace Tests {

  extern uint16_t FindAvailableSocket();
  namespace MessageTests {

#if NEW_MOCK_SERVER
    static std::ostream& operator<<(
        std::ostream& os,
        Azure::Core::Amqp::_internal::SessionRole role)
    {
      switch (role)
      {
        case Azure::Core::Amqp::_internal::SessionRole::Sender:
          os << "Sender";
          break;
        case Azure::Core::Amqp::_internal::SessionRole::Receiver:
          os << "Receiver";
          break;
        default:
          os << "Unknown";
          break;
      }
      return os;
    }
    struct MockServiceEndpointOptions
    {
      bool EnableTrace{true};
      Azure::Core::Context ListenerContext;
    };

    class MockServiceEndpoint : public Azure::Core::Amqp::_internal::MessageReceiverEvents,
                                public Azure::Core::Amqp::_internal::MessageSenderEvents {
    public:
      MockServiceEndpoint(std::string const& name, MockServiceEndpointOptions const& options)
          : m_listenerContext{options.ListenerContext},
            m_enableTrace{options.EnableTrace}, m_name{name}
      {
      }

      const std::string& GetName() const { return m_name; }

      bool OnLinkAttached(
          _internal::Session const& session,
          std::string const& linkName,
          _internal::LinkEndpoint& linkEndpoint,
          _internal::SessionRole role,
          Models::_internal::MessageSource const& source,
          Models::_internal::MessageTarget const& target)
      {
        GTEST_LOG_(INFO) << "MockServiceEndpoint::OnLinkAttached for name: " << m_name
                         << " Source : " << source << " Target : " << target;

        // If the incoming role is receiver, then we want to create a sender to talk to it.
        // Similarly, if the incoming role is sender, we want to create a receiver to receive
        // from it.
        if (role == Azure::Core::Amqp::_internal::SessionRole::Receiver)
        {
          GTEST_LOG_(INFO) << "Role is receiver, create sender.";
          if (!m_sender)
          {
            GTEST_LOG_(INFO) << "No sender found, create new.";
            Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;
            senderOptions.EnableTrace = m_enableTrace;
            senderOptions.Name = linkName;
            senderOptions.MessageSource = source;
            senderOptions.InitialDeliveryCount = 0;
            m_sender = std::make_unique<Azure::Core::Amqp::_internal::MessageSender>(
                session.CreateMessageSender(linkEndpoint, target, senderOptions, this));
            m_sender->Open();
          }
          else
          {
            GTEST_LOG_(INFO) << "Sender already created for target " << target;
            throw std::runtime_error(
                "Sender already created for target "
                + static_cast<std::string>(target.GetAddress()));
          }
        }
        else if (role == Azure::Core::Amqp::_internal::SessionRole::Sender)
        {
          GTEST_LOG_(INFO) << "Role is sender, create receiver.";
          if (!m_receiver)
          {
            GTEST_LOG_(INFO) << "No receiver found, create new.";
            Azure::Core::Amqp::_internal::MessageReceiverOptions receiverOptions;
            receiverOptions.EnableTrace = m_enableTrace;
            receiverOptions.Name = linkName;
            receiverOptions.MessageTarget = target;
            receiverOptions.InitialDeliveryCount = 0;
            m_receiver = std::make_unique<Azure::Core::Amqp::_internal::MessageReceiver>(
                session.CreateMessageReceiver(linkEndpoint, source, receiverOptions, this));
            GTEST_LOG_(INFO) << "Open new message receiver.";
            m_receiver->Open();
          }
          else
          {
            GTEST_LOG_(INFO) << "Receiver already created for source " << source;
            throw std::runtime_error(
                "Receiver already created for source "
                + static_cast<std::string>(source.GetAddress()));
          }
        }
        else
        {
          GTEST_LOG_(INFO) << "Unknown role: " << role;
          throw std::runtime_error(
              "Unknown role: "
              + std::to_string(static_cast<std::underlying_type_t<decltype(role)>>(role)));
        }

        // Spin up a thread to process incoming messages.
        if (!m_serverThread.joinable())
        {
          m_serverThread = std::thread([this]() { MessageLoop(); });
        }
        return true;
      }

      void StopProcessing()
      {
        if (m_serverThread.joinable())
        {
          m_listenerContext.Cancel();
          m_serverThread.join();
        }

        if (m_receiver)
        {
          m_receiver->Close();
        }
        if (m_sender)
        {
          m_sender->Close();
        }
      }

    protected:
      bool HasMessageSender() const { return static_cast<bool>(m_sender); }
      Azure::Core::Amqp::_internal::MessageSender& GetMessageSender() const
      {
        if (!m_sender)
        {
          throw std::runtime_error("Message sender not created.");
        }
        return *m_sender;
      }

      Azure::Core::Amqp::_internal::MessageReceiver& GetMessageReceiver() const
      {
        if (!m_receiver)
        {
          throw std::runtime_error("Message receiver not created.");
        }
        return *m_receiver;
      }

      virtual void Poll() const {}

      Azure::Core::Context& GetListenerContext() { return m_listenerContext; }

    private:
      Azure::Core::Context m_listenerContext; // Used to cancel the listener if necessary.
      bool m_enableTrace{true};
      std::string m_name;
      std::thread m_serverThread;
      std::unique_ptr<Azure::Core::Amqp::_internal::MessageSender> m_sender;
      std::unique_ptr<Azure::Core::Amqp::_internal::MessageReceiver> m_receiver;

      Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage>>
          m_messageQueue;
      Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool>
          m_messageSenderDisconnectedQueue;
      Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool>
          m_messageReceiverDisconnectedQueue;

      virtual void MessageReceived(
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message)
          = 0;

      void MessageLoop()
      {
        while (!m_listenerContext.IsCancelled())
        {
          Poll();
          auto message = m_messageQueue.TryWaitForResult();
          if (message)
          {

            GTEST_LOG_(INFO) << "Received message: " << *std::get<0>(*message);

            MessageReceived(std::get<0>(*message));
          }
          auto senderDisconnected = m_messageSenderDisconnectedQueue.TryWaitForResult();
          if (senderDisconnected)
          {
            GTEST_LOG_(INFO) << "Sender disconnected: " << std::get<0>(*senderDisconnected);
            m_sender->Close(m_listenerContext);
            m_sender.reset();
          }

          auto receiverDisconnected = m_messageReceiverDisconnectedQueue.TryWaitForResult();
          if (receiverDisconnected)
          {
            GTEST_LOG_(INFO) << "Receiver disconnected: " << std::get<0>(*receiverDisconnected);
            m_receiver->Close(m_listenerContext);
            m_receiver.reset();
          }

          if (!m_receiver && !m_sender)
          {
            GTEST_LOG_(INFO) << "No more links, exiting message loop.";
            break;
          }

          std::this_thread::yield();
        }
      }

      // Inherited via MessageReceiverEvents
      void OnMessageReceiverStateChanged(
          Azure::Core::Amqp::_internal::MessageReceiver const&,
          Azure::Core::Amqp::_internal::MessageReceiverState newState,
          Azure::Core::Amqp::_internal::MessageReceiverState oldState) override
      {
        GTEST_LOG_(INFO) << "MockServiceEndpoint(" << m_name
                         << "): Message Receiver State changed.Old state : " << oldState
                         << " New state: " << newState;
      }

    protected:
      Azure::Core::Amqp::Models::AmqpValue OnMessageReceived(
          Azure::Core::Amqp::_internal::MessageReceiver const&,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO) << "MockServiceEndpoint(" << m_name << ") Received a message " << *message;
        m_messageQueue.CompleteOperation(message);
        return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryAccepted();
      }

    private:
      virtual void OnMessageReceiverDisconnected(
          Azure::Core::Amqp::Models::_internal::AmqpError const& error) override
      {
        GTEST_LOG_(INFO) << "Message receiver disconnected: " << error << std::endl;
        m_messageReceiverDisconnectedQueue.CompleteOperation(true);
      }

      // Inherited via MessageSenderEvents
      void OnMessageSenderStateChanged(
          Azure::Core::Amqp::_internal::MessageSender const&,
          Azure::Core::Amqp::_internal::MessageSenderState newState,
          Azure::Core::Amqp::_internal::MessageSenderState oldState) override
      {
        GTEST_LOG_(INFO) << "MockServiceEndpoint(" << m_name
                         << ") Message Sender State changed.Old state : " << oldState
                         << " New state: " << newState;
      }

      void OnMessageSenderDisconnected(
          Azure::Core::Amqp::Models::_internal::AmqpError const& error) override
      {
        GTEST_LOG_(INFO) << "Message Sender Disconnected: Error: " << error;
        m_messageSenderDisconnectedQueue.CompleteOperation(true);
      }
    };

    class AmqpClaimBasedSecurity final : public MockServiceEndpoint {
    public:
      AmqpClaimBasedSecurity(MockServiceEndpointOptions const& options)
          : MockServiceEndpoint("$cbs", options)
      {
      }
      virtual ~AmqpClaimBasedSecurity() = default;

      void ForceCbsError(bool forceError) { m_forceCbsError = forceError; }

    private:
      bool m_forceCbsError{false};

      void MessageReceived(
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        if (IsCbsMessage(message))
        {
          ProcessCbsMessage(message);
        }
      }

      bool IsCbsMessage(std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message)
      {
        if (!message->ApplicationProperties.empty())
        {
          Azure::Core::Amqp::Models::AmqpValue operation
              = message->ApplicationProperties.at("operation");
          Azure::Core::Amqp::Models::AmqpValue type = message->ApplicationProperties.at("type");

          // If we're processing a put-token message, then we should get a "type" and "name"
          // value.
          EXPECT_EQ(operation.GetType(), Azure::Core::Amqp::Models::AmqpValueType::String);
          if (static_cast<std::string>(operation) == "put-token")
          {
            return true;
          }
          else if (static_cast<std::string>(operation) == "delete-token")
          {
            return true;
          }
        }
        return false;
      }

      void ProcessCbsMessage(std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message)
      {
        Azure::Core::Amqp::Models::AmqpValue operation
            = message->ApplicationProperties.at("operation");
        Azure::Core::Amqp::Models::AmqpValue type = message->ApplicationProperties.at("type");
        Azure::Core::Amqp::Models::AmqpValue name = message->ApplicationProperties.at("name");
        // If we're processing a put-token message, then we should get a "type" and "name"
        // value.
        EXPECT_EQ(operation.GetType(), Azure::Core::Amqp::Models::AmqpValueType::String);
        if (static_cast<std::string>(operation) == "put-token")
        {
          EXPECT_EQ(type.GetType(), Azure::Core::Amqp::Models::AmqpValueType::String);
          EXPECT_EQ(name.GetType(), Azure::Core::Amqp::Models::AmqpValueType::String);
          // The body of a put-token operation MUST be an AMQP AmqpValue.
          EXPECT_EQ(message->BodyType, Azure::Core::Amqp::Models::MessageBodyType::Value);

          // Respond to the operation.
          Azure::Core::Amqp::Models::AmqpMessage response;
          Azure::Core::Amqp::Models::MessageProperties responseProperties;

          // Management specification section 3.2: The correlation-id of the response message
          // MUST be the correlation-id from the request message (if present), else the
          // message-id from the request message.
          Azure::Nullable<Azure::Core::Amqp::Models::AmqpValue> requestCorrelationId
              = message->Properties.CorrelationId;
          if (!message->Properties.CorrelationId.HasValue())
          {
            requestCorrelationId = message->Properties.MessageId.Value();
          }
          response.Properties.CorrelationId = requestCorrelationId;

          // Populate the response application properties.

          if (m_forceCbsError)
          {
            response.ApplicationProperties["status-code"] = 500;
            response.ApplicationProperties["status-description"] = "Internal Server Error";
          }
          else
          {
            response.ApplicationProperties["status-code"] = 200;
            response.ApplicationProperties["status-description"] = "OK-put";
          }

          response.SetBody(Azure::Core::Amqp::Models::AmqpValue());

          // Set the response body type to an empty AMQP value.
          if (GetListenerContext().IsCancelled())
          {
            return;
          }
          try
          {
            auto result = GetMessageSender().Send(response, GetListenerContext());
            if (std::get<0>(result) != Azure::Core::Amqp::_internal::MessageSendStatus::Ok)
            {
              GTEST_LOG_(INFO) << "Failed to send CBS response: " << std::get<1>(result);
              return;
            }
          }
          catch (std::exception& ex)
          {
            GTEST_LOG_(INFO) << "Exception thrown sending CBS response: " << ex.what();
            return;
          }
        }
        else if (static_cast<std::string>(operation) == "delete-token")
        {
          Azure::Core::Amqp::Models::AmqpMessage response;
          Azure::Core::Amqp::Models::MessageProperties responseProperties;

          // Management specification section 3.2: The correlation-id of the response message
          // MUST be the correlation-id from the request message (if present), else the
          // message-id from the request message.
          Azure::Nullable<Azure::Core::Amqp::Models::AmqpValue> requestCorrelationId
              = message->Properties.CorrelationId;
          if (!message->Properties.CorrelationId.HasValue())
          {
            requestCorrelationId = message->Properties.MessageId;
          }
          response.Properties.CorrelationId = requestCorrelationId;
          response.ApplicationProperties["status-code"] = 200;
          response.ApplicationProperties["status-description"] = "OK-delete";

          response.SetBody(Azure::Core::Amqp::Models::AmqpValue());

          // Set the response body type to an empty AMQP value.
          if (GetListenerContext().IsCancelled())
          {
            return;
          }

          auto sendResult = GetMessageSender().Send(response, GetListenerContext());
          if (std::get<0>(sendResult) != Azure::Core::Amqp::_internal::MessageSendStatus::Ok)
          {
            GTEST_LOG_(INFO) << "Failed to send CBS response: " << std::get<1>(sendResult);
            return;
          }
        }
      }
    };

    class AmqpServerMock : public Azure::Core::Amqp::Network::_detail::SocketListenerEvents,
                           public Azure::Core::Amqp::_internal::ConnectionEvents,
                           public Azure::Core::Amqp::_internal::SessionEvents {
    public:
      AmqpServerMock(
          std::string name = testing::UnitTest::GetInstance()->current_test_info()->name())
          : m_connectionId{"Mock Server for " + name}, m_testPort{FindAvailableSocket()}
      {
        // Every server mock has CBS endpoint support
        MockServiceEndpointOptions options;
        options.EnableTrace = m_enableTrace;
        options.ListenerContext = m_listenerContext;
        AddServiceEndpoint(std::make_shared<AmqpClaimBasedSecurity>(options));
      }
      AmqpServerMock(
          uint16_t listeningPort,
          std::string name = testing::UnitTest::GetInstance()->current_test_info()->name())
          : m_connectionId{"Mock Server for " + name}, m_testPort{listeningPort}
      {
        // Every server mock has CBS endpoint support
        MockServiceEndpointOptions options;
        options.EnableTrace = m_enableTrace;
        options.ListenerContext = m_listenerContext;
        AddServiceEndpoint(std::make_shared<AmqpClaimBasedSecurity>(options));
      }

      void AddServiceEndpoint(std::shared_ptr<MockServiceEndpoint> const& endpoint)
      {
        m_serviceEndpoints.push_back(endpoint);
      }

    private:
      std::vector<std::shared_ptr<MockServiceEndpoint>> m_serviceEndpoints;

    public:
      uint16_t GetPort() const { return m_testPort; }
      Azure::Core::Context& GetListenerContext() { return m_listenerContext; }

      void StartListening()
      {
        // Start the mock AMQP server which will be used to receive the connect open.
        // Ensure that the thread is started before we start using the message sender.
        std::mutex threadRunningMutex;
        std::condition_variable threadStarted;
        bool running = false;

        m_serverThread = std::thread([this, &threadStarted, &running]() {
          Azure::Core::Amqp::Network::_detail::SocketListener listener(GetPort(), this);
          try
          {
            GTEST_LOG_(INFO) << "Start test listener on port " << GetPort();
            listener.Start();
            GTEST_LOG_(INFO) << "listener started";
            running = true;
            threadStarted.notify_one();

            while (!m_listenerContext.IsCancelled())
            {
              // Poll for incoming connections on the listener.
              listener.Poll();
            }
          }
          catch (std::exception& ex)
          {
            GTEST_LOG_(ERROR) << "Exception " << ex.what() << " thrown in listener thread.";
          }
          listener.Stop();
        });

        // Wait until our running thread is actually listening before we return.
        GTEST_LOG_(INFO) << "Wait 5 seconds for listener to start.";
        std::unique_lock<std::mutex> waitForThreadStart(threadRunningMutex);
        threadStarted.wait_until(
            waitForThreadStart,
            std::chrono::system_clock::now() + std::chrono::seconds(5),
            [&running]() { return running == true; });
        GTEST_LOG_(INFO) << "Listener running.";
      }

      void StopListening()
      {
        GTEST_LOG_(INFO) << "Stop listening";
        // Cancel the listener context, which will cause any WaitForXxx calls to exit.

        m_listenerContext.Cancel();
        m_serverThread.join();

        for (auto& serviceEndpoint : m_serviceEndpoints)
        {
          serviceEndpoint->StopProcessing();
        }

        if (!m_sessions.empty())
        {
          // Note: clearing the sessions list destroys the session and calls session_end always, so
          // it does not need to be called here.
          m_sessions.clear();
        }
        for (const auto& connection : m_connections)
        {
          connection->Close();
        }
      }

      void ForceCbsError(bool forceError)
      {
        for (const auto& serviceEndpoint : m_serviceEndpoints)
        {
          if (serviceEndpoint->GetName() == "$cbs")
          {
            static_cast<AmqpClaimBasedSecurity*>(serviceEndpoint.get())->ForceCbsError(forceError);
          }
        }
      }

      void EnableTrace(bool enableTrace) { m_enableTrace = enableTrace; }

    private:
      // The set of incoming connections, used when tearing down the mock server.
      std::list<std::shared_ptr<Azure::Core::Amqp::_internal::Connection>> m_connections;

      // The set of sessions.
      std::list<std::shared_ptr<Azure::Core::Amqp::_internal::Session>> m_sessions;

      //      bool m_connectionValid{false};
      bool m_enableTrace{true};

      std::string m_connectionId;
      std::thread m_serverThread;
      std::uint16_t m_testPort;

    protected:
      // For each incoming message source, we create a queue of messages intended for that
      // message source.
      //
      // Each message queue is keyed by the message-id.
      // std::map < std::string, MessageLinkComponents> m_linkMessageQueues;
      Azure::Core::Context m_listenerContext; // Used to cancel the listener if necessary.

      // Inherited from SocketListenerEvents
      virtual void OnSocketAccepted(
          std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> transport) override
      {
        GTEST_LOG_(INFO) << "OnSocketAccepted - Socket connection received.";
        auto amqpTransport{
            Azure::Core::Amqp::Network::_internal::AmqpHeaderDetectTransportFactory::Create(
                transport, nullptr)};
        Azure::Core::Amqp::_internal::ConnectionOptions options;
        options.ContainerId = m_connectionId;
        options.IdleTimeout = std::chrono::minutes(2);
        options.EnableTrace = m_enableTrace;
        auto newConnection = std::make_shared<Azure::Core::Amqp::_internal::Connection>(
            amqpTransport, options, this);
        m_connections.push_back(newConnection);
        newConnection->Listen();
      }

      // Inherited from ConnectionEvents
      virtual void OnConnectionStateChanged(
          Azure::Core::Amqp::_internal::Connection const&,
          Azure::Core::Amqp::_internal::ConnectionState newState,
          Azure::Core::Amqp::_internal::ConnectionState oldState) override
      {
        GTEST_LOG_(INFO) << "Connection State changed. Connection: " << m_connectionId
                         << " Old state : " << oldState << " New state: " << newState;
        if (newState == Azure::Core::Amqp::_internal::ConnectionState::End
            || newState == Azure::Core::Amqp::_internal::ConnectionState::Error)
        {
          // If the connection is closed, then we should close the connection.
          m_listenerContext.Cancel();
        }
      }
      virtual bool OnNewEndpoint(
          Azure::Core::Amqp::_internal::Connection const& connection,
          Azure::Core::Amqp::_internal::Endpoint& endpoint) override
      {
        GTEST_LOG_(INFO) << "OnNewEndpoint - Incoming endpoint created, create session.";
        Azure::Core::Amqp::_internal::SessionOptions options;
        options.InitialIncomingWindowSize = 10000;

        auto newSession = std::make_shared<Azure::Core::Amqp::_internal::Session>(
            connection.CreateSession(endpoint, options, this));
        m_sessions.push_back(newSession);
        newSession->Begin();
        return true;
      }
      virtual void OnIOError(Azure::Core::Amqp::_internal::Connection const&) override
      {
        GTEST_LOG_(INFO) << "On I/O Error - connection closed.";
      }

      // Inherited from SessionEvents.
      virtual bool OnLinkAttached(
          Azure::Core::Amqp::_internal::Session const& session,
          Azure::Core::Amqp::_internal::LinkEndpoint& newLinkInstance,
          std::string const& name,
          Azure::Core::Amqp::_internal::SessionRole role,
          Azure::Core::Amqp::Models::AmqpValue const& source,
          Azure::Core::Amqp::Models::AmqpValue const& target,
          Azure::Core::Amqp::Models::AmqpValue const&) override
      {
        Azure::Core::Amqp::Models::_internal::MessageSource msgSource(source);
        Azure::Core::Amqp::Models::_internal::MessageTarget msgTarget(target);

        GTEST_LOG_(INFO) << "OnLinkAttached for name: " << name << " Source : " << msgSource
                         << " Target : " << msgTarget << " Role: " << role;

        std::string endpointName;
        if (role == Azure::Core::Amqp::_internal::SessionRole::Receiver)
        {
          endpointName = static_cast<std::string>(msgSource.GetAddress());
        }
        else
        {
          endpointName = static_cast<std::string>(msgTarget.GetAddress());
        }
        // Check if this is a service endpoint.
        for (const auto& endpoint : m_serviceEndpoints)
        {
          if (endpoint->GetName() == endpointName)
          {
            // This is a service endpoint, so we need to handle it.
            return endpoint->OnLinkAttached(
                session, name, newLinkInstance, role, msgSource, msgTarget);
          }
        }

        // We didn't find this endpoint in our list of known names. Punt.
        GTEST_LOG_(INFO) << "Unknown endpoint name: " << endpointName;
        return false;
      }
    };
#else
    class AmqpServerMock : public Azure::Core::Amqp::Network::_detail::SocketListenerEvents,
                           public Azure::Core::Amqp::_internal::ConnectionEvents,
                           public Azure::Core::Amqp::_internal::SessionEvents,
                           public Azure::Core::Amqp::_internal::MessageReceiverEvents,
                           public Azure::Core::Amqp::_internal::MessageSenderEvents {
    public:
      struct MessageLinkComponents
      {
        std::unique_ptr<Azure::Core::Amqp::_internal::MessageSender> LinkSender;
        std::unique_ptr<Azure::Core::Amqp::_internal::MessageReceiver> LinkReceiver;
        Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
            std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage>>
            MessageQueue;
        Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> MessageReceiverPresentQueue;
        Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> MessageSenderPresentQueue;
      };

      AmqpServerMock(
          std::string name = testing::UnitTest::GetInstance()->current_test_info()->name())
          : m_connectionId{"Mock Server for " + name}, m_testPort{FindAvailableSocket()}
      {
      }
      AmqpServerMock(
          uint16_t listeningPort,
          std::string name = testing::UnitTest::GetInstance()->current_test_info()->name())
          : m_connectionId{"Mock Server for " + name}, m_testPort{listeningPort}
      {
      }

      virtual void Poll() const
      {
        if (!m_connectionValid)
        {
          throw std::runtime_error("Polling with invalid connection.");
        }
      }

      bool WaitForConnection(Azure::Core::Context const& context = {})
      {
        GTEST_LOG_(INFO) << "Wait for connection to be established on Mock Server.";
        auto result = m_externalConnectionQueue.WaitForResult(context);
        if (!result)
        {
          throw std::runtime_error("Connection not received");
        }
        GTEST_LOG_(INFO) << "Connection has been established.";
        return result != nullptr;
      }

    private:
      bool WaitForConnection(
          Azure::Core::Amqp::Network::_detail::SocketListener const& listener,
          Azure::Core::Context const& context = {})
      {
        auto result = m_connectionQueue.WaitForPolledResult(context, listener);
        if (result)
        {
          m_connectionValid = true;
          m_externalConnectionQueue.CompleteOperation(true);
        }
        return result != nullptr;
      }
      bool WaitForMessageReceiver(
          std::string const& nodeName,
          Azure::Core::Context const& context = {})
      {
        auto result
            = m_linkMessageQueues[nodeName].MessageReceiverPresentQueue.WaitForResult(context);
        return result != nullptr;
      }
      bool WaitForMessageSender(
          std::string const& nodeName,
          Azure::Core::Context const& context = {})
      {
        auto result
            = m_linkMessageQueues[nodeName].MessageSenderPresentQueue.WaitForResult(context);
        return result != nullptr;
      }

      std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> TryWaitForMessage(
          std::string const& nodeName)
      {
        // Poll for completion on both the mock server and the connection, that ensures that
        // we can implement unsolicited sends from the Poll function.
        auto result = m_linkMessageQueues[nodeName].MessageQueue.TryWaitForResult();
        if (result)
        {
          return std::move(std::get<0>(*result));
        }
        else
        {
          Poll();
          return nullptr;
        }
      }

      std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> WaitForMessage(
          std::string const& nodeName)
      {
        // Poll for completion on both the mock server and the connection, that ensures that
        // we can implement unsolicited sends from the Poll function.
        auto result
            = m_linkMessageQueues[nodeName].MessageQueue.WaitForResult(m_listenerContext, *this);
        if (result)
        {
          return std::move(std::get<0>(*result));
        }
        else
        {
          return nullptr;
        }
      }

      /** @brief Override for non CBS message receive operations which allows a specialization
       * to customize the behavior for received messages.
       */
      virtual void MessageReceived(
          std::string const&,
          MessageLinkComponents const&,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const&) const {};

      virtual void MessageLoop(
          std::string const& nodeName,
          MessageLinkComponents const& linkComponents)
      {
        auto message = TryWaitForMessage(nodeName);
        if (message)
        {
          GTEST_LOG_(INFO) << "Received message: " << *message;
          if (nodeName == "$cbs" && IsCbsMessage(message))
          {
            ProcessCbsMessage(linkComponents, message);
          }
          else
          {
            MessageReceived(nodeName, linkComponents, message);
          }
        }
        std::this_thread::yield();
      };

    public:
      uint16_t GetPort() const { return m_testPort; }
      Azure::Core::Context& GetListenerContext() { return m_listenerContext; }

      void StartListening()
      {
        // Start the mock AMQP server which will be used to receive the connect open.
        // Ensure that the thread is started before we start using the message sender.
        std::mutex threadRunningMutex;
        std::condition_variable threadStarted;
        bool running = false;

        m_serverThread = std::thread([this, &threadStarted, &running]() {
          Azure::Core::Amqp::Network::_detail::SocketListener listener(GetPort(), this);
          try
          {
            GTEST_LOG_(INFO) << "Start test listener on port " << GetPort();
            listener.Start();
            GTEST_LOG_(INFO) << "listener started";
            running = true;
            threadStarted.notify_one();

            GTEST_LOG_(INFO) << "Wait for connection on listener.";
            if (!WaitForConnection(listener, m_listenerContext))
            {
              GTEST_LOG_(INFO) << "Cancelling thread.";
              return;
            }
            while (!m_listenerContext.IsCancelled())
            {
              std::this_thread::yield();
              for (const auto& val : m_linkMessageQueues)
              {
                MessageLoop(val.first, val.second);
              }
            }
          }
          catch (std::exception& ex)
          {
            GTEST_LOG_(ERROR) << "Exception " << ex.what() << " thrown in listener thread.";
          }
          listener.Stop();
        });

        // Wait until our running thread is actually listening before we return.
        GTEST_LOG_(INFO) << "Wait 10 seconds for listener to start.";
        std::unique_lock<std::mutex> waitForThreadStart(threadRunningMutex);
        threadStarted.wait_until(
            waitForThreadStart,
            std::chrono::system_clock::now() + std::chrono::seconds(10),
            [&running]() { return running == true; });
        GTEST_LOG_(INFO) << "Listener running.";
      }

      void StopListening()
      {
        GTEST_LOG_(INFO) << "Stop listening";
        // Cancel the listener context, which will cause any WaitForXxx calls to exit.

        m_listenerContext.Cancel();
        m_serverThread.join();
        for (auto& val : m_linkMessageQueues)
        {
          if (val.second.LinkSender)
          {
            val.second.LinkSender->Close();
            val.second.LinkSender.reset();
          }
          if (val.second.LinkReceiver)
          {
            val.second.LinkReceiver->Close();
            val.second.LinkReceiver.reset();
          }
        }
        if (m_session)
        {
          // Note: resetting the m_session calls session_end always, so it does not need to be
          // called here.
          m_session.reset();
        }
        if (m_connection)
        {
          m_connection->Close();
          m_connection.reset();
        }
      }

      void ForceCbsError(bool forceError) { m_forceCbsError = forceError; }

      void EnableTrace(bool enableTrace) { m_enableTrace = enableTrace; }

    private:
      std::shared_ptr<Azure::Core::Amqp::_internal::Connection> m_connection;
      bool m_connectionValid{false};
      bool m_enableTrace{true};
      std::shared_ptr<Azure::Core::Amqp::_internal::Session> m_session;

      Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> m_connectionQueue;
      Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<bool> m_externalConnectionQueue;

      std::string m_connectionId;
      std::thread m_serverThread;
      std::uint16_t m_testPort;
      bool m_forceCbsError{false};

    protected:
      // For each incoming message source, we create a queue of messages intended for that
      // message source.
      //
      // Each message queue is keyed by the message-id.
      std::map<std::string, MessageLinkComponents> m_linkMessageQueues;
      Azure::Core::Context m_listenerContext; // Used to cancel the listener if necessary.

      bool IsCbsMessage(std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message)
      {
        if (!message->ApplicationProperties.empty())
        {
          Azure::Core::Amqp::Models::AmqpValue operation
              = message->ApplicationProperties.at("operation");
          Azure::Core::Amqp::Models::AmqpValue type = message->ApplicationProperties.at("type");

          // If we're processing a put-token message, then we should get a "type" and "name"
          // value.
          EXPECT_EQ(operation.GetType(), Azure::Core::Amqp::Models::AmqpValueType::String);
          if (static_cast<std::string>(operation) == "put-token")
          {
            return true;
          }
          else if (static_cast<std::string>(operation) == "delete-token")
          {
            return true;
          }
        }
        return false;
      }

      void ProcessCbsMessage(
          MessageLinkComponents const& linkComponents,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message)
      {
        Azure::Core::Amqp::Models::AmqpValue operation
            = message->ApplicationProperties.at("operation");
        Azure::Core::Amqp::Models::AmqpValue type = message->ApplicationProperties.at("type");
        Azure::Core::Amqp::Models::AmqpValue name = message->ApplicationProperties.at("name");
        // If we're processing a put-token message, then we should get a "type" and "name"
        // value.
        EXPECT_EQ(operation.GetType(), Azure::Core::Amqp::Models::AmqpValueType::String);
        if (static_cast<std::string>(operation) == "put-token")
        {
          EXPECT_EQ(type.GetType(), Azure::Core::Amqp::Models::AmqpValueType::String);
          EXPECT_EQ(name.GetType(), Azure::Core::Amqp::Models::AmqpValueType::String);
          // The body of a put-token operation MUST be an AMQP AmqpValue.
          EXPECT_EQ(message->BodyType, Azure::Core::Amqp::Models::MessageBodyType::Value);

          // Respond to the operation.
          Azure::Core::Amqp::Models::AmqpMessage response;
          Azure::Core::Amqp::Models::MessageProperties responseProperties;

          // Management specification section 3.2: The correlation-id of the response message
          // MUST be the correlation-id from the request message (if present), else the
          // message-id from the request message.
          Azure::Nullable<Azure::Core::Amqp::Models::AmqpValue> requestCorrelationId
              = message->Properties.CorrelationId;
          if (!message->Properties.CorrelationId.HasValue())
          {
            requestCorrelationId = message->Properties.MessageId.Value();
          }
          response.Properties.CorrelationId = requestCorrelationId;

          // Populate the response application properties.

          if (m_forceCbsError)
          {
            response.ApplicationProperties["status-code"] = 500;
            response.ApplicationProperties["status-description"] = "Internal Server Error";
          }
          else
          {
            response.ApplicationProperties["status-code"] = 200;
            response.ApplicationProperties["status-description"] = "OK-put";
          }

          response.SetBody(Azure::Core::Amqp::Models::AmqpValue());

          // Set the response body type to an empty AMQP value.
          if (m_listenerContext.IsCancelled())
          {
            return;
          }
          try
          {
            auto result = linkComponents.LinkSender->Send(response, m_listenerContext);
            if (std::get<0>(result) != Azure::Core::Amqp::_internal::MessageSendStatus::Ok)
            {
              GTEST_LOG_(INFO) << "Failed to send CBS response: " << std::get<1>(result);
              return;
            }
          }
          catch (std::exception& ex)
          {
            GTEST_LOG_(INFO) << "Exception thrown sending CBS response: " << ex.what();
            return;
          }
        }
        else if (static_cast<std::string>(operation) == "delete-token")
        {
          Azure::Core::Amqp::Models::AmqpMessage response;
          Azure::Core::Amqp::Models::MessageProperties responseProperties;

          // Management specification section 3.2: The correlation-id of the response message
          // MUST be the correlation-id from the request message (if present), else the
          // message-id from the request message.
          Azure::Nullable<Azure::Core::Amqp::Models::AmqpValue> requestCorrelationId
              = message->Properties.CorrelationId;
          if (!message->Properties.CorrelationId.HasValue())
          {
            requestCorrelationId = message->Properties.MessageId;
          }
          response.Properties.CorrelationId = requestCorrelationId;
          response.ApplicationProperties["status-code"] = 200;
          response.ApplicationProperties["status-description"] = "OK-delete";

          response.SetBody(Azure::Core::Amqp::Models::AmqpValue());

          // Set the response body type to an empty AMQP value.
          if (m_listenerContext.IsCancelled())
          {
            return;
          }

          auto sendResult = linkComponents.LinkSender->Send(response, m_listenerContext);
          if (std::get<0>(sendResult) != Azure::Core::Amqp::_internal::MessageSendStatus::Ok)
          {
            GTEST_LOG_(INFO) << "Failed to send CBS response: " << std::get<1>(sendResult);
            return;
          }
        }
      }

      virtual void OnSocketAccepted(
          std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> transport) override
      {
        GTEST_LOG_(INFO) << "OnSocketAccepted - Socket connection received.";
        auto amqpTransport{
            Azure::Core::Amqp::Network::_internal::AmqpHeaderDetectTransportFactory::Create(
                transport, nullptr)};
        Azure::Core::Amqp::_internal::ConnectionOptions options;
        options.ContainerId = m_connectionId;
        options.IdleTimeout = std::chrono::minutes(2);
        options.EnableTrace = m_enableTrace;
        m_connection = std::make_shared<Azure::Core::Amqp::_internal::Connection>(
            amqpTransport, options, this);
        m_connection->Listen();
        m_connectionQueue.CompleteOperation(true);
      }

      virtual void OnConnectionStateChanged(
          Azure::Core::Amqp::_internal::Connection const&,
          Azure::Core::Amqp::_internal::ConnectionState newState,
          Azure::Core::Amqp::_internal::ConnectionState oldState) override
      {
        GTEST_LOG_(INFO) << "Connection State changed. Connection: " << m_connectionId
                         << " Old state : " << oldState << " New state: " << newState;
        if (newState == Azure::Core::Amqp::_internal::ConnectionState::End
            || newState == Azure::Core::Amqp::_internal::ConnectionState::Error)
        {
          // If the connection is closed, then we should close the connection.
          m_connectionValid = false;
          m_listenerContext.Cancel();
        }
      }
      virtual bool OnNewEndpoint(
          Azure::Core::Amqp::_internal::Connection const& connection,
          Azure::Core::Amqp::_internal::Endpoint& endpoint) override
      {
        GTEST_LOG_(INFO) << "OnNewEndpoint - Incoming endpoint created, create session.";
        Azure::Core::Amqp::_internal::SessionOptions options;
        options.InitialIncomingWindowSize = 10000;

        m_session = std::make_shared<Azure::Core::Amqp::_internal::Session>(
            connection.CreateSession(endpoint, options, this));
        m_session->Begin();
        return true;
      }
      virtual void OnIOError(Azure::Core::Amqp::_internal::Connection const&) override
      {
        GTEST_LOG_(INFO) << "On I/O Error - connection closed.";
      }

      // Inherited via Session
      virtual bool OnLinkAttached(
          Azure::Core::Amqp::_internal::Session const& session,
          Azure::Core::Amqp::_internal::LinkEndpoint& newLinkInstance,
          std::string const& name,
          Azure::Core::Amqp::_internal::SessionRole role,
          Azure::Core::Amqp::Models::AmqpValue const& source,
          Azure::Core::Amqp::Models::AmqpValue const& target,
          Azure::Core::Amqp::Models::AmqpValue const&) override
      {
        Azure::Core::Amqp::Models::_internal::MessageSource msgSource(source);
        Azure::Core::Amqp::Models::_internal::MessageTarget msgTarget(target);

        GTEST_LOG_(INFO) << "OnLinkAttached. Source: " << msgSource << " Target: " << msgTarget
                         << " Role: " << static_cast<int>(role);

        // If the incoming role is receiver, then we want to create a sender to talk to it.
        // Similarly, if the incoming role is sender, we want to create a receiver to receive
        // from it.
        if (role == Azure::Core::Amqp::_internal::SessionRole::Receiver)
        {
          GTEST_LOG_(INFO) << "Role is receiver, create sender.";
          std::string targetAddress = static_cast<std::string>(msgTarget.GetAddress());
          MessageLinkComponents& linkComponents
              = m_linkMessageQueues[static_cast<std::string>(msgSource.GetAddress())];

          if (!linkComponents.LinkSender)
          {
            GTEST_LOG_(INFO) << "No sender found, create new.";
            Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;
            senderOptions.EnableTrace = m_enableTrace;
            senderOptions.Name = name;
            senderOptions.MessageSource = msgSource;
            senderOptions.InitialDeliveryCount = 0;
            linkComponents.LinkSender
                = std::make_unique<Azure::Core::Amqp::_internal::MessageSender>(
                    session.CreateMessageSender(
                        newLinkInstance, targetAddress, senderOptions, this));
            linkComponents.LinkSender->Open();
            linkComponents.MessageSenderPresentQueue.CompleteOperation(true);
          }
        }
        else if (role == Azure::Core::Amqp::_internal::SessionRole::Sender)
        {
          GTEST_LOG_(INFO) << "Role is sender, create receiver.";
          MessageLinkComponents& linkComponents
              = m_linkMessageQueues[static_cast<std::string>(msgTarget.GetAddress())];
          if (!linkComponents.LinkReceiver)
          {
            GTEST_LOG_(INFO) << "No receiver found, create new.";
            Azure::Core::Amqp::_internal::MessageReceiverOptions receiverOptions;
            receiverOptions.EnableTrace = m_enableTrace;
            receiverOptions.Name = name;
            receiverOptions.MessageTarget = msgTarget;
            receiverOptions.InitialDeliveryCount = 0;
            std::string sourceAddress = static_cast<std::string>(msgSource.GetAddress());
            linkComponents.LinkReceiver
                = std::make_unique<Azure::Core::Amqp::_internal::MessageReceiver>(
                    session.CreateMessageReceiver(
                        newLinkInstance, sourceAddress, receiverOptions, this));
            linkComponents.LinkReceiver->Open();
            linkComponents.MessageReceiverPresentQueue.CompleteOperation(true);
          }
        }
        return true;
      }

      // Inherited via MessageReceiverEvents
      void OnMessageReceiverStateChanged(
          Azure::Core::Amqp::_internal::MessageReceiver const&,
          Azure::Core::Amqp::_internal::MessageReceiverState newState,
          Azure::Core::Amqp::_internal::MessageReceiverState oldState) override
      {
        GTEST_LOG_(INFO) << "Message Receiver State changed. Old state: " << oldState
                         << " New state: " << newState;
      }
      Azure::Core::Amqp::Models::AmqpValue OnMessageReceived(
          Azure::Core::Amqp::_internal::MessageReceiver const& receiver,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO) << "Received a message " << message;
        m_linkMessageQueues[receiver.GetSourceName()].MessageQueue.CompleteOperation(message);
        return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryAccepted();
      }

      // Inherited via MessageSenderEvents
      void OnMessageSenderStateChanged(
          Azure::Core::Amqp::_internal::MessageSender const&,
          Azure::Core::Amqp::_internal::MessageSenderState newState,
          Azure::Core::Amqp::_internal::MessageSenderState oldState) override
      {
        GTEST_LOG_(INFO) << "Message Sender State changed. Old state: " << oldState
                         << " New state: " << newState;
      }

      void OnMessageSenderDisconnected(
          Azure::Core::Amqp::Models::_internal::AmqpError const& error) override
      {
        GTEST_LOG_(INFO) << "Message Sender Disconnected: Error: " << error;
      }
      virtual void OnMessageReceiverDisconnected(
          Azure::Core::Amqp::Models::_internal::AmqpError const& error) override
      {
        GTEST_LOG_(INFO) << "Message receiver disconnected: " << error << std::endl;
      }
    };
#endif
  } // namespace MessageTests
}}}} // namespace Azure::Core::Amqp::Tests
