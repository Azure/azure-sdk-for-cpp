// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include <gtest/gtest.h>

#include "azure/core/amqp/models/amqp_properties.hpp"

using namespace Azure::Core::Amqp::Models;

class TestProperties : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestProperties, SimpleCreate)
{
  {
    MessageProperties properties;
    GTEST_LOG_(INFO) << properties;
  }

  {
    MessageProperties properties;
    EXPECT_ANY_THROW(
        properties
            .GetAbsoluteExpiryTime()); // Cannot get absolute expiry time before it has been set.
    EXPECT_ANY_THROW(
        properties.GetContentEncoding()); // Cannot get content encoding before it has been set.
    EXPECT_ANY_THROW(properties.GetContentType());
    EXPECT_EQ(AmqpValueType::Null, properties.GetCorrelationId().GetType());
    EXPECT_ANY_THROW(properties.GetCreationTime());
    EXPECT_ANY_THROW(properties.GetGroupId());
    EXPECT_ANY_THROW(properties.GetGroupSequence());
    EXPECT_ANY_THROW(properties.GetMessageId());
    EXPECT_ANY_THROW(properties.GetReplyTo());
    EXPECT_ANY_THROW(properties.GetReplyToGroupId());
  }
}

TEST_F(TestProperties, SetAbsoluteExpiryTime)
{
  // AMQP MessageProperties represent times in milliseconds, so we need to reduce the accuracy of
  // std::chrono::system_clock::now to milliseconds before we check it.
  MessageProperties properties;
  auto testTimestamp = std::chrono::system_clock::now();
  auto testTimestampMs{
      std::chrono::duration_cast<std::chrono::milliseconds>(testTimestamp.time_since_epoch())};
  std::chrono::system_clock::time_point testTimestampToCheck{
      std::chrono::duration_cast<std::chrono::system_clock::duration>(testTimestampMs)};

  // Set the test timestamp and verify that the returned value is accurate to milliseconds.
  properties.SetAbsoluteExpiryTime(testTimestamp);
  EXPECT_EQ(properties.GetAbsoluteExpiryTime(), testTimestampToCheck);
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetContentEncoding)
{
  MessageProperties properties;
  std::string contentEncoding = "utf-8";
  properties.SetContentEncoding(contentEncoding);
  EXPECT_EQ(properties.GetContentEncoding(), contentEncoding);
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetContentType)
{
  MessageProperties properties;
  std::string contentType = "text/plain";
  properties.SetContentType(contentType);
  EXPECT_EQ(properties.GetContentType(), contentType);
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetCorrelationId)
{
  MessageProperties properties;
  std::string correlationId = "1234";
  properties.SetCorrelationId(AmqpValue{correlationId});
  EXPECT_EQ(properties.GetCorrelationId(), AmqpValue{correlationId});
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetCreationTime)
{
  MessageProperties properties;
  auto testTimestamp = std::chrono::system_clock::now();
  auto testTimestampMs{
      std::chrono::duration_cast<std::chrono::milliseconds>(testTimestamp.time_since_epoch())};
  std::chrono::system_clock::time_point testTimestampToCheck{
      std::chrono::duration_cast<std::chrono::system_clock::duration>(testTimestampMs)};

  properties.SetCreationTime(testTimestamp);
  EXPECT_EQ(properties.GetCreationTime(), testTimestampToCheck);

  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetGroupId)
{
  MessageProperties properties;
  std::string groupId = "1234";
  properties.SetGroupId(groupId);
  EXPECT_EQ(properties.GetGroupId(), groupId);
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetGroupSequence)
{
  MessageProperties properties;
  uint32_t groupSequence = 1234;
  properties.SetGroupSequence(groupSequence);
  EXPECT_EQ(properties.GetGroupSequence(), groupSequence);
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetMessageId)
{
  MessageProperties properties;
  std::string messageId = "1234";
  properties.SetMessageId(AmqpValue{messageId});
  EXPECT_EQ(properties.GetMessageId(), AmqpValue{messageId});
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetReplyTo)
{
  MessageProperties properties;
  std::string replyTo = "1234";
  properties.SetReplyTo(AmqpValue{replyTo});
  EXPECT_EQ(properties.GetReplyTo(), AmqpValue{replyTo});
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetReplyToGroupId)
{
  MessageProperties properties;
  std::string replyToGroupId = "1234";
  properties.SetReplyToGroupId(replyToGroupId);
  EXPECT_EQ(properties.GetReplyToGroupId(), replyToGroupId);
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetTo)
{
  MessageProperties properties;
  std::string to = "1234";
  properties.SetTo(AmqpValue{to});
  EXPECT_EQ(properties.GetTo(), AmqpValue{to});
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetUserId)
{
  MessageProperties properties;
  properties.SetUserId({'1', '2', '3', '4', '\0'});
  EXPECT_EQ(properties.GetUserId().size(), 5);
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetSubject)
{
  MessageProperties properties;
  std::string subject = "1234";
  properties.SetSubject(subject);
  EXPECT_EQ(properties.GetSubject(), subject);
  GTEST_LOG_(INFO) << properties;
}
