// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#if ENABLE_RUST_AMQP
#include "../../../../core/azure-core-amqp/test/ut/rust_cleanup_peer.hpp"
#include "private/eventhubs_utilities.hpp"

#include <azure/messaging/eventhubs/consumer_client.hpp>
#include <azure/messaging/eventhubs/producer_client.hpp>

#include <future>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  namespace {
    namespace Amqp = Azure::Core::Amqp;
    using Peer = Amqp::Tests::RustCleanupPeer;
    using Watchdog = Amqp::Tests::CleanupWatchdog;
    using _detail::EventHubsPropertiesClient;
    Amqp::_internal::Connection OpenConnection(Peer const& peer)
    {
      Amqp::_internal::ConnectionOptions options;
      options.Port = peer.Port();
      Amqp::_internal::Connection connection("localhost", nullptr, options);
      connection.Open({});
      return connection;
    }
    Core::Context Deadline()
    {
      return Core::Context().WithDeadline(
          std::chrono::system_clock::now() + std::chrono::seconds(3));
    }
    void ConcurrentClose(bool partition, bool cancelCaller)
    {
      Watchdog watchdog;
      Peer peer(Peer::HoldQuery);
      auto connection = OpenConnection(peer);
      {
        EventHubsPropertiesClient properties(connection, "hub");
        Core::Context queryContext; // No caller deadline; Close must cancel its child.
        auto query = std::async(std::launch::async, [&] {
          if (partition)
          {
            properties.GetEventHubsPartitionProperties("hub", "0", queryContext);
          }
          else
          {
            properties.GetEventHubsProperties("hub", queryContext);
          }
        });
        ASSERT_TRUE(peer.Wait(Peer::Request));
        if (cancelCaller)
        {
          queryContext.Cancel();
        }
        auto close = std::async(std::launch::async, [&] { properties.Close(Deadline()); });
        EXPECT_THROW(query.get(), Core::OperationCancelledException);
        EXPECT_NO_THROW(close.get());
        EXPECT_EQ(queryContext.IsCancelled(), cancelCaller);
        properties.Close({});
        EXPECT_THROW(
            properties.GetEventHubsProperties("hub", {}), Core::OperationCancelledException);
        EXPECT_THROW(
            properties.GetEventHubsPartitionProperties("hub", "0", {}),
            Core::OperationCancelledException);
      }
      connection.Close(Deadline());
    }
    void ConsumerCloseDuringPendingQuery(bool partition, bool expiredClose)
    {
      Watchdog watchdog;
      Peer peer(Peer::HoldQuery);
      ConsumerClientOptions options;
      options.RetryOptions.MaxRetries = 0;
      ConsumerClient consumer(
          "Endpoint=sb://localhost:" + std::to_string(peer.Port())
              + "/;SharedAccessKeyName=test;SharedAccessKey=YWJj;UseDevelopmentEmulator=true",
          "hub",
          DefaultConsumerGroup,
          options);
      Core::Context caller;
      auto query = std::async(std::launch::async, [&] {
        if (partition)
        {
          consumer.GetPartitionProperties("0", caller);
        }
        else
        {
          consumer.GetEventHubProperties(caller);
        }
      });
      ASSERT_TRUE(peer.Wait(Peer::Request));
      auto closeContext = Deadline();
      if (expiredClose)
      {
        closeContext = Core::Context().WithDeadline(std::chrono::system_clock::now());
      }
      auto close = std::async(std::launch::async, [&] { consumer.Close(closeContext); });
      EXPECT_THROW(query.get(), Core::OperationCancelledException);
      EXPECT_NO_THROW(close.get());
      EXPECT_FALSE(caller.IsCancelled());
      consumer.Close(Deadline());
    }

    void QueryDeadlineThenRediscover()
    {
      Watchdog watchdog;
      Peer peer(Peer::HoldQuery);
      auto connection = OpenConnection(peer);
      {
        EventHubsPropertiesClient properties(connection, "hub");
        auto context = Deadline();
        auto query = std::async(
            std::launch::async, [&] { properties.GetEventHubsProperties("hub", context); });
        ASSERT_TRUE(peer.Wait(Peer::Request));
        EXPECT_THROW(query.get(), Core::OperationCancelledException);
        peer.Resume();
        EXPECT_EQ(properties.GetEventHubsProperties("hub", Deadline()).Name, "hub");
        properties.Close(Deadline());
      }
      connection.Close(Deadline());
    }

    void CancelThenRediscover(bool partition)
    {
      Watchdog watchdog;
      Peer peer(Peer::HoldQuery);
      ProducerClientOptions options;
      options.RetryOptions.MaxRetries = 0;
      ProducerClient producer(
          "Endpoint=sb://localhost:" + std::to_string(peer.Port())
              + "/;SharedAccessKeyName=test;SharedAccessKey=YWJj;UseDevelopmentEmulator=true",
          "hub",
          options);
      Core::Context caller;
      auto first = std::async(std::launch::async, [&] {
        if (partition)
        {
          producer.GetPartitionProperties("0", caller);
        }
        else
        {
          producer.GetEventHubProperties(caller);
        }
      });
      ASSERT_TRUE(peer.Wait(Peer::Request));
      caller.Cancel();
      EXPECT_THROW(first.get(), Core::OperationCancelledException);
      peer.Resume();
      EXPECT_EQ(producer.GetEventHubProperties(Deadline()).Name, "hub");
      EXPECT_EQ(producer.GetPartitionProperties("0", Deadline()).PartitionId, "0");
      producer.Close(Deadline());
      producer.Close(Deadline());
    }
    void ServiceError()
    {
      Watchdog watchdog;
      Peer peer(Peer::ServiceError);
      auto connection = OpenConnection(peer);
      {
        EventHubsPropertiesClient properties(connection, "hub");
        try
        {
          properties.GetEventHubsProperties("hub", Deadline());
          ADD_FAILURE();
        }
        catch (std::runtime_error const& error)
        {
          EXPECT_NE(std::string(error.what()).find("500"), std::string::npos);
          EXPECT_NE(std::string(error.what()).find("cleanup peer response"), std::string::npos);
        }
        peer.Resume();
        EXPECT_EQ(properties.GetEventHubsProperties("hub", Deadline()).Name, "hub");
        properties.Close(Deadline());
      }
      connection.Close(Deadline());
    }
    void DirectManagementRejectsReuseAfterFailure()
    {
      Watchdog watchdog;
      Peer peer(Peer::ServiceError);
      auto connection = OpenConnection(peer);
      auto session = connection.CreateSession();
      session.Begin(Deadline());
      auto management = session.CreateManagementClient("hub", {});
      EXPECT_EQ(management.Open(Deadline()), Amqp::_internal::ManagementOpenStatus::Ok);
      Amqp::Models::AmqpMessage message;
      message.ApplicationProperties["name"] = "hub";
      EXPECT_THROW(
          management.ExecuteOperation("READ", "com.microsoft:eventhub", "", message, Deadline()),
          std::runtime_error);
      peer.Resume();
      try
      {
        auto result = management.ExecuteOperation(
            "READ", "com.microsoft:eventhub", "", message, Deadline());
        (void)result;
        ADD_FAILURE() << "Failed management client was reused";
      }
      catch (std::runtime_error const& error)
      {
        EXPECT_NE(std::string(error.what()).find("close and reopen"), std::string::npos);
      }
      management.Close(Deadline());
      EXPECT_EQ(management.Open(Deadline()), Amqp::_internal::ManagementOpenStatus::Ok);
      EXPECT_NO_THROW(
          management.ExecuteOperation("READ", "com.microsoft:eventhub", "", message, Deadline()));
      management.Close(Deadline());
      session.End(Deadline());
      connection.Close(Deadline());
    }
  } // namespace
  class RustPropertiesCleanupDeathTest : public ::testing::Test {
  protected:
    void SetUp() override { ::testing::FLAGS_gtest_death_test_style = "threadsafe"; }
  };
  TEST_F(RustPropertiesCleanupDeathTest, CancelledPropertiesThenRediscover)
  {
    EXPECT_EXIT(
        {
          CancelThenRediscover(false);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(RustPropertiesCleanupDeathTest, CancelledPartitionThenRediscover)
  {
    EXPECT_EXIT(
        {
          CancelThenRediscover(true);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(RustPropertiesCleanupDeathTest, ServiceErrorIsExceptionThenRediscover)
  {
    EXPECT_EXIT(
        {
          ServiceError();
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }

  TEST_F(RustPropertiesCleanupDeathTest, DirectManagementFailureRequiresReopen)
  {
    EXPECT_EXIT(
        {
          DirectManagementRejectsReuseAfterFailure();
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }
  class RustConcurrentPropertiesCleanupDeathTest
      : public RustPropertiesCleanupDeathTest,
        public ::testing::WithParamInterface<std::tuple<bool, bool>> {};
  TEST_P(RustConcurrentPropertiesCleanupDeathTest, CloseDuringPendingQuery)
  {
    auto partition = std::get<0>(GetParam());
    auto cancelCaller = std::get<1>(GetParam());
    EXPECT_EXIT(
        {
          ConcurrentClose(partition, cancelCaller);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }
  TEST_P(RustConcurrentPropertiesCleanupDeathTest, ConsumerCloseDuringPendingQuery)
  {
    auto partition = std::get<0>(GetParam());
    auto expiredClose = std::get<1>(GetParam());
    EXPECT_EXIT(
        {
          ConsumerCloseDuringPendingQuery(partition, expiredClose);
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }
  TEST_F(RustPropertiesCleanupDeathTest, QueryDeadlineThenRediscover)
  {
    EXPECT_EXIT(
        {
          QueryDeadlineThenRediscover();
          std::exit(::testing::Test::HasFailure() ? 1 : 0);
        },
        ::testing::ExitedWithCode(0),
        "");
  }
  INSTANTIATE_TEST_SUITE_P(
      PropertiesAndPartitions,
      RustConcurrentPropertiesCleanupDeathTest,
      ::testing::Combine(::testing::Bool(), ::testing::Bool()));
}}}} // namespace Azure::Messaging::EventHubs::Test
#endif
