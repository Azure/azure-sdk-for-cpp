// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include "azure/core/amqp/internal/models/amqp_error.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"

#include <azure/core/amqp/internal/claims_based_security.hpp>
#include <azure/core/amqp/internal/connection.hpp>
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

namespace Azure { namespace Core { namespace Amqp { namespace Tests {

  extern uint16_t FindAvailableSocket();
  namespace MessageTests {

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

        //    Azure::Core::Context listenerContext;
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

    private:
      std::shared_ptr<Azure::Core::Amqp::_internal::Connection> m_connection;
      bool m_connectionValid{false};
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
        options.EnableTrace = true;
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
            senderOptions.EnableTrace = true;
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
            receiverOptions.EnableTrace = true;
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
  } // namespace MessageTests
}}}} // namespace Azure::Core::Amqp::Tests
