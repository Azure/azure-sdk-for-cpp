// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words hehe

#include "eventhubs_admin_client.hpp"
#include "eventhubs_test_base.hpp"

#include <azure/core/context.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <chrono>
#include <string>
#include <thread>

#include <gtest/gtest.h>

namespace LocalTest {

int i = 0;
void ProcessMessageSuccess(Azure::Core::Amqp::Models::AmqpMessage const& message)
{
  (void)message;
  GTEST_LOG_(INFO) << "Message Id: " << i++ << std::endl;
}
} // namespace LocalTest
namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  class ConsumerClientTest : public EventHubsTestBaseParameterized {
    void SetUp() override
    {
      EventHubsTestBaseParameterized::SetUp();
      if (m_testContext.IsLiveMode())
      {
        std::unique_ptr<ProducerClient> producer = CreateProducerClient();
        EventDataBatchOptions eventBatchOptions;
        eventBatchOptions.PartitionId = "1";
        EventDataBatch batch{producer->CreateBatch(eventBatchOptions)};
        EXPECT_TRUE(batch.TryAdd(Models::EventData{"Test"}));
        EXPECT_NO_THROW(producer->Send(batch));
      }
    }

  protected:
    std::string GetEventHubName()
    {
      if (GetParam() == AuthType::Emulator)
      {
        return "eh1";
      }
      return GetEnv("EVENTHUB_NAME");
    }
  };

  TEST_P(ConsumerClientTest, ConnectToPartition_LIVEONLY_)
  {
    Azure::Messaging::EventHubs::ConsumerClientOptions options;
    options.ApplicationID
        = std::string(testing::UnitTest::GetInstance()->current_test_info()->name())
        + " Application";

    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
    auto client = CreateConsumerClient("", options);
    Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Inclusive = true;
    // We want to consume all messages from the earliest.
    partitionOptions.StartPosition.Earliest = true;

    Azure::Messaging::EventHubs::PartitionClient partitionClient
        = client->CreatePartitionClient("1", partitionOptions);
    auto events = partitionClient.ReceiveEvents(1);
    EXPECT_EQ(events.size(), 1ul);
    GTEST_LOG_(INFO) << "Received message " << events[0]->GetRawAmqpMessage();
    EXPECT_TRUE(events[0]->EnqueuedTime.HasValue());
    EXPECT_TRUE(events[0]->SequenceNumber.HasValue());
    EXPECT_TRUE(events[0]->Offset.HasValue());
  }

  TEST_P(ConsumerClientTest, GetEventHubProperties_LIVEONLY_)
  {
    std::string eventHubName{GetEventHubName()};
    Azure::Messaging::EventHubs::ConsumerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
    auto client = CreateConsumerClient("", options);
    Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Inclusive = true;

    Azure::Messaging::EventHubs::PartitionClient partitionClient
        = client->CreatePartitionClient("0", partitionOptions);

    Azure::Messaging::EventHubs::Models::EventHubProperties result;
    ASSERT_NO_THROW(result = client->GetEventHubProperties());
    EXPECT_EQ(result.Name, eventHubName);
    EXPECT_TRUE(result.PartitionIds.size() > 0);
  }

  TEST_P(ConsumerClientTest, GetPartitionProperties_LIVEONLY_)
  {
    std::string eventHubName{GetEventHubName()};

    Azure::Messaging::EventHubs::ConsumerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();

    auto client = CreateConsumerClient("", options);
    Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Inclusive = true;

    Azure::Messaging::EventHubs::PartitionClient partitionClient
        = client->CreatePartitionClient("0", partitionOptions);

    auto result = client->GetPartitionProperties("0");
    EXPECT_EQ(result.Name, eventHubName);
    EXPECT_EQ(result.PartitionId, "0");
  }

  TEST_P(ConsumerClientTest, GetPartitionPropertiesAuthError_LIVEONLY_)
  {
    auto credentials{
        std::make_shared<Azure::Identity::ClientSecretCredential>("abc", "def", "ghi")};
    std::string eventHubName{GetEnv("EVENTHUB_NAME")};
    std::string hostName{GetEnv("EVENTHUBS_HOST")};
    std::string consumerGroup{GetEnv("EVENTHUB_CONSUMER_GROUP")};

    Azure::Messaging::EventHubs::ConsumerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();

    Azure::Messaging::EventHubs::ConsumerClient client(
        hostName, eventHubName, credentials, consumerGroup);
    Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Inclusive = true;

    EXPECT_THROW(
        client.CreatePartitionClient("0", partitionOptions),
        Azure::Core::Credentials::AuthenticationException);
  }

  TEST_P(ConsumerClientTest, GetEventHubProperties_Multithreaded_LIVEONLY_)
  {
    std::string eventHubName{GetEventHubName()};

    Azure::Messaging::EventHubs::ConsumerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
    auto client = CreateConsumerClient();
    Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Inclusive = true;

    std::vector<std::thread> threads;
    std::vector<size_t> iterationsPerThread;
    for (int i = 0; i < 20; i++)
    {
      threads.emplace_back([&client, eventHubName, &iterationsPerThread]() {
        size_t iterations = 0;
        std::chrono::system_clock::duration timeout = std::chrono::seconds(3);
        std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
        while ((std::chrono::system_clock::now() - start) <= timeout)
        {
          Azure::Messaging::EventHubs::Models::EventHubProperties result;
          ASSERT_NO_THROW(result = client->GetEventHubProperties());
          EXPECT_EQ(result.Name, eventHubName);
          EXPECT_TRUE(result.PartitionIds.size() > 0);
          std::this_thread::yield();
          iterations++;
        }
        iterationsPerThread.push_back(iterations);
      });
    }
    GTEST_LOG_(INFO) << "Waiting for threads to finish.";
    for (auto& t : threads)
    {
      if (t.joinable())
      {
        t.join();
      }
    }
    GTEST_LOG_(INFO) << "Threads finished.";
    for (const auto i : iterationsPerThread)
    {
      GTEST_LOG_(INFO) << "Thread iterations: " << i;
    }
  }

