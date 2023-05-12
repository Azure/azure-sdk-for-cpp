// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "mock_amqp_server.hpp"

#include <azure/core/amqp/claims_based_security.hpp>
#include <azure/core/amqp/connection.hpp>
#include <azure/core/amqp/message_receiver.hpp>
#include <azure/core/amqp/message_sender.hpp>
#include <azure/core/amqp/models/message_source.hpp>
#include <azure/core/amqp/models/message_target.hpp>
#include <azure/core/amqp/models/messaging_values.hpp>
#include <azure/core/amqp/network/amqp_header_detect_transport.hpp>
#include <azure/core/amqp/network/socket_listener.hpp>
#include <azure/core/amqp/session.hpp>
#include <azure/core/platform.hpp>

#include <gtest/gtest.h>

// extern uint16_t FindAvailableSocket();

class TestCbs : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

using namespace Azure::Core::Amqp;
using namespace Azure::Core::Amqp::_internal;
using namespace Azure::Core::Amqp::_detail;

#if !defined(AZ_PLATFORM_MAC)
TEST_F(TestCbs, SimpleCbs)
{

  // Create a connection
  Connection connection("amqp://localhost:5672", {});
  // Create a session
  Session session(connection, nullptr);

  {
    ClaimsBasedSecurity cbs(session);
  }

  {
    // Create two cbs objects
    ClaimsBasedSecurity cbs1(session);
    ClaimsBasedSecurity cbs2(session);
  }
}
#endif // !defined(AZ_PLATFORM_MAC)

#if !defined(AZ_PLATFORM_MAC)
TEST_F(TestCbs, CbsOpen)
{
  {
    MessageTests::AmqpServerMock mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), {});
    Session session(connection);
    {
      ClaimsBasedSecurity cbs(session);
      GTEST_LOG_(INFO) << "Expected failure for Open because no listener." << mockServer.GetPort();

      EXPECT_EQ(CbsOpenResult::Error, cbs.Open());
    }
  }
  {
    MessageTests::AmqpServerMock mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), {});
    Session session(connection);

    mockServer.StartListening();

    {
      ClaimsBasedSecurity cbs(session);
      cbs.SetTrace(true);
      EXPECT_EQ(CbsOpenResult::Ok, cbs.Open());
      GTEST_LOG_(INFO) << "Open Completed.";

      cbs.Close();
    }
    mockServer.StopListening();
  }
}
#endif // !defined(AZ_PLATFORM_MAC)

#if !defined(AZ_PLATFORM_MAC)
TEST_F(TestCbs, CbsOpenAndPut)
{
  {
    MessageTests::AmqpServerMock mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), {});
    Session session(connection);

    mockServer.StartListening();

    {
      ClaimsBasedSecurity cbs(session);
      cbs.SetTrace(true);

      EXPECT_EQ(CbsOpenResult::Ok, cbs.Open());
      GTEST_LOG_(INFO) << "Open Completed.";

      auto putResult = cbs.PutToken(
          Azure::Core::Amqp::_detail::CbsTokenType::Sas, "of one", "stringizedToken");
      EXPECT_EQ(CbsOperationResult::Ok, std::get<0>(putResult));
      EXPECT_EQ("OK-put", std::get<2>(putResult));

      cbs.Close();
    }

    mockServer.StopListening();
  }
}
#endif // !defined(AZ_PLATFORM_MAC)

#if !defined(AZ_PLATFORM_MAC)
TEST_F(TestCbs, CbsOpenAndPutError)
{
  {
    MessageTests::AmqpServerMock mockServer;

    Connection connection("amqp://localhost:" + std::to_string(mockServer.GetPort()), {});
    Session session(connection);

    mockServer.StartListening();

    {
      ClaimsBasedSecurity cbs(session);
      cbs.SetTrace(true);

      EXPECT_EQ(CbsOpenResult::Ok, cbs.Open());
      GTEST_LOG_(INFO) << "Open Completed.";

      mockServer.ForceCbsError(true);
      EXPECT_ANY_THROW(
          auto putResult = cbs.PutToken(
              Azure::Core::Amqp::_detail::CbsTokenType::Sas, "of one", "stringizedToken"););

      cbs.Close();
    }

    mockServer.StopListening();
  }
}
#endif // !defined(AZ_PLATFORM_MAC)
