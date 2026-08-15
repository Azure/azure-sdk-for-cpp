// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Link reattach tests for a namespace that keeps its shared access keys. The standard live
// resource sets disableLocalAuth, so it has no shared access key, and the parameterized
// fixture takes a token credential only. The first three tests mirror the reattach tests in
// consumer_client_test.cpp and producer_client_test.cpp. PropertiesCloseSurvivesAnIdleDetach
// has no mirror; it covers the management link close after an idle detach (issue #7335).
// Each test skips itself when the connection string is empty.

#include "eventhubs_test_base.hpp"

#include <azure/core/internal/environment.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <chrono>
#include <string>
#include <thread>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  class ReattachConnectionStringTest : public EventHubsTestBase {
  protected:
    std::string ConnectionString() const
    {
      return Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING");
    }

    std::string EventHubName() const
    {
      return Azure::Core::_internal::Environment::GetVariable("EVENTHUB_NAME");
    }

    std::string ConsumerGroup() const
    {
      std::string group
          = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONSUMER_GROUP");
      if (group.empty())
      {
        group = DefaultConsumerGroup;
      }
      return group;
    }

    std::string PartitionId() const
    {
      std::string partition
          = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_PARTITION_ID");
      if (partition.empty())
      {
        partition = "1";
      }
      return partition;
    }

    bool HasLiveEnvironment() const
    {
      return !ConnectionString().empty() && !EventHubName().empty();
    }

    bool WantsIdleDetachTests() const
    {
      return !Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_ENABLE_IDLE_DETACH_TESTS")
                  .empty();
    }

    // Give each read its own deadline. A deadline is an absolute time, so a context shared
    // across several reads would starve the later ones.
    static Azure::Core::Context ReadTimeout()
    {
      return Azure::Core::Context{Azure::DateTime::clock::now() + std::chrono::seconds(60)};
    }

    void SendOneEvent(std::string const& body)
    {
      ProducerClient producer{ConnectionString(), EventHubName()};
      EventDataBatchOptions batchOptions;
      batchOptions.PartitionId = PartitionId();
      EventDataBatch batch{producer.CreateBatch(batchOptions)};
      ASSERT_TRUE(batch.TryAdd(Models::EventData{body}));
      ASSERT_NO_THROW(producer.Send(batch));
      producer.Close();
    }
  };

  // uAMQP only. The Rust transport never returns the last read below.
