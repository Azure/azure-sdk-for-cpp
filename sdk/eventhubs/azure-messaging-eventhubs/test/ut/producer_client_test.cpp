// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include <azure/core/context.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>
#include <numeric>
TEST(ProducerClientTest, ConnectionStringNoEntityPath)
{
  std::string const connStringNoEntityPath
      = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING");

  auto client = Azure::Messaging::EventHubs::ProducerClient(connStringNoEntityPath, "eventhub");
  EXPECT_EQ("eventhub", client.GetEventHubName());
}

TEST(ProducerClientTest, ConnectionStringEntityPath)
{
  std::string const connStringEntityPath
      = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING")
      + ";EntityPath=eventhub";

  auto client = Azure::Messaging::EventHubs::ProducerClient(connStringEntityPath, "eventhub");
  EXPECT_EQ("eventhub", client.GetEventHubName());
}

TEST(ProducerClientTest, TokenCredential)
{
  auto credential{std::make_shared<Azure::Identity::ClientSecretCredential>(
      Azure::Core::_internal::Environment::GetVariable("EVENTHUB_TENANT_ID"),
      Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CLIENT_ID"),
      Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CLIENT_SECRET"))};
  Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
  producerOptions.ApplicationID = "appId";
  auto client = Azure::Messaging::EventHubs::ProducerClient(
      "gearamaeh1.servicebus.windows.net", "eventhub", credential);
  EXPECT_EQ("eventhub", client.GetEventHubName());
}

TEST(ProducerClientTest, SendMessage)
{
  std::string const connStringEntityPath
      = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING")
      + ";EntityPath=eventhub";

  Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
  producerOptions.SenderOptions.Name = "sender-link";
  producerOptions.SenderOptions.EnableTrace = true;
  producerOptions.SenderOptions.SourceAddress = "ingress";
  producerOptions.SenderOptions.SettleMode
      = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
  producerOptions.SenderOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();
  producerOptions.ApplicationID = "some";

  Azure::Messaging::EventHubs::Models::AmqpAnnotatedMessage message2;
  Azure::Messaging::EventHubs::Models::EventData message1;
  message2.Body.Value = Azure::Core::Amqp::Models::AmqpValue("Hello5");

  message1.Body.Data.push_back(
      Azure::Core::Amqp::Models::AmqpBinaryData{'H', 'e', 'l', 'l', 'o', '2'});

  Azure::Messaging::EventHubs::Models::EventData message3;
  message2.Body.Sequence = Azure::Core::Amqp::Models::AmqpList{'H', 'e', 'l', 'l', 'o', '3'};

  Azure::Messaging::EventHubs::EventDataBatchOptions edboptions;
  edboptions.MaxBytes = std::numeric_limits<uint16_t>::max();
  edboptions.PartitionID = "1";
  Azure::Messaging::EventHubs::EventDataBatch eventBatch(edboptions);

  Azure::Messaging::EventHubs::EventDataBatchOptions edboptions2;
  edboptions2.MaxBytes = std::numeric_limits<uint16_t>::max();
  ;
  edboptions2.PartitionID = "2";
  Azure::Messaging::EventHubs::EventDataBatch eventBatch2(edboptions2);

  eventBatch.AddMessage(message1);
  eventBatch.AddMessage(message2);

  eventBatch2.AddMessage(message3);
  eventBatch2.AddMessage(message2);

  auto client = Azure::Messaging::EventHubs::ProducerClient(
      connStringEntityPath, "eventhub", producerOptions);
  for (int i = 0; i < 5; i++)
  {
    auto result = client.SendEventDataBatch(eventBatch);
    EXPECT_TRUE(result);
  }
}

TEST(ProducerClientTest, GetEventHubProperties)
{
  std::string const connStringEntityPath
      = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING")
      + ";EntityPath=eventhub";

  Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
  producerOptions.SenderOptions.Name = "sender-link";
  producerOptions.SenderOptions.EnableTrace = true;
  producerOptions.SenderOptions.SourceAddress = "ingress";
  producerOptions.SenderOptions.SettleMode
      = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
  producerOptions.SenderOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();
  producerOptions.ApplicationID = "some";

  auto client = Azure::Messaging::EventHubs::ProducerClient(
      connStringEntityPath, "eventhub", producerOptions);
  
  auto result = client.GetEventHubProperties();
}
