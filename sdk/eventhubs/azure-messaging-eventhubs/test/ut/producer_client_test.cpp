// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words gearamaeh1

#include "eventhubs_test_base.hpp"

#include <azure/core/context.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <numeric>

#include <gtest/gtest.h>

// cspell: ignore edboptions

class ProducerClientTest : public EventHubsTestBase {
};

TEST_F(ProducerClientTest, ConnectionStringNoEntityPath)
{
  std::string const connStringNoEntityPath = GetEnv("EVENTHUB_CONNECTION_STRING");
  std::string eventHubName{GetEnv("EVENTHUB_NAME")};

  Azure::Messaging::EventHubs::ProducerClient client{connStringNoEntityPath, eventHubName};
  EXPECT_EQ(eventHubName, client.GetEventHubName());
}

TEST_F(ProducerClientTest, ConnectionStringEntityPath)
{
  std::string eventHubName{GetEnv("EVENTHUB_NAME")};
  std::string const connString = GetEnv("EVENTHUB_CONNECTION_STRING");

  Azure::Messaging::EventHubs::ProducerClient client{connString, eventHubName};
  EXPECT_EQ(eventHubName, client.GetEventHubName());
}

TEST_F(ProducerClientTest, TokenCredential_LIVEONLY_)
{
  auto credential{std::make_shared<Azure::Identity::ClientSecretCredential>(
      GetEnv("EVENTHUBS_TENANT_ID"),
      GetEnv("EVENTHUBS_CLIENT_ID"),
      GetEnv("EVENTHUBS_CLIENT_SECRET"))};
  std::string eventHubName{GetEnv("EVENTHUB_NAME")};
  Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
  producerOptions.ApplicationID = "appId";
  Azure::Messaging::EventHubs::ProducerClient client{
      "gearamaeh1.servicebus.windows.net", eventHubName, credential};
  EXPECT_EQ(eventHubName, client.GetEventHubName());
}

TEST_F(ProducerClientTest, SendMessage_LIVEONLY_)
{
  std::string eventHubName{GetEnv("EVENTHUB_NAME")};
  std::string const connString = GetEnv("EVENTHUB_CONNECTION_STRING");

  Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
  producerOptions.Name = "sender-link";
  producerOptions.ApplicationID = "some";

  Azure::Messaging::EventHubs::ProducerClient client{connString, eventHubName, producerOptions};

  auto message2{std::make_shared<Azure::Core::Amqp::Models::AmqpMessage>()};
  Azure::Messaging::EventHubs::Models::EventData message1;
  message2->SetBody(Azure::Core::Amqp::Models::AmqpValue("Hello7"));

  message1.Body = {'H', 'e', 'l', 'l', 'o', '2'};

  Azure::Messaging::EventHubs::Models::EventData message3;
  message3.Body = {'H', 'e', 'l', 'l', 'o', '3'};

  Azure::Messaging::EventHubs::EventDataBatchOptions edboptions;
  edboptions.MaxBytes = std::numeric_limits<uint16_t>::max();
  edboptions.PartitionId = "1";
  Azure::Messaging::EventHubs::EventDataBatch eventBatch{client.CreateBatch(edboptions)};

  Azure::Messaging::EventHubs::EventDataBatchOptions edboptions2;
  edboptions2.MaxBytes = std::numeric_limits<uint16_t>::max();
  ;
  edboptions2.PartitionId = "2";
  Azure::Messaging::EventHubs::EventDataBatch eventBatch2{client.CreateBatch(edboptions2)};

  eventBatch.TryAddMessage(message1);
  eventBatch.TryAddMessage(message2);

  eventBatch2.TryAddMessage(message3);
  eventBatch2.TryAddMessage(message2);

  for (int i = 0; i < 5; i++)
  {
    EXPECT_NO_THROW(client.Send(eventBatch));
  }
}