#if ENABLE_UAMQP
  TEST_F(ReattachConnectionStringTest, StolenReceiverFailsWithoutARebuild_LIVEONLY_)
  {
    if (!HasLiveEnvironment())
    {
      GTEST_SKIP() << "Set EVENTHUB_CONNECTION_STRING and EVENTHUB_NAME to run this test.";
    }

    // Each receiver needs its own client. One client keeps one session for each partition
    // id and reuses one link name, so two partition clients on the same partition would
    // attach duplicate link names and the service would never answer the second one.
    ConsumerClientOptions firstClientOptions;
    firstClientOptions.ApplicationID
        = testing::UnitTest::GetInstance()->current_test_info()->name();
    firstClientOptions.Name = "first-receiver";
    // Keep the rebuild budget large. A rebuild loop then takes far longer than the bound
    // below, so the elapsed time check proves that no rebuild happened.
    firstClientOptions.RetryOptions.MaxRetries = 10;
    firstClientOptions.RetryOptions.RetryDelay = std::chrono::seconds(2);
    ConsumerClient firstConsumer{
        ConnectionString(), EventHubName(), ConsumerGroup(), firstClientOptions};

    ConsumerClientOptions secondClientOptions;
    secondClientOptions.ApplicationID
        = testing::UnitTest::GetInstance()->current_test_info()->name();
    secondClientOptions.Name = "second-receiver";
    ConsumerClient secondConsumer{
        ConnectionString(), EventHubName(), ConsumerGroup(), secondClientOptions};

    // Start both at the latest position. A receiver that starts at the earliest position
    // prefetches the backlog, and the last read below would then answer from that buffer
    // instead of going to the link.
    PartitionClientOptions firstOptions;
    firstOptions.StartPosition.Latest = true;
    firstOptions.OwnerLevel = 1;
    PartitionClient firstClient = firstConsumer.CreatePartitionClient(PartitionId(), firstOptions);

    SendOneEvent("Before the steal");
    EXPECT_NO_THROW(firstClient.ReceiveEvents(1, ReadTimeout()));

    PartitionClientOptions secondOptions;
    secondOptions.StartPosition.Latest = true;
    secondOptions.OwnerLevel = 2;
    PartitionClient secondClient
        = secondConsumer.CreatePartitionClient(PartitionId(), secondOptions);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    auto startTime = std::chrono::steady_clock::now();
    try
    {
      auto stolenEvents = firstClient.ReceiveEvents(1, ReadTimeout());
      std::string body;
      if (!stolenEvents.empty() && !stolenEvents[0]->Body.empty())
      {
        body.assign(stolenEvents[0]->Body.begin(), stolenEvents[0]->Body.end());
      }
      std::string offset;
      if (!stolenEvents.empty() && stolenEvents[0]->Offset.HasValue())
      {
        offset = stolenEvents[0]->Offset.Value();
      }
      FAIL() << "The stolen receiver must throw. It gave back " << stolenEvents.size()
             << " event(s). The first body is '" << body << "' and its offset is '" << offset
             << "'.";
    }
    catch (EventHubsException const& ex)
    {
      auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::steady_clock::now() - startTime);
      GTEST_LOG_(INFO) << "The stolen receiver threw after " << elapsed.count()
                       << " seconds with the condition '" << ex.ErrorCondition << "'.";
      EXPECT_FALSE(ex.IsTransient);
      EXPECT_LT(elapsed, std::chrono::seconds(30));
    }

    SendOneEvent("After the steal");
    EXPECT_NO_THROW(secondClient.ReceiveEvents(1, ReadTimeout()));
  }
