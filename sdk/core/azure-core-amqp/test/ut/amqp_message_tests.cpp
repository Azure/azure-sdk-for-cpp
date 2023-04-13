// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <gtest/gtest.h>

#include "azure/core/amqp/models/amqp_message.hpp"

using namespace Azure::Core::Amqp::Models;

class TestMessage : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestMessage, SimpleCreate)
{
  {
    Message message;
  }

  {
    Message message1;
    Message message2(std::move(message1));
    Message message3(message2);
    Message message4;
    message4 = message2;
    GTEST_LOG_(INFO) << message4;
    Message message5 = std::move(message3);
    GTEST_LOG_(INFO) << message5;
  }

  {
    Message message;

    EXPECT_TRUE(message.ApplicationProperties.empty());
    // By default, the body type is None, so retrieving the body as any other type should throw.
    EXPECT_EQ(MessageBodyType::None, message.BodyType);
    EXPECT_ANY_THROW(message.GetBodyAsAmqpList());
    EXPECT_ANY_THROW(message.GetBodyAsAmqpValue());
    EXPECT_ANY_THROW(message.GetBodyAsBinary());
  }
}

TEST_F(TestMessage, TestApplicationProperties)
{
  Message message;

  // Ensure that ApplicationProperties values round-trip through uAMQP value serialization.
  message.ApplicationProperties["Blagh"] = 19532;

  MESSAGE_INSTANCE_TAG* messageInstance = message;
  Message message2(messageInstance);

  EXPECT_EQ(message2.ApplicationProperties["Blagh"], AmqpValue(19532));

  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestDeliveryAnnotations)
{
  Message message;
  message.DeliveryAnnotations["12345"] = 19532;

  MESSAGE_INSTANCE_TAG* messageInstance = message;
  Message message2(messageInstance);
  EXPECT_EQ(AmqpValue{19532}, message2.DeliveryAnnotations["12345"]);
  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestAnnotations)
{
  Message message;
  message.MessageAnnotations["12345"] = 19532;

  MESSAGE_INSTANCE_TAG* messageInstance = message;
  Message message2(messageInstance);
  EXPECT_EQ(AmqpValue{19532}, message2.MessageAnnotations["12345"]);
  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestFooter)
{
  Message message;
  message.Footer["12345"] = 37.2;

  MESSAGE_INSTANCE_TAG* messageInstance = message;
  Message message2(messageInstance);
  EXPECT_EQ(AmqpValue{37.2}, message2.Footer["12345"]);

  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestHeader)
{
  Message message;
  message.Header.DeliveryCount = 1;

  MESSAGE_INSTANCE_TAG* messageInstance = message;
  Message message2(messageInstance);

  // Ensure that message values survive across round-trips through MESSAGE.
  EXPECT_EQ(message2.Header.DeliveryCount, 1);
  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestProperties)
{
  Message message;
  MessageProperties properties;
  properties.Subject = "Message subject.";
  message.Properties = properties;

  MESSAGE_INSTANCE_TAG* messageInstance = message;
  Message message2(messageInstance);

  auto newProperties{message2.Properties};
  EXPECT_EQ(newProperties.Subject.Value(), properties.Subject.Value());
  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestFormat)
{
  Message message;
  message.MessageFormat = 12345;

  MESSAGE_INSTANCE_TAG* messageInstance = message;
  Message message2(messageInstance);

  EXPECT_EQ(message2.MessageFormat.Value(), 12345);
  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestBodyAmqpSequence)
{
  Message message;

  message.SetBody({"Test", 95, AmqpMap{{3,5},{4,9}}});

  EXPECT_EQ(3, message.GetBodyAsAmqpList().size());
  EXPECT_EQ("Test", static_cast<std::string>(message.GetBodyAsAmqpList()[0]));
  EXPECT_EQ(95, static_cast<int32_t>(message.GetBodyAsAmqpList()[1]));
  EXPECT_EQ(message.BodyType, MessageBodyType::Sequence);

  MESSAGE_INSTANCE_TAG* messageInstance = message;
  Message message2(messageInstance);
  EXPECT_EQ(3, message2.GetBodyAsAmqpList().size());
  EXPECT_EQ("Test", static_cast<std::string>(message2.GetBodyAsAmqpList()[0]));
  EXPECT_EQ(95, static_cast<int32_t>(message2.GetBodyAsAmqpList()[1]));
  EXPECT_EQ(message2.BodyType, MessageBodyType::Sequence);

  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestBodyAmqpData)
{
  Message message;
  uint8_t testBody[] = "Test body";
  message.SetBody(AmqpBinaryData{'T', 'e', 's', 't', ' ', 'b', 'o', 'd', 'y', 0});
  EXPECT_EQ(message.GetBodyAsBinary().size(), 1);

  auto body = message.GetBodyAsBinary()[0];
  EXPECT_EQ(body.size(), sizeof(testBody));
  EXPECT_EQ(memcmp(body.data(), testBody, sizeof(testBody)), 0);

  EXPECT_EQ(message.BodyType, MessageBodyType::Data);

  MESSAGE_INSTANCE_TAG* messageInstance = message;
  Message message2(messageInstance);
  EXPECT_EQ(message2.GetBodyAsBinary().size(), 1);

  auto body2 = message2.GetBodyAsBinary()[0];
  EXPECT_EQ(body2.size(), sizeof(testBody));
  EXPECT_EQ(memcmp(body2.data(), testBody, sizeof(testBody)), 0);

  EXPECT_EQ(message2.BodyType, MessageBodyType::Data);

  GTEST_LOG_(INFO) << message;
}
