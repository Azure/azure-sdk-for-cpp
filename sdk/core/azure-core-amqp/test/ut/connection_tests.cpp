// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../src/amqp/private/token_refresh.hpp"
#include "azure/core/amqp/internal/common/async_operation_queue.hpp"
#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/message_receiver.hpp"
#include "azure/core/amqp/internal/models/amqp_protocol.hpp"
#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/internal/network/amqp_header_detect_transport.hpp"
#include "azure/core/amqp/internal/network/socket_listener.hpp"
#include "azure/core/amqp/internal/network/socket_transport.hpp"
#include "azure/core/amqp/internal/session.hpp"
#include "azure/core/internal/environment.hpp"
#include "azure/core/url.hpp"
#include "mock_amqp_server.hpp"

#include <azure/core/context.hpp>
#include <azure/core/platform.hpp>

#include <chrono>
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

  // Tests for the rules that decide when a cached CBS token is still good, and
  // when the connection must replace it. These rules are pure functions, so they
  // run on every platform and need no service.
  class TestTokenRefresh : public testing::Test {
  protected:
    static Azure::Core::Credentials::AccessToken TokenExpiringIn(std::chrono::seconds lifetime)
    {
      Azure::Core::Credentials::AccessToken token;
      token.Token = "TestToken";
      token.ExpiresOn = std::chrono::system_clock::now() + lifetime;
      return token;
    }
  };

  TEST_F(TestTokenRefresh, CachedTokenIsUsableWhileItHasLifeLeft)
  {
    auto const now = std::chrono::system_clock::now();
    EXPECT_TRUE(Azure::Core::Amqp::_detail::IsCachedTokenUsable(
        TokenExpiringIn(std::chrono::hours(1)), now));
    EXPECT_TRUE(Azure::Core::Amqp::_detail::IsCachedTokenUsable(
        TokenExpiringIn(std::chrono::minutes(2)), now));
  }

  TEST_F(TestTokenRefresh, CachedTokenIsNotUsableNearOrAfterExpiry)
  {
    auto const now = std::chrono::system_clock::now();
    // Inside the minimum lifetime that a caller may use.
    EXPECT_FALSE(Azure::Core::Amqp::_detail::IsCachedTokenUsable(
        TokenExpiringIn(std::chrono::seconds(10)), now));
    // Already expired.
    EXPECT_FALSE(Azure::Core::Amqp::_detail::IsCachedTokenUsable(
        TokenExpiringIn(std::chrono::seconds(-30)), now));
  }

  TEST_F(TestTokenRefresh, RefreshIsDueOneBufferBeforeExpiry)
  {
    auto const now = std::chrono::system_clock::now();
    // A normal token has a long life, so no refresh is due yet.
    EXPECT_FALSE(Azure::Core::Amqp::_detail::IsTokenRefreshDue(
        TokenExpiringIn(std::chrono::minutes(90)), now));
    // Inside the buffer, so the refresh thread must replace the token.
    EXPECT_TRUE(Azure::Core::Amqp::_detail::IsTokenRefreshDue(
        TokenExpiringIn(std::chrono::minutes(6)), now));
  }

  // A credential can put any value in ExpiresOn, and the cast from
  // Azure::DateTime to a system clock time point throws outside the range of
  // that clock. A default constructed ExpiresOn is year 1. These rules run on
  // the refresh thread, where an exception would end the process, so they must
  // not throw for any value.
  TEST_F(TestTokenRefresh, ExtremeExpiryValuesDoNotThrow)
  {
    auto const now = std::chrono::system_clock::now();

    // A default constructed token reports year 1.
    Azure::Core::Credentials::AccessToken defaultToken;
    EXPECT_NO_THROW({
      EXPECT_FALSE(Azure::Core::Amqp::_detail::IsCachedTokenUsable(defaultToken, now));
      EXPECT_TRUE(Azure::Core::Amqp::_detail::IsTokenRefreshDue(defaultToken, now));
    });

    // A token that reports a year past the range of the system clock.
    Azure::Core::Credentials::AccessToken farFutureToken;
    farFutureToken.Token = "TestToken";
    farFutureToken.ExpiresOn = Azure::DateTime(9999, 12, 31);
    EXPECT_NO_THROW({
      EXPECT_TRUE(Azure::Core::Amqp::_detail::IsCachedTokenUsable(farFutureToken, now));
      EXPECT_FALSE(Azure::Core::Amqp::_detail::IsTokenRefreshDue(farFutureToken, now));
    });
  }

  // A token with a lifetime shorter than the buffer is due as soon as it
  // arrives. The connection must still hand it to a caller, because refusing it
  // would make every call authenticate again.
  TEST_F(TestTokenRefresh, ShortLivedTokenIsDueButStillUsable)
  {
    auto const now = std::chrono::system_clock::now();
    auto const token = TokenExpiringIn(std::chrono::seconds(80));
    EXPECT_TRUE(Azure::Core::Amqp::_detail::IsTokenRefreshDue(token, now));
    EXPECT_TRUE(Azure::Core::Amqp::_detail::IsCachedTokenUsable(token, now));
  }

#if !defined(AZ_PLATFORM_MAC)
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
#if ENABLE_UAMQP
    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      auto socketTransport{Azure::Core::Amqp::Network::_internal::SocketTransportFactory::Create(
          "localhost", Azure::Core::Amqp::_internal::AmqpPort)};

      Azure::Core::Amqp::_internal::Connection connection(
          socketTransport, options, nullptr, nullptr);
    }
#endif
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
#if ENABLE_UAMQP
      EXPECT_NO_THROW(
          connection.GetRemoteMaxFrameSize()); // Likely doesn't work unless there's a remote.
#endif
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
#if ENABLE_UAMQP
      // Ratio must be a number between 0 and 1.
      EXPECT_NO_THROW(connection.SetIdleEmptyFrameSendPercentage(0.5));
#endif
    }

    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      options.MaxChannelCount = 128;
      options.Properties[Azure::Core::Amqp::Models::AmqpSymbol{"test"}] = "test";

      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, options);
      GTEST_LOG_(INFO) << connection.GetProperties();
      EXPECT_EQ(
          Azure::Core::Amqp::Models::AmqpValue{"test"},
          connection.GetProperties()[Azure::Core::Amqp::Models::AmqpSymbol{"test"}]);
    }
  }

  TEST_F(TestConnections, ConnectionOpenClose)
  {
#if ENABLE_UAMQP
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
      connection.Open({});

      // Ensure that we got an OnComplete callback within 5 seconds.
      auto transport = listenerEvents.WaitForResult(
          listener,
          Azure::Core::Context{std::chrono::system_clock::now() + std::chrono::seconds(5)});

      // Now we can close the connection.
      connection.Close("xxx", "yyy", {}, {});
      listener.Stop();
    }
#else
    // Create a connection
    auto testBrokerUrl = Azure::Core::_internal::Environment::GetVariable("TEST_BROKER_ADDRESS");
    if (testBrokerUrl.empty())
    {
      GTEST_FATAL_FAILURE_("Could not find required environment variable TEST_BROKER_ADDRESS");
    }
    Azure::Core::Url brokerUrl(testBrokerUrl);
    Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
    connectionOptions.Port = brokerUrl.GetPort();
    Azure::Core::Amqp::_internal::Connection connection(
        brokerUrl.GetHost(), nullptr, connectionOptions);

    // Open the connection
    connection.Open({});

    connection.Close({});

#endif
  }
#endif // !defined(AZ_PLATFORM_MAC)

}}}} // namespace Azure::Core::Amqp::Tests
