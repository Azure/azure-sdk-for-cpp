// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "mock_amqp_server.hpp"

#include <azure/core/amqp/internal/claims_based_security.hpp>
#include <azure/core/amqp/internal/common/global_state.hpp>
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
    void TearDown() override
    { // When the test is torn down, the global state MUST be idle. If it is not, something leaked.
      Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }
  };

  using namespace Azure::Core::Amqp;
  using namespace Azure::Core::Amqp::_internal;
  using namespace Azure::Core::Amqp::_detail;

#if !defined(AZURE_PLATFORM_MAC)

#define TEST_OSTREAM_INSERTER(ENUMERATION, ENUMERATOR) \
  { \
    std::stringstream ss; \
    ss << ENUMERATION::ENUMERATOR; \
    EXPECT_EQ(#ENUMERATOR, ss.str()); \
  }

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
      TEST_OSTREAM_INSERTER(CbsOperationResult, Failed);
      TEST_OSTREAM_INSERTER(CbsOperationResult, Ok);
      TEST_OSTREAM_INSERTER(CbsOperationResult, Cancelled);
      TEST_OSTREAM_INSERTER(CbsOperationResult, InstanceClosed);
      TEST_OSTREAM_INSERTER(CbsOperationResult, Invalid);
      TEST_OSTREAM_INSERTER(CbsOperationResult, Error);
      GTEST_LOG_(INFO) << "CbsOperations" << static_cast<CbsOperationResult>(32768);
    }
    {
      TEST_OSTREAM_INSERTER(CbsOpenResult, Ok);
      TEST_OSTREAM_INSERTER(CbsOpenResult, Cancelled);
      TEST_OSTREAM_INSERTER(CbsOpenResult, Invalid);
      TEST_OSTREAM_INSERTER(CbsOpenResult, Error);

      GTEST_LOG_(INFO) << "CbsOpens" << static_cast<CbsOpenResult>(32768);
    }
  }
#endif // !defined(AZURE_PLATFORM_MAC)

#if !defined(AZURE_PLATFORM_MAC)
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

    mockServer.EnableTrace(false);

    ConnectionOptions options;
    options.Port = mockServer.GetPort();
    options.EnableTrace = true;
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

  TEST_F(TestCbs, CbsCancelledOpen)
  {
    MessageTests::AmqpServerMock mockServer;

    mockServer.EnableTrace(false);

    ConnectionOptions options;
    options.Port = mockServer.GetPort();
    options.EnableTrace = true;
    options.ContainerId = testing::UnitTest::GetInstance()->current_test_info()->test_case_name();
    Connection connection("localhost", nullptr, options);
    Session session{connection.CreateSession()};

    mockServer.StartListening();

    {
      GTEST_LOG_(INFO) << "Create CBS object.";
      ClaimsBasedSecurity cbs(session);
      Azure::Core::Context openContext;
      openContext.Cancel();
      CbsOpenResult openResult = cbs.Open(openContext);
      EXPECT_EQ(CbsOpenResult::Cancelled, openResult);
    }
    mockServer.StopListening();
  }

#endif // !defined(AZURE_PLATFORM_MAC)

#if !defined(AZURE_PLATFORM_MAC)
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
#endif // !defined(AZURE_PLATFORM_MAC)

#if !defined(AZURE_PLATFORM_MAC)
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
        auto putResult = cbs.PutToken(
            Azure::Core::Amqp::_detail::CbsTokenType::Sas, "of one", "stringizedToken");
        EXPECT_EQ(CbsOperationResult::Failed, std::get<0>(putResult));
        cbs.Close();
      }

      mockServer.StopListening();
    }
  }

  TEST_F(TestCbs, CbsOpenAndPutCancelled)
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

        Azure::Core::Context putContext;
        putContext.Cancel();

        //        mockServer.ForceCbsError(true);
        EXPECT_EQ(
            CbsOperationResult::Cancelled,
            std::get<0>(cbs.PutToken(
                Azure::Core::Amqp::_detail::CbsTokenType::Sas,
                "of one",
                "stringizedToken",
                putContext)));

        cbs.Close();
      }

      mockServer.StopListening();
    }
  }

#endif // !defined(AZURE_PLATFORM_MAC)
}}}} // namespace Azure::Core::Amqp::Tests
