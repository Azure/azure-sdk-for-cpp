// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#if ENABLE_UAMQP
#include "../../src/impl/uamqp/amqp/private/session_impl.hpp"
#include "cleanup_test_listener.hpp"

#include <azure/core/amqp/internal/management.hpp>

#include <cstdlib>

#include <gtest/gtest.h>

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  namespace {
    int FailEnd(SESSION_HANDLE, char const*, char const*) { return 1; }

    void FailedEndWithSharedOwner(bool withError)
    {
      std::weak_ptr<_detail::SessionImpl> sessionLifetime;
      std::weak_ptr<_detail::ConnectionImpl> connectionLifetime;
      {
        CleanupTestListener server;
        _internal::ConnectionOptions options;
        options.Port = server.GetPort();
        _internal::Connection connection("localhost", nullptr, options);
        connectionLifetime = _detail::ConnectionFactory::GetImpl(connection);
        std::unique_ptr<_internal::ManagementClient> management;
        {
          auto impl = std::make_shared<_detail::SessionImpl>(
              _detail::ConnectionFactory::GetImpl(connection),
              _internal::SessionOptions{},
              nullptr,
              FailEnd);
          sessionLifetime = impl;
          auto session = _detail::SessionFactory::CreateFromInternal(impl);
          session.Begin({});
          management = std::make_unique<_internal::ManagementClient>(
              session.CreateManagementClient("hub", {}));
          if (withError)
          {
            EXPECT_THROW(session.End("amqp:internal-error", "shutdown", {}), std::runtime_error);
          }
          else
          {
            EXPECT_THROW(session.End({}), std::runtime_error);
          }
        }
        // A management client still owns the session. Native destruction must wait for it.
        EXPECT_FALSE(sessionLifetime.expired());
        management.reset();
        EXPECT_TRUE(sessionLifetime.expired());
      }
      Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
      EXPECT_TRUE(connectionLifetime.expired());
    }

    int FailOnceEnd(SESSION_HANDLE session, char const* condition, char const* description)
    {
      static bool failed = false;
      if (!failed)
      {
        failed = true;
        return 1;
      }
      return session_end(session, condition, description);
    }
  } // namespace

  class SessionCleanupDeathTest : public ::testing::Test {
  protected:
    void SetUp() override
    {
      // Re-exec the child instead of inheriting AMQP polling-thread mutexes across fork.
      ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    }
  };

  TEST_F(SessionCleanupDeathTest, FailedEndReleasesLastOwner)
  {
    EXPECT_EXIT(
        {
          FailedEndWithSharedOwner(false);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(SessionCleanupDeathTest, FailedEndWithErrorReleasesLastOwner)
  {
    EXPECT_EXIT(
        {
          FailedEndWithSharedOwner(true);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(SessionCleanupDeathTest, FailedEndCanBeRetried)
  {
    EXPECT_EXIT(
        {
          {
            CleanupTestListener server;
            _internal::ConnectionOptions options;
            options.Port = server.GetPort();
            _internal::Connection connection("localhost", nullptr, options);
            auto session = _detail::SessionFactory::CreateFromInternal(
                std::make_shared<_detail::SessionImpl>(
                    _detail::ConnectionFactory::GetImpl(connection),
                    _internal::SessionOptions{},
                    nullptr,
                    FailOnceEnd));
            session.Begin({});
            EXPECT_THROW(session.End({}), std::runtime_error);
            EXPECT_NO_THROW(session.End("amqp:internal-error", "retry", {}));
            EXPECT_THROW(session.End({}), std::runtime_error);
          }
          Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(SessionCleanupDeathTest, MissingEndStillAsserts)
  {
    EXPECT_DEATH(
        {
          CleanupTestListener server;
          _internal::ConnectionOptions options;
          options.Port = server.GetPort();
          _internal::Connection connection("localhost", nullptr, options);
          auto session = connection.CreateSession();
          session.Begin({});
        },
        "");
  }
}}}} // namespace Azure::Core::Amqp::Tests
#endif
