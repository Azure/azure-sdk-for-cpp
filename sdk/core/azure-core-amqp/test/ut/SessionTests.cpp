// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#include <gtest/gtest.h>

#include "azure/core/amqp/common/async_operation_queue.hpp"
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

using namespace Azure::Core::_internal::Amqp;

TEST_F(TestSessions, SimpleSession)
{

  // Create a connection
  Connection connection("amqp://localhost:5672", nullptr, {});
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
  Connection connection("amqp://localhost:5672", nullptr, {});

  {
    Session session(connection, nullptr);

    // Verify defaults are something "reasonable".
    EXPECT_EQ(1, session.GetIncomingWindow());
    EXPECT_EQ(std::numeric_limits<uint32_t>::max(), session.GetHandleMax());
    EXPECT_EQ(1, session.GetOutgoingWindow());
  }

  {
    Session session(connection, nullptr);
    EXPECT_NO_THROW(session.SetHandleMax(37));
    EXPECT_EQ(37, session.GetHandleMax());
  }
  {
    Session session(connection, nullptr);
    EXPECT_NO_THROW(session.SetIncomingWindow(9278789));
    EXPECT_EQ(9278789, session.GetIncomingWindow());
  }
  {
    Session session(connection, nullptr);
    EXPECT_NO_THROW(session.SetOutgoingWindow(32798));
    EXPECT_EQ(32798, session.GetOutgoingWindow());
  }
}

uint16_t FindAvailableSocket()
{
  std::random_device dev;

  uint16_t testPort = dev() % 1000 + 0x5000;

  int count = 0;
  while (count < 20)
  {
    GTEST_LOG_(INFO) << "Trying Test port: " << testPort;

    auto sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(testPort);

    if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0)
    {
      // We were able to bind to the port, so it's available.
#if defined(AZ_PLATFORM_WINDOWS)
      closesocket(sock);
#else
      close(sock);
#endif
      return testPort;
    }
    count += 1;
  }
  throw std::runtime_error("Could not find available test port.");
}

TEST_F(TestSessions, SessionBeginEnd)
{
  class TestListenerEvents : public Network::SocketListenerEvents {
  public:
    std::shared_ptr<Network::Transport> WaitForResult(
        Network::SocketListener const& listener,
        Azure::Core::Context context = {})
    {
      auto result = m_listenerQueue.WaitForPolledResult(context, listener);
      return std::get<0>(*result);
    }

  private:
    Common::AsyncOperationQueue<std::shared_ptr<Network::Transport>> m_listenerQueue;

    virtual void OnSocketAccepted(XIO_INSTANCE_TAG* xio)
    {
      // Capture the XIO into a transport so it won't leak.
      m_listenerQueue.CompleteOperation(std::make_shared<Network::Transport>(xio, nullptr));
    }
  };

  // Ensure someone is listening on the connection for when we call Session.Begin.
  TestListenerEvents events;
  uint16_t testPort = FindAvailableSocket();
  Network::SocketListener listener(testPort, &events);
  listener.Start();

  // Create a connection
  Connection connection("amqp://localhost:" + std::to_string(testPort), nullptr, {});

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
