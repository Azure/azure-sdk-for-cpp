// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words

#include "eventhubs_test_base.hpp"

#include <azure/core/context.hpp>
#include <azure/core/uuid.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <numeric>

#include <gtest/gtest.h>

// cspell: ignore edboptions

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  class ProducerClientTest : public EventHubsTestBaseParameterized {
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

  TEST_P(ProducerClientTest, SimpleProducerClient)
  {
    std::string const connStringNoEntityPath = GetEnv("EVENTHUBS_HOST");
    std::string eventHubName{GetEnv("EVENTHUB_NAME")};

    Azure::Messaging::EventHubs::ProducerClient client{
        connStringNoEntityPath, eventHubName, GetTestCredential()};
    EXPECT_EQ(eventHubName, client.GetEventHubName());
  }

  TEST_P(ProducerClientTest, SendMessage_LIVEONLY_)
  {
    Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
    producerOptions.Name = "sender-link";
    producerOptions.ApplicationID = "some";

    auto client{CreateProducerClient("", producerOptions)};

    auto message2{std::make_shared<Azure::Core::Amqp::Models::AmqpMessage>()};
    Azure::Messaging::EventHubs::Models::EventData message1;
    message2->SetBody(Azure::Core::Amqp::Models::AmqpValue("Hello7"));

    message1.Body = {'H', 'e', 'l', 'l', 'o', '2'};

    Azure::Messaging::EventHubs::Models::EventData message3;
    message3.Body = {'H', 'e', 'l', 'l', 'o', '3'};

    Azure::Messaging::EventHubs::EventDataBatchOptions edboptions;
    edboptions.MaxBytes = (std::numeric_limits<uint16_t>::max)();
    edboptions.PartitionId = "1";
    Azure::Messaging::EventHubs::EventDataBatch eventBatch{client->CreateBatch(edboptions)};

    Azure::Messaging::EventHubs::EventDataBatchOptions edboptions2;
    edboptions2.MaxBytes = (std::numeric_limits<uint16_t>::max)();
    ;
    edboptions2.PartitionId = "2";
    Azure::Messaging::EventHubs::EventDataBatch eventBatch2{client->CreateBatch(edboptions2)};

    EXPECT_TRUE(eventBatch.TryAdd(message1));
    EXPECT_TRUE(eventBatch.TryAdd(message2));

    EXPECT_TRUE(eventBatch2.TryAdd(message3));
    EXPECT_TRUE(eventBatch2.TryAdd(message2));

    for (int i = 0; i < 5; i++)
    {
      EXPECT_NO_THROW(client->Send(eventBatch));
    }
  }

  TEST_P(ProducerClientTest, EventHubRawMessageSend_LIVEONLY_)
  {
    Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
    producerOptions.Name = "sender-link";
    producerOptions.ApplicationID = "some";

    auto client{CreateProducerClient("", producerOptions)};

    client->Send(Azure::Messaging::EventHubs::Models::EventData{"This is a test message"});

    // Send using the implicit EventData constructor.
    client->Send(std::string{"String test message"});

    // Send using a vector of implicit EventData constructor with a binary buffer.
    client->Send({{12, 13, 14, 15}, {16, 17, 18, 19}});
  }

  TEST_P(ProducerClientTest, GetEventHubProperties_LIVEONLY_)
  {
    Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
    producerOptions.Name = "sender-link";
    producerOptions.ApplicationID = "some";

    auto client{CreateProducerClient("", producerOptions)};

    auto result = client->GetEventHubProperties();
    EXPECT_EQ(result.Name, GetEventHubName());
    EXPECT_TRUE(result.PartitionIds.size() > 0);
    client->Close();
  }

  TEST_P(ProducerClientTest, GetPartitionProperties_LIVEONLY_)
  {
    Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
    producerOptions.Name = "sender-link";
    producerOptions.ApplicationID = "some";

    auto client{CreateProducerClient("", producerOptions)};

    ASSERT_NO_THROW([&]() {
      auto result = client->GetPartitionProperties("0");
      EXPECT_EQ(result.Name, GetEventHubName());
      EXPECT_EQ(result.PartitionId, "0");
    }());
  }

  TEST_P(ProducerClientTest, GetEventHubProperties_Multithreaded_LIVEONLY_)
  {
    Azure::Messaging::EventHubs::ProducerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
    auto client(CreateProducerClient("", options));

    std::string eventHubName{GetEventHubName()};

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

  constexpr size_t threadsPerPartition = 20;

  TEST_P(ProducerClientTest, GetPartitionProperties_Multithreaded_LIVEONLY_)
  {
    Azure::Messaging::EventHubs::ProducerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
    auto client{CreateProducerClient()};

    auto ehProperties = client->GetEventHubProperties();
    std::mutex iterationLock;
    std::vector<std::thread> threads;
    std::map<std::thread::id, size_t> iterationsPerThread;
    for (const auto& partition : ehProperties.PartitionIds)
    {
      threads.emplace_back(
          std::thread([&client, partition, ehProperties, &iterationsPerThread, &iterationLock]() {
            GTEST_LOG_(INFO) << "Thread started for partition: " << partition << ".\n";
            GTEST_LOG_(INFO) << "Start " << threadsPerPartition
                             << " threads to retrieve properties.";
            for (size_t i = 0; i < threadsPerPartition; i++)
            {
              std::vector<std::thread> partitionThreads;
              partitionThreads.emplace_back(
                  [&client, &partition, ehProperties, &iterationsPerThread, &iterationLock]() {
                    size_t iterations = 0;
                    auto threadId = std::this_thread::get_id();
                    std::chrono::system_clock::duration timeout = std::chrono::seconds(3);
                    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

                    while ((std::chrono::system_clock::now() - start) <= timeout)
                    {
                      Azure::Messaging::EventHubs::Models::EventHubPartitionProperties result;
                      ASSERT_NO_THROW(result = client->GetPartitionProperties(partition));
                      EXPECT_EQ(result.Name, ehProperties.Name);
                      EXPECT_EQ(result.PartitionId, partition);
                      // Attempt to avoid service throttling.
                      std::this_thread::sleep_for(std::chrono::milliseconds(100));
                      iterations++;
                    }
                    {
                      std::unique_lock<std::mutex> lock(iterationLock);
                      iterationsPerThread.emplace(threadId, iterations);
                    }
                  });
              for (auto& t : partitionThreads)
              {
                if (t.joinable())
                {
                  t.join();
                }
              }
            }
            GTEST_LOG_(INFO) << "Threads finished for partition: " << partition << ".\n";
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
    GTEST_LOG_(INFO) << "Threads finished.";
    GTEST_LOG_(INFO) << iterationsPerThread.size() << " threads finished.";
    for (const auto i : iterationsPerThread)
    {
      GTEST_LOG_(INFO) << "Thread iterations: " << i.second;
    }
  }

  // Send a batch that has a partition key and make sure that every event in the batch landed on one
  // partition. The Event Hubs service routes on the partition key annotation in the message
  // annotations of the batch envelope, so this test fails if the annotation is missing or if it is
  // in the delivery annotations.
  TEST_P(ProducerClientTest, SendBatchWithPartitionKey_LIVEONLY_)
  {
    constexpr uint32_t eventCount = 20;

    auto client{CreateProducerClient()};

    auto const partitionIds = client->GetEventHubProperties().PartitionIds;
    ASSERT_GT(partitionIds.size(), 1ul) << "This test needs more than one partition.";

    std::string const partitionKey{"ws5-" + Azure::Core::Uuid::CreateUuid().ToString()};

    std::map<std::string, int64_t> sequenceNumberBeforeSend;
    for (auto const& partitionId : partitionIds)
    {
      sequenceNumberBeforeSend[partitionId]
          = client->GetPartitionProperties(partitionId).LastEnqueuedSequenceNumber;
      // Attempt to avoid service throttling.
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    {
      Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
      batchOptions.MaxBytes = (std::numeric_limits<uint16_t>::max)();
      batchOptions.PartitionKey = partitionKey;
      Azure::Messaging::EventHubs::EventDataBatch eventBatch{client->CreateBatch(batchOptions)};
      for (uint32_t i = 0; i < eventCount; i++)
      {
        // The body starts with the unique marker of this run. The test finds its own events by
        // that marker, so the check on the partition key below stays independent.
        EXPECT_TRUE(eventBatch.TryAdd(Azure::Messaging::EventHubs::Models::EventData{
            partitionKey + " message " + std::to_string(i)}));
      }
      // Stop here if the send fails, because the partition counts below would then report a
      // misleading failure.
      ASSERT_NO_THROW(client->Send(eventBatch));
    }

    // Give the service time to report the new events.
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Read every partition from the sequence number that it had before the send, and keep only the
    // events of this batch. The unique marker in the body identifies them. A count of new events
    // cannot do this, because another producer can write to the same Event Hub while this test
    // runs and can add events to any partition.
    auto consumer{CreateConsumerClient()};
    std::vector<std::string> partitionsWithBatch;
    std::vector<std::shared_ptr<const Azure::Messaging::EventHubs::Models::ReceivedEventData>>
        batchEvents;

    for (auto const& partitionId : partitionIds)
    {
      Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
      partitionOptions.StartPosition.SequenceNumber = sequenceNumberBeforeSend[partitionId];
      auto receiver = consumer->CreatePartitionClient(partitionId, partitionOptions);

      // ReceiveEvents returns as soon as the receiver queue is empty, so one call can return fewer
      // events than the batch holds. Read until this partition holds the whole batch, or until the
      // partition stays quiet, or until the time runs out.
      std::vector<std::shared_ptr<const Azure::Messaging::EventHubs::Models::ReceivedEventData>>
          eventsOnPartition;
      auto const deadline = std::chrono::system_clock::now() + std::chrono::seconds(60);
      int quietReads = 0;
      while (eventsOnPartition.size() < eventCount && quietReads < 6
             && std::chrono::system_clock::now() < deadline)
      {
        auto batchOfEvents = receiver.ReceiveEvents(eventCount);
        if (batchOfEvents.empty())
        {
          quietReads++;
          std::this_thread::sleep_for(std::chrono::milliseconds(500));
          continue;
        }
        quietReads = 0;
        for (auto const& receivedEvent : batchOfEvents)
        {
          std::string const body(receivedEvent->Body.begin(), receivedEvent->Body.end());
          if (body.rfind(partitionKey, 0) == 0)
          {
            eventsOnPartition.push_back(receivedEvent);
          }
        }
      }

      GTEST_LOG_(INFO) << "Partition " << partitionId << " holds " << eventsOnPartition.size()
                       << " events of this batch.";
      if (!eventsOnPartition.empty())
      {
        partitionsWithBatch.push_back(partitionId);
        batchEvents.insert(batchEvents.end(), eventsOnPartition.begin(), eventsOnPartition.end());
      }
    }

    // Before the fix the service spread the batch over every partition, so more than one partition
    // holds events of the batch and this assertion fails.
    ASSERT_EQ(1ul, partitionsWithBatch.size())
        << "A batch with a partition key must land on exactly one partition.";
    ASSERT_EQ(static_cast<size_t>(eventCount), batchEvents.size())
        << "The partition must hold every event of the batch.";

    // The marker in the body found these events, so this check on the partition key does not
    // depend on the way the test found them.
    for (auto const& receivedEvent : batchEvents)
    {
      ASSERT_TRUE(receivedEvent->PartitionKey);
      EXPECT_EQ(partitionKey, receivedEvent->PartitionKey.Value());
    }
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

#if ENABLE_UAMQP
  INSTANTIATE_TEST_SUITE_P(
      EventHubs,
      ProducerClientTest,
      ::testing::Values(AuthType::Key),
      GetSuffix);
#else
  INSTANTIATE_TEST_SUITE_P(
      EventHubs,
      ProducerClientTest,
      ::testing::Values(AuthType::Key),
      GetSuffix);
#endif
}}}} // namespace Azure::Messaging::EventHubs::Test