#if 0
  TEST_P(ConsumerClientTest, GetPartitionProperties_Multithreaded)
  {
    std::string eventHubName{GetEventHubName()};

    Azure::Messaging::EventHubs::ConsumerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
    auto client = CreateConsumerClient();

    auto ehProperties = client->GetEventHubProperties();
    std::vector<std::thread> threads;
    std::vector<size_t> iterationsPerThread;
    for (const auto& partition : ehProperties.PartitionIds)
    {
      threads.emplace_back(std::thread([&client, partition, eventHubName, &iterationsPerThread]() {
        GTEST_LOG_(INFO) << "Thread started for partition: " << partition << ".\n";
        for (int i = 0; i < 20; i++)
        {
          std::vector<std::thread> partitionThreads;
          partitionThreads.emplace_back(
              [&client, &partition, eventHubName, &iterationsPerThread]() {
                size_t iterations = 0;
                std::chrono::system_clock::duration timeout = std::chrono::seconds(3);
                std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
                while ((std::chrono::system_clock::now() - start) <= timeout)
                {
                  Azure::Messaging::EventHubs::Models::EventHubPartitionProperties result;
                  ASSERT_NO_THROW(result = client->GetPartitionProperties(partition));
                  EXPECT_EQ(result.Name, eventHubName);
                  EXPECT_EQ(result.PartitionId, partition);
                  // Slow things down a bit to avoid EH throttling.
                  std::this_thread::sleep_for(std::chrono::milliseconds(100));
                  iterations++;
                }
                iterationsPerThread.push_back(iterations);
              });
          for (auto& t : partitionThreads)
          {
            if (t.joinable())
            {
              t.join();
            }
          }
        }
        GTEST_LOG_(INFO) << "Thread finished for partition: " << partition << ".\n";
      }));
    }
    GTEST_LOG_(INFO) << "Waiting for threads to finish.";
    for (auto& t : threads)
    {
      if (t.joinable())
      {
        t.join();
      }
    }
    GTEST_LOG_(INFO) << iterationsPerThread.size() << " threads finished.";
  }
