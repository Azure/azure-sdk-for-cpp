// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <gtest/gtest.h>

#include "azure/core/amqp/common/async_operation_queue.hpp"
#include "azure/core/amqp/common/global_state.hpp"
#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/message_receiver.hpp"
#include "azure/core/amqp/models/messaging_values.hpp"
#include "azure/core/amqp/network/amqp_header_detect_transport.hpp"
#include "azure/core/amqp/network/socket_listener.hpp"
#include "azure/core/amqp/network/socket_transport.hpp"
#include "azure/core/amqp/session.hpp"
#include <azure/core/context.hpp>
#include <functional>
#include <random>

#include <azure/core/platform.hpp>
#if defined(AZ_PLATFORM_POSIX)
#include <netinet/in.h> // for sockaddr_in
#include <poll.h> // for poll()
#include <sys/socket.h> // for socket shutdown
#elif defined(AZ_PLATFORM_WINDOWS)
#include <winsock2.h> // for WSAPoll();
#ifdef max
#undef max
#endif
#endif // AZ_PLATFORM_POSIX/AZ_PLATFORM_WINDOWS

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
  Azure::Core::Amqp::_internal::Connection connection("localhost", {});
  {
    // Create a session
    Session session(connection, nullptr);
  }

  {
    // Create two sessions
    Session session1(connection, nullptr);
    Session session2(connection, nullptr);

    session1.End("", "");
  }

#if 0
  // Create a sender
  Sender sender(session, "test");

  // Create a receiver
  Receiver receiver(session, "test");

  // Create a message
  Message message;

  // Send a message
  sender.Send(message);

  // Receive a message
  receiver.Receive();

  // Close the connection
  connection.Close();
#endif
}

TEST_F(TestSessions, SessionProperties)
{ // Create a connection
  Azure::Core::Amqp::_internal::Connection connection("localhost", {});

  {
    Session session(connection, nullptr);

    // Verify defaults are something "reasonable".
    EXPECT_EQ(1, session.GetIncomingWindow());
    EXPECT_EQ(std::numeric_limits<uint32_t>::max(), session.GetHandleMax());
    EXPECT_EQ(1, session.GetOutgoingWindow());
  }

  {
    SessionOptions options;
    options.MaximumLinkCount = 37;
    Session session(connection, nullptr, options);
    EXPECT_EQ(37, session.GetHandleMax());
  }
  {
    SessionOptions options;
    options.InitialIncomingWindowSize = 1909119;
    Session session(connection, nullptr, options);
    EXPECT_EQ(1909119, session.GetIncomingWindow());
  }
  {
    SessionOptions options;
    options.InitialOutgoingWindowSize = 1909119;
    Session session(connection, nullptr, options);
    EXPECT_EQ(1909119, session.GetOutgoingWindow());
  }
}
#endif // !AZ_PLATFORM_MAC
uint16_t FindAvailableSocket()
{
  // Ensure that the global state for the AMQP stack is initialized. Normally this is done by the
  // network facing objects, but this is called before those objects are initialized.
  //
  // This may hide bugs in some of the global objects, but it is needed to ensure that the port we
  // choose for the tests is available.
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
      testPort = dev() % 1000 + 5000;
    } while (testPort == AmqpTlsPort);

    GTEST_LOG_(INFO) << "Trying Test port: " << testPort;

    auto sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock != -1)
    {
      sockaddr_in addr;
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

#if !defined(AZ_PLATFORM_MAC)
TEST_F(TestSessions, SessionBeginEnd)
{
  class TestListenerEvents : public Network::_internal::SocketListenerEvents {
  public:
    std::shared_ptr<Network::_internal::Transport> WaitForResult(
        Network::_internal::SocketListener const& listener,
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
  Network::_internal::SocketListener listener(testPort, &events);
  listener.Start();

  // Create a connection
  Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
  connectionOptions.Port = testPort;
  Azure::Core::Amqp::_internal::Connection connection("localhost", connectionOptions);

  {
    Session session(connection, nullptr);

    session.Begin();
  }

  {
    Session session(connection, nullptr);

    session.Begin();
    session.End("", "");
  }

  listener.Stop();
}
#endif // !AZ_PLATFORM_MAC
