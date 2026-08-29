// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/amqp/internal/claims_based_security.hpp>
#include <azure/core/amqp/internal/common/global_state.hpp>
#include <azure/core/amqp/internal/connection.hpp>
#include <azure/core/amqp/internal/message_receiver.hpp>
#include <azure/core/amqp/internal/message_sender.hpp>
#include <azure/core/amqp/internal/session.hpp>
#include <azure/core/credentials/credentials.hpp>
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/platform.hpp>
#include <azure/core/url.hpp>

#include <chrono>
#include <cstddef>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#if ENABLE_RUST_AMQP
#define USE_NATIVE_BROKER
#elif ENABLE_UAMQP
#undef USE_NATIVE_BROKER
#endif

#if defined(USE_NATIVE_BROKER)
#include <azure/core/internal/environment.hpp>
#else
#include "mock_amqp_server.hpp"
#endif

#if ENABLE_UAMQP
#define ENABLE_RUST_CANCEL 1
#elif ENABLE_RUST_AMQP
#define ENABLE_RUST_CANCEL 0
#endif

#if !defined(AZ_PLATFORM_MAC)
#if ENABLE_UAMQP
namespace {
// Collects the log lines that the code under test writes. The destructor
// removes the listener before the vector goes away, because the AMQP polling
// thread and the token refresh thread can write to the log while this object is
// destroyed.
class LogCapture final {
public:
  LogCapture()
  {
    Azure::Core::Diagnostics::Logger::SetListener(
        [this](Azure::Core::Diagnostics::Logger::Level level, std::string const& message) {
          std::lock_guard<std::mutex> guard(m_mutex);
          m_lines.emplace_back(level, message);
        });
    Azure::Core::Diagnostics::Logger::SetLevel(Azure::Core::Diagnostics::Logger::Level::Verbose);
  }

  ~LogCapture() { Azure::Core::Diagnostics::Logger::SetListener(nullptr); }

  LogCapture(LogCapture const&) = delete;
  LogCapture& operator=(LogCapture const&) = delete;

  std::vector<std::string> Lines(Azure::Core::Diagnostics::Logger::Level level)
  {
    std::lock_guard<std::mutex> guard(m_mutex);
    std::vector<std::string> result;
    for (auto const& line : m_lines)
    {
      if (line.first == level)
      {
        result.push_back(line.second);
      }
    }
    return result;
  }

