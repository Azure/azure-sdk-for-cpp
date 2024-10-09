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
constexpr const uint16_t nativeBrokerPort = 25672;
#else
#include "mock_amqp_server.hpp"
#endif

#if ENABLE_UAMQP
#define ENABLE_RUST_CANCEL 1
#elif ENABLE_RUST_AMQP
#define ENABLE_RUST_CANCEL 0
#endif

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  class TestCbs : public testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override
    { // When the test is torn down, the global state MUST be idle. If it is not, something leaked.
      Azure::Core::Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }

    std::uint16_t GetPort()
    {
#if defined(USE_NATIVE_BROKER)
      return nativeBrokerPort;
#else ENABLE_RUST_AMQP
      return m_mockServer.GetPort();
#endif
    }

    Azure::Core::Amqp::_internal::Connection CreateConnection(
        Azure::Core::Context const& context = {})
    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      options.Port = GetPort();
      options.EnableTrace = true;
      auto connection{Azure::Core::Amqp::_internal::Connection("localhost", nullptr, options)};

#if ENABLE_RUST_AMQP
      connection.Open(context);
#endif
      return connection;
      (void)context;
    }

    Azure::Core::Amqp::_internal::Session CreateSession(
        Azure::Core::Amqp::_internal::Connection& connection,
        Azure::Core::Context const& context = {})
    {
      auto session{connection.CreateSession()};
#if ENABLE_RUST_AMQP
      session.Begin(context);
#endif
      return session;
      (void)context;
    }

    void Cleanup(Azure::Core::Amqp::_internal::Connection& connection)
    {
#if ENABLE_RUST_AMQP
      connection.Close({});
#endif
      (void)connection;
    }

    void Cleanup(Azure::Core::Amqp::_internal::Session& session)
    {
#if ENABLE_RUST_AMQP
      session.End({});
#endif
      (void)session;
    }

    void StartListening()
    {
#if !defined(USE_NATIVE_BROKER)
      m_mockServer.StartListening();
#endif
    }
    void CleanupListening()
    {
#if !defined(USE_NATIVE_BROKER)
      m_mockServer.StopListening();
#endif
    }
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
    auto connection{CreateConnection()};
    auto session{CreateSession(connection)};

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

    Cleanup(session);
    Cleanup(connection);
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
    auto connection{CreateConnection()};
    auto session{CreateSession(connection)};

    StartListening();

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
    Cleanup(session);
    Cleanup(connection);
    CleanupListening();
  }

  TEST_F(TestCbs, CbsCancelledOpen)
  {
    auto connection{CreateConnection()};
    auto session{CreateSession(connection)};
    StartListening();

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
    CleanupListening();
    Cleanup(session);
    Cleanup(connection);
  }

#endif // !defined(AZ_PLATFORM_MAC)

#if !defined(AZ_PLATFORM_MAC)
  TEST_F(TestCbs, CbsOpenAndPut)
  {
    auto connection{CreateConnection()};
    auto session{CreateSession(connection)};

    StartListening();
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

    CleanupListening();
    Cleanup(session);
    Cleanup(connection);
  }
#endif // !defined(AZ_PLATFORM_MAC)

#if !defined(AZ_PLATFORM_MAC)
  TEST_F(TestCbs, CbsOpenAndPutError)
  {
    {
      auto connection{CreateConnection()};
      auto session{CreateSession(connection)};
      StartListening();

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

      CleanupListening();
      Cleanup(session);
      Cleanup(connection);
    }
  }

#if ENABLE_RUST_CANCEL

  TEST_F(TestCbs, CbsOpenAndPutCancelled)
  {
    {
      auto connection{CreateConnection()};
      auto session{CreateSession(connection)};
      StartListening();

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
      CleanupListening();
      Cleanup(session);
      Cleanup(connection);
    }
  }
#endif

#endif // !defined(AZ_PLATFORM_MAC)
}}}} // namespace Azure::Core::Amqp::Tests
