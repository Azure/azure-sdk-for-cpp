// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../../src/amqp/private/cbs_open_failure.hpp"
#include "../../src/amqp/private/token_refresh.hpp"
#include "azure/core/amqp/internal/common/async_operation_queue.hpp"
#include "azure/core/amqp/internal/connection.hpp"
#include "azure/core/amqp/internal/message_receiver.hpp"
#include "azure/core/amqp/internal/models/amqp_protocol.hpp"
#include "azure/core/amqp/internal/models/messaging_values.hpp"
#include "azure/core/amqp/internal/network/amqp_header_detect_transport.hpp"
#include "azure/core/amqp/internal/network/socket_listener.hpp"
#include "azure/core/amqp/internal/network/socket_transport.hpp"
#include "azure/core/amqp/internal/session.hpp"
#include "azure/core/internal/environment.hpp"
#include "azure/core/url.hpp"
#include "mock_amqp_server.hpp"

#include <azure/core/context.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/platform.hpp>

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>

#include <gtest/gtest.h>

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  extern uint16_t FindAvailableSocket();

  class TestConnections : public testing::Test {
  protected:
    void SetUp() override {}
    void TearDown() override {}
  };

  // Tests for the rules that decide when a cached CBS token is still good, and
  // when the connection must replace it. These rules are pure functions, so they
  // run on every platform and need no service.
  class TestTokenRefresh : public testing::Test {
  protected:
    static Azure::Core::Credentials::AccessToken TokenExpiringIn(std::chrono::seconds lifetime)
    {
      Azure::Core::Credentials::AccessToken token;
      token.Token = "TestToken";
      token.ExpiresOn = std::chrono::system_clock::now() + lifetime;
      return token;
    }
  };

  TEST_F(TestTokenRefresh, CachedTokenIsUsableWhileItHasLifeLeft)
  {
    auto const now = std::chrono::system_clock::now();
    EXPECT_TRUE(Azure::Core::Amqp::_detail::IsCachedTokenUsable(
        TokenExpiringIn(std::chrono::hours(1)), now));
    EXPECT_TRUE(Azure::Core::Amqp::_detail::IsCachedTokenUsable(
        TokenExpiringIn(std::chrono::minutes(2)), now));
  }

  TEST_F(TestTokenRefresh, CachedTokenIsNotUsableNearOrAfterExpiry)
  {
    auto const now = std::chrono::system_clock::now();
    // Inside the minimum lifetime that a caller may use.
    EXPECT_FALSE(Azure::Core::Amqp::_detail::IsCachedTokenUsable(
        TokenExpiringIn(std::chrono::seconds(10)), now));
    // Already expired.
    EXPECT_FALSE(Azure::Core::Amqp::_detail::IsCachedTokenUsable(
        TokenExpiringIn(std::chrono::seconds(-30)), now));
  }

  TEST_F(TestTokenRefresh, RefreshIsDueOneBufferBeforeExpiry)
  {
    auto const now = std::chrono::system_clock::now();
    // A normal token has a long life, so no refresh is due yet.
    EXPECT_FALSE(Azure::Core::Amqp::_detail::IsTokenRefreshDue(
        TokenExpiringIn(std::chrono::minutes(90)), now));
    // Inside the buffer, so the refresh thread must replace the token.
    EXPECT_TRUE(Azure::Core::Amqp::_detail::IsTokenRefreshDue(
        TokenExpiringIn(std::chrono::minutes(6)), now));
  }

  // A credential can put any value in ExpiresOn, and the cast from
  // Azure::DateTime to a system clock time point throws outside the range of
  // that clock. A default constructed ExpiresOn is year 1. These rules run on
  // the refresh thread, where an exception would end the process, so they must
  // not throw for any value.
  TEST_F(TestTokenRefresh, ExtremeExpiryValuesDoNotThrow)
  {
    auto const now = std::chrono::system_clock::now();

    // A default constructed token reports year 1.
    Azure::Core::Credentials::AccessToken defaultToken;
    EXPECT_NO_THROW({
      EXPECT_FALSE(Azure::Core::Amqp::_detail::IsCachedTokenUsable(defaultToken, now));
      EXPECT_TRUE(Azure::Core::Amqp::_detail::IsTokenRefreshDue(defaultToken, now));
    });

    // A token that reports a year past the range of the system clock.
    Azure::Core::Credentials::AccessToken farFutureToken;
    farFutureToken.Token = "TestToken";
    farFutureToken.ExpiresOn = Azure::DateTime(9999, 12, 31);
    EXPECT_NO_THROW({
      EXPECT_TRUE(Azure::Core::Amqp::_detail::IsCachedTokenUsable(farFutureToken, now));
      EXPECT_FALSE(Azure::Core::Amqp::_detail::IsTokenRefreshDue(farFutureToken, now));
    });
  }

  // A token with a lifetime shorter than the buffer is due as soon as it
  // arrives. The connection must still hand it to a caller, because refusing it
  // would make every call authenticate again.
  TEST_F(TestTokenRefresh, ShortLivedTokenIsDueButStillUsable)
  {
    auto const now = std::chrono::system_clock::now();
    auto const token = TokenExpiringIn(std::chrono::seconds(80));
    EXPECT_TRUE(Azure::Core::Amqp::_detail::IsTokenRefreshDue(token, now));
    EXPECT_TRUE(Azure::Core::Amqp::_detail::IsCachedTokenUsable(token, now));
  }

  // A refresh that fails must keep a token that still works. A sender that is
  // already open never authenticates again, so a token that goes away here
  // never comes back, and the link dies at the expiry.
  TEST_F(TestTokenRefresh, AFailedRefreshKeepsATokenThatStillWorks)
  {
    auto const now = std::chrono::system_clock::now();
    EXPECT_FALSE(Azure::Core::Amqp::_detail::ShouldDropTokenAfterFailedRefresh(
        TokenExpiringIn(std::chrono::minutes(6)), now));
    // A token inside the refresh buffer is due, and it still works.
    EXPECT_FALSE(Azure::Core::Amqp::_detail::ShouldDropTokenAfterFailedRefresh(
        TokenExpiringIn(std::chrono::seconds(80)), now));
  }

  // A token with no life left cannot help a caller, and it cannot save a link
  // that the service already dropped. The next link open must authenticate the
  // audience again.
  TEST_F(TestTokenRefresh, AFailedRefreshDropsATokenWithNoLifeLeft)
  {
    auto const now = std::chrono::system_clock::now();
    EXPECT_TRUE(Azure::Core::Amqp::_detail::ShouldDropTokenAfterFailedRefresh(
        TokenExpiringIn(std::chrono::seconds(10)), now));
    EXPECT_TRUE(Azure::Core::Amqp::_detail::ShouldDropTokenAfterFailedRefresh(
        TokenExpiringIn(std::chrono::seconds(-30)), now));

    // A default token reports year 1, and the cast must not throw.
    Azure::Core::Credentials::AccessToken defaultToken;
    EXPECT_NO_THROW(EXPECT_TRUE(
        Azure::Core::Amqp::_detail::ShouldDropTokenAfterFailedRefresh(defaultToken, now)));
  }

  // The state that the connection shares with the refresh thread.
  TEST_F(TestTokenRefresh, TheSharedStateStartsEmptyAndRuns)
  {
    Azure::Core::Amqp::_detail::TokenRefreshState state;
    EXPECT_FALSE(state.Stop);
    EXPECT_TRUE(state.TokenStore.empty());
    EXPECT_TRUE(state.TokenSessions.empty());
    EXPECT_EQ(0u, state.Generation);
  }

  // The tests below cover the predicates that the refresh thread polls: when a change to
  // the token store must wake it, and when the refresh floor must defer its work.
  // A change to TokenStore must move the sleep deadline of the refresh thread.
  TEST_F(TestTokenRefresh, ShouldWakeTokenRefreshAnswersEachCaseForTheRefreshThread)
  {
    Azure::Core::Amqp::_detail::TokenRefreshState state;
    std::unique_lock<std::mutex> lock(state.Mutex);

    auto const observed = state.Generation;

    EXPECT_FALSE(Azure::Core::Amqp::_detail::ShouldWakeTokenRefresh(state, observed, lock));

    state.TokenStore["amqps://host/audience"] = TokenExpiringIn(std::chrono::seconds(40));
    ++state.Generation;
    EXPECT_TRUE(Azure::Core::Amqp::_detail::ShouldWakeTokenRefresh(state, observed, lock));

    EXPECT_FALSE(Azure::Core::Amqp::_detail::ShouldWakeTokenRefresh(state, state.Generation, lock));

    // A replacement counts, because the map is the only record of the expiry.
    state.TokenStore["amqps://host/audience"] = TokenExpiringIn(std::chrono::hours(1));
    auto const afterAdd = state.Generation;
    ++state.Generation;
    EXPECT_TRUE(Azure::Core::Amqp::_detail::ShouldWakeTokenRefresh(state, afterAdd, lock));

    state.TokenStore.erase("amqps://host/audience");
    auto const afterReplace = state.Generation;
    ++state.Generation;
    EXPECT_TRUE(Azure::Core::Amqp::_detail::ShouldWakeTokenRefresh(state, afterReplace, lock));
    EXPECT_TRUE(state.TokenStore.empty());

    state.Stop = true;
    EXPECT_TRUE(Azure::Core::Amqp::_detail::ShouldWakeTokenRefresh(state, state.Generation, lock));
  }

  TEST_F(TestTokenRefresh, TheRefreshFloorDecidesWhetherAPassDefersItsWork)
  {
    auto const now = std::chrono::system_clock::now();

    EXPECT_FALSE(Azure::Core::Amqp::_detail::ShouldDeferTokenRefreshPass(
        true, now, std::chrono::system_clock::time_point::min()));

    EXPECT_FALSE(Azure::Core::Amqp::_detail::ShouldDeferTokenRefreshPass(
        true, now, now - std::chrono::seconds(1)));

    EXPECT_TRUE(Azure::Core::Amqp::_detail::ShouldDeferTokenRefreshPass(
        true, now, now + Azure::Core::Amqp::_detail::MinimumTokenRefreshInterval));

    EXPECT_FALSE(Azure::Core::Amqp::_detail::ShouldDeferTokenRefreshPass(true, now, now));

    EXPECT_FALSE(Azure::Core::Amqp::_detail::ShouldDeferTokenRefreshPass(
        false, now, now + Azure::Core::Amqp::_detail::MinimumTokenRefreshInterval));
    EXPECT_FALSE(Azure::Core::Amqp::_detail::ShouldDeferTokenRefreshPass(
        false, now, std::chrono::system_clock::time_point::min()));
  }

  // A token due on every scan must still get a refresh once the deferred pass reaches the floor.
  TEST_F(TestTokenRefresh, ADeferredPassRunsAtTheFloorForATokenThatIsAlwaysDue)
  {
    auto const now = std::chrono::system_clock::now();

    auto const shortLivedToken = TokenExpiringIn(std::chrono::seconds(80));
    ASSERT_TRUE(Azure::Core::Amqp::_detail::IsTokenRefreshDue(shortLivedToken, now));

    auto const refreshFloor = now + Azure::Core::Amqp::_detail::MinimumTokenRefreshInterval;

    EXPECT_TRUE(Azure::Core::Amqp::_detail::ShouldDeferTokenRefreshPass(true, now, refreshFloor));

    ASSERT_TRUE(Azure::Core::Amqp::_detail::IsTokenRefreshDue(shortLivedToken, refreshFloor));
    EXPECT_FALSE(
        Azure::Core::Amqp::_detail::ShouldDeferTokenRefreshPass(true, refreshFloor, refreshFloor));
  }

  // A kept token writes nothing to the map, so a short-lived token could expire inside the sleep.
  TEST_F(TestTokenRefresh, AFailedPassRetriesTheKeptTokenAtTheFloor)
  {
    auto const now = std::chrono::system_clock::now();
    auto const scanWake = now + Azure::Core::Amqp::_detail::IdleTokenRefreshPoll;
    auto const refreshFloor = now + Azure::Core::Amqp::_detail::MinimumTokenRefreshInterval;

    auto const keptToken = TokenExpiringIn(std::chrono::seconds(40));
    ASSERT_FALSE(Azure::Core::Amqp::_detail::ShouldDropTokenAfterFailedRefresh(keptToken, now));

    Azure::Core::Amqp::_detail::TokenRefreshState state;
    std::unique_lock<std::mutex> lock(state.Mutex);
    EXPECT_FALSE(Azure::Core::Amqp::_detail::ShouldWakeTokenRefresh(state, state.Generation, lock));

    ASSERT_TRUE(keptToken.ExpiresOn <= Azure::DateTime(scanWake));

    auto const wake = Azure::Core::Amqp::_detail::NextWakeAfterRefreshPass(
        scanWake, refreshFloor, /* keptTokenAfterFailedRefresh */ true);
    EXPECT_EQ(refreshFloor, wake);
    EXPECT_TRUE(keptToken.ExpiresOn > Azure::DateTime(wake));

    EXPECT_FALSE(Azure::Core::Amqp::_detail::ShouldDeferTokenRefreshPass(true, wake, refreshFloor));
  }

  TEST_F(TestTokenRefresh, ASuccessfulPassKeepsTheLaterOfScanWakeAndFloor)
  {
    auto const now = std::chrono::system_clock::now();
    auto const refreshFloor = now + Azure::Core::Amqp::_detail::MinimumTokenRefreshInterval;

    auto const idleWake = now + Azure::Core::Amqp::_detail::IdleTokenRefreshPoll;
    EXPECT_EQ(
        idleWake,
        Azure::Core::Amqp::_detail::NextWakeAfterRefreshPass(idleWake, refreshFloor, false));

    auto const earlyWake = now + std::chrono::seconds(5);
    EXPECT_EQ(
        refreshFloor,
        Azure::Core::Amqp::_detail::NextWakeAfterRefreshPass(earlyWake, refreshFloor, false));
  }

  // The refresh thread co-owns the state block, so the block stays alive after
  // the connection that made it is gone. That is what lets the thread come back
  // from a release that destroyed the connection, take the mutex, and read the
  // stop flag.
  TEST_F(TestTokenRefresh, TheSharedStateOutlivesTheOwnerThatMadeIt)
  {
    std::weak_ptr<Azure::Core::Amqp::_detail::TokenRefreshState> observer;
    std::shared_ptr<Azure::Core::Amqp::_detail::TokenRefreshState> threadCopy;
    {
      auto connectionCopy = std::make_shared<Azure::Core::Amqp::_detail::TokenRefreshState>();
      observer = connectionCopy;
      threadCopy = connectionCopy;
    }
    ASSERT_FALSE(observer.expired());

    // The owner is gone, so this stands for the connection destructor setting
    // the stop flag before it detaches the thread.
    {
      std::unique_lock<std::mutex> lock(threadCopy->Mutex);
      threadCopy->Stop = true;
    }
    EXPECT_TRUE(threadCopy->Stop);

    threadCopy.reset();
    EXPECT_TRUE(observer.expired());
  }

  // Tests for ReleaseOutsideLock, the hold that the refresh thread puts the
  // promoted session in. The session can be the last reference to the
  // connection, and the connection destructor takes the token mutex, so the
  // hold must always drop the pointer with that mutex free.
  class TestReleaseOutsideLock : public testing::Test {
  protected:
    // Records the state of the lock at the moment it is destroyed.
    class LockObserver final {
    public:
      LockObserver(std::unique_lock<std::mutex>& lock, bool& destroyed, bool& lockWasHeld)
          : m_lock{lock}, m_destroyed{destroyed}, m_lockWasHeld{lockWasHeld}
      {
      }

      ~LockObserver()
      {
        m_destroyed = true;
        m_lockWasHeld = m_lock.owns_lock();
      }

    private:
      std::unique_lock<std::mutex>& m_lock;
      bool& m_destroyed;
      bool& m_lockWasHeld;
    };

    std::mutex m_mutex;
    bool m_destroyed{false};
    bool m_lockWasHeld{true};

    std::shared_ptr<LockObserver> MakeObserver(std::unique_lock<std::mutex>& lock)
    {
      return std::make_shared<LockObserver>(lock, m_destroyed, m_lockWasHeld);
    }
  };

  TEST_F(TestReleaseOutsideLock, ReleaseDropsThePointerWithTheLockFree)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    {
      Azure::Core::Amqp::_detail::ReleaseOutsideLock<LockObserver> hold{MakeObserver(lock), lock};
      EXPECT_TRUE(lock.owns_lock());
      EXPECT_FALSE(m_destroyed);
      hold.Release();
      EXPECT_TRUE(m_destroyed);
    }
    EXPECT_FALSE(m_lockWasHeld);
    // The lock is back in the state the caller left it in.
    EXPECT_TRUE(lock.owns_lock());
  }

  // The early return in the refresh path leaves the scope with the lock held
  // and the session still in the hold. The destructor must give up the lock for
  // that release too, or it locks a mutex this thread already holds.
  TEST_F(TestReleaseOutsideLock, TheDestructorAlsoReleasesWithTheLockFree)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    {
      Azure::Core::Amqp::_detail::ReleaseOutsideLock<LockObserver> hold{MakeObserver(lock), lock};
      EXPECT_FALSE(m_destroyed);
    }
    EXPECT_TRUE(m_destroyed);
    EXPECT_FALSE(m_lockWasHeld);
    EXPECT_TRUE(lock.owns_lock());
  }

  // An exception on the refresh path must not leave the release under the lock
  // either.
  TEST_F(TestReleaseOutsideLock, AnExceptionStillReleasesWithTheLockFree)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    bool caught{false};
    try
    {
      Azure::Core::Amqp::_detail::ReleaseOutsideLock<LockObserver> hold{MakeObserver(lock), lock};
      throw std::runtime_error("the refresh failed");
    }
    catch (std::runtime_error const&)
    {
      caught = true;
    }
    EXPECT_TRUE(caught);
    EXPECT_TRUE(m_destroyed);
    EXPECT_FALSE(m_lockWasHeld);
    EXPECT_TRUE(lock.owns_lock());
  }

  // The refresh path releases on the normal path and then leaves the scope, so
  // the pointer is released once and the second call does nothing.
  TEST_F(TestReleaseOutsideLock, ASecondReleaseDoesNothing)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    Azure::Core::Amqp::_detail::ReleaseOutsideLock<LockObserver> hold{MakeObserver(lock), lock};
    hold.Release();
    EXPECT_EQ(nullptr, hold.Get());

    m_destroyed = false;
    hold.Release();
    EXPECT_FALSE(m_destroyed);
    EXPECT_TRUE(lock.owns_lock());
  }

  // The refresh path gives up the token mutex for the network work. A release
  // that happens then must leave the lock free, not take it.
  TEST_F(TestReleaseOutsideLock, AFreeLockStaysFree)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    lock.unlock();
    {
      Azure::Core::Amqp::_detail::ReleaseOutsideLock<LockObserver> hold{MakeObserver(lock), lock};
      hold.Release();
    }
    EXPECT_TRUE(m_destroyed);
    EXPECT_FALSE(m_lockWasHeld);
    EXPECT_FALSE(lock.owns_lock());
  }

  // The hold keeps the pointer usable for the whole refresh, and it does not
  // destroy an object that another owner still holds.
  TEST_F(TestReleaseOutsideLock, TheHoldKeepsThePointerAndSharesIt)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    auto observer = MakeObserver(lock);
    {
      Azure::Core::Amqp::_detail::ReleaseOutsideLock<LockObserver> hold{observer, lock};
      EXPECT_EQ(observer.get(), hold.Get().get());
      hold.Release();
      // This test still owns the object, so the release did not destroy it.
      EXPECT_FALSE(m_destroyed);
    }
    EXPECT_FALSE(m_destroyed);
    observer.reset();
    EXPECT_TRUE(m_destroyed);
  }

  // An empty hold is what the refresh path never builds, but the class must not
  // touch the lock for one.
  TEST_F(TestReleaseOutsideLock, AnEmptyHoldLeavesTheLockAlone)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    {
      Azure::Core::Amqp::_detail::ReleaseOutsideLock<LockObserver> hold{nullptr, lock};
      hold.Release();
      EXPECT_TRUE(lock.owns_lock());
    }
    EXPECT_TRUE(lock.owns_lock());
    EXPECT_FALSE(m_destroyed);
  }

  // A CBS open that fails gives the caller one sentence. That sentence must name
  // the result, the audience, and the function that asked for the token, because
  // the two callers fail for different reasons. The text builders are pure
  // functions, so they run on every platform and need no service.
  class TestCbsOpenFailureText : public testing::Test {
  protected:
    static std::string const& Audience()
    {
      static std::string const audience{"amqps://example.servicebus.windows.net/eh1"};
      return audience;
    }
  };

  TEST_F(TestCbsOpenFailureText, TheFailureTextNamesTheResultTheAudienceAndTheCaller)
  {
    struct
    {
      Azure::Core::Amqp::_detail::CbsOpenResult Result;
      std::string EnumeratorName;
    } const testCases[]{
        {Azure::Core::Amqp::_detail::CbsOpenResult::Invalid, "Invalid"},
        {Azure::Core::Amqp::_detail::CbsOpenResult::Error, "Error"},
        {Azure::Core::Amqp::_detail::CbsOpenResult::Cancelled, "Cancelled"},
    };

    for (auto const& testCase : testCases)
    {
      auto const text = Azure::Core::Amqp::_detail::DescribeCbsOpenFailure(
          testCase.Result, Audience(), Azure::Core::Amqp::_detail::CbsOpenCaller::Authenticate);
      EXPECT_NE(std::string::npos, text.find(testCase.EnumeratorName)) << text;
      EXPECT_NE(std::string::npos, text.find(Audience())) << text;
      EXPECT_NE(std::string::npos, text.find("ConnectionImpl::AuthenticateAudience")) << text;
    }

    auto const refreshText = Azure::Core::Amqp::_detail::DescribeCbsOpenFailure(
        Azure::Core::Amqp::_detail::CbsOpenResult::Error,
        Audience(),
        Azure::Core::Amqp::_detail::CbsOpenCaller::Refresh);
    EXPECT_NE(std::string::npos, refreshText.find("ConnectionImpl::RefreshTokenForAudience"))
        << refreshText;
    EXPECT_EQ(std::string::npos, refreshText.find("ConnectionImpl::AuthenticateAudience"))
        << refreshText;
  }

  TEST_F(TestCbsOpenFailureText, TheLogTextAddsTheTokenTypeAndTheExpiry)
  {
    Azure::DateTime const expiresOn(2035, 1, 2, 3, 4, 5);

    auto const sasText = Azure::Core::Amqp::_detail::FormatCbsOpenFailureLog(
        Azure::Core::Amqp::_detail::CbsOpenResult::Error,
        Audience(),
        Azure::Core::Amqp::_detail::CbsTokenType::Sas,
        expiresOn,
        Azure::Core::Amqp::_detail::CbsOpenCaller::Refresh);
    EXPECT_NE(std::string::npos, sasText.find("Sas")) << sasText;
    // Look for the year only. A change to the date format must not break this
    // test.
    EXPECT_NE(std::string::npos, sasText.find("2035")) << sasText;
    EXPECT_NE(std::string::npos, sasText.find(Audience())) << sasText;
    EXPECT_NE(std::string::npos, sasText.find("ConnectionImpl::RefreshTokenForAudience"))
        << sasText;

    auto const jwtText = Azure::Core::Amqp::_detail::FormatCbsOpenFailureLog(
        Azure::Core::Amqp::_detail::CbsOpenResult::Error,
        Audience(),
        Azure::Core::Amqp::_detail::CbsTokenType::Jwt,
        expiresOn,
        Azure::Core::Amqp::_detail::CbsOpenCaller::Refresh);
    EXPECT_NE(std::string::npos, jwtText.find("Jwt")) << jwtText;
    EXPECT_NE(std::string::npos, jwtText.find("2035")) << jwtText;
  }

