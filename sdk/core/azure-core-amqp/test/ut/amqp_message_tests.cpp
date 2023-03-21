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
    Message message;

    EXPECT_EQ(
        message.GetApplicationProperties().GetType(),
        Azure::Core::Amqp::Models::AmqpValueType::Null);
    EXPECT_ANY_THROW(message.GetBodyAmqpDataCount());
    EXPECT_ANY_THROW(message.GetBodyAmqpSequence(0));
    EXPECT_ANY_THROW(message.GetBodyAmqpSequenceCount());

    EXPECT_EQ(MessageBodyType::None, message.GetBodyType());
    EXPECT_EQ(
        Azure::Core::Amqp::Models::AmqpValueType::Null, message.GetDeliveryAnnotations().GetType());
    EXPECT_EQ(Azure::Core::Amqp::Models::AmqpValueType::Null, message.GetFooter().GetType());
    EXPECT_EQ(0, message.GetFormat());
    EXPECT_FALSE(message.GetHeader());
    EXPECT_EQ(
        Azure::Core::Amqp::Models::AmqpValueType::Null, message.GetMessageAnnotations().GetType());
    EXPECT_FALSE(message.GetProperties());
  }
}

TEST_F(TestMessage, TestApplicationProperties)
{
  Message message;
  Properties properties;
  properties.SetSubject("Message subject.");
  message.SetApplicationProperties(Value::CreateProperties(properties));

  auto propertiesAsValue{message.GetApplicationProperties()};
  //  EXPECT_TRUE(propertiesAsValue.IsPropertiesTypeByDescriptor());
  auto newProperties{propertiesAsValue.GetPropertiesFromValue()};

  EXPECT_EQ(newProperties.GetSubject(), properties.GetSubject());
}

TEST_F(TestMessage, TestBodyAmqpValue) {}

TEST_F(TestMessage, TestDeliveryAnnotations)
{
  Message message;
  message.SetDeliveryAnnotations("12345");
  EXPECT_EQ(message.GetDeliveryAnnotations(), "12345");
}

TEST_F(TestMessage, TestFooter)
{
  Message message;
  message.SetFooter(32.7);
  EXPECT_EQ(static_cast<double>(message.GetFooter()), 32.7);
}

TEST_F(TestMessage, TestHeader)
{
  Message message;
  message.SetHeader(Header());
  EXPECT_EQ(message.GetHeader().GetDeliveryCount(), 0);
}

TEST_F(TestMessage, TestMessageAnnotations)
{
  Message message;
  message.SetMessageAnnotations("12345");
  EXPECT_EQ(message.GetMessageAnnotations(), "12345");
}

TEST_F(TestMessage, TestProperties)
{
  Message message;
  Properties properties;
  properties.SetSubject("Message subject.");
  message.SetProperties(properties);

  auto newProperties{message.GetProperties()};
  EXPECT_EQ(newProperties.GetSubject(), properties.GetSubject());
}

TEST_F(TestMessage, TestFormat)
{
  Message message;
  message.SetFormat(12345);
  EXPECT_EQ(message.GetFormat(), 12345);
}

TEST_F(TestMessage, TestBodyAmqpSequence)
{
  Message message;
  EXPECT_ANY_THROW(message.GetBodyAmqpSequence(0));

  message.AddBodyAmqpSequence("Test");
  message.AddBodyAmqpSequence(static_cast<uint32_t>(95));
  message.AddBodyAmqpSequence(Value::CreateProperties(Properties()));

  EXPECT_EQ(3, message.GetBodyAmqpSequenceCount());
  EXPECT_EQ("Test", static_cast<std::string>(message.GetBodyAmqpSequence(0)));
  EXPECT_EQ(95, static_cast<uint32_t>(message.GetBodyAmqpSequence(1)));

  EXPECT_EQ(message.GetBodyType(), MessageBodyType::Sequence);
}

TEST_F(TestMessage, TestBodyAmqpData)
{
  Message message;
  uint8_t testBody[] = "Test body";
  message.AddBodyAmqpData(BinaryData{testBody, sizeof(testBody)});
  EXPECT_EQ(message.GetBodyAmqpDataCount(), 1);

  auto body = message.GetBodyAmqpData(0);
  EXPECT_EQ(body.length, sizeof(testBody));
  EXPECT_EQ(memcmp(body.bytes, testBody, sizeof(testBody)), 0);

  EXPECT_EQ(message.GetBodyType(), MessageBodyType::Data);
}