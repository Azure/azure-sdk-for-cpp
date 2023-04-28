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
  (void) message;
  std::cout << "Message Id: " << i++<< std::endl;
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
        + ";EntityPath=hehe";

    auto client = Azure::Messaging::EventHubs::ConsumerClient(
        connStringNoEntityPath, "eventhub", "$DefaultZ");
    EXPECT_EQ("hehe", client.GetEventHubName());
    EXPECT_EQ("$DefaultZ", client.GetConsumerGroup());
  }

  TEST(ConsumerClientTest, ConnectionStringEntityPathNoConsumerGroup)
  {
    std::string const connStringNoEntityPath
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING")
        + ";EntityPath=hehe";
    auto client = Azure::Messaging::EventHubs::ConsumerClient(connStringNoEntityPath, "eventhub");
    EXPECT_EQ("hehe", client.GetEventHubName());
    EXPECT_EQ("$Default", client.GetConsumerGroup());
  }

  TEST(ConsumerClientTest, ConnectionStringEntityPathNoConsumerGroupNoEventHub)
  {
    std::string const connStringNoEntityPath
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING")
        + ";EntityPath=hehe";
    auto client = Azure::Messaging::EventHubs::ConsumerClient(connStringNoEntityPath);
    EXPECT_EQ("hehe", client.GetEventHubName());
    EXPECT_EQ("$Default", client.GetConsumerGroup());
  }

  TEST(ConsumerClientTest, ConnectToPartition)
  {
    std::string const connStringNoEntityPath
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING")
        + ";EntityPath=eventhub";
    Azure::Messaging::EventHubs::ConsumerClientOptions options;
    options.ApplicationID = "unit-test";
    options.ReceiverOptions.Name = "unit-test";
    options.ReceiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    options.ReceiverOptions.TargetAddress = "ingress";
    options.ReceiverOptions.EnableTrace = true;
    
    auto client = Azure::Messaging::EventHubs::ConsumerClient(connStringNoEntityPath);

    client.SetConsumer(LocalTest::ProcesMessageSuccess, "1");
    client.StartConsuming(5, "1");
  }
}}}} // namespace Azure::Messaging::EventHubs::Test