#if !defined(AZ_PLATFORM_MAC)
  TEST_F(TestConnections, SimpleConnection)
  {
    {
      // Create a connection
      Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
      connectionOptions.Port = Azure::Core::Amqp::_internal::AmqpPort;

      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, {});
    }
    {
      // Create a connection
      Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
      connectionOptions.Port = Azure::Core::Amqp::_internal::AmqpPort;

      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, connectionOptions);
    }
#if ENABLE_UAMQP
    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      auto socketTransport{Azure::Core::Amqp::Network::_internal::SocketTransportFactory::Create(
          "localhost", Azure::Core::Amqp::_internal::AmqpPort)};

      Azure::Core::Amqp::_internal::Connection connection(
          socketTransport, options, nullptr, nullptr);
    }
#endif
  }

  TEST_F(TestConnections, ConnectionAttributes)
  {
    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      options.IdleTimeout = std::chrono::milliseconds(1532);

      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, options);

      EXPECT_EQ(connection.GetPort(), 5671);
      EXPECT_EQ(connection.GetHost(), "localhost");

      auto idleTimeout = connection.GetIdleTimeout();
      (void)idleTimeout;
      EXPECT_EQ(std::chrono::milliseconds(1532), connection.GetIdleTimeout());
    }
    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      options.MaxFrameSize = 1024 * 64;
      options.Port = Azure::Core::Amqp::_internal::AmqpPort;
      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, options);
      EXPECT_EQ(connection.GetPort(), 5672);
      EXPECT_EQ(connection.GetHost(), "localhost");

      auto maxFrameSize = connection.GetMaxFrameSize();
      (void)maxFrameSize;
      EXPECT_EQ(1024 * 64, connection.GetMaxFrameSize());