  void Clear()
  {
    std::lock_guard<std::mutex> guard(m_mutex);
    m_lines.clear();
  }

private:
  std::mutex m_mutex;
  std::vector<std::pair<Azure::Core::Diagnostics::Logger::Level, std::string>> m_lines;
};

Azure::Core::Diagnostics::Logger::Level const AllLogLevels[]{
    Azure::Core::Diagnostics::Logger::Level::Verbose,
    Azure::Core::Diagnostics::Logger::Level::Informational,
    Azure::Core::Diagnostics::Logger::Level::Warning,
    Azure::Core::Diagnostics::Logger::Level::Error};
} // namespace
#endif // ENABLE_UAMQP
#endif // !defined(AZ_PLATFORM_MAC)

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  using namespace Azure::Core::Amqp::_internal;
  class TestCbs : public testing::Test {
  protected:
    void SetUp() override
    {
#if defined(USE_NATIVE_BROKER)
      auto testBrokerUrl = Azure::Core::_internal::Environment::GetVariable("TEST_BROKER_ADDRESS");
      if (testBrokerUrl.empty())
      {
        GTEST_FATAL_FAILURE_("Could not find required environment variable TEST_BROKER_ADDRESS");
      }
      GTEST_LOG_(INFO) << "Use broker address: " << testBrokerUrl;
      Azure::Core::Url brokerUrl(testBrokerUrl);
#else
      Azure::Core::Url brokerUrl("amqp://localhost:" + std::to_string(m_mockServer.GetPort()));
#endif
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
      GTEST_LOG_(INFO) << "Create connection with container id: " << containerId;
      auto connection = Connection("localhost", nullptr, options);
#if ENABLE_RUST_AMQP
      connection.Open(context);
#endif
      return connection;
      (void)context;
    }
    auto CreateAmqpSession(Connection const& connection, Context const& context = {})
    {
      GTEST_LOG_(INFO) << "Create session on connection";
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
      GTEST_LOG_(INFO) << "Create and destroy CBS object.";
      ClaimsBasedSecurity cbs(session);
    }

    {
      // Create two cbs objects
      GTEST_LOG_(INFO) << "Create and destroy Two CBS objects.";
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

      // Build the capture after the connection and the session, so its
      // destructor removes the listener before those objects go away.
      LogCapture logCapture;

      EXPECT_EQ(CbsOpenResult::Error, cbs.Open());

      auto const firstOpenWarnings
          = logCapture.Lines(Azure::Core::Diagnostics::Logger::Level::Warning);

      bool foundConnectionIoError = false;
      bool foundManagementSenderFailure = false;
      for (auto const& line : firstOpenWarnings)
      {
        if (line.find("Connection I/O error.") != std::string::npos
            && line.find("instance ") != std::string::npos
            && line.find("host localhost:") != std::string::npos
            && line.find(", state ") != std::string::npos)
        {
          foundConnectionIoError = true;
        }
        if (line.find("ManagementClientImpl::Open: Message sender open failed.")
                != std::string::npos
            && line.find("Node: $cbs.") != std::string::npos)
        {
          foundManagementSenderFailure = true;
        }
      }
      EXPECT_TRUE(foundConnectionIoError);
      EXPECT_TRUE(foundManagementSenderFailure);

      logCapture.Clear();

      // The second open takes the branch that refuses to open the object again.
      // That branch does no network work, so it must give the reader a line of
      // its own, and that line must differ from the line the failed connection
      // wrote.
      EXPECT_EQ(CbsOpenResult::Error, cbs.Open());

      auto const secondOpenWarnings
          = logCapture.Lines(Azure::Core::Diagnostics::Logger::Level::Warning);
      ASSERT_EQ(static_cast<std::size_t>(1), secondOpenWarnings.size());
      for (auto const& firstOpenWarning : firstOpenWarnings)
      {
        EXPECT_NE(firstOpenWarning, secondOpenWarnings[0]);
      }
    }
  }

  // A CBS open that fails while the connection authenticates an audience must
  // name the result, the audience, and the function that asked for the token.
  // The token itself must stay out of the exception and out of the log.
  TEST_F(TestCbs, AuthenticationFailureNamesTheCbsOpenFailure)
  {
    class SentinelTokenCredential final : public Azure::Core::Credentials::TokenCredential {
      Azure::Core::Credentials::AccessToken GetToken(
          Azure::Core::Credentials::TokenRequestContext const& requestContext,
          Azure::Core::Context const& context) const override
      {
        Azure::Core::Credentials::AccessToken rv;
        rv.Token = "SENTINEL-TOKEN-MUST-NOT-BE-LOGGED";
        rv.ExpiresOn = std::chrono::system_clock::now() + std::chrono::hours(1);
        (void)requestContext;
        (void)context;
        return rv;
      }

    public:
      SentinelTokenCredential() : Azure::Core::Credentials::TokenCredential("Testing") {}
    };

    std::string const sentinel{"SENTINEL-TOKEN-MUST-NOT-BE-LOGGED"};
    auto credential = std::make_shared<SentinelTokenCredential>();

    ConnectionOptions options;
    // Trace is off, so every line that the capture holds comes from the failure
    // path and not from the AMQP trace.
    options.EnableTrace = false;
    // Pick a port separate from the one that the listener is normally at, so the
    // CBS open fails.
    options.Port = GetPort() + 10;

    {
      Connection connection("localhost", credential, options);
      Session session{connection.CreateSession()};
      MessageSenderOptions senderOptions;
      MessageSender sender{session.CreateMessageSender("testEntity", senderOptions, nullptr)};

      // Build the capture after the connection, so its destructor removes the
      // listener before the connection and its polling thread go away.
      LogCapture logCapture;

      bool caught = false;
      try
      {
        auto const openError = sender.Open();
        FAIL() << "Expected the open to throw because there is no listener. "
               << openError.Description;
      }
      catch (std::runtime_error const& e)
      {
        caught = true;
        std::string const what{e.what()};
        EXPECT_NE(std::string::npos, what.find("Error")) << what;
        EXPECT_NE(std::string::npos, what.find("testEntity")) << what;
        EXPECT_NE(std::string::npos, what.find("ConnectionImpl::AuthenticateAudience")) << what;
        EXPECT_EQ(std::string::npos, what.find(sentinel)) << what;

        // The failed open must arrive as the typed exception, so a caller can read the result
        // without matching this text. Catching the base type above is the other half of the
        // contract: an existing handler still sees it.
        auto const* typed
            = dynamic_cast<Azure::Core::Amqp::_detail::CbsOpenFailedException const*>(&e);
        ASSERT_NE(nullptr, typed);
        EXPECT_EQ(Azure::Core::Amqp::_detail::CbsOpenResult::Error, typed->Result);

        // The result alone collapses every transport, TLS and link failure into one value, so
        // the sentence must also carry the reason the layer below gave. A caller that logs the
        // exception and has no log listener has nothing else to read.
        EXPECT_NE(std::string::npos, what.find("reason:")) << what;
        // The reason must name which link failed, not "sender or receiver".
        EXPECT_NE(std::string::npos, what.find("the message sender")) << what;
        // A generic state error must never be the whole story the caller gets.
        EXPECT_EQ(std::string::npos, what.find("Message Sender entered the Error State.")) << what;
      }
      EXPECT_TRUE(caught);

      std::size_t warningsThatNameTheFailure = 0;
      for (auto const& line : logCapture.Lines(Azure::Core::Diagnostics::Logger::Level::Warning))
      {
        if (line.find("testEntity") != std::string::npos
            && line.find("ConnectionImpl::AuthenticateAudience") != std::string::npos)
        {
          ++warningsThatNameTheFailure;
        }
      }
      EXPECT_LT(static_cast<std::size_t>(0), warningsThatNameTheFailure);

      for (auto const& level : AllLogLevels)
      {
        for (auto const& line : logCapture.Lines(level))
        {
          EXPECT_EQ(std::string::npos, line.find(sentinel)) << line;
        }
      }
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
