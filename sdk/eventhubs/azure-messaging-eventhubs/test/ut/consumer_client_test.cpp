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

    void SendOneEventToPartitionOne(std::string const& body)
    {
      std::unique_ptr<ProducerClient> producer = CreateProducerClient();
      EventDataBatchOptions batchOptions;
      batchOptions.PartitionId = "1";
      EventDataBatch batch{producer->CreateBatch(batchOptions)};
      ASSERT_TRUE(batch.TryAdd(Models::EventData{body}));
      ASSERT_NO_THROW(producer->Send(batch));
      producer->Close();
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

  // uAMQP only. The Rust transport never returns the last read below.
#if ENABLE_UAMQP
  TEST_P(ConsumerClientTest, StolenReceiverFailsWithoutARebuild_LIVEONLY_)
  {
    Azure::Messaging::EventHubs::ConsumerClientOptions firstClientOptions;
    firstClientOptions.ApplicationID
        = testing::UnitTest::GetInstance()->current_test_info()->name();
    firstClientOptions.Name = "first-receiver";
    firstClientOptions.RetryOptions.MaxRetries = 10;
    firstClientOptions.RetryOptions.RetryDelay = std::chrono::seconds(2);
    auto firstConsumer = CreateConsumerClient("", firstClientOptions);

    Azure::Messaging::EventHubs::ConsumerClientOptions secondClientOptions;
    secondClientOptions.ApplicationID
        = testing::UnitTest::GetInstance()->current_test_info()->name();
    secondClientOptions.Name = "second-receiver";
    auto secondConsumer = CreateConsumerClient("", secondClientOptions);

    // Each read needs its own deadline, because a deadline is an absolute time.
    auto readTimeout = []() {
      return Azure::Core::Context{Azure::DateTime::clock::now() + std::chrono::seconds(60)};
    };

    // Start both at the latest position; an earliest-position receiver prefetches the backlog.
    Azure::Messaging::EventHubs::PartitionClientOptions firstOptions;
    firstOptions.StartPosition.Latest = true;
    firstOptions.OwnerLevel = 1;
    Azure::Messaging::EventHubs::PartitionClient firstClient
        = firstConsumer->CreatePartitionClient("1", firstOptions);

    SendOneEventToPartitionOne("Before the steal");
    EXPECT_NO_THROW(firstClient.ReceiveEvents(1, readTimeout()));

    Azure::Messaging::EventHubs::PartitionClientOptions secondOptions;
    secondOptions.StartPosition.Latest = true;
    secondOptions.OwnerLevel = 2;
    Azure::Messaging::EventHubs::PartitionClient secondClient
        = secondConsumer->CreatePartitionClient("1", secondOptions);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    auto startTime = std::chrono::steady_clock::now();
    try
    {
      firstClient.ReceiveEvents(1, readTimeout());
      FAIL() << "The stolen receiver must throw.";
    }
    catch (Azure::Messaging::EventHubs::EventHubsException const& ex)
    {
      auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::steady_clock::now() - startTime);
      GTEST_LOG_(INFO) << "The stolen receiver threw after " << elapsed.count()
                       << " seconds with the condition '" << ex.ErrorCondition << "'.";
      EXPECT_FALSE(ex.IsTransient);
      EXPECT_LT(elapsed, std::chrono::seconds(30));
    }

    SendOneEventToPartitionOne("After the steal");
    EXPECT_NO_THROW(secondClient.ReceiveEvents(1, readTimeout()));
  }
#endif // ENABLE_UAMQP

  TEST_P(ConsumerClientTest, ReceiveSurvivesAnIdleDetachAndResumes_LIVEONLY_)
  {
    // The live pipeline caps the whole binary at 120 minutes, so this 35 minute wait runs
    // only when explicitly enabled.
    if (Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_ENABLE_IDLE_DETACH_TESTS")
            .empty())
    {
      GTEST_SKIP() << "Set EVENTHUBS_ENABLE_IDLE_DETACH_TESTS to run this test. The test sleeps "
                      "for 35 minutes, and the live pipeline gives all of the tests 120 minutes.";
    }

    constexpr std::chrono::minutes idleWait{35};

    Azure::Messaging::EventHubs::ConsumerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();
    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
    auto client = CreateConsumerClient("", options);

    // A resume that regressed to the latest position would block here for ever.
    auto readTimeout = []() {
      return Azure::Core::Context{Azure::DateTime::clock::now() + std::chrono::seconds(60)};
    };

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

    auto firstEvents = partitionClient.ReceiveEvents(1, readTimeout());
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

    auto secondEvents = partitionClient.ReceiveEvents(1, readTimeout());
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