#endif // ENABLE_UAMQP

  TEST_F(ReattachConnectionStringTest, SendSurvivesAnIdleDetach_LIVEONLY_)
  {
    if (!HasLiveEnvironment())
    {
      GTEST_SKIP() << "Set EVENTHUB_CONNECTION_STRING and EVENTHUB_NAME to run this test.";
    }
    if (!WantsIdleDetachTests())
    {
      GTEST_SKIP() << "Set EVENTHUBS_ENABLE_IDLE_DETACH_TESTS to run this test. The test sleeps "
                      "for 35 minutes.";
    }

    constexpr std::chrono::minutes idleWait{35};

    ProducerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();
    ProducerClient client{ConnectionString(), EventHubName(), options};

    EventDataBatchOptions batchOptions;
    batchOptions.PartitionId = PartitionId();

    EventDataBatch firstBatch{client.CreateBatch(batchOptions)};
    EXPECT_TRUE(firstBatch.TryAdd(Models::EventData{"Before the idle period"}));
    ASSERT_NO_THROW(client.Send(firstBatch));

    GTEST_LOG_(INFO) << "Wait " << idleWait.count()
                     << " minutes, so that the service detaches the sender link.";
    std::this_thread::sleep_for(idleWait);

    EventDataBatch secondBatch{client.CreateBatch(batchOptions)};
    EXPECT_TRUE(secondBatch.TryAdd(Models::EventData{"After the idle period"}));
    EXPECT_NO_THROW(client.Send(secondBatch));

    client.Close();
  }

  TEST_F(ReattachConnectionStringTest, ReceiveSurvivesAnIdleDetachAndResumes_LIVEONLY_)
  {
    if (!HasLiveEnvironment())
    {
      GTEST_SKIP() << "Set EVENTHUB_CONNECTION_STRING and EVENTHUB_NAME to run this test.";
    }
    if (!WantsIdleDetachTests())
    {
      GTEST_SKIP() << "Set EVENTHUBS_ENABLE_IDLE_DETACH_TESTS to run this test. The test sleeps "
                      "for 35 minutes.";
    }

    constexpr std::chrono::minutes idleWait{35};

    ConsumerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();
    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
    ConsumerClient client{ConnectionString(), EventHubName(), ConsumerGroup(), options};

    PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Latest = true;
    PartitionClient partitionClient = client.CreatePartitionClient(PartitionId(), partitionOptions);

    ProducerClient producer{ConnectionString(), EventHubName()};
    EventDataBatchOptions batchOptions;
    batchOptions.PartitionId = PartitionId();

    {
      EventDataBatch batch{producer.CreateBatch(batchOptions)};
      EXPECT_TRUE(batch.TryAdd(Models::EventData{"Before the idle period"}));
      ASSERT_NO_THROW(producer.Send(batch));
    }

    auto firstEvents = partitionClient.ReceiveEvents(1, ReadTimeout());
    ASSERT_EQ(firstEvents.size(), 1ul);
    ASSERT_TRUE(firstEvents[0]->Offset.HasValue())
        << "The resume needs the offset of the last event.";
    std::string lastOffset{firstEvents[0]->Offset.Value()};
    GTEST_LOG_(INFO) << "The last offset before the idle period is " << lastOffset << ".";

    GTEST_LOG_(INFO) << "Wait " << idleWait.count()
                     << " minutes, so that the service detaches the receiver link.";
    std::this_thread::sleep_for(idleWait);

    {
      EventDataBatch batch{producer.CreateBatch(batchOptions)};
      EXPECT_TRUE(batch.TryAdd(Models::EventData{"After the idle period"}));
      ASSERT_NO_THROW(producer.Send(batch));
    }

    auto secondEvents = partitionClient.ReceiveEvents(1, ReadTimeout());
    ASSERT_EQ(secondEvents.size(), 1ul);
    ASSERT_TRUE(secondEvents[0]->Offset.HasValue());
    GTEST_LOG_(INFO) << "The first offset after the idle period is "
                     << secondEvents[0]->Offset.Value() << ".";
    EXPECT_NE(secondEvents[0]->Offset.Value(), lastOffset)
        << "The rebuilt receiver gave the caller the same event twice.";
  }

  // This test proves that a producer can read properties, idle past the detach, close
  // without a throw, and read properties again. It does NOT prove the crash in issue #7335:
  // that needs ManagementClient::Close to throw, and this test passed against the code
  // before the fix. Do not claim it covers #7335 without a run that fails on the old code.
  TEST_F(ReattachConnectionStringTest, PropertiesCloseSurvivesAnIdleDetach_LIVEONLY_)
  {
    if (!HasLiveEnvironment())
    {
      GTEST_SKIP() << "Set EVENTHUB_CONNECTION_STRING and EVENTHUB_NAME to run this test.";
    }
    if (!WantsIdleDetachTests())
    {
      GTEST_SKIP() << "Set EVENTHUBS_ENABLE_IDLE_DETACH_TESTS to run this test. The test sleeps "
                      "for 35 minutes.";
    }

    constexpr std::chrono::minutes idleWait{35};

    ProducerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();
    ProducerClient client{ConnectionString(), EventHubName(), options};

    Models::EventHubProperties before;
    ASSERT_NO_THROW(before = client.GetEventHubProperties());
    ASSERT_FALSE(before.PartitionIds.empty());
    GTEST_LOG_(INFO) << "The event hub '" << before.Name << "' has " << before.PartitionIds.size()
                     << " partitions.";

    GTEST_LOG_(INFO) << "Wait " << idleWait.count()
                     << " minutes, so that the service detaches the management link.";
    std::this_thread::sleep_for(idleWait);

    EXPECT_NO_THROW(client.Close());

    // A call after Close builds a new stack today. This pins that behavior, and
    // not a documented contract.
    Models::EventHubProperties after;
    EXPECT_NO_THROW(after = client.GetEventHubProperties());
    EXPECT_EQ(after.PartitionIds.size(), before.PartitionIds.size());

    client.Close();
  }

}}}} // namespace Azure::Messaging::EventHubs::Test
