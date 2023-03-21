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
    Properties properties;
  }

  {
    Properties properties;
    EXPECT_ANY_THROW(
        properties
            .GetAbsoluteExpiryTime()); // Cannot get absolute expiry time before it has been set.
    EXPECT_ANY_THROW(
        properties.GetContentEncoding()); // Cannot get content encoding before it has been set.
    EXPECT_ANY_THROW(properties.GetContentType());
    EXPECT_ANY_THROW(properties.GetCorrelationId());
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
  // AMQP Properties represent times in milliseconds, so we need to reduce the accuracy of
  // std::chrono::system_clock::now to milliseconds before we check it.
  Properties properties;
  auto testTimestamp = std::chrono::system_clock::now();
  auto testTimestampMs{
      std::chrono::duration_cast<std::chrono::milliseconds>(testTimestamp.time_since_epoch())};
  std::chrono::system_clock::time_point testTimestampToCheck{
      std::chrono::duration_cast<std::chrono::system_clock::duration>(testTimestampMs)};

  // Set the test timestamp and verify that the returned value is accurate to milliseconds.
  properties.SetAbsoluteExpiryTime(testTimestamp);
  EXPECT_EQ(properties.GetAbsoluteExpiryTime(), testTimestampToCheck);
}

TEST_F(TestProperties, SetContentEncoding)
{
  Properties properties;
  std::string contentEncoding = "utf-8";
  properties.SetContentEncoding(contentEncoding);
  EXPECT_EQ(properties.GetContentEncoding(), contentEncoding);
}

TEST_F(TestProperties, SetContentType)
{
  Properties properties;
  std::string contentType = "text/plain";
  properties.SetContentType(contentType);
  EXPECT_EQ(properties.GetContentType(), contentType);
}

TEST_F(TestProperties, SetCorrelationId)
{
  Properties properties;
  std::string correlationId = "1234";
  properties.SetCorrelationId(correlationId);
  EXPECT_EQ(properties.GetCorrelationId(), correlationId);
}

TEST_F(TestProperties, SetCreationTime)
{
  Properties properties;
  auto testTimestamp = std::chrono::system_clock::now();
  auto testTimestampMs{
      std::chrono::duration_cast<std::chrono::milliseconds>(testTimestamp.time_since_epoch())};
  std::chrono::system_clock::time_point testTimestampToCheck{
      std::chrono::duration_cast<std::chrono::system_clock::duration>(testTimestampMs)};

  properties.SetCreationTime(testTimestamp);
  EXPECT_EQ(properties.GetCreationTime(), testTimestampToCheck);
}

TEST_F(TestProperties, SetGroupId)
{
  Properties properties;
  std::string groupId = "1234";
  properties.SetGroupId(groupId);
  EXPECT_EQ(properties.GetGroupId(), groupId);
}

TEST_F(TestProperties, SetGroupSequence)
{
  Properties properties;
  uint32_t groupSequence = 1234;
  properties.SetGroupSequence(groupSequence);
  EXPECT_EQ(properties.GetGroupSequence(), groupSequence);
}

TEST_F(TestProperties, SetMessageId)
{
  Properties properties;
  std::string messageId = "1234";
  properties.SetMessageId(messageId);
  EXPECT_EQ(properties.GetMessageId(), messageId);
}

TEST_F(TestProperties, SetReplyTo)
{
  Properties properties;
  std::string replyTo = "1234";
  properties.SetReplyTo(replyTo);
  EXPECT_EQ(properties.GetReplyTo(), replyTo);
}

TEST_F(TestProperties, SetReplyToGroupId)
{
  Properties properties;
  std::string replyToGroupId = "1234";
  properties.SetReplyToGroupId(replyToGroupId);
  EXPECT_EQ(properties.GetReplyToGroupId(), replyToGroupId);
}

TEST_F(TestProperties, SetTo)
{
  Properties properties;
  std::string to = "1234";
  properties.SetTo(to);
  EXPECT_EQ(properties.GetTo(), to);
}

TEST_F(TestProperties, SetUserId)
{
  Properties properties;
  BinaryData userId = {reinterpret_cast<const uint8_t*>("1234"), 5};
  properties.SetUserId(userId);
  EXPECT_EQ(properties.GetUserId().length, 5);
}

TEST_F(TestProperties, SetSubject)
{
  Properties properties;
  std::string subject = "1234";
  properties.SetSubject(subject);
  EXPECT_EQ(properties.GetSubject(), subject);
}
