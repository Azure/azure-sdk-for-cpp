// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/common/async_operation_queue.hpp"
#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/message_receiver.hpp"
#include "azure/core/amqp/internal/models/amqp_protocol.hpp"
#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/internal/network/amqp_header_detect_transport.hpp"
#include "azure/core/amqp/internal/network/socket_listener.hpp"
#include "azure/core/amqp/internal/network/socket_transport.hpp"
#include "azure/core/amqp/internal/session.hpp"
#include "mock_amqp_server.hpp"

#include <azure/core/context.hpp>
#include <azure/core/platform.hpp>

#include <functional>
#include <random>

#include <gtest/gtest.h>

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  extern uint16_t FindAvailableSocket();

  class TestConnections : public testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
  };

#if !defined(AZURE_PLATFORM_MAC)
  TEST_F(TestConnections, SimpleConnection)
  {
    {
      // Create a connection
      Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
      connectionOptions.Port = Azure::Core::Amqp::_internal::AmqpPort;

      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, {});
    }
    {
      // Create a connection
      Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
      connectionOptions.Port = Azure::Core::Amqp::_internal::AmqpPort;

      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, connectionOptions);
    }
    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      auto socketTransport{Azure::Core::Amqp::Network::_internal::SocketTransportFactory::Create(
          "localhost", Azure::Core::Amqp::_internal::AmqpPort)};

      Azure::Core::Amqp::_internal::Connection connection(
          socketTransport, options, nullptr, nullptr);
    }
  }

  TEST_F(TestConnections, ConnectionAttributes)
  {
    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      options.IdleTimeout = std::chrono::milliseconds(1532);

      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, options);

      EXPECT_EQ(connection.GetPort(), 5671);
      EXPECT_EQ(connection.GetHost(), "localhost");

      auto idleTimeout = connection.GetIdleTimeout();
      (void)idleTimeout;
      EXPECT_EQ(std::chrono::milliseconds(1532), connection.GetIdleTimeout());
    }
    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      options.MaxFrameSize = 1024 * 64;
      options.Port = Azure::Core::Amqp::_internal::AmqpPort;
      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, options);
      EXPECT_EQ(connection.GetPort(), 5672);
      EXPECT_EQ(connection.GetHost(), "localhost");

      auto maxFrameSize = connection.GetMaxFrameSize();
      (void)maxFrameSize;
      EXPECT_EQ(1024 * 64, connection.GetMaxFrameSize());

      EXPECT_NO_THROW(
          connection.GetRemoteMaxFrameSize()); // Likely doesn't work unless there's a remote.
    }

    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      options.MaxChannelCount = 128;
      options.Port = Azure::Core::Amqp::_internal::AmqpPort;

      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, options);
      EXPECT_EQ(connection.GetPort(), 5672);
      EXPECT_EQ(connection.GetHost(), "localhost");

      auto maxChannel = connection.GetMaxChannel();
      EXPECT_EQ(128, connection.GetMaxChannel());
      (void)maxChannel;
    }

    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      options.MaxChannelCount = 128;

      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, options);
      // Ratio must be a number between 0 and 1.
      EXPECT_NO_THROW(connection.SetIdleEmptyFrameSendPercentage(0.5));
    }

    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      options.MaxChannelCount = 128;
      options.Properties["test"] = "test";

      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, options);
      EXPECT_EQ(Azure::Core::Amqp::Models::AmqpValue{"test"}, connection.GetProperties()["test"]);
    }
  }

  TEST_F(TestConnections, ConnectionOpenClose)
  {
    class TestListener : public Azure::Core::Amqp::Network::_detail::SocketListenerEvents {
    public:
      std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> WaitForResult(
          Azure::Core::Amqp::Network::_detail::SocketListener const& listener,
          Azure::Core::Context const& context = {})
      {
        GTEST_LOG_(INFO) << "Waiting for listener to accept connection.";
        auto result = m_listenerQueue.WaitForPolledResult(context, listener);
        return std::get<0>(*result);
      }

    private:
      Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
          std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport>>
          m_listenerQueue;

      virtual void OnSocketAccepted(
          std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> transport)
      {
        GTEST_LOG_(INFO) << "Socket for listener accepted connection.";
        m_listenerQueue.CompleteOperation(transport);
      }
    };

    {
      // Ensure someone is listening on the connection for when we call connection.Open.

      uint16_t testPort = FindAvailableSocket();

      GTEST_LOG_(INFO) << "Test listener using port: " << testPort;

      TestListener listenerEvents;
      Azure::Core::Amqp::Network::_detail::SocketListener listener(testPort, &listenerEvents);
      EXPECT_NO_THROW(listener.Start());

      // Create a connection
      Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
      connectionOptions.Port = testPort;
      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, connectionOptions);

      // Open the connection
      connection.Open();

      // Ensure that we got an OnComplete callback within 5 seconds.
      auto transport = listenerEvents.WaitForResult(
          listener,
          Azure::Core::Context{std::chrono::system_clock::now() + std::chrono::seconds(5)});

      // Now we can close the connection.
      connection.Close("xxx", "yyy", {});
      listener.Stop();
    }
  }
#endif // !defined(AZURE_PLATFORM_MAC)

}}}} // namespace Azure::Core::Amqp::Tests
