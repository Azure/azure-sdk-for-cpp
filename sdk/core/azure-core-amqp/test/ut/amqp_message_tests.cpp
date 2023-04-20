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
    AmqpMessage message;
  }

  {
    AmqpMessage message1;
    AmqpMessage message2(std::move(message1));
    AmqpMessage message3(message2);
    AmqpMessage message4;
    message4 = message2;
    GTEST_LOG_(INFO) << message4;
    AmqpMessage message5 = std::move(message3);
    GTEST_LOG_(INFO) << message5;
  }

  {
    AmqpMessage message;

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
  AmqpMessage message;

  // Ensure that ApplicationProperties values round-trip through uAMQP value serialization.
  message.ApplicationProperties["Blah"] = 19532;

  auto messageInstance = _internal::AmqpMessageFactory::ToUamqp(message);
  AmqpMessage message2(_internal::AmqpMessageFactory::FromUamqp(messageInstance));

  EXPECT_EQ(message2.ApplicationProperties["Blah"], AmqpValue(19532));

  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestDeliveryAnnotations)
{
  AmqpMessage message;
  message.DeliveryAnnotations["12345"] = 19532;

  auto messageInstance = _internal::AmqpMessageFactory::ToUamqp(message);
  AmqpMessage message2(_internal::AmqpMessageFactory::FromUamqp(messageInstance));
  EXPECT_EQ(AmqpValue{19532}, message2.DeliveryAnnotations["12345"]);
  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestAnnotations)
{
  AmqpMessage message;
  message.MessageAnnotations["12345"] = 19532;

  auto messageInstance = _internal::AmqpMessageFactory::ToUamqp(message);
  AmqpMessage message2(_internal::AmqpMessageFactory::FromUamqp(messageInstance));
  EXPECT_EQ(AmqpValue{19532}, message2.MessageAnnotations["12345"]);
  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestFooter)
{
  AmqpMessage message;
  message.Footer["12345"] = 37.2;

  auto messageInstance = _internal::AmqpMessageFactory::ToUamqp(message);
  AmqpMessage message2(_internal::AmqpMessageFactory::FromUamqp(messageInstance));
  EXPECT_EQ(AmqpValue{37.2}, message2.Footer["12345"]);

  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestHeader)
{
  AmqpMessage message;
  message.Header.DeliveryCount = 1;

  auto messageInstance = _internal::AmqpMessageFactory::ToUamqp(message);
  AmqpMessage message2(_internal::AmqpMessageFactory::FromUamqp(messageInstance));

  // Ensure that message values survive across round-trips through MESSAGE.
  EXPECT_EQ(message2.Header.DeliveryCount, 1);
  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestProperties)
{
  AmqpMessage message;
  MessageProperties properties;
  properties.Subject = "Message subject.";
  message.Properties = properties;

  auto messageInstance = _internal::AmqpMessageFactory::ToUamqp(message);
  AmqpMessage message2(_internal::AmqpMessageFactory::FromUamqp(messageInstance));

  auto newProperties{message2.Properties};
  EXPECT_EQ(newProperties.Subject.Value(), properties.Subject.Value());
  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestFormat)
{
  AmqpMessage message;
  message.MessageFormat = 12345;

  auto messageInstance = _internal::AmqpMessageFactory::ToUamqp(message);
  AmqpMessage message2(_internal::AmqpMessageFactory::FromUamqp(messageInstance));

  EXPECT_EQ(message2.MessageFormat.Value(), 12345);
  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestBodyAmqpSequence)
{
  AmqpMessage message;

  message.SetBody({"Test", 95, AmqpMap{{3, 5}, {4, 9}}});

  EXPECT_EQ(3, message.GetBodyAsAmqpList().size());
  EXPECT_EQ("Test", static_cast<std::string>(message.GetBodyAsAmqpList().at(0)));
  EXPECT_EQ(95, static_cast<int32_t>(message.GetBodyAsAmqpList().at(1)));
  EXPECT_EQ(message.BodyType, MessageBodyType::Sequence);

  auto messageInstance = _internal::AmqpMessageFactory::ToUamqp(message);
  AmqpMessage message2(_internal::AmqpMessageFactory::FromUamqp(messageInstance));
  EXPECT_EQ(3, message2.GetBodyAsAmqpList().size());
  EXPECT_EQ("Test", static_cast<std::string>(message2.GetBodyAsAmqpList().at(0)));
  EXPECT_EQ(95, static_cast<int32_t>(message2.GetBodyAsAmqpList().at(1)));
  EXPECT_EQ(message2.BodyType, MessageBodyType::Sequence);

  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessage, TestBodyAmqpData)
{
  AmqpMessage message;
  uint8_t testBody[] = "Test body";
  message.SetBody(AmqpBinaryData{'T', 'e', 's', 't', ' ', 'b', 'o', 'd', 'y', 0});
  EXPECT_EQ(message.GetBodyAsBinary().size(), 1);

  auto body = message.GetBodyAsBinary()[0];
  EXPECT_EQ(body.size(), sizeof(testBody));
  EXPECT_EQ(memcmp(body.data(), testBody, sizeof(testBody)), 0);

  EXPECT_EQ(message.BodyType, MessageBodyType::Data);

  auto messageInstance = _internal::AmqpMessageFactory::ToUamqp(message);
  AmqpMessage message2(_internal::AmqpMessageFactory::FromUamqp(messageInstance));
  EXPECT_EQ(message2.GetBodyAsBinary().size(), 1);

  auto body2 = message2.GetBodyAsBinary()[0];
  EXPECT_EQ(body2.size(), sizeof(testBody));
  EXPECT_EQ(memcmp(body2.data(), testBody, sizeof(testBody)), 0);

  EXPECT_EQ(message2.BodyType, MessageBodyType::Data);

  GTEST_LOG_(INFO) << message;
}
