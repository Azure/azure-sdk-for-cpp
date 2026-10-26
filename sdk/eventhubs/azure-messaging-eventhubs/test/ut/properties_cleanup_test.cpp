// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#if ENABLE_UAMQP
#include "../../../../core/azure-core-amqp/src/impl/uamqp/amqp/private/session_impl.hpp"
#include "../../../../core/azure-core-amqp/test/ut/cleanup_test_listener.hpp"
#include "../../../../core/azure-core-amqp/test/ut/mock_amqp_server.hpp"
#include "private/eventhubs_utilities.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/platform.hpp>
#include <azure/messaging/eventhubs/producer_client.hpp>

#include <atomic>
#include <cstdlib>
#include <future>

#include <gtest/gtest.h>

namespace {
std::uint16_t AvailablePort()
{
  Azure::Core::Amqp::Tests::CleanupTestListener listener;
  return listener.GetPort();
}
} // namespace

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  namespace {
    namespace Amqp = Azure::Core::Amqp;
    using _detail::EventHubsPropertiesClient;

    Amqp::_internal::ManagementClient FailCreation(
        Amqp::_internal::Session const&,
        std::string const&,
        Amqp::_internal::ManagementClientOptions const&)
    {
      throw std::runtime_error("injected management creation failure");
    }

    int FailEnd(SESSION_HANDLE, char const*, char const*) { return 1; }

    class PropertiesEndpoint final : public Amqp::Tests::MessageTests::MockServiceEndpoint {
    public:
      PropertiesEndpoint(bool failDiscovery = false, std::promise<void>* requestReceived = nullptr)
          : MockServiceEndpoint("$management", {}), m_failDiscovery{failDiscovery},
            m_requestReceived{requestReceived}
      {
      }

    private:
      bool m_failDiscovery;
      std::promise<void>* m_requestReceived;
      void MessageReceived(
          std::string const&,
          std::shared_ptr<Amqp::Models::AmqpMessage> const& request) override
      {
        if (m_requestReceived)
        {
          // Acknowledge receipt without replying. Close must interrupt the client's response wait.
          m_requestReceived->set_value();
          return;
        }
        Amqp::Models::AmqpMessage response;
        response.ApplicationProperties["status-code"] = m_failDiscovery ? 500 : 200;
        response.Properties.CorrelationId = request->Properties.MessageId;
        Amqp::Models::AmqpMap body;
        body["name"] = "hub";
        body["created_at"]
            = Amqp::Models::AmqpTimestamp(std::chrono::milliseconds(0)).AsAmqpValue();
        body["partition_ids"] = Amqp::Models::AmqpArray{"0", "1"}.AsAmqpValue();
        body["partition"] = "0";
        body["begin_sequence_number"] = std::int64_t{0};
        body["last_enqueued_sequence_number"] = std::int64_t{1};
        body["last_enqueued_offset"] = "1";
        body["last_enqueued_time_utc"]
            = Amqp::Models::AmqpTimestamp(std::chrono::milliseconds(0)).AsAmqpValue();
        body["is_partition_empty"] = false;
        response.SetBody(body.AsAmqpValue());
        auto result = GetMessageSender().Send(response, GetListenerContext());
        EXPECT_EQ(std::get<0>(result), Amqp::_internal::MessageSendStatus::Ok);
      }
    };

    void ProducerDiscoveryFailure()
    {
      {
        Amqp::Tests::MessageTests::AmqpServerMock server(AvailablePort());
        server.AddServiceEndpoint(std::make_shared<PropertiesEndpoint>(true));
        server.StartListening();
        {
          ProducerClientOptions options;
          options.RetryOptions.MaxRetries = 0;
          ProducerClient producer(
              "Endpoint=sb://localhost:" + std::to_string(server.GetPort())
                  + "/;SharedAccessKeyName=test;SharedAccessKey=YWJj;UseDevelopmentEmulator=true",
              "hub",
              options);
          auto context = Azure::Core::Context().WithDeadline(
              std::chrono::system_clock::now() + std::chrono::seconds(10));
          EXPECT_THROW(producer.GetEventHubProperties(context), EventHubsException);
          producer.Close(context);
          producer.Close(context);
        }
        server.StopListening();
      }
      Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }

    std::atomic<int> endCalls{0};
    int CountEnd(SESSION_HANDLE session, char const* condition, char const* description)
    {
      ++endCalls;
      return session_end(session, condition, description);
    }

    template <typename T> void RequireReady(std::future<T>& operation)
    {
      if (operation.wait_for(std::chrono::seconds(10)) != std::future_status::ready)
      {
        ADD_FAILURE() << "Concurrent properties operation did not finish";
        // Avoid hanging in a future destructor; the parent death test reports the failure.
        std::exit(1);
      }
    }

    void ConcurrentPropertiesClose(
        bool partitionProperties,
        bool cancelCaller,
        bool pauseBeforeSend = false)
    {
      {
        std::promise<void> requestReceived;
        auto received = requestReceived.get_future();
        std::promise<void> resumeQuery;
        auto resume = resumeQuery.get_future();
        if (pauseBeforeSend)
        {
          Azure::Core::Diagnostics::Logger::SetLevel(
              Azure::Core::Diagnostics::Logger::Level::Verbose);
          Azure::Core::Diagnostics::Logger::SetListener(
              [&](Azure::Core::Diagnostics::Logger::Level, std::string const& line) {
                if (line.find("ManagementClient::ExecuteOperation:") != std::string::npos
                    && line.find("Create Queue for request.") != std::string::npos)
                {
                  // Pause inside ExecuteOperation, before it sends, to expose destruction of an
                  // active management client even when ordinary close callbacks finish quickly.
                  requestReceived.set_value();
                  RequireReady(resume);
                }
              });
        }
        Amqp::Tests::MessageTests::AmqpServerMock server(AvailablePort());
        server.AddServiceEndpoint(
            std::make_shared<PropertiesEndpoint>(
                false, pauseBeforeSend ? nullptr : &requestReceived));
        server.StartListening();
        {
          Amqp::_internal::ConnectionOptions options;
          options.Port = server.GetPort();
          Amqp::_internal::Connection connection("localhost", nullptr, options);
          auto session = Amqp::_detail::SessionFactory::CreateFromInternal(
              std::make_shared<Amqp::_detail::SessionImpl>(
                  Amqp::_detail::ConnectionFactory::GetImpl(connection),
                  Amqp::_internal::SessionOptions{},
                  nullptr,
                  CountEnd));
          endCalls = 0;
          EventHubsPropertiesClient properties(session, "hub");
          // No deadline: Close must not depend on the caller eventually timing out.
          Azure::Core::Context queryContext;
          auto query = std::async(std::launch::async, [&]() {
            if (partitionProperties)
            {
              properties.GetEventHubsPartitionProperties("hub", "0", queryContext);
            }
            else
            {
              properties.GetEventHubsProperties("hub", queryContext);
            }
          });
          RequireReady(received);
          received.get();
          if (cancelCaller)
          {
            queryContext.Cancel();
          }
          std::promise<void> closeStarted;
          auto started = closeStarted.get_future();
          auto close = std::async(std::launch::async, [&]() {
            closeStarted.set_value();
            properties.Close({});
          });
          RequireReady(started);
          if (pauseBeforeSend)
          {
            EXPECT_EQ(close.wait_for(std::chrono::milliseconds(200)), std::future_status::timeout);
            EXPECT_EQ(endCalls.load(), 0);
            resumeQuery.set_value();
          }
          RequireReady(query);
          EXPECT_ANY_THROW(query.get());
          RequireReady(close);
          EXPECT_NO_THROW(close.get());
          EXPECT_EQ(queryContext.IsCancelled(), cancelCaller);
          properties.Close({});
          EXPECT_EQ(endCalls.load(), 1);

          // A stale properties-client reference must not reopen the session after invalidation.
          EXPECT_THROW(
              properties.GetEventHubsProperties("hub", {}), Core::OperationCancelledException);
          EXPECT_THROW(
              properties.GetEventHubsPartitionProperties("hub", "0", {}),
              Core::OperationCancelledException);
          EXPECT_EQ(endCalls.load(), 1);
          Azure::Core::Diagnostics::Logger::SetListener(nullptr);
        }
        server.StopListening();
      }
      Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }

    void SuccessfulDiscovery(bool cancelledClose)
    {
      std::atomic<bool> closeFailureLogged{false};
      Azure::Core::Diagnostics::Logger::SetLevel(Azure::Core::Diagnostics::Logger::Level::Warning);
      Azure::Core::Diagnostics::Logger::SetListener(
          [&](Azure::Core::Diagnostics::Logger::Level, std::string const& line) {
            if (line.find("Exception while closing the management client") != std::string::npos)
            {
              closeFailureLogged = true;
            }
          });
      {
        Amqp::Tests::MessageTests::AmqpServerMock server(AvailablePort());
        server.AddServiceEndpoint(std::make_shared<PropertiesEndpoint>());
        server.StartListening();
        {
          Amqp::_internal::ConnectionOptions options;
          options.Port = server.GetPort();
          Amqp::_internal::Connection connection("localhost", nullptr, options);
          auto session = Amqp::_detail::SessionFactory::CreateFromInternal(
              std::make_shared<Amqp::_detail::SessionImpl>(
                  Amqp::_detail::ConnectionFactory::GetImpl(connection),
                  Amqp::_internal::SessionOptions{},
                  nullptr,
                  CountEnd));
          endCalls = 0;
          {
            EventHubsPropertiesClient properties(session, "hub");
            auto context = Azure::Core::Context().WithDeadline(
                std::chrono::system_clock::now() + std::chrono::seconds(10));
            auto result = properties.GetEventHubsProperties("hub", context);
            EXPECT_EQ(result.Name, "hub");
            EXPECT_EQ(result.PartitionIds, (std::vector<std::string>{"0", "1"}));
            auto partition = properties.GetEventHubsPartitionProperties("hub", "0", context);
            EXPECT_EQ(partition.PartitionId, "0");
            EXPECT_EQ(partition.LastEnqueuedOffset, "1");
            if (cancelledClose)
            {
              context.Cancel();
              // Stop client connection polling after draining its current poll. The peer cannot
              // complete DETACH before the cancelled close waits, so the close reliably throws.
              Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->RemovePollable(
                  Amqp::_detail::ConnectionFactory::GetImpl(connection));
            }
            properties.Close(context);
            properties.Close(context);
          }
          EXPECT_EQ(endCalls.load(), 1);
        }
        server.StopListening();
      }
      Azure::Core::Diagnostics::Logger::SetListener(nullptr);
      EXPECT_EQ(closeFailureLogged.load(), cancelledClose);
      Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }

    void PartialInitialization(bool explicitClose, bool failEnd)
    {
      std::weak_ptr<Amqp::_detail::ConnectionImpl> connectionLifetime;
      {
        Amqp::Tests::CleanupTestListener server;
        Amqp::_internal::ConnectionOptions options;
        options.Port = server.GetPort();
        Amqp::_internal::Connection connection("localhost", nullptr, options);
        connectionLifetime = Amqp::_detail::ConnectionFactory::GetImpl(connection);
        auto session = Amqp::_detail::SessionFactory::CreateFromInternal(
            std::make_shared<Amqp::_detail::SessionImpl>(
                Amqp::_detail::ConnectionFactory::GetImpl(connection),
                Amqp::_internal::SessionOptions{},
                nullptr,
                failEnd ? FailEnd : session_end));
        {
          EventHubsPropertiesClient properties(session, "hub", FailCreation);
          // Retrying discovery must not acquire a second polling reference.
          for (int attempt = 0; attempt != 2; ++attempt)
          {
            try
            {
              properties.GetEventHubsProperties("hub", {});
              ADD_FAILURE() << "Expected management creation to fail";
            }
            catch (std::runtime_error const& error)
            {
              EXPECT_STREQ(error.what(), "injected management creation failure");
            }
          }
          if (explicitClose)
          {
            properties.Close({});
            properties.Close({});
          }
        }
      }
      Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
      EXPECT_TRUE(connectionLifetime.expired());
    }

    void FailedOpen(bool failEnd)
    {
      {
        Amqp::Tests::CleanupTestListener server;
        Amqp::_internal::ConnectionOptions options;
        options.Port = server.GetPort();
        Amqp::_internal::Connection connection("localhost", nullptr, options);
        auto session = Amqp::_detail::SessionFactory::CreateFromInternal(
            std::make_shared<Amqp::_detail::SessionImpl>(
                Amqp::_detail::ConnectionFactory::GetImpl(connection),
                Amqp::_internal::SessionOptions{},
                nullptr,
                failEnd ? FailEnd : session_end));
        EventHubsPropertiesClient properties(session, "hub");
        Azure::Core::Context cancelled;
        cancelled.Cancel();
        EXPECT_ANY_THROW(properties.GetEventHubsProperties("hub", cancelled));
        properties.Close({});
        properties.Close({});
      }
      Amqp::Common::_detail::GlobalStateHolder::GlobalStateInstance()->AssertIdle();
    }
  } // namespace

  class PropertiesCleanupDeathTest : public ::testing::Test {
  protected:
    void SetUp() override
    {
      // Re-exec the child instead of inheriting AMQP polling-thread mutexes across fork.
      ::testing::FLAGS_gtest_death_test_style = "threadsafe";
    }
  };

  TEST_F(PropertiesCleanupDeathTest, SuccessfulDiscoveryAndRepeatedClose)
  {
    EXPECT_EXIT(
        {
          SuccessfulDiscovery(false);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  class ConcurrentPropertiesCleanupDeathTest
      : public PropertiesCleanupDeathTest,
        public ::testing::WithParamInterface<std::tuple<bool, bool>> {};

  TEST_P(ConcurrentPropertiesCleanupDeathTest, CloseDuringPendingQuery)
  {
    auto const partitionProperties = std::get<0>(GetParam());
    auto const cancelCaller = std::get<1>(GetParam());
    EXPECT_EXIT(
        {
          ConcurrentPropertiesClose(partitionProperties, cancelCaller);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_P(ConcurrentPropertiesCleanupDeathTest, CloseBeforeManagementSend)
  {
    auto const partitionProperties = std::get<0>(GetParam());
    auto const cancelCaller = std::get<1>(GetParam());
    EXPECT_EXIT(
        {
          ConcurrentPropertiesClose(partitionProperties, cancelCaller, true);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  INSTANTIATE_TEST_SUITE_P(
      PropertiesAndPartitions,
      ConcurrentPropertiesCleanupDeathTest,
      ::testing::Combine(::testing::Bool(), ::testing::Bool()));

  TEST_F(PropertiesCleanupDeathTest, ManagementCloseFailureStillEndsSession)
  {
    EXPECT_EXIT(
        {
          SuccessfulDiscovery(true);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(PropertiesCleanupDeathTest, CreationFailureThenRepeatedClose)
  {
    EXPECT_EXIT(
        {
          PartialInitialization(true, false);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(PropertiesCleanupDeathTest, CreationFailureThenDestruction)
  {
    EXPECT_EXIT(
        {
          PartialInitialization(false, false);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(PropertiesCleanupDeathTest, CreationFailureAndSessionEndFailure)
  {
    EXPECT_EXIT(
        {
          PartialInitialization(true, true);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(PropertiesCleanupDeathTest, OpenFailureThenRepeatedClose)
  {
    EXPECT_EXIT(
        {
          FailedOpen(false);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(PropertiesCleanupDeathTest, OpenFailureAndSessionEndFailure)
  {
    EXPECT_EXIT(
        {
          FailedOpen(true);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(PropertiesCleanupDeathTest, ProducerDiscoveryFailureThenRepeatedClose)
  {
    EXPECT_EXIT(
        {
          ProducerDiscoveryFailure();
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }
}}}} // namespace Azure::Messaging::EventHubs::Test
#endif
