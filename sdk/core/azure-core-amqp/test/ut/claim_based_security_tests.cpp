// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/amqp/internal/claims_based_security.hpp>
#include <azure/core/amqp/internal/common/global_state.hpp>
#include <azure/core/amqp/internal/connection.hpp>
#include <azure/core/amqp/internal/message_receiver.hpp>
#include <azure/core/amqp/internal/message_sender.hpp>
#include <azure/core/amqp/internal/session.hpp>
#include <azure/core/platform.hpp>

#include <gtest/gtest.h>

#if ENABLE_RUST_AMQP
#define USE_NATIVE_BROKER
#elif ENABLE_UAMQP
#undef USE_NATIVE_BROKER
#endif

#if defined(USE_NATIVE_BROKER)
#include <azure/core/internal/environment.hpp>
#include <azure/core/url.hpp>
#else
#include "mock_amqp_server.hpp"
#endif

#if ENABLE_UAMQP
#define ENABLE_RUST_CANCEL 1
#elif ENABLE_RUST_AMQP
#define ENABLE_RUST_CANCEL 0
#endif

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  using namespace Azure::Core::Amqp::_internal;
  class TestCbs : public testing::Test {
  protected:
    void SetUp() override {
      auto testBrokerUrl = Azure::Core::_internal::Environment::GetVariable("TEST_BROKER_ADDRESS");
      if (testBrokerUrl.empty())
      {
        GTEST_FATAL_FAILURE_("Could not find required environment variable TEST_BROKER_ADDRESS");
      }
      GTEST_LOG_(INFO) << "Use broker address: " << testBrokerUrl;
      Azure::Core::Url brokerUrl(testBrokerUrl);
      m_brokerEndpoint = brokerUrl;
    }
    void TearDown() override
    { // When the test is torn down, the global state MUST be idle. If it is not, something leaked.
      Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }

    std::string GetBrokerEndpoint() { return m_brokerEndpoint.GetAbsoluteUrl(); }

    std::uint16_t GetPort() { return m_brokerEndpoint.GetPort(); }

    auto CreateAmqpConnection(
        std::string const& containerId
        = testing::UnitTest::GetInstance()->current_test_info()->name(),
        bool enableTracing = false,
        Azure::Core::Context const& context = {})
    {
      ConnectionOptions options;
      options.ContainerId = containerId;
      options.EnableTrace = enableTracing;
      options.Port = GetPort();

      auto connection = Connection("localhost", nullptr, options);
#if ENABLE_RUST_AMQP
      connection.Open(context);
#endif
      return connection;
      (void)context;
    }
    auto CreateAmqpSession(Connection const& connection, Context const& context = {})
    {
      auto session = connection.CreateSession();
#if ENABLE_RUST_AMQP
      session.Begin(context);
#endif
      return session;
      (void)context;
    }

    void CloseAmqpConnection(Connection& connection, Azure::Core::Context const& context = {})
    {
#if ENABLE_RUST_AMQP
      connection.Close(context);
#endif
      (void)connection;
      (void)context;
    }
    void EndAmqpSession(Session& session, Azure::Core::Context const& context = {})
    {
#if ENABLE_RUST_AMQP
      session.End(context);
#endif
      (void)session;
      (void)context;
    }

    void StartServerListening()
    {
#if !defined(USE_NATIVE_BROKER)
      m_mockServer.StartListening();
#endif
    }

    void StopServerListening()
    {
#if !defined(USE_NATIVE_BROKER)
      m_mockServer.StopListening();
#endif
    }

#if !defined(USE_NATIVE_BROKER)
  protected:
    MessageTests::AmqpServerMock m_mockServer;
#endif
  private:
    Azure::Core::Url m_brokerEndpoint{};
#if !defined(USE_NATIVE_BROKER)
    MessageTests::AmqpServerMock m_mockServer;
#endif
  };

  using namespace Azure::Core::Amqp;
  using namespace Azure::Core::Amqp::_internal;
  using namespace Azure::Core::Amqp::_detail;

#if !defined(AZ_PLATFORM_MAC)

