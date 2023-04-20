// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include <azure/core/context.hpp>
#include <azure/messaging/eventhubs.hpp>
#include <azure/identity.hpp>
#include <azure/core/internal/environment.hpp>

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
      = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING") + ";EntityPath=eventhub";

  auto client = Azure::Messaging::EventHubs::ProducerClient(connStringEntityPath, "eventhub");
  EXPECT_EQ("eventhub", client.GetEventHubName());
}

TEST(ProducerClientTest, TokenCredential)
{
  auto credential{std::make_shared<Azure::Identity::ClientSecretCredential>(
      Azure::Core::_internal::Environment::GetVariable("EVENTHUB_TENANT_ID"),
      Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CLIENT_ID"),
      Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CLIENT_SECRET"))};

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
  producerOptions.SenderOptions.Name = "unit-test";
  producerOptions.SenderOptions.EnableTrace = true;
  producerOptions.SenderOptions.SourceAddress = "ingress";
  producerOptions.SenderOptions.SettleMode = Azure::Core::Amqp::_internal::SenderSettleMode::Settled;
  producerOptions.SenderOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();
  producerOptions.ApplicationID = "unit-test";
  

  Azure::Core::Amqp::Models::Message message1;
  message1.SetBody(Azure::Core::Amqp::Models::AmqpValue{"Hello"});

  Azure::Core::Amqp::Models::Message message2;
  message2.SetBody(Azure::Core::Amqp::Models::AmqpBinaryData{'H', 'e', 'l', 'l', 'o','2'});
  
  
  Azure::Core::Amqp::Models::Message message3;
  message2.SetBody(Azure::Core::Amqp::Models::AmqpList{'H', 'e', 'l', 'l', 'o', '3'});
  
  Azure::Messaging::EventHubs::EventDataBatch eventBatch ;

  eventBatch.AddMessage(message1);
  eventBatch.AddMessage(message2);
  auto client = Azure::Messaging::EventHubs::ProducerClient(connStringEntityPath, "eventhub", producerOptions);

  auto result = client.SendEventDataBatch(eventBatch);

  EXPECT_EQ(std::get<0>(result[0]), Azure::Core::Amqp::_internal::MessageSendResult::Ok);
}