// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include "azure/core/amqp/internal/models/amqp_error.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"

#include <azure/core/amqp/internal/connection.hpp>
#include <azure/core/amqp/internal/link.hpp>
#include <azure/core/amqp/internal/message_receiver.hpp>
#include <azure/core/amqp/internal/message_sender.hpp>
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
          Azure::Core::Amqp::_internal::Session const& session,
          std::string const& linkName,
          Azure::Core::Amqp::_internal::LinkEndpoint& linkEndpoint,
          Azure::Core::Amqp::_internal::SessionRole role,
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
          if (!HasMessageSender(linkName))
          {
            GTEST_LOG_(INFO) << "No sender found, create new sender for " << linkName;
            Azure::Core::Amqp::_internal::MessageSenderOptions senderOptions;
            senderOptions.EnableTrace = m_enableTrace;
            senderOptions.Name = linkName;
            senderOptions.MessageSource = source;
            senderOptions.InitialDeliveryCount = 0;
            m_sender[linkName] = std::make_unique<Azure::Core::Amqp::_internal::MessageSender>(
                session.CreateMessageSender(linkEndpoint, target, senderOptions, this));
            // NOTE: The linkEndpoint needs to be attached before this function returns in order to
            // correctly process incoming attach requests. Otherwise, the attach request will be
            // discarded, and the link will be in a half attached state.
            (void)!m_sender[linkName]->HalfOpen(m_listenerContext);
          }
          else
          {
            GTEST_LOG_(INFO) << "Sender already created for link name " << linkName << " on target "
                             << target;
            Models::_internal::AmqpError error;
            error.Condition = Models::_internal::AmqpErrorCondition::EntityAlreadyExists;
            error.Description = "Link already exists.";
            session.SendDetach(linkEndpoint, true, error);
            return false;
          }
        }
        else if (role == Azure::Core::Amqp::_internal::SessionRole::Sender)
        {
          GTEST_LOG_(INFO) << "Role is sender, create receiver.";
          if (!HasMessageReceiver(linkName))
          {
            GTEST_LOG_(INFO) << "No receiver found, create new receiver for " << linkName;
            Azure::Core::Amqp::_internal::MessageReceiverOptions receiverOptions;
            receiverOptions.EnableTrace = m_enableTrace;
            receiverOptions.Name = linkName;
            receiverOptions.MessageTarget = target;
            receiverOptions.InitialDeliveryCount = 0;
            m_receiver[linkName] = std::make_unique<Azure::Core::Amqp::_internal::MessageReceiver>(
                session.CreateMessageReceiver(linkEndpoint, source, receiverOptions, this));

            // NOTE: The linkEndpoint needs to be attached before this function returns in order to
            // correctly process incoming attach requests. Otherwise, the attach request will be
            // discarded, and the link will be in a half attached state.

            // Note that there is a potential deadlock when opening the message receiver - the
            // connection lock cannot be held when link polling is enabled on the incoming link.
            // This is because the link polling will try to acquire the connection lock, which is
            // already held by the current thread. To avoid this deadlock, we defer enabling link
            // polling until later when it is safe to do so.
            m_receiver[linkName]->Open(m_listenerContext);

            // Now that the receiver is open, we can enable link polling from the message loop.
            m_receiverPollingEnableQueue.CompleteOperation(linkName);
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
          m_serverThread = std::thread([this]() {
            MessageLoop();
            GTEST_LOG_(INFO) << "Exiting message loop for " << m_name;
          });
        }
        return true;
      }

      void StopProcessing()
      {
        GTEST_LOG_(INFO) << "Stop processing for " << m_name;
        if (m_serverThread.joinable())
        {
          m_listenerContext.Cancel();
          m_serverThread.join();
        }

        if (!m_receiver.empty())
        {
          for (const auto& receiver : m_receiver)
          {
            receiver.second->Close();
          }
        }
        if (!m_sender.empty())
        {
          for (const auto& sender : m_sender)
          {
            sender.second->Close();
          }
        }
      }

    protected:
      bool HasMessageSender(std::string const& linkName = {}) const
      {
        if (linkName.empty())
        {
          if (m_sender.size() == 0)
          {
            return false;
          }
          if (m_sender.size() != 1)
          {
            throw std::runtime_error("Ambiguous sender link name.");
          }
          return true;
        }
        return (m_sender.find(linkName) != m_sender.end());
      }
      Azure::Core::Amqp::_internal::MessageSender& GetMessageSender(
          std::string const& linkName = {}) const
      {
        if (linkName.empty())
        {
          if (m_sender.size() != 1)
          {
            throw std::runtime_error("Ambiguous sender link name.");
          }
          return *m_sender.begin()->second;
        }
        auto sender = m_sender.find(linkName);
        if (sender == m_sender.end())
        {
          throw std::runtime_error("Message sender not created.");
        }
        return *(sender->second);
      }

      bool HasMessageReceiver(std::string const& linkName = {}) const
      {
        if (linkName.empty())
        {
          if (m_receiver.size() == 0)
          {
            return false;
          }
          if (m_receiver.size() != 1)
          {
            throw std::runtime_error("Ambiguous sender link name.");
          }
          return true;
        }
        return (m_receiver.find(linkName) != m_receiver.end());
      }
      Azure::Core::Amqp::_internal::MessageReceiver& GetMessageReceiver(
          std::string const& linkName = {}) const
      {
        if (linkName.empty())
        {
          if (m_receiver.size() != 1)
          {
            throw std::runtime_error("Ambiguous receiver link name.");
          }
          return *m_receiver.begin()->second;
        }
        auto receiver = m_receiver.find(linkName);
        if (receiver == m_receiver.end())
        {
          throw std::runtime_error("Message receiver not created.");
        }
        return *(receiver->second);
      }

      virtual void Poll() const {}

      Azure::Core::Context& GetListenerContext() { return m_listenerContext; }

      Azure::Core::Amqp::Models::AmqpValue OnMessageReceived(
          Azure::Core::Amqp::_internal::MessageReceiver const& receiver,
          std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> const& message) override
      {
        GTEST_LOG_(INFO) << "MockServiceEndpoint(" << m_name << ") Received a message " << *message;
        m_messageQueue.CompleteOperation(receiver.GetLinkName(), message);
        return Azure::Core::Amqp::Models::_internal::Messaging::DeliveryAccepted();
      }

    private:
      Azure::Core::Context m_listenerContext; // Used to cancel the listener if necessary.
      bool m_enableTrace{true};
      std::string m_name;
      std::thread m_serverThread;
      std::map<std::string, std::unique_ptr<Azure::Core::Amqp::_internal::MessageSender>> m_sender;
      std::map<std::string, std::unique_ptr<Azure::Core::Amqp::_internal::MessageReceiver>>
          m_receiver;

      Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<std::string>
          m_receiverPollingEnableQueue;
      Azure::Core::Amqp::Common::_internal::
          AsyncOperationQueue<std::string, std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage>>
              m_messageQueue;
      Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<std::string>
          m_messageSenderDisconnectedQueue;
      Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<std::string>
          m_messageReceiverDisconnectedQueue;

      virtual void MessageReceived(
          std::string const& linkName,
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

            GTEST_LOG_(INFO) << "Received message on link " << std::get<0>(*message) << ": "
                             << *std::get<1>(*message);

            MessageReceived(std::get<0>(*message), std::get<1>(*message));
          }

          auto senderDisconnected = m_messageSenderDisconnectedQueue.TryWaitForResult();
          if (senderDisconnected)
          {
            std::string senderName = std::get<0>(*senderDisconnected);
            GTEST_LOG_(INFO) << "Sender disconnected: " << senderName;
            std::unique_ptr<Azure::Core::Amqp::_internal::MessageSender> sender{
                m_sender[senderName].release()};
            m_sender.erase(senderName);
            sender->Close(m_listenerContext);
          }

          auto receiverDisconnected = m_messageReceiverDisconnectedQueue.TryWaitForResult();
          if (receiverDisconnected)
          {
            std::string receiverName = std::get<0>(*receiverDisconnected);
            GTEST_LOG_(INFO) << "Receiver disconnected: " << receiverName;
            std::unique_ptr<Azure::Core::Amqp::_internal::MessageReceiver> receiver{
                m_receiver[receiverName].release()};
            m_receiver.erase(receiverName);
            receiver->Close(m_listenerContext);
          }

          auto receiverPollingEnable = m_receiverPollingEnableQueue.TryWaitForResult();
          if (receiverPollingEnable)
          {
            std::string receiverName = std::get<0>(*receiverPollingEnable);
            GTEST_LOG_(INFO) << "Enable link polling for receiver: " << receiverName;
            m_receiver[receiverName]->EnableLinkPolling();
          }

          if (m_receiver.empty() && m_sender.empty())
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

      virtual void OnMessageReceiverDisconnected(
          Azure::Core::Amqp::_internal::MessageReceiver const& receiver,
          Azure::Core::Amqp::Models::_internal::AmqpError const& error) override
      {
        GTEST_LOG_(INFO) << "Message receiver disconnected: " << error << std::endl;
        m_messageReceiverDisconnectedQueue.CompleteOperation(receiver.GetLinkName());
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
          Azure::Core::Amqp::_internal::MessageSender const& sender,
          Azure::Core::Amqp::Models::_internal::AmqpError const& error) override
      {
        GTEST_LOG_(INFO) << "Message Sender Disconnected: Error: " << error;
        m_messageSenderDisconnectedQueue.CompleteOperation(sender.GetLinkName());
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
          std::string const&,
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

    class AmqpServerMock final : public Azure::Core::Amqp::Network::_detail::SocketListenerEvents,
                                 public Azure::Core::Amqp::_internal::ConnectionEvents,
                                 public Azure::Core::Amqp::_internal::ConnectionEndpointEvents,
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

      virtual ~AmqpServerMock()
      {
        // If we're destroyed while still listening, stop listening.
        if (m_listening)
        {
          StopListening();
        }
      }

      void AddServiceEndpoint(std::shared_ptr<MockServiceEndpoint> const& endpoint)
      {
        m_serviceEndpoints.push_back(endpoint);
      }

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
        m_listening = true;
      }

      void StopListening()
      {
        GTEST_LOG_(INFO) << "Stop listening";
        // Cancel the listener context, which will cause any WaitForXxx calls to exit.

        m_listenerContext.Cancel();
        if (m_serverThread.joinable())
        {
          m_serverThread.join();
        }

        for (auto& serviceEndpoint : m_serviceEndpoints)
        {
          serviceEndpoint->StopProcessing();
        }

        if (!m_sessions.empty())
        {
          for (const auto& session : m_sessions)
          {
            session->End();
          }
          // Note: clearing the sessions list destroys the session and calls session_end always, so
          // it does not need to be called here.
          m_sessions.clear();
        }
        for (const auto& connection : m_connections)
        {
          connection->Close();
        }
        m_listening = false;
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

    protected:
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
            amqpTransport, options, this, this);
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

    private:
      std::vector<std::shared_ptr<MockServiceEndpoint>> m_serviceEndpoints;

      // The set of incoming connections, used when tearing down the mock server.
      std::list<std::shared_ptr<Azure::Core::Amqp::_internal::Connection>> m_connections;

      // The set of sessions.
      std::list<std::shared_ptr<Azure::Core::Amqp::_internal::Session>> m_sessions;

      bool m_enableTrace{true};
      bool m_listening{false};

      std::string m_connectionId;
      std::thread m_serverThread;
      std::uint16_t m_testPort;

      Azure::Core::Context m_listenerContext; // Used to cancel the listener if necessary.
    };
  } // namespace MessageTests
}}}} // namespace Azure::Core::Amqp::Tests
