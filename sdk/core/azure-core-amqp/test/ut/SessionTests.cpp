// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#include <gtest/gtest.h>

#include "azure/core/amqp/session.hpp"
#include "azure/core/amqp/common/async_operation_queue.hpp"
#include "azure/core/amqp/connection.hpp"
#include "azure/core/amqp/message_receiver.hpp"
#include "azure/core/amqp/models/messaging_values.hpp"
#include "azure/core/amqp/network/amqp_header_detect_transport.hpp"
#include "azure/core/amqp/network/socket_listener.hpp"
#include "azure/core/amqp/network/socket_transport.hpp"
#include <functional>

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

TEST_F(TestSessions, SessionBeginEnd)
{
  class TestListener : public Network::SocketListener {
  public:
    TestListener(uint16_t port) : SocketListener(port, nullptr) {}

    std::shared_ptr<Network::Transport> WaitForResult()
    {
      auto result = m_listenerQueue.WaitForPolledResult(*this);
      return std::get<0>(*result);
    }

  private:
    Common::AsyncOperationQueue<std::shared_ptr<Network::Transport>> m_listenerQueue;

    virtual void OnSocketAccepted(XIO_INSTANCE_TAG* xio)
    {
      // Capture the XIO into a transport so it won't leak.
      m_listenerQueue.CompleteOperation(std::make_shared<Network::Transport>(xio));
    }
  };

  // Ensure someone is listening on the connection for when we call Session.Begin.
  TestListener listener(5672);
  listener.Start();

  // Create a connection
  Connection connection("amqp://localhost:5672", nullptr, {});

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
