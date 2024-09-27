// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/internal/common/async_operation_queue.hpp"
#include "azure/core/amqp/internal/common/global_state.hpp"
#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/message_receiver.hpp"
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
#if defined(AZ_PLATFORM_POSIX)
#include <poll.h> // for poll()

#include <netinet/in.h> // for sockaddr_in
#include <sys/socket.h> // for socket shutdown
#elif defined(AZ_PLATFORM_WINDOWS)
#include <winsock2.h> // for WSAPoll();
#ifdef max
#undef max
#endif
#endif // AZ_PLATFORM_POSIX/AZ_PLATFORM_WINDOWS

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  class TestSessions : public testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
  };

  using namespace Azure::Core::Amqp::_internal;
  using namespace Azure::Core::Amqp;

#if !defined(AZ_PLATFORM_MAC)
  TEST_F(TestSessions, SimpleSession)
  {

    // Create a connection
    Azure::Core::Amqp::_internal::ConnectionOptions options;
#if ENABLE_RUST_AMQP
    options.Port = 25672;
#endif
    Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, options);

#if ENABLE_RUST_AMQP
    connection.Open({});
#endif
    {
      // Create a session
      Session session{connection.CreateSession()};
    }

    {
      // Create two sessions
      Session session1{connection.CreateSession({})};
      Session session2{connection.CreateSession({})};

      EXPECT_ANY_THROW(session1.End({}));
    }
  }

  TEST_F(TestSessions, SessionProperties)
  { // Create a connection
    Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, {});

    {
      Session session{connection.CreateSession()};

      // Verify defaults are something "reasonable".
      EXPECT_EQ(1, session.GetIncomingWindow());
      EXPECT_EQ((std::numeric_limits<uint32_t>::max)(), session.GetHandleMax());
      EXPECT_EQ(1, session.GetOutgoingWindow());
    }

    {
      SessionOptions options;
      options.MaximumLinkCount = 37;
      Session session{connection.CreateSession(options)};
      EXPECT_EQ(37, session.GetHandleMax());
    }
    {
      SessionOptions options;
      options.InitialIncomingWindowSize = 1909119;
      Session session{connection.CreateSession(options)};
      EXPECT_EQ(1909119, session.GetIncomingWindow());
    }
    {
      SessionOptions options;
      options.InitialOutgoingWindowSize = 1909119;
      Session session{connection.CreateSession(options)};
      EXPECT_EQ(1909119, session.GetOutgoingWindow());
    }
  }
#endif // !AZ_PLATFORM_MAC

#if ENABLE_UAMQP

  uint16_t FindAvailableSocket()
  {
    // Ensure that the global state for the AMQP stack is initialized. Normally this is done by
    // the network facing objects, but this is called before those objects are initialized.
    //
    // This may hide bugs in some of the global objects, but it is needed to ensure that the
    // port we choose for the tests is available.
    {
      auto instance = Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance();
      (void)instance;
    }

    std::random_device dev;
    int count = 0;
    while (count < 20)
    {
      uint16_t testPort;
      // Make absolutely sure that we don't accidentally use the TLS port.
      do
      {
        testPort = dev() % 1000 + 0xBFFF;
      } while (testPort == AmqpTlsPort);

      GTEST_LOG_(INFO) << "Trying Test port: " << testPort;

      auto sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (sock != -1)
      {
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(testPort);

        auto bindResult = bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        // We were able to bind to the port, so it's available.
#if defined(AZ_PLATFORM_WINDOWS)
        closesocket(sock);
#else
        close(sock);
#endif
        if (bindResult != -1)
        {
          return testPort;
        }
        else
        {
#if defined(AZ_PLATFORM_WINDOWS)
          auto err = WSAGetLastError();
#else
          auto err = errno;
#endif
          GTEST_LOG_(INFO) << "Error " << std::to_string(err) << " binding to socket.";
        }
      }
      else
      {
#if defined(AZ_PLATFORM_WINDOWS)
        auto err = WSAGetLastError();
#else
        auto err = errno;
#endif
        GTEST_LOG_(INFO) << "Error " << std::to_string(err) << " opening port.";
      }
      count += 1;
    }
    throw std::runtime_error("Could not find available test port.");
  }
