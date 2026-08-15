// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// These tests are the connection string equivalent of the three link reattach tests that need a
// live namespace. The standard live resource sets disableLocalAuth, so it has no shared access
// key, and the parameterized fixture takes a token credential only. A namespace that keeps its
// shared access keys can run the same scenarios through this file.
//
// Four of the tests below mirror one test each in the parameterized suites:
//   StolenReceiverFailsWithoutARebuild_LIVEONLY_  -> consumer_client_test.cpp
//   SendSurvivesAnIdleDetach_LIVEONLY_            -> producer_client_test.cpp
//   ReceiveSurvivesAnIdleDetachAndResumes_LIVEONLY_ -> consumer_client_test.cpp
//   ReceiveAfterCloseThrowsWithoutARebuild_LIVEONLY_ -> consumer_client_test.cpp
// PropertiesCloseSurvivesAnIdleDetach_LIVEONLY_ has no mirror. It covers the close of the
// management link after an idle detach, which used to end the process (#7335).
//
// The tests skip themselves when EVENTHUB_CONNECTION_STRING is empty.

#include "eventhubs_test_base.hpp"

#include <azure/core/internal/environment.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <chrono>
#include <string>
#include <thread>
#include <utility>

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

    // Return true when the environment can run a live test. GTEST_SKIP expands to a return, so
    // the test body calls it and this helper only reports the state.
    bool HasLiveEnvironment() const
    {
      return !ConnectionString().empty() && !EventHubName().empty();
    }

    // Return true when the caller asked for the tests that sleep for 35 minutes.
    bool WantsIdleDetachTests() const
    {
      return !Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_ENABLE_IDLE_DETACH_TESTS")
                  .empty();
    }

    // Give each read its own deadline. A deadline is an absolute time, so one context that the
    // test shares across several reads gives the later reads less time than the earlier ones, and
    // it cancels a read once the total elapsed time passes the deadline.
    static Azure::Core::Context ReadTimeout()
    {
      return Azure::Core::Context{Azure::DateTime::clock::now() + std::chrono::seconds(60)};
    }

    // Send one event to the partition, so a receiver on that partition has something to read.
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

  // A second receiver with a higher owner level takes the partition, and the service detaches the
  // first receiver with the condition amqp:link:stolen. That condition is permanent, so the first
  // receiver must report it at once. A client that attaches again fights the new owner, and it
  // spends its whole rebuild budget on a failure that stays.
  //
  // This test needs the uAMQP transport. The last read below goes to a link that the service
  // detached, and the Rust transport neither returns that read nor honours the deadline on the
  // context, so the test never ends on that transport. A live run of this test on the Rust
  // transport was still inside that read after five minutes.
