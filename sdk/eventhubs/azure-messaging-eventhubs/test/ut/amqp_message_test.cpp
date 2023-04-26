// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include <azure/messaging/eventhubs.hpp>

using namespace Azure::Core::Amqp::Models;
using namespace Azure::Messaging::EventHubs::Models;
TEST(AMQPMessageTest, EventData1)
{
  Azure::Messaging::EventHubs::Models::EventData eventData;

  eventData.Body.Value = Azure::Core::Amqp::Models::AmqpValue(5);
  eventData.ContentType = "ct";
  eventData.ApplicationProperties.emplace("abc", AmqpValue(23));
  eventData.CorrelationId = AmqpValue("ci");
  eventData.MessageId = AmqpValue("mi");

  auto message = eventData.ToAMQPMessage();

  EXPECT_EQ(1, message.ApplicationProperties.size());
  EXPECT_EQ(eventData.Body.Value, message.GetBodyAsAmqpValue());
  EXPECT_EQ("ct", message.Properties.ContentType.Value());
  EXPECT_EQ(AmqpValue("ci"), message.Properties.CorrelationId.Value());
  EXPECT_FALSE(message.Properties.MessageId.HasValue());
}

TEST(AMQPMessageTest, EventDataNew)
{
  Azure::Messaging::EventHubs::Models::EventData eventData;

  auto message = eventData.ToAMQPMessage();

  EXPECT_EQ(0, message.ApplicationProperties.size());
  EXPECT_FALSE(message.Properties.ContentType.HasValue());
  EXPECT_FALSE(message.Properties.CorrelationId.HasValue());
  EXPECT_FALSE(message.Properties.MessageId.HasValue());
}

TEST(AmqpMessageTest, AmqpMessage)
{
  Azure::Messaging::EventHubs::Models::AmqpAnnotatedMessage msg;

  auto message = msg.ToAMQPMessage();

  EXPECT_EQ(0, message.ApplicationProperties.size());
  EXPECT_FALSE(message.Properties.ContentType.HasValue());
  EXPECT_FALSE(message.Properties.CorrelationId.HasValue());
  EXPECT_FALSE(message.Properties.MessageId.HasValue());
}

TEST(AmqpMessageTest, AmqpMessage2)
{
  Azure::Messaging::EventHubs::Models::AmqpAnnotatedMessage msg;
  msg.Body.Value = AmqpValue("3");
  auto message = msg.ToAMQPMessage();

  EXPECT_EQ(msg.Body.Value, message.GetBodyAsAmqpValue());
  EXPECT_EQ(0, message.ApplicationProperties.size());
  EXPECT_FALSE(message.Properties.ContentType.HasValue());
  EXPECT_FALSE(message.Properties.CorrelationId.HasValue());
  EXPECT_FALSE(message.Properties.MessageId.HasValue());
}