// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words gearamaeh1

#include "eventhubs_test_base.hpp"

#include <azure/core/context.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <numeric>

#include <gtest/gtest.h>

class ProducerClientTest : public EventHubsTestBase {
};

TEST_F(ProducerClientTest, ConnectionStringNoEntityPath_LIVEONLY_)
{
  std::string const connStringNoEntityPath = GetEnv("EVENTHUB_CONNECTION_STRING");
  std::string eventHubName{GetEnv("EVENTHUB_NAME")};

  auto client = Azure::Messaging::EventHubs::ProducerClient(connStringNoEntityPath, eventHubName);
  EXPECT_EQ(eventHubName, client.GetEventHubName());
}

TEST_F(ProducerClientTest, ConnectionStringEntityPath_LIVEONLY_)
{
  std::string eventHubName{GetEnv("EVENTHUB_NAME")};
  std::string const connStringEntityPath
      = GetEnv("EVENTHUB_CONNECTION_STRING") + ";EntityPath=" + eventHubName;

  auto client = Azure::Messaging::EventHubs::ProducerClient(connStringEntityPath, eventHubName);
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
  auto client = Azure::Messaging::EventHubs::ProducerClient(
      "gearamaeh1.servicebus.windows.net", eventHubName, credential);
  EXPECT_EQ(eventHubName, client.GetEventHubName());
}

TEST_F(ProducerClientTest, SendMessage_LIVEONLY_)
{
  std::string eventHubName{GetEnv("EVENTHUB_NAME")};
  std::string const connStringEntityPath
      = GetEnv("EVENTHUB_CONNECTION_STRING") + ";EntityPath=" + eventHubName;

  Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
  producerOptions.Name = "sender-link";
  producerOptions.VerboseLogging = true;
  producerOptions.MessageSource = "ingress";
  producerOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
  producerOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();
  producerOptions.ApplicationID = "some";

  Azure::Core::Amqp::Models::AmqpMessage message2;
  Azure::Messaging::EventHubs::Models::EventData message1;
  message2.SetBody(Azure::Core::Amqp::Models::AmqpValue("Hello7"));

  message1.Body.Data = {'H', 'e', 'l', 'l', 'o', '2'};

  Azure::Messaging::EventHubs::Models::EventData message3;
  message3.Body.Sequence = {'H', 'e', 'l', 'l', 'o', '3'};

  Azure::Messaging::EventHubs::EventDataBatchOptions edboptions;
  edboptions.MaxBytes = std::numeric_limits<uint16_t>::max();
  edboptions.PartitionId = "1";
  Azure::Messaging::EventHubs::EventDataBatch eventBatch(edboptions);

  Azure::Messaging::EventHubs::EventDataBatchOptions edboptions2;
  edboptions2.MaxBytes = std::numeric_limits<uint16_t>::max();
  ;
  edboptions2.PartitionId = "2";
  Azure::Messaging::EventHubs::EventDataBatch eventBatch2(edboptions2);

  eventBatch.AddMessage(message1);
  eventBatch.AddMessage(message2);

  eventBatch2.AddMessage(message3);
  eventBatch2.AddMessage(message2);

  auto client = Azure::Messaging::EventHubs::ProducerClient(
      connStringEntityPath, eventHubName, producerOptions);
  for (int i = 0; i < 5; i++)
  {
    auto result = client.SendEventDataBatch(eventBatch);
    EXPECT_TRUE(result);
  }
}

TEST_F(ProducerClientTest, GetEventHubProperties_LIVEONLY_)
{
  std::string eventHubName{GetEnv("EVENTHUB_NAME")};
  std::string const connStringEntityPath
      = GetEnv("EVENTHUB_CONNECTION_STRING") + ";EntityPath=" + eventHubName;

  Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
  producerOptions.Name = "sender-link";
  producerOptions.VerboseLogging = true;
  producerOptions.MessageSource = "ingress";
  producerOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
  producerOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();
  producerOptions.ApplicationID = "some";

  auto client = Azure::Messaging::EventHubs::ProducerClient(
      connStringEntityPath, eventHubName, producerOptions);

  auto result = client.GetEventHubProperties();
  EXPECT_EQ(result.Name, eventHubName);
  EXPECT_TRUE(result.PartitionIds.size() > 0);
}

TEST_F(ProducerClientTest, GetPartitionProperties_LIVEONLY_)
{
  std::string eventHubName{GetEnv("EVENTHUB_NAME")};
  std::string const connStringEntityPath
      = GetEnv("EVENTHUB_CONNECTION_STRING") + ";EntityPath=" + eventHubName;

  Azure::Messaging::EventHubs::ProducerClientOptions producerOptions;
  producerOptions.Name = "sender-link";
  producerOptions.VerboseLogging = true;
  producerOptions.MessageSource = "ingress";
  producerOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
  producerOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();
  producerOptions.ApplicationID = "some";

  auto client = Azure::Messaging::EventHubs::ProducerClient(
      connStringEntityPath, eventHubName, producerOptions);

  auto result = client.GetPartitionProperties("0");
  EXPECT_EQ(result.Name, eventHubName);
  EXPECT_EQ(result.PartitionId, "0");
}