#endif

#if !defined(AZ_PLATFORM_MAC)
  TEST_F(TestSessions, SessionBeginEnd)
  {
#if ENABLE_UAMQP
    class TestListenerEvents : public Network::_detail::SocketListenerEvents {
    public:
      std::shared_ptr<Network::_internal::Transport> WaitForResult(
          Network::_detail::SocketListener const& listener,
          Azure::Core::Context const& context = {})
      {
        auto result = m_listenerQueue.WaitForPolledResult(context, listener);
        return std::get<0>(*result);
      }

    private:
      Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
          std::shared_ptr<Network::_internal::Transport>>
          m_listenerQueue;

      virtual void OnSocketAccepted(std::shared_ptr<Network::_internal::Transport> transport)
      {
        // Capture the XIO into a transport so it won't leak.
        m_listenerQueue.CompleteOperation(transport);
      }
    };

    // Ensure someone is listening on the connection for when we call Session.Begin.
    TestListenerEvents events;
    uint16_t testPort = FindAvailableSocket();
    Network::_detail::SocketListener listener(testPort, &events);
    listener.Start();
#elif ENABLE_RUST_AMQP
    // Port of AZURE_AMQP test broker
    uint16_t testPort = 25672;
#endif
    // Create a connection
    Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
    connectionOptions.Port = testPort;
    Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, connectionOptions);

#if ENABLE_RUST_AMQP
    // Open the connection
    GTEST_LOG_(INFO) << "Open connection.";
    connection.Open({});
#endif

    {
      Session session{connection.CreateSession()};

      session.Begin({});
      session.End({});
    }

    {
      Session session{connection.CreateSession()};

      session.Begin({});
      session.End("", "",{});
    }

    {
      Session session{connection.CreateSession()};

      session.Begin({});
      session.End("amqp:link:detach-forced", "Forced detach.", {});
    }
#if ENABLE_UAMQP
    listener.Stop();
#endif
  }

  TEST_F(TestSessions, MultipleSessionBeginEnd)
  {
#if ENABLE_UAMQP
    MessageTests::AmqpServerMock mockServer;
    mockServer.EnableTrace(false);
    mockServer.StartListening();

    // Create a connection
    Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
    connectionOptions.Port = mockServer.GetPort();
    connectionOptions.EnableTrace = true;

    class OutgoingConnectionEvents : public ConnectionEvents {
      /** @brief Called when the connection state changes.
       *
       * @param newState The new state of the connection.
       * @param oldState The previous state of the connection.
       */
      void OnConnectionStateChanged(
          Connection const&,
          ConnectionState newState,
          ConnectionState oldState) override
      {
        GTEST_LOG_(INFO) << "Connection state changed. OldState: " << oldState << " -> "
                         << newState;
      };

      /** @brief called when an I/O error has occurred on the connection.
       *
       */
      void OnIOError(Connection const&) override { GTEST_LOG_(INFO) << "Connection IO Error."; };
    };

    OutgoingConnectionEvents connectionEvents;
    Azure::Core::Amqp::_internal::Connection connection(
        "localhost", nullptr, connectionOptions, &connectionEvents);

    connection.Open();
#elif ENABLE_RUST_AMQP
    Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
    connectionOptions.Port = 25672;
    Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, connectionOptions);

    connection.Open({});

#endif

    {
      constexpr const size_t sessionCount = 30;
      GTEST_LOG_(INFO) << "Opening " << sessionCount << " sessions.";
      std::vector<Session> sessions;
      for (size_t i = 0; i < sessionCount; i += 1)
      {
        sessions.push_back(connection.CreateSession());
        sessions.back().Begin({});
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      GTEST_LOG_(INFO) << "Closing " << sessionCount << " sessions.";
      for (auto& session : sessions)
      {
        session.End({});
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    }
    connection.Close({});
#if ENABLE_UAMQP
    mockServer.StopListening();
#endif
  }
#endif // !AZ_PLATFORM_MAC
}}}} // namespace Azure::Core::Amqp::Tests
