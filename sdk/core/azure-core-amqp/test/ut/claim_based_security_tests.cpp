// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "mock_amqp_server.hpp"

#include <azure/core/amqp/internal/claims_based_security.hpp>
#include <azure/core/amqp/internal/connection.hpp>
#include <azure/core/amqp/internal/message_receiver.hpp>
#include <azure/core/amqp/internal/message_sender.hpp>
#include <azure/core/amqp/internal/session.hpp>
#include <azure/core/platform.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
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
    Connection connection("localhost", nullptr, {});
    // Create a session
    Session session{connection.CreateSession()};

    {
      ClaimsBasedSecurity cbs(session);
    }

    {
      // Create two cbs objects
      ClaimsBasedSecurity cbs1(session);
      ClaimsBasedSecurity cbs2(session);
    }

    {
      GTEST_LOG_(INFO) << "CbsOperations" << CbsOperationResult::Error;
      GTEST_LOG_(INFO) << "CbsOperations" << CbsOperationResult::Invalid;
      GTEST_LOG_(INFO) << "CbsOperations" << CbsOperationResult::Failed;
      GTEST_LOG_(INFO) << "CbsOperations" << CbsOperationResult::InstanceClosed;
      GTEST_LOG_(INFO) << "CbsOperations" << static_cast<CbsOperationResult>(32768);
    }
    {
      GTEST_LOG_(INFO) << "CbsOpens" << CbsOpenResult::Cancelled;
      GTEST_LOG_(INFO) << "CbsOpens" << CbsOpenResult::Error;
      GTEST_LOG_(INFO) << "CbsOpens" << CbsOpenResult::Ok;
      GTEST_LOG_(INFO) << "CbsOpens" << CbsOpenResult::Invalid;
      GTEST_LOG_(INFO) << "CbsOpens" << static_cast<CbsOpenResult>(32768);
    }
  }
#endif // !defined(AZ_PLATFORM_MAC)

#if !defined(AZ_PLATFORM_MAC)
  TEST_F(TestCbs, CbsOpenNoListener)
  {
    MessageTests::AmqpServerMock mockServer;
    ConnectionOptions options;
    options.EnableTrace = true;
    options.Port = mockServer.GetPort();
    Connection connection("localhost", nullptr, options);
    Session session{connection.CreateSession()};
    {
      ClaimsBasedSecurity cbs(session);
      GTEST_LOG_(INFO) << "Expected failure for Open because no listener." << mockServer.GetPort();

      EXPECT_EQ(CbsOpenResult::Error, cbs.Open());
    }
  }
  TEST_F(TestCbs, CbsOpen)
  {
    MessageTests::AmqpServerMock mockServer;

    ConnectionOptions options;
    options.Port = mockServer.GetPort();
    options.EnableTrace = false;
    options.ContainerId = testing::UnitTest::GetInstance()->current_test_info()->test_case_name();
    Connection connection("localhost", nullptr, options);
    Session session{connection.CreateSession()};

    mockServer.StartListening();

    {
      GTEST_LOG_(INFO) << "Create CBS object.";
      ClaimsBasedSecurity cbs(session);
      CbsOpenResult openResult = cbs.Open();
      EXPECT_EQ(CbsOpenResult::Ok, openResult);
      GTEST_LOG_(INFO) << "Open Completed.";
      if (openResult == CbsOpenResult::Ok)
      {
        cbs.Close();
      }
    }
    mockServer.StopListening();
  }
#endif // !defined(AZ_PLATFORM_MAC)

#if !defined(AZ_PLATFORM_MAC)
  TEST_F(TestCbs, CbsOpenAndPut)
  {
    MessageTests::AmqpServerMock mockServer;

    ConnectionOptions options;
    options.Port = mockServer.GetPort();
    options.EnableTrace = true;
    Connection connection("localhost", nullptr, options);
    Session session{connection.CreateSession()};

    mockServer.StartListening();

    {
      ClaimsBasedSecurity cbs(session);

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
#endif // !defined(AZ_PLATFORM_MAC)

#if !defined(AZ_PLATFORM_MAC)
  TEST_F(TestCbs, CbsOpenAndPutError)
  {
    {
      MessageTests::AmqpServerMock mockServer;

      ConnectionOptions options;
      options.Port = mockServer.GetPort();
      options.EnableTrace = true;
      Connection connection("localhost", nullptr, options);
      Session session{connection.CreateSession()};

      mockServer.StartListening();

      {
        ClaimsBasedSecurity cbs(session);

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
}}}} // namespace Azure::Core::Amqp::Tests