TEST_F(ProducerClientTest, EventHubRawMessageSend_LIVEONLY_)
{
  std::string eventHubName{GetEnv("EVENTHUB_NAME")};
  std::string const connString = GetEnv("EVENTHUB_CONNECTION_STRING");

  Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
  producerOptions.Name = "sender-link";
  producerOptions.ApplicationID = "some";

  Azure::Messaging::EventHubs::ProducerClient client{connString, eventHubName, producerOptions};

  client.Send(Azure::Messaging::EventHubs::Models::EventData{"This is a test message"});

  // Send using the implicit EventData constructor.
  client.Send(std::string{"String test message"});

  // Send using a vector of implicit EventData constructor with a binary buffer.
  client.Send({{12, 13, 14, 15}, {16, 17, 18, 19}});
}

TEST_F(ProducerClientTest, GetEventHubProperties_LIVEONLY_)
{
  std::string eventHubName{GetEnv("EVENTHUB_NAME")};
  std::string const connString = GetEnv("EVENTHUB_CONNECTION_STRING");

  Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
  producerOptions.Name = "sender-link";
  producerOptions.ApplicationID = "some";

  Azure::Messaging::EventHubs::ProducerClient client{connString, eventHubName, producerOptions};

  auto result = client.GetEventHubProperties();
  EXPECT_EQ(result.Name, eventHubName);
  EXPECT_TRUE(result.PartitionIds.size() > 0);
}

TEST_F(ProducerClientTest, GetPartitionProperties_LIVEONLY_)
{
  std::string eventHubName{GetEnv("EVENTHUB_NAME")};
  std::string const connString = GetEnv("EVENTHUB_CONNECTION_STRING");

  Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
  producerOptions.Name = "sender-link";
  producerOptions.ApplicationID = "some";

  Azure::Messaging::EventHubs::ProducerClient client{connString, eventHubName, producerOptions};

  ASSERT_NO_THROW([&]() {
    auto result = client.GetPartitionProperties("0");
    EXPECT_EQ(result.Name, eventHubName);
    EXPECT_EQ(result.PartitionId, "0");
  }());
}

TEST_F(ProducerClientTest, GetEventHubProperties_Multithreaded_LIVEONLY_)
{
  std::string eventHubName{GetEnv("EVENTHUB_NAME")};
  std::string const connString = GetEnv("EVENTHUB_CONNECTION_STRING");

  Azure::Messaging::EventHubs::ProducerClientOptions options;
  options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

  options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
  Azure::Messaging::EventHubs::ProducerClient client(connString, eventHubName);

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
        ASSERT_NO_THROW(result = client.GetEventHubProperties());
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

TEST_F(ProducerClientTest, GetPartitionProperties_Multithreaded_LIVEONLY_)
{
  std::string eventHubName{GetEnv("EVENTHUB_NAME")};
  std::string const connString = GetEnv("EVENTHUB_CONNECTION_STRING");

  Azure::Messaging::EventHubs::ProducerClientOptions options;
  options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

  options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
  Azure::Messaging::EventHubs::ProducerClient client(connString, eventHubName);

  auto ehProperties = client.GetEventHubProperties();
  std::vector<std::thread> threads;
  std::vector<size_t> iterationsPerThread;
  for (const auto& partition : ehProperties.PartitionIds)
  {
    threads.emplace_back(std::thread([&client, partition, eventHubName, &iterationsPerThread]() {
      GTEST_LOG_(INFO) << "Thread started for partition: " << partition << ".\n";
      for (int i = 0; i < 20; i++)
      {
        std::vector<std::thread> partitionThreads;
        partitionThreads.emplace_back([&client, &partition, eventHubName, &iterationsPerThread]() {
          size_t iterations = 0;
          std::chrono::system_clock::duration timeout = std::chrono::seconds(3);
          std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
          while ((std::chrono::system_clock::now() - start) <= timeout)
          {
            Azure::Messaging::EventHubs::Models::EventHubPartitionProperties result;
            ASSERT_NO_THROW(result = client.GetPartitionProperties(partition));
            EXPECT_EQ(result.Name, eventHubName);
            EXPECT_EQ(result.PartitionId, partition);
            std::this_thread::yield();
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
