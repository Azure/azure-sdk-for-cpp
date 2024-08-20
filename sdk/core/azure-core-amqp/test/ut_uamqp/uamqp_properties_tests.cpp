// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/models/private/properties_impl.hpp"
#include "azure/core/amqp/models/amqp_properties.hpp"

#include <gtest/gtest.h>

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
  properties.AbsoluteExpiryTime = testTimestamp;

  auto data = _detail::MessagePropertiesFactory::ToImplementation(properties);
  MessageProperties properties2(_detail::MessagePropertiesFactory::FromImplementation(data));

  EXPECT_EQ(properties2.AbsoluteExpiryTime.Value(), testTimestampToCheck);
  GTEST_LOG_(INFO) << properties;
  GTEST_LOG_(INFO) << properties2;
}

TEST_F(TestProperties, SetContentEncoding)
{
  MessageProperties properties;
  std::string contentEncoding = "utf-8";
  properties.ContentEncoding = contentEncoding;

  auto data = _detail::MessagePropertiesFactory::ToImplementation(properties);
  MessageProperties properties2(_detail::MessagePropertiesFactory::FromImplementation(data));

  EXPECT_EQ(properties2.ContentEncoding.Value(), contentEncoding);
  EXPECT_EQ(properties.ContentEncoding.Value(), properties2.ContentEncoding.Value());
  GTEST_LOG_(INFO) << properties;
  GTEST_LOG_(INFO) << properties2;
}

TEST_F(TestProperties, SetContentType)
{
  MessageProperties properties;
  std::string contentType = "text/plain";
  properties.ContentType = contentType;

  auto data = _detail::MessagePropertiesFactory::ToImplementation(properties);
  MessageProperties properties2(_detail::MessagePropertiesFactory::FromImplementation(data));

  EXPECT_EQ(properties2.ContentType.Value(), contentType);
  EXPECT_EQ(properties.ContentType.Value(), properties2.ContentType.Value());
  GTEST_LOG_(INFO) << properties;
  GTEST_LOG_(INFO) << properties2;
}

TEST_F(TestProperties, SetCorrelationId)
{
  MessageProperties properties;
  std::string correlationId = "1234";
  properties.CorrelationId = AmqpValue{correlationId};

  auto data = _detail::MessagePropertiesFactory::ToImplementation(properties);
  MessageProperties properties2(_detail::MessagePropertiesFactory::FromImplementation(data));

  EXPECT_EQ(properties2.CorrelationId.Value(), AmqpValue{correlationId});
  GTEST_LOG_(INFO) << properties;
  GTEST_LOG_(INFO) << properties2;
}

TEST_F(TestProperties, SetCreationTime)
{
  MessageProperties properties;
  auto testTimestamp = std::chrono::system_clock::now();
  auto testTimestampMs{
      std::chrono::duration_cast<std::chrono::milliseconds>(testTimestamp.time_since_epoch())};
  std::chrono::system_clock::time_point testTimestampToCheck{
      std::chrono::duration_cast<std::chrono::system_clock::duration>(testTimestampMs)};

  properties.CreationTime = testTimestamp;

  auto data = _detail::MessagePropertiesFactory::ToImplementation(properties);
  MessageProperties properties2(_detail::MessagePropertiesFactory::FromImplementation(data));

  EXPECT_EQ(properties2.CreationTime.Value(), testTimestampToCheck);

  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetGroupId)
{
  MessageProperties properties;
  std::string groupId = "1234";
  properties.GroupId = groupId;

  auto data = _detail::MessagePropertiesFactory::ToImplementation(properties);
  MessageProperties properties2(_detail::MessagePropertiesFactory::FromImplementation(data));

  EXPECT_EQ(properties.GroupId.Value(), groupId);
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetGroupSequence)
{
  MessageProperties properties;
  uint32_t groupSequence = 1234;
  properties.GroupSequence = groupSequence;

  auto data = _detail::MessagePropertiesFactory::ToImplementation(properties);
  MessageProperties properties2(_detail::MessagePropertiesFactory::FromImplementation(data));

  EXPECT_EQ(properties2.GroupSequence.Value(), groupSequence);
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetMessageId)
{
  MessageProperties properties;
  std::string messageId = "1234";
  properties.MessageId = AmqpValue{messageId};

  auto data = _detail::MessagePropertiesFactory::ToImplementation(properties);
  MessageProperties properties2(_detail::MessagePropertiesFactory::FromImplementation(data));

  EXPECT_EQ(properties2.MessageId.Value(), AmqpValue{messageId});
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetReplyTo)
{
  MessageProperties properties;
  std::string replyTo = "1234";
  properties.ReplyTo = AmqpValue{replyTo};

  auto data = _detail::MessagePropertiesFactory::ToImplementation(properties);
  MessageProperties properties2(_detail::MessagePropertiesFactory::FromImplementation(data));

  EXPECT_EQ(properties2.ReplyTo.Value(), AmqpValue{replyTo});
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetReplyToGroupId)
{
  MessageProperties properties;
  std::string replyToGroupId = "1234";
  properties.ReplyToGroupId = replyToGroupId;

  auto data = _detail::MessagePropertiesFactory::ToImplementation(properties);
  MessageProperties properties2(_detail::MessagePropertiesFactory::FromImplementation(data));

  EXPECT_EQ(properties2.ReplyToGroupId.Value(), replyToGroupId);
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetTo)
{
  MessageProperties properties;
  std::string to = "1234";
  properties.To = AmqpValue{to};
  auto data = _detail::MessagePropertiesFactory::ToImplementation(properties);
  MessageProperties properties2(_detail::MessagePropertiesFactory::FromImplementation(data));
  EXPECT_EQ(properties2.To.Value(), AmqpValue{to});
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetUserId)
{
  MessageProperties properties;
  properties.UserId = {'1', '2', '3', '4', '\0'};
  auto data = _detail::MessagePropertiesFactory::ToImplementation(properties);
  MessageProperties properties2(_detail::MessagePropertiesFactory::FromImplementation(data));
  EXPECT_EQ(properties2.UserId.Value().size(), 5);
  GTEST_LOG_(INFO) << properties;
}

TEST_F(TestProperties, SetSubject)
{
  MessageProperties properties;
  std::string subject = "1234";
  properties.Subject = subject;
  auto data = _detail::MessagePropertiesFactory::ToImplementation(properties);
  MessageProperties properties2(_detail::MessagePropertiesFactory::FromImplementation(data));
  EXPECT_EQ(properties2.Subject.Value(), subject);
  GTEST_LOG_(INFO) << properties;
}
