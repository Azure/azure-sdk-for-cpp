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
  };

  TEST_F(ConsumerClientTest, ConnectionStringNoEntityPath_LIVEONLY_)
  {
    std::string const connStringNoEntityPath = GetEnv("EVENTHUB_CONNECTION_STRING");

    auto client = Azure::Messaging::EventHubs::ConsumerClient(
        connStringNoEntityPath, "eventhub", "$Default");
    EXPECT_EQ("eventhub", client.GetEventHubName());
  }

  TEST_F(ConsumerClientTest, ConnectionStringEntityPath_LIVEONLY_)
  {
    std::string const connStringNoEntityPath
        = GetEnv("EVENTHUB_CONNECTION_STRING") + ";EntityPath=hehe";

    auto client = Azure::Messaging::EventHubs::ConsumerClient(
        connStringNoEntityPath, "eventhub", "$DefaultZ");
    EXPECT_EQ("hehe", client.GetEventHubName());
    EXPECT_EQ("$DefaultZ", client.GetConsumerGroup());
  }

  TEST_F(ConsumerClientTest, ConnectionStringEntityPathNoConsumerGroup_LIVEONLY_)
  {
    std::string const connStringNoEntityPath = GetEnv("EVENTHUB_CONNECTION_STRING");
    auto client = Azure::Messaging::EventHubs::ConsumerClient(connStringNoEntityPath, "eventhub");
    EXPECT_EQ("eventhub", client.GetEventHubName());
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
    options.ApplicationID = "unit-test";

    options.Name = "unit-test";

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
    options.ApplicationID = "unit-test";

    options.Name = "unit-test";
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
    options.ApplicationID = "unit-test";

    options.Name = "unit-test";
    options.MaxMessageSize = std::numeric_limits<uint16_t>::max();

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