#define TEST_OSTREAM_INSERTER(ENUMERATION, ENUMERATOR) \
  { \
    std::stringstream ss; \
    ss << ENUMERATION::ENUMERATOR; \
    EXPECT_EQ(#ENUMERATOR, ss.str()); \
  }

  TEST_F(TestCbs, SimpleCbs)
  {
    auto connection{CreateAmqpConnection()};
    auto session{CreateAmqpSession(connection)};

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

    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }
#endif // !defined(AZ_PLATFORM_MAC)

#if !defined(AZ_PLATFORM_MAC)
#if ENABLE_UAMQP
  // Rust AMQP will fail at the connection level if there is no listener, so this particular test
  // isn't very interesting.
  TEST_F(TestCbs, CbsOpenNoListener)
  {
    ConnectionOptions options;
    options.EnableTrace = true;
    // Pick a port separate from the one that the listener is normally at so we will fail to connect
    // to the server.
    options.Port = GetPort() + 10;
    Connection connection("localhost", nullptr, options);
    Session session{connection.CreateSession()};
    {
      ClaimsBasedSecurity cbs(session);
      GTEST_LOG_(INFO) << "Expected failure for Open because no listener.";

      EXPECT_EQ(CbsOpenResult::Error, cbs.Open());
    }
  }
#endif

  TEST_F(TestCbs, CbsOpen)
  {
    auto connection{CreateAmqpConnection()};
    auto session{CreateAmqpSession(connection)};

    StartServerListening();

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
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
    StopServerListening();
  }

  TEST_F(TestCbs, CbsCancelledOpen)
  {
    auto connection{CreateAmqpConnection()};
    auto session{CreateAmqpSession(connection)};
    StartServerListening();

#if ENABLE_RUST_CANCEL
    {
      GTEST_LOG_(INFO) << "Create CBS object.";
      ClaimsBasedSecurity cbs(session);
      Azure::Core::Context openContext;
      openContext.Cancel();
      CbsOpenResult openResult = cbs.Open(openContext);
      EXPECT_EQ(CbsOpenResult::Cancelled, openResult);
    }
#endif
    StopServerListening();
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }

#endif // !defined(AZ_PLATFORM_MAC)

#if !defined(AZ_PLATFORM_MAC)
  // The native broker doesn't support CBS or management APIs.
#if !defined(USE_NATIVE_BROKER)
  TEST_F(TestCbs, CbsOpenAndPut)
  {
    auto connection{CreateAmqpConnection()};
    auto session{CreateAmqpSession(connection)};

    StartServerListening();
    ConnectionOptions options;

    {
      ClaimsBasedSecurity cbs(session);

      EXPECT_EQ(CbsOpenResult::Ok, cbs.Open());
      GTEST_LOG_(INFO) << "Open Completed.";

      auto putResult = cbs.PutToken(
          Azure::Core::Amqp::_detail::CbsTokenType::Jwt,
          "of one",
          "stringizedToken",
          Azure::DateTime::clock::now() + std::chrono::seconds(90),
          {});
      EXPECT_EQ(CbsOperationResult::Ok, std::get<0>(putResult));
      EXPECT_EQ("OK-put", std::get<2>(putResult));

      cbs.Close();
    }

    StopServerListening();
    EndAmqpSession(session);
    CloseAmqpConnection(connection);
  }
#endif
#endif // !defined(AZ_PLATFORM_MAC)

#if !defined(AZ_PLATFORM_MAC)
  // The native broker doesn't support CBS or management APIs.
#if !defined(USE_NATIVE_BROKER)
  TEST_F(TestCbs, CbsOpenAndPutError)
  {
    {
      auto connection{CreateAmqpConnection()};
      auto session{CreateAmqpSession(connection)};
      StartServerListening();

      {
        ClaimsBasedSecurity cbs(session);

        EXPECT_EQ(CbsOpenResult::Ok, cbs.Open());
        GTEST_LOG_(INFO) << "Open Completed.";
#if !defined(USE_NATIVE_BROKER)
        m_mockServer.ForceCbsError(true);
#endif
        auto putResult = cbs.PutToken(
            Azure::Core::Amqp::_detail::CbsTokenType::Jwt,
            "of one",
            "stringizedToken",
            Azure::DateTime::clock::now() + std::chrono::seconds(90),
            {});
        EXPECT_EQ(CbsOperationResult::Failed, std::get<0>(putResult));
        cbs.Close();
      }

      StopServerListening();
      EndAmqpSession(session);
      CloseAmqpConnection(connection);
    }
  }
#endif

#if ENABLE_RUST_CANCEL

  TEST_F(TestCbs, CbsOpenAndPutCancelled)
  {
    {
      auto connection{CreateAmqpConnection()};
      auto session{CreateAmqpSession(connection)};
      StartServerListening();

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
                Azure::DateTime::clock::now() + std::chrono::seconds(90),
                putContext)));

        cbs.Close();
      }
      StopServerListening();
      EndAmqpSession(session);
      CloseAmqpConnection(connection);
    }
  }
#endif

#endif // !defined(AZ_PLATFORM_MAC)
}}}} // namespace Azure::Core::Amqp::Tests