#if ENABLE_UAMQP
  TEST_F(ReattachConnectionStringTest, StolenReceiverFailsWithoutARebuild_LIVEONLY_)
  {
    if (!HasLiveEnvironment())
    {
      GTEST_SKIP() << "Set EVENTHUB_CONNECTION_STRING and EVENTHUB_NAME to run this test.";
    }

    // The two receivers need their own client. ConsumerClient::CreatePartitionClient gives every
    // partition client the one link name in ConsumerClientOptions::Name, and it keeps one session
    // for each partition id. Two partition clients on the same partition from one consumer client
    // therefore attach two links with the same name on one session. The service answers the first
    // attach only, and the second receiver then waits for ever. A second client models the real
    // case too, because a steal comes from another consumer.
    ConsumerClientOptions firstClientOptions;
    firstClientOptions.ApplicationID
        = testing::UnitTest::GetInstance()->current_test_info()->name();
    firstClientOptions.Name = "first-receiver";
    // Keep the budget large. A rebuild loop would then take far longer than the bound below, so
    // the elapsed time proves that no rebuild happened.
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

    // Both receivers start at the latest position, so neither one holds a backlog. A receiver
    // that starts at the earliest position prefetches the whole history of the partition, and
    // ReceiveEvents then answers from that buffer without a read on the link. The last read below
    // must go to the link, so it must find an empty buffer.
    PartitionClientOptions firstOptions;
    firstOptions.StartPosition.Latest = true;
    firstOptions.OwnerLevel = 1;
    PartitionClient firstClient = firstConsumer.CreatePartitionClient(PartitionId(), firstOptions);

    // Make sure that the first receiver works before the steal. The receiver attaches inside
    // CreatePartitionClient, so an event that this test sends now lands after that position.
    SendOneEvent("Before the steal");
    EXPECT_NO_THROW(firstClient.ReceiveEvents(1, ReadTimeout()));

    PartitionClientOptions secondOptions;
    secondOptions.StartPosition.Latest = true;
    secondOptions.OwnerLevel = 2;
    PartitionClient secondClient
        = secondConsumer.CreatePartitionClient(PartitionId(), secondOptions);

    // The service takes the partition from the lower owner level and detaches that link. It does
    // this after it answers the attach of the new owner, so give it time.
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // The first receiver holds no event now, and this test sends no event until the read below
    // ends. So the read must go to the link, and the link is gone.
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
      // Ten rebuild attempts with a two second base delay take far more than 30 seconds.
      EXPECT_LT(elapsed, std::chrono::seconds(30));
    }

    // Make sure that the new owner still has the partition.
    SendOneEvent("After the steal");
    EXPECT_NO_THROW(secondClient.ReceiveEvents(1, ReadTimeout()));
  }
