// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include <azure/core/context.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>
namespace LocalTest {
int i = 0;
void ProcesMessageSuccess(Azure::Core::Amqp::Models::AmqpMessage const& message)
{
  (void)message;
  std::cout << "Message Id: " << i++ << std::endl;
}
} // namespace LocalTest
namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  TEST(ConsumerClientTest, ConnectionStringNoEntityPath)
  {
    std::string const connStringNoEntityPath
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING");

    auto client = Azure::Messaging::EventHubs::ConsumerClient(
        connStringNoEntityPath, "eventhub", "$Default");
    EXPECT_EQ("eventhub", client.GetEventHubName());
  }

  TEST(ConsumerClientTest, ConnectionStringEntityPath)
  {
    std::string const connStringNoEntityPath
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING")
        + ";EntityPath=haha";

    auto client = Azure::Messaging::EventHubs::ConsumerClient(
        connStringNoEntityPath, "eventhub", "$DefaultZ");
    EXPECT_EQ("haha", client.GetEventHubName());
    EXPECT_EQ("$DefaultZ", client.GetConsumerGroup());
  }

  TEST(ConsumerClientTest, ConnectionStringEntityPathNoConsumerGroup)
  {
    std::string const connStringNoEntityPath
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING")
        + ";EntityPath=haha";
    auto client = Azure::Messaging::EventHubs::ConsumerClient(connStringNoEntityPath, "eventhub");
    EXPECT_EQ("haha", client.GetEventHubName());
    EXPECT_EQ("$Default", client.GetConsumerGroup());
  }

  TEST(ConsumerClientTest, ConnectionStringEntityPathNoConsumerGroupNoEventHub)
  {
    std::string const connStringNoEntityPath
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING")
        + ";EntityPath=haha";
    auto client = Azure::Messaging::EventHubs::ConsumerClient(connStringNoEntityPath);
    EXPECT_EQ("haha", client.GetEventHubName());
    EXPECT_EQ("$Default", client.GetConsumerGroup());
  }

  TEST(ConsumerClientTest, ConnectToPartition)
  {
    std::string const connStringNoEntityPath
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING")
        + ";EntityPath=eventhub";
    Azure::Messaging::EventHubs::Models::ConsumerClientOptions options;
    options.ApplicationID = "unit-test";

    options.ReceiverOptions.Name = "unit-test";
    options.ReceiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    options.ReceiverOptions.MessageTarget = "ingress";
    options.ReceiverOptions.EnableTrace = true;
    options.ReceiverOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();

    auto client = Azure::Messaging::EventHubs::ConsumerClient(connStringNoEntityPath);
    Azure::Messaging::EventHubs::Models::PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Inclusive = true;

    Azure::Messaging::EventHubs::PartitionClient partitionClient
        = client.NewPartitionClient("1", partitionOptions);
    auto events = partitionClient.ReceiveEvents(1);
    EXPECT_EQ(events.size(), 1);
  }

  TEST(ConsumerClientTest, GetEventHubProperties)
  {
    std::string const connStringEntityPath
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING")
        + ";EntityPath=eventhub";

    Azure::Messaging::EventHubs::Models::ConsumerClientOptions options;
    options.ApplicationID = "unit-test";

    options.ReceiverOptions.Name = "unit-test";
    options.ReceiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    options.ReceiverOptions.MessageTarget = "ingress";
    options.ReceiverOptions.EnableTrace = true;
    options.ReceiverOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();
    auto client = Azure::Messaging::EventHubs::ConsumerClient(connStringEntityPath);
    Azure::Messaging::EventHubs::Models::PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Inclusive = true;

    Azure::Messaging::EventHubs::PartitionClient partitionClient
        = client.NewPartitionClient("0", partitionOptions);

    auto result = client.GetEventHubProperties();
    EXPECT_EQ(result.Name, "eventhub");
    EXPECT_TRUE(result.PartitionIDs.size() > 0);
  }

  TEST(ConsumerClientTest, GetPartitionProperties)
  {
    std::string const connStringEntityPath
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING")
        + ";EntityPath=eventhub";

    Azure::Messaging::EventHubs::Models::ConsumerClientOptions options;
    options.ApplicationID = "unit-test";

    options.ReceiverOptions.Name = "unit-test";
    options.ReceiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    options.ReceiverOptions.MessageTarget = "ingress";
    options.ReceiverOptions.EnableTrace = true;
    options.ReceiverOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();

    auto client = Azure::Messaging::EventHubs::ConsumerClient(connStringEntityPath);
    Azure::Messaging::EventHubs::Models::PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition.Inclusive = true;

    Azure::Messaging::EventHubs::PartitionClient partitionClient
        = client.NewPartitionClient("0", partitionOptions);

    auto result = client.GetPartitionProperties("0");
    EXPECT_EQ(result.Name, "eventhub");
    EXPECT_EQ(result.PartitionId, "0");
  }
}}}} // namespace Azure::Messaging::EventHubs::Test
