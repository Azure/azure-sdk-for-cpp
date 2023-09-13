// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words hehe

#include "eventhubs_test_base.hpp"

#include <azure/core/context.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

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
  class ConsumerClientTest : public EventHubsTestBase {
    void SetUp() override
    {
      EventHubsTestBase::SetUp();
      if (m_testContext.IsLiveMode())
      {
        std::string const connStringNoEntityPath = GetEnv("EVENTHUB_CONNECTION_STRING");
        std::string eventHubName = GetEnv("EVENTHUB_NAME");

        Azure::Messaging::EventHubs::ProducerClient producer{connStringNoEntityPath, eventHubName};
        EventDataBatchOptions eventBatchOptions;
        eventBatchOptions.PartitionId = "1";
        EventDataBatch batch{producer.CreateBatch(eventBatchOptions)};
        EXPECT_TRUE(batch.TryAddMessage(Models::EventData{"Test"}));
        EXPECT_NO_THROW(producer.Send(batch));
      }
    }
  };

  TEST_F(ConsumerClientTest, ConnectionStringNoEntityPath_LIVEONLY_)
  {
    std::string const connStringNoEntityPath = GetEnv("EVENTHUB_CONNECTION_STRING");
    std::string consumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");
    std::string eventHubName = GetEnv("EVENTHUB_NAME");

    auto client = Azure::Messaging::EventHubs::ConsumerClient(
        connStringNoEntityPath, eventHubName, consumerGroup);
    EXPECT_EQ(eventHubName, client.GetEventHubName());
  }

  TEST_F(ConsumerClientTest, ConnectionStringEntityPath_LIVEONLY_)
  {
    std::string const connStringNoEntityPath
        = GetEnv("EVENTHUB_CONNECTION_STRING") + ";EntityPath=hehe";

    std::string consumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");
    std::string eventHubName = GetEnv("EVENTHUB_NAME");

    // The eventHubName parameter is ignored because the eventhub name is in the connection string.
    auto client = Azure::Messaging::EventHubs::ConsumerClient(
        connStringNoEntityPath, eventHubName, "$DefaultZ");
    EXPECT_EQ("hehe", client.GetEventHubName());
    EXPECT_EQ("$DefaultZ", client.GetConsumerGroup());
  }

  TEST_F(ConsumerClientTest, ConnectionStringEntityPathNoConsumerGroup_LIVEONLY_)
  {
    std::string const connStringNoEntityPath = GetEnv("EVENTHUB_CONNECTION_STRING");
    std::string eventHubName = GetEnv("EVENTHUB_NAME");
    auto client = Azure::Messaging::EventHubs::ConsumerClient(connStringNoEntityPath, eventHubName);
    EXPECT_EQ(eventHubName, client.GetEventHubName());
    EXPECT_EQ("$Default", client.GetConsumerGroup());
  }

  TEST_F(ConsumerClientTest, ConnectionStringEntityPathNoConsumerGroupNoEventHub_LIVEONLY_)
  {
    std::string const connStringNoEntityPath
        = GetEnv("EVENTHUB_CONNECTION_STRING") + ";EntityPath=hehe";
    auto client = Azure::Messaging::EventHubs::ConsumerClient(connStringNoEntityPath);
    EXPECT_EQ("hehe", client.GetEventHubName());
    EXPECT_EQ("$Default", client.GetConsumerGroup());
  }

  TEST_F(ConsumerClientTest, ConnectToPartition_LIVEONLY_)
  {
    std::string eventHubName{GetEnv("EVENTHUB_NAME")};
    std::string const connStringNoEntityPath
        = GetEnv("EVENTHUB_CONNECTION_STRING") + ";EntityPath=" + eventHubName;
    Azure::Messaging::EventHubs::ConsumerClientOptions options;
    options.ApplicationID
        = std::string(testing::UnitTest::GetInstance()->current_test_info()->name())
        + " Application";

    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();

    auto client = Azure::Messaging::EventHubs::ConsumerClient(
        connStringNoEntityPath, eventHubName, "$Default", options);
    Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Inclusive = true;
    // We want to consume all messages from the earliest.
    partitionOptions.StartPosition.Earliest = true;

    Azure::Messaging::EventHubs::PartitionClient partitionClient
        = client.CreatePartitionClient("1", partitionOptions);
    auto events = partitionClient.ReceiveEvents(1);
    EXPECT_EQ(events.size(), 1ul);
    GTEST_LOG_(INFO) << "Received message " << events[0].GetRawAmqpMessage();
    EXPECT_TRUE(events[0].EnqueuedTime.HasValue());
    EXPECT_TRUE(events[0].SequenceNumber.HasValue());
    EXPECT_TRUE(events[0].Offset.HasValue());
  }

  TEST_F(ConsumerClientTest, GetEventHubProperties_LIVEONLY_)
  {
    std::string eventHubName{GetEnv("EVENTHUB_NAME")};
    std::string const connStringEntityPath
        = GetEnv("EVENTHUB_CONNECTION_STRING") + ";EntityPath=" + eventHubName;

    Azure::Messaging::EventHubs::ConsumerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();
    auto client = Azure::Messaging::EventHubs::ConsumerClient(connStringEntityPath);
    Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Inclusive = true;

    Azure::Messaging::EventHubs::PartitionClient partitionClient
        = client.CreatePartitionClient("0", partitionOptions);

    auto result = client.GetEventHubProperties();
    EXPECT_EQ(result.Name, eventHubName);
    EXPECT_TRUE(result.PartitionIds.size() > 0);
  }

  TEST_F(ConsumerClientTest, GetPartitionProperties_LIVEONLY_)
  {

    std::string eventHubName{GetEnv("EVENTHUB_NAME")};
    std::string const connStringEntityPath
        = GetEnv("EVENTHUB_CONNECTION_STRING") + ";EntityPath=" + eventHubName;

    Azure::Messaging::EventHubs::ConsumerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();

    options.Name = testing::UnitTest::GetInstance()->current_test_case()->name();

    auto client = Azure::Messaging::EventHubs::ConsumerClient(connStringEntityPath);
    Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Inclusive = true;

    Azure::Messaging::EventHubs::PartitionClient partitionClient
        = client.CreatePartitionClient("0", partitionOptions);

    auto result = client.GetPartitionProperties("0");
    EXPECT_EQ(result.Name, eventHubName);
    EXPECT_EQ(result.PartitionId, "0");
  }

}}}} // namespace Azure::Messaging::EventHubs::Test