#if ENABLE_UAMQP
      EXPECT_NO_THROW(
          connection.GetRemoteMaxFrameSize()); // Likely doesn't work unless there's a remote.
#endif
    }

    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      options.MaxChannelCount = 128;
      options.Port = Azure::Core::Amqp::_internal::AmqpPort;

      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, options);
      EXPECT_EQ(connection.GetPort(), 5672);
      EXPECT_EQ(connection.GetHost(), "localhost");

      auto maxChannel = connection.GetMaxChannel();
      EXPECT_EQ(128, connection.GetMaxChannel());
      (void)maxChannel;
    }

    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      options.MaxChannelCount = 128;

      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, options);
#if ENABLE_UAMQP
      // Ratio must be a number between 0 and 1.
      EXPECT_NO_THROW(connection.SetIdleEmptyFrameSendPercentage(0.5));
#endif
    }

    {
      Azure::Core::Amqp::_internal::ConnectionOptions options;
      options.MaxChannelCount = 128;
      options.Properties[Azure::Core::Amqp::Models::AmqpSymbol{"test"}] = "test";

      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, options);
      GTEST_LOG_(INFO) << connection.GetProperties();
      EXPECT_EQ(
          Azure::Core::Amqp::Models::AmqpValue{"test"},
          connection.GetProperties()[Azure::Core::Amqp::Models::AmqpSymbol{"test"}]);
    }
  }

  TEST_F(TestConnections, ConnectionOpenClose)
  {
#if ENABLE_UAMQP
    class TestListener : public Azure::Core::Amqp::Network::_detail::SocketListenerEvents {
    public:
      std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> WaitForResult(
          Azure::Core::Amqp::Network::_detail::SocketListener const& listener,
          Azure::Core::Context const& context = {})
      {
        GTEST_LOG_(INFO) << "Waiting for listener to accept connection.";
        auto result = m_listenerQueue.WaitForPolledResult(context, listener);
        return std::get<0>(*result);
      }

    private:
      Azure::Core::Amqp::Common::_internal::AsyncOperationQueue<
          std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport>>
          m_listenerQueue;

      virtual void OnSocketAccepted(
          std::shared_ptr<Azure::Core::Amqp::Network::_internal::Transport> transport)
      {
        GTEST_LOG_(INFO) << "Socket for listener accepted connection.";
        m_listenerQueue.CompleteOperation(transport);
      }
    };

    {
      // Ensure someone is listening on the connection for when we call connection.Open.

      uint16_t testPort = FindAvailableSocket();

      GTEST_LOG_(INFO) << "Test listener using port: " << testPort;

      TestListener listenerEvents;
      Azure::Core::Amqp::Network::_detail::SocketListener listener(testPort, &listenerEvents);
      EXPECT_NO_THROW(listener.Start());

      // Create a connection
      Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
      connectionOptions.Port = testPort;
      Azure::Core::Amqp::_internal::Connection connection("localhost", nullptr, connectionOptions);

      // Open the connection
      connection.Open({});

      // Ensure that we got an OnComplete callback within 5 seconds.
      auto transport = listenerEvents.WaitForResult(
          listener,
          Azure::Core::Context{std::chrono::system_clock::now() + std::chrono::seconds(5)});

      // Now we can close the connection.
      connection.Close("xxx", "yyy", {}, {});
      listener.Stop();
    }
#else
    // Create a connection
    auto testBrokerUrl = Azure::Core::_internal::Environment::GetVariable("TEST_BROKER_ADDRESS");
    if (testBrokerUrl.empty())
    {
      GTEST_FATAL_FAILURE_("Could not find required environment variable TEST_BROKER_ADDRESS");
    }
    Azure::Core::Url brokerUrl(testBrokerUrl);
    Azure::Core::Amqp::_internal::ConnectionOptions connectionOptions;
    connectionOptions.Port = brokerUrl.GetPort();
    Azure::Core::Amqp::_internal::Connection connection(
        brokerUrl.GetHost(), nullptr, connectionOptions);

    // Open the connection
    connection.Open({});

    connection.Close({});

#endif
  }
#endif // !defined(AZ_PLATFORM_MAC)

}}}} // namespace Azure::Core::Amqp::Tests