#endif

  std::string GetRandomName(const char* baseName = "checkpoint")
  {
    std::string name = baseName;
    name.append(Azure::Core::Uuid::CreateUuid().ToString());
    return name;
  }

  TEST_P(ConsumerClientTest, RetrieveMultipleEvents)
  {
    // This test depends on being able to create a new eventhub instance, so skip it on the
    // emulator.
    if (GetParam() == AuthType::Emulator)
    {
      GTEST_SKIP();
    }

    // Disabled test for now.
    EventHubsManagement administrationClient{GetTestCredential()};
    auto eventhubNamespace{administrationClient.GetNamespace(GetEnv("EVENTHUBS_NAMESPACE"))};

    std::string eventHubName{GetRandomName("eventhub")};
    auto eventHub{eventhubNamespace.CreateEventHub(eventHubName)};
    eventHub.CreateConsumerGroup(GetEnv("EVENTHUB_CONSUMER_GROUP"));

    // Populate the eventhub instance with 50 messages.
    constexpr size_t numberOfEvents = 50;
    GTEST_LOG_(INFO) << "Populate eventhubs instance.";
    {
      Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
      producerOptions.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();
      producerOptions.Name = testing::UnitTest::GetInstance()->current_test_info()->name();
      auto producer{CreateProducerClient(eventHubName)};
      EventDataBatchOptions eventBatchOptions;
      eventBatchOptions.PartitionId = "0";
      EventDataBatch batch{producer->CreateBatch(eventBatchOptions)};
      for (size_t i = 0; i < numberOfEvents; ++i)
      {
        EXPECT_TRUE(batch.TryAdd(Models::EventData{"Test"}));
      }
      EXPECT_NO_THROW(producer->Send(batch));
    }

    // Now receive the messages - it should take almost no time because they should have been
    // queued up asynchronously.
    GTEST_LOG_(INFO) << "Receive events from instance.";
    {
      Azure::Messaging::EventHubs::ConsumerClientOptions options;
      options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();
      options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();

      auto client{CreateConsumerClient(eventHubName)};
      Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
      partitionOptions.StartPosition.Earliest = true;
      partitionOptions.StartPosition.Inclusive = true;

      Azure::Messaging::EventHubs::PartitionClient partitionClient
          = client->CreatePartitionClient("0", partitionOptions);

      // Sleep for a bit for the messages to be received.
      GTEST_LOG_(INFO) << "Sleep until messages received.";
      std::this_thread::sleep_for(std::chrono::seconds(2));

      size_t totalReceived{0};
      {
        std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
        auto messages = partitionClient.ReceiveEvents(5);
        std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        EXPECT_GE(messages.size(), 1ul);
        EXPECT_LE(messages.size(), 5ul);
        EXPECT_TRUE(elapsed_seconds.count() < 1);
        totalReceived += messages.size();
      }

      // We should have 45 messages left, which we should get immediately.
      do
      {
        std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
        auto messages = partitionClient.ReceiveEvents(50);
        std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        EXPECT_LE(messages.size(), 45ul);
        totalReceived += messages.size();
        EXPECT_TRUE(elapsed_seconds.count() < 1);
      } while (totalReceived < numberOfEvents);

      EXPECT_EQ(totalReceived, numberOfEvents);

      // The Rust AMQP stack doesn't support cancellation yet.
#if ENABLE_UAMQP
      // We have consumed all the events. Attempting to consume one more should block.
      Azure::Core::Context timeout{Azure::DateTime::clock::now() + std::chrono::seconds(3)};
      EXPECT_THROW(
          partitionClient.ReceiveEvents(50, timeout), Azure::Core::OperationCancelledException);
#endif
    }
    eventhubNamespace.DeleteEventHub(eventHubName);
  }

  // A second receiver with a higher owner level takes the partition, and the service detaches
  // the first receiver with the condition amqp:link:stolen. That condition is permanent, so
  // the first receiver must report it at once. A client that attaches again fights the new
  // owner, and it spends its whole rebuild budget on a failure that stays.
  TEST_P(ConsumerClientTest, StolenReceiverFailsWithoutARebuild_LIVEONLY_)
  {
    Azure::Messaging::EventHubs::ConsumerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();
    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
    // Keep the budget large. A rebuild loop would then take far longer than the bound below,
    // so the elapsed time proves that no rebuild happened.
    options.RetryOptions.MaxRetries = 10;
    options.RetryOptions.RetryDelay = std::chrono::seconds(2);

    auto client = CreateConsumerClient("", options);

    Azure::Messaging::EventHubs::PartitionClientOptions firstOptions;
    firstOptions.StartPosition.Earliest = true;
    firstOptions.StartPosition.Inclusive = true;
    firstOptions.OwnerLevel = 1;
    Azure::Messaging::EventHubs::PartitionClient firstClient
        = client->CreatePartitionClient("1", firstOptions);

    // Make sure that the first receiver works before the steal.
    EXPECT_NO_THROW(firstClient.ReceiveEvents(1));

    Azure::Messaging::EventHubs::PartitionClientOptions secondOptions;
    secondOptions.StartPosition.Earliest = true;
    secondOptions.StartPosition.Inclusive = true;
    secondOptions.OwnerLevel = 2;
    Azure::Messaging::EventHubs::PartitionClient secondClient
        = client->CreatePartitionClient("1", secondOptions);
    EXPECT_NO_THROW(secondClient.ReceiveEvents(1));

    auto startTime = std::chrono::steady_clock::now();
    try
    {
      firstClient.ReceiveEvents(1);
      FAIL() << "The stolen receiver must throw.";
    }
    catch (Azure::Messaging::EventHubs::EventHubsException const& ex)
    {
      auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::steady_clock::now() - startTime);
      GTEST_LOG_(INFO) << "The stolen receiver threw after " << elapsed.count()
                       << " seconds with the condition '" << ex.ErrorCondition << "'.";
      EXPECT_FALSE(ex.IsTransient);
      // Ten rebuild attempts with a two second base delay take far more than 30 seconds.
      EXPECT_LT(elapsed, std::chrono::seconds(30));
    }
  }

  // The Event Hubs service detaches a link that sends nothing and receives nothing for 30
  // minutes, and it keeps the connection open. Source: the Event Hubs AMQP troubleshooting
  // document. Before this change, PartitionClient threw on that detach and never received
  // again. It must now attach a new receiver and start after the last event that it gave the
  // caller, so the caller sees the new events and no duplicate.
  //
  // This test waits longer than the documented interval, so one run takes over half an hour.
  TEST_P(ConsumerClientTest, ReceiveSurvivesAnIdleDetachAndResumes_LIVEONLY_)
  {
    // The live pipeline gives the whole test binary 120 minutes. See LiveTestTimeoutInMinutes in
    // sdk/eventhubs/ci.yml. The 35 minute wait below takes too much of that budget, so this test
    // is not part of the standard live pass. The test is correct, and you must ask for it.
    if (Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_ENABLE_IDLE_DETACH_TESTS")
            .empty())
    {
      GTEST_SKIP() << "Set EVENTHUBS_ENABLE_IDLE_DETACH_TESTS to run this test. The test sleeps "
                      "for 35 minutes, and the live pipeline gives all of the tests 120 minutes.";
    }

    // The documented idle detach is 30 minutes. Wait past it with a margin.
    constexpr std::chrono::minutes idleWait{35};

    Azure::Messaging::EventHubs::ConsumerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();
    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
    auto client = CreateConsumerClient("", options);

    Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Latest = true;
    Azure::Messaging::EventHubs::PartitionClient partitionClient
        = client->CreatePartitionClient("1", partitionOptions);

    auto producer = CreateProducerClient();
    EventDataBatchOptions batchOptions;
    batchOptions.PartitionId = "1";

    {
      EventDataBatch batch{producer->CreateBatch(batchOptions)};
      EXPECT_TRUE(batch.TryAdd(Models::EventData{"Before the idle period"}));
      ASSERT_NO_THROW(producer->Send(batch));
    }

    auto firstEvents = partitionClient.ReceiveEvents(1);
    ASSERT_EQ(firstEvents.size(), 1ul);
    ASSERT_TRUE(firstEvents[0]->Offset.HasValue())
        << "The resume needs the offset of the last event.";
    std::string lastOffset{firstEvents[0]->Offset.Value()};
    GTEST_LOG_(INFO) << "The last offset before the idle period is " << lastOffset << ".";

    GTEST_LOG_(INFO) << "Wait " << idleWait.count()
                     << " minutes, so that the service detaches the receiver link.";
    std::this_thread::sleep_for(idleWait);

    {
      EventDataBatch batch{producer->CreateBatch(batchOptions)};
      EXPECT_TRUE(batch.TryAdd(Models::EventData{"After the idle period"}));
      ASSERT_NO_THROW(producer->Send(batch));
    }

    // The same partition client, with no restart. ReceiveEvents attaches a new receiver and
    // starts after lastOffset.
    auto secondEvents = partitionClient.ReceiveEvents(1);
    ASSERT_EQ(secondEvents.size(), 1ul);
    ASSERT_TRUE(secondEvents[0]->Offset.HasValue());
    GTEST_LOG_(INFO) << "The first offset after the idle period is "
                     << secondEvents[0]->Offset.Value() << ".";
    EXPECT_NE(secondEvents[0]->Offset.Value(), lastOffset)
        << "The rebuilt receiver gave the caller the same event twice.";
  }

  namespace {
    static std::string GetSuffix(const testing::TestParamInfo<AuthType>& info)
    {
      std::string stringValue = "";
      switch (info.param)
      {
        case AuthType::Key:
          stringValue = "Key_LIVEONLY_";
          break;
        case AuthType::Emulator:
          stringValue = "Emulator";
          break;
      }
      return stringValue;
    }
  } // namespace
  INSTANTIATE_TEST_SUITE_P(
      EventHubs,
      ConsumerClientTest,
      ::testing::Values(AuthType::Key /*, AuthType::Emulator*/),
      GetSuffix);
}}}} // namespace Azure::Messaging::EventHubs::Test