#endif // ENABLE_UAMQP

  // The Event Hubs service detaches a link that sends nothing and receives nothing for 30 minutes,
  // and it keeps the connection open. Before this change, ProducerClient cached one sender for the
  // life of the client, so the second send below failed and every later send failed with it.
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

    // The documented idle detach is 30 minutes. Wait past it with a margin.
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

    // The same client, with no restart. Send rebuilds the sender inside its retry loop.
    EventDataBatch secondBatch{client.CreateBatch(batchOptions)};
    EXPECT_TRUE(secondBatch.TryAdd(Models::EventData{"After the idle period"}));
    EXPECT_NO_THROW(client.Send(secondBatch));

    client.Close();
  }

  // Before this change, PartitionClient threw on an idle detach and never received again. It must
  // now attach a new receiver and start after the last event that it gave the caller, so the
  // caller sees the new events and no duplicate.
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

    // The documented idle detach is 30 minutes. Wait past it with a margin.
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

    // The same partition client, with no restart. ReceiveEvents attaches a new receiver and starts
    // after lastOffset.
    auto secondEvents = partitionClient.ReceiveEvents(1, ReadTimeout());
    ASSERT_EQ(secondEvents.size(), 1ul);
    ASSERT_TRUE(secondEvents[0]->Offset.HasValue());
    GTEST_LOG_(INFO) << "The first offset after the idle period is "
                     << secondEvents[0]->Offset.Value() << ".";
    EXPECT_NE(secondEvents[0]->Offset.Value(), lastOffset)
        << "The rebuilt receiver gave the caller the same event twice.";
  }

  // GetEventHubProperties opens a management link on the shared gateway connection, and the
  // client caches that link for its whole life. This test opens that link, leaves it idle
  // past the 30 minute service detach, and then closes and uses the client again.
  //
  // Read what this test does and does not prove before you rely on it. It proves that a
  // producer which reads the properties, then idles past the detach, closes without a throw
  // and reads the properties again. That covers the gateway and management path across an
  // idle period, which the send and receive idle tests do not touch.
  //
  // It does not prove the crash in #7335. That crash needs ManagementClient::Close to throw,
  // which leaves the client marked open for the destructor to close a second time. This test
  // was written for that crash and it was run against the code before the fix, where it
  // passed. So the uAMQP close of a link that the service already detached does not throw,
  // and the crash needs some other fault to reach it. The fix stands as a guard, because a
  // destructor is noexcept and ManagementClientImpl::Close does rethrow its first exception
  // (impl/uamqp/amqp/management.cpp), but no test covers that path today.
  //
  // Do not add a claim here that this test covers #7335 without a run that fails against the
  // code before commit 55e0de54c.
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

    // The documented idle detach is 30 minutes. Wait past it with a margin.
    constexpr std::chrono::minutes idleWait{35};

    ProducerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();
    ProducerClient client{ConnectionString(), EventHubName(), options};

    // Open the management link before the idle period, so that the link is alive and idle
    // while the test waits. The test sends nothing, so this link is the only link that the
    // client holds, and the detach below can only hit it.
    Models::EventHubProperties before;
    ASSERT_NO_THROW(before = client.GetEventHubProperties());
    ASSERT_FALSE(before.PartitionIds.empty());
    GTEST_LOG_(INFO) << "The event hub '" << before.Name << "' has " << before.PartitionIds.size()
                     << " partitions.";

    GTEST_LOG_(INFO) << "Wait " << idleWait.count()
                     << " minutes, so that the service detaches the management link.";
    std::this_thread::sleep_for(idleWait);

    // The close finds the dead link. It must log that failure and return. Before the fix,
    // this call never returned: the process ended inside it.
    EXPECT_NO_THROW(client.Close());

    // Close discarded the properties client and the gateway connection, so this read builds
    // a new stack. This pins what the client does today, and not a documented contract:
    // ProducerClient keeps no closed flag, so a call after Close builds the stack again.
    // A change that makes Close final must change this check with it.
    Models::EventHubProperties after;
    EXPECT_NO_THROW(after = client.GetEventHubProperties());
    EXPECT_EQ(after.PartitionIds.size(), before.PartitionIds.size());

    client.Close();
  }

  // PartitionClient::Close must end the client. A read after the close must throw at once, and
  // the client must not attach a new receiver. Before this change, Close set no state on the
  // client, so the next ReceiveEvents read the local close as a link fault. It then rebuilt the
  // receiver on the session, which the close left open, and it gave the caller events (#7334).
  //
  // Close is idempotent today, and the two close calls below pin that behaviour.
  TEST_F(ReattachConnectionStringTest, ReceiveAfterCloseThrowsWithoutARebuild_LIVEONLY_)
  {
    if (!HasLiveEnvironment())
    {
      GTEST_SKIP() << "Set EVENTHUB_CONNECTION_STRING and EVENTHUB_NAME to run this test.";
    }

    // Put at least one event on the partition. A receiver that starts at the earliest position
    // then has something to read, so a read that gives back an event proves the rebuild.
    SendOneEvent("Before the close");

    ConsumerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();
    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
    ConsumerClient consumer{ConnectionString(), EventHubName(), ConsumerGroup(), options};

    PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Earliest = true;
    partitionOptions.StartPosition.Inclusive = true;
    PartitionClient partitionClient
        = consumer.CreatePartitionClient(PartitionId(), partitionOptions);

    // Close takes a context and it has no default value for it.
    EXPECT_NO_THROW(partitionClient.Close({}));
    // A second close must do nothing and it must not throw.
    EXPECT_NO_THROW(partitionClient.Close({}));

    try
    {
      auto events = partitionClient.ReceiveEvents(1, ReadTimeout());
      FAIL() << "ReceiveEvents after Close returned " << events.size()
             << " events instead of throwing.";
    }
    catch (EventHubsException const& ex)
    {
      // The local close is not a transient fault, and it carries no AMQP error condition,
      // because no peer sent one.
      EXPECT_FALSE(ex.IsTransient);
      EXPECT_TRUE(ex.ErrorCondition.empty());
    }

    // A move must carry the closed state with it. A caller that moves a closed client must not
    // get a working client back.
    PartitionClient movedClient{std::move(partitionClient)};
    try
    {
      auto events = movedClient.ReceiveEvents(1, ReadTimeout());
      FAIL() << "ReceiveEvents on a moved closed client returned " << events.size()
             << " events instead of throwing.";
    }
    catch (EventHubsException const& ex)
    {
      EXPECT_FALSE(ex.IsTransient);
      EXPECT_TRUE(ex.ErrorCondition.empty());
    }
  }

}}}} // namespace Azure::Messaging::EventHubs::Test
