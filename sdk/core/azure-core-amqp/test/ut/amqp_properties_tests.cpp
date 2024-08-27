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

    auto nativeProperties = _detail::MessagePropertiesFactory::ToImplementation(properties);
    auto round_trip_properties
        = _detail::MessagePropertiesFactory::FromImplementation(nativeProperties);
    EXPECT_EQ(properties, round_trip_properties);
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
  properties.AbsoluteExpiryTime = testTimestampToCheck;

  MessageProperties properties2{properties};

  EXPECT_EQ(properties2.AbsoluteExpiryTime.Value(), properties.AbsoluteExpiryTime.Value());
  GTEST_LOG_(INFO) << properties;
  GTEST_LOG_(INFO) << properties2;

  auto nativeProperties = _detail::MessagePropertiesFactory::ToImplementation(properties);
  auto round_trip_properties
      = _detail::MessagePropertiesFactory::FromImplementation(nativeProperties);
  EXPECT_EQ(properties, round_trip_properties);
}

TEST_F(TestProperties, SetContentEncoding)
{
  MessageProperties properties;
  std::string contentEncoding = "utf-8";
  properties.ContentEncoding = contentEncoding;

  MessageProperties properties2{properties};

  EXPECT_EQ(properties2.ContentEncoding.Value(), contentEncoding);
  EXPECT_EQ(properties.ContentEncoding.Value(), properties2.ContentEncoding.Value());
  GTEST_LOG_(INFO) << properties;
  GTEST_LOG_(INFO) << properties2;

  auto nativeProperties = _detail::MessagePropertiesFactory::ToImplementation(properties);
  auto round_trip_properties
      = _detail::MessagePropertiesFactory::FromImplementation(nativeProperties);
  EXPECT_EQ(properties, round_trip_properties);
}

TEST_F(TestProperties, SetContentType)
{
  MessageProperties properties;
  std::string contentType = "text/plain";
  properties.ContentType = contentType;

  MessageProperties properties2{properties};

  EXPECT_EQ(properties2.ContentType.Value(), contentType);
  EXPECT_EQ(properties.ContentType.Value(), properties2.ContentType.Value());
  GTEST_LOG_(INFO) << properties;
  GTEST_LOG_(INFO) << properties2;

  auto nativeProperties = _detail::MessagePropertiesFactory::ToImplementation(properties);
  auto round_trip_properties
      = _detail::MessagePropertiesFactory::FromImplementation(nativeProperties);
  EXPECT_EQ(properties, round_trip_properties);
}

TEST_F(TestProperties, SetCorrelationId)
{
  MessageProperties properties;
  std::string correlationId = "1234";
  properties.CorrelationId = AmqpValue{correlationId};

  MessageProperties properties2{properties};

  EXPECT_EQ(properties2.CorrelationId.Value(), AmqpValue{correlationId});
  GTEST_LOG_(INFO) << properties;
  GTEST_LOG_(INFO) << properties2;

  auto nativeProperties = _detail::MessagePropertiesFactory::ToImplementation(properties);
  auto round_trip_properties
      = _detail::MessagePropertiesFactory::FromImplementation(nativeProperties);
  EXPECT_EQ(properties, round_trip_properties);
}

TEST_F(TestProperties, SetCreationTime)
{
  MessageProperties properties;
  auto testTimestamp = std::chrono::system_clock::now();
  auto testTimestampMs{
      std::chrono::duration_cast<std::chrono::milliseconds>(testTimestamp.time_since_epoch())};
  std::chrono::system_clock::time_point testTimestampToCheck{
      std::chrono::duration_cast<std::chrono::system_clock::duration>(testTimestampMs)};

  properties.CreationTime = testTimestampToCheck;

  MessageProperties properties2{properties};

  EXPECT_EQ(properties2.CreationTime.Value(), properties.CreationTime.Value());

  GTEST_LOG_(INFO) << properties;

  auto nativeProperties = _detail::MessagePropertiesFactory::ToImplementation(properties);
  auto round_trip_properties
      = _detail::MessagePropertiesFactory::FromImplementation(nativeProperties);
  EXPECT_EQ(properties, round_trip_properties);
}

TEST_F(TestProperties, SetGroupId)
{
  MessageProperties properties;
  std::string groupId = "1234";
  properties.GroupId = groupId;

  MessageProperties properties2{properties};

  EXPECT_EQ(properties.GroupId.Value(), groupId);
  GTEST_LOG_(INFO) << properties;

  auto nativeProperties = _detail::MessagePropertiesFactory::ToImplementation(properties);
  auto round_trip_properties
      = _detail::MessagePropertiesFactory::FromImplementation(nativeProperties);
  EXPECT_EQ(properties, round_trip_properties);
}

TEST_F(TestProperties, SetGroupSequence)
{
  MessageProperties properties;
  uint32_t groupSequence = 1234;
  properties.GroupSequence = groupSequence;

  MessageProperties properties2{properties};

  EXPECT_EQ(properties2.GroupSequence.Value(), groupSequence);
  GTEST_LOG_(INFO) << properties;

  auto nativeProperties = _detail::MessagePropertiesFactory::ToImplementation(properties);
  auto round_trip_properties
      = _detail::MessagePropertiesFactory::FromImplementation(nativeProperties);
  EXPECT_EQ(properties, round_trip_properties);
}

TEST_F(TestProperties, SetMessageId)
{
  MessageProperties properties;
  std::string messageId = "1234";
  properties.MessageId = AmqpValue{messageId};

  MessageProperties properties2{properties};

  EXPECT_EQ(properties2.MessageId.Value(), AmqpValue{messageId});
  GTEST_LOG_(INFO) << properties;

  auto nativeProperties = _detail::MessagePropertiesFactory::ToImplementation(properties);
  auto round_trip_properties
      = _detail::MessagePropertiesFactory::FromImplementation(nativeProperties);
  EXPECT_EQ(properties, round_trip_properties);
}

TEST_F(TestProperties, SetReplyTo)
{
  MessageProperties properties;
  std::string replyTo = "1234";
  properties.ReplyTo = AmqpValue{replyTo};

  MessageProperties properties2{properties};

  EXPECT_EQ(properties2.ReplyTo.Value(), AmqpValue{replyTo});
  GTEST_LOG_(INFO) << properties;

  auto nativeProperties = _detail::MessagePropertiesFactory::ToImplementation(properties);
  auto round_trip_properties
      = _detail::MessagePropertiesFactory::FromImplementation(nativeProperties);
  EXPECT_EQ(properties, round_trip_properties);
}

TEST_F(TestProperties, SetReplyToGroupId)
{
  MessageProperties properties;
  std::string replyToGroupId = "1234";
  properties.ReplyToGroupId = replyToGroupId;

  MessageProperties properties2{properties};

  EXPECT_EQ(properties2.ReplyToGroupId.Value(), replyToGroupId);
  GTEST_LOG_(INFO) << properties;

  auto nativeProperties = _detail::MessagePropertiesFactory::ToImplementation(properties);
  auto round_trip_properties
      = _detail::MessagePropertiesFactory::FromImplementation(nativeProperties);
  EXPECT_EQ(properties, round_trip_properties);
}

TEST_F(TestProperties, SetTo)
{
  MessageProperties properties;
  std::string to = "1234";
  properties.To = AmqpValue{to};
  MessageProperties properties2{properties};
  EXPECT_EQ(properties2.To.Value(), AmqpValue{to});
  GTEST_LOG_(INFO) << properties;

  auto nativeProperties = _detail::MessagePropertiesFactory::ToImplementation(properties);
  auto round_trip_properties
      = _detail::MessagePropertiesFactory::FromImplementation(nativeProperties);
  EXPECT_EQ(properties, round_trip_properties);
}

TEST_F(TestProperties, SetUserId)
{
  MessageProperties properties;
  properties.UserId = {'1', '2', '3', '4', '\0'};
  MessageProperties properties2{properties};
  EXPECT_EQ(properties2.UserId.Value().size(), 5);
  GTEST_LOG_(INFO) << properties;

  auto nativeProperties = _detail::MessagePropertiesFactory::ToImplementation(properties);
  auto round_trip_properties
      = _detail::MessagePropertiesFactory::FromImplementation(nativeProperties);
  EXPECT_EQ(properties, round_trip_properties);
}

TEST_F(TestProperties, SetSubject)
{
  MessageProperties properties;
  std::string subject = "1234";
  properties.Subject = subject;
  MessageProperties properties2{properties};
  EXPECT_EQ(properties2.Subject.Value(), subject);
  GTEST_LOG_(INFO) << properties;

  auto nativeProperties = _detail::MessagePropertiesFactory::ToImplementation(properties);
  auto round_trip_properties
      = _detail::MessagePropertiesFactory::FromImplementation(nativeProperties);
  EXPECT_EQ(properties, round_trip_properties);
}

class PropertySerialization : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(PropertySerialization, SerializePropertyMessageId)
{
  {
    std::vector<uint8_t> buffer;
    MessageProperties properties;
    properties.MessageId = "MessageId1";
    buffer = MessageProperties::Serialize(properties);

    MessageProperties deserialized = MessageProperties::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(properties, deserialized);
    EXPECT_EQ(AmqpValue("MessageId1"), deserialized.MessageId.Value());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x73, // Descriptor is for a message properties
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-properties).
        0xc0, // List
        0x0d, // 2 bytes long.
        0x01, // 1 elements.
        0xa1, // String constructor
        0x0a, // String length.
        'M',
        'e',
        's',
        's',
        'a',
        'g',
        'e',
        'I',
        'd',
        '1'};

    MessageProperties deserialized
        = MessageProperties::Deserialize(testValue.data(), testValue.size());
    EXPECT_EQ(AmqpValue("MessageId1"), deserialized.MessageId.Value());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());

    auto reserialized = MessageProperties::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);
  }
}

TEST_F(PropertySerialization, SerializePropertyUserId)
{
  {
    std::vector<uint8_t> buffer;
    MessageProperties properties;
    properties.UserId = {1, 2, 3, 5, 7, 9};
    buffer = MessageProperties::Serialize(properties);

    MessageProperties deserialized = MessageProperties::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(properties, deserialized);
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_EQ(deserialized.UserId.Value()[0], 1);
    EXPECT_EQ(deserialized.UserId.Value()[5], 9);
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x73, // Descriptor is for a message properties
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-properties).
        0xc0, // List
        0x0a, // 10 bytes long.
        0x02, // 2 elements.
        0x40, // NIL (MessageId)
        0xa0, // Binary constructor length.
        0x06, // 6 bytes in the binary data.
        0x01,
        0x02,
        0x03,
        0x05,
        0x07,
        0x09};

    MessageProperties deserialized
        = MessageProperties::Deserialize(testValue.data(), testValue.size());
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_EQ(deserialized.UserId.Value()[0], 1);
    EXPECT_EQ(deserialized.UserId.Value()[5], 9);
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());

    auto reserialized = MessageProperties::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);

  }
}

TEST_F(PropertySerialization, SerializePropertyTo)
{
  {
    std::vector<uint8_t> buffer;
    MessageProperties properties;
    properties.To = AmqpValue("MessageTo");
    buffer = MessageProperties::Serialize(properties);

    MessageProperties deserialized = MessageProperties::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(properties, deserialized);
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    //    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_EQ(AmqpValue("MessageTo"), deserialized.To.Value());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x73, // Descriptor is for a message properties
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-properties).
        0xc0, // List
        0x0e, // 14 bytes long.
        0x03, // 3 elements.
        0x40, // NIL
        0x40, // NIL
        0xa1, // String constructor
        0x09, // String length.
        'M',
        'e',
        's',
        's',
        'a',
        'g',
        'e',
        'T',
        'o'};

    MessageProperties deserialized
        = MessageProperties::Deserialize(testValue.data(), testValue.size());
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    //    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_EQ(AmqpValue("MessageTo"), deserialized.To.Value());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());

    auto reserialized = MessageProperties::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);
  }
}

TEST_F(PropertySerialization, SerializePropertySubject)
{
  {
    std::vector<uint8_t> buffer;
    MessageProperties properties;
    properties.Subject = "Subject";
    buffer = MessageProperties::Serialize(properties);

    MessageProperties deserialized = MessageProperties::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(properties, deserialized);
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_EQ("Subject", deserialized.Subject.Value());
    //    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x73, // Descriptor is for a message properties
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-properties).
        0xc0, // List
        0x0d, // 15 bytes long.
        0x04, // 4 elements.
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0xa1, // String constructor
        0x07, // String length.
        'S',
        'u',
        'b',
        'j',
        'e',
        'c',
        't'};

    MessageProperties deserialized
        = MessageProperties::Deserialize(testValue.data(), testValue.size());
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_EQ("Subject", deserialized.Subject.Value());
    //    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());

    auto reserialized = MessageProperties::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);
  }
}

TEST_F(PropertySerialization, SerializePropertyReplyTo)
{
  {
    std::vector<uint8_t> buffer;
    MessageProperties properties;
    properties.ReplyTo = AmqpValue("ReplyTo");
    buffer = MessageProperties::Serialize(properties);

    MessageProperties deserialized = MessageProperties::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(properties, deserialized);
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_EQ(AmqpValue("ReplyTo"), deserialized.ReplyTo.Value());
    //    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x73, // Descriptor is for a message properties
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-properties).
        0xc0, // List
        0x0e, // 15 bytes long.
        0x05, // 5 elements.
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0xa1, // String constructor
        0x07, // String length.
        'R',
        'e',
        'p',
        'l',
        'y',
        'T',
        'o'};

    MessageProperties deserialized
        = MessageProperties::Deserialize(testValue.data(), testValue.size());
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_EQ(AmqpValue("ReplyTo"), deserialized.ReplyTo.Value());
    //    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());

    auto reserialized = MessageProperties::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);
  }
}

TEST_F(PropertySerialization, SerializePropertyCorrelationId)
{
  {
    std::vector<uint8_t> buffer;
    MessageProperties properties;
    properties.CorrelationId = AmqpValue("CorrelationId");
    buffer = MessageProperties::Serialize(properties);

    MessageProperties deserialized = MessageProperties::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(properties, deserialized);
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_EQ(AmqpValue("CorrelationId"), deserialized.CorrelationId.Value());

    // EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x73, // Descriptor is for a message properties
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-properties).
        0xc0, // List
        0x15, // 15 bytes long.
        0x06, // 6 elements.
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0xa1, // String constructor
        0x0d, // String length.
        'C',
        'o',
        'r',
        'r',
        'e',
        'l',
        'a',
        't',
        'i',
        'o',
        'n',
        'I',
        'd'};

    MessageProperties deserialized
        = MessageProperties::Deserialize(testValue.data(), testValue.size());
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_EQ(AmqpValue("CorrelationId"), deserialized.CorrelationId.Value());

    // EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());

    auto reserialized = MessageProperties::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);
  }
}

TEST_F(PropertySerialization, SerializePropertyContentType)
{
  {
    std::vector<uint8_t> buffer;
    MessageProperties properties;
    properties.ContentType = "Text/Plain";
    buffer = MessageProperties::Serialize(properties);

    MessageProperties deserialized = MessageProperties::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(properties, deserialized);
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    //    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_EQ("Text/Plain", deserialized.ContentType.Value());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x73, // Descriptor is for a message properties
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-properties).
        0xc0, // List
        0x13, // 13 bytes long.
        0x07, // 6 elements.
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0xa3, // Symbol constructor
        0x0a, // String length.
        'T',
        'e',
        'x',
        't',
        '/',
        'P',
        'l',
        'a',
        'i',
        'n'};

    MessageProperties deserialized
        = MessageProperties::Deserialize(testValue.data(), testValue.size());
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    //    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_EQ("Text/Plain", deserialized.ContentType.Value());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());

    auto reserialized = MessageProperties::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);
  }
}

TEST_F(PropertySerialization, SerializePropertyContentEncoding)
{
  {
    std::vector<uint8_t> buffer;
    MessageProperties properties;
    properties.ContentEncoding = "Utf-8";
    buffer = MessageProperties::Serialize(properties);

    MessageProperties deserialized = MessageProperties::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(properties, deserialized);
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    // EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_EQ("Utf-8", deserialized.ContentEncoding.Value());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x73, // Descriptor is for a message properties
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-properties).
        0xc0, // List
        0x0f, // 15 bytes long.
        0x08, // 8 elements.
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0xa3, // Symbol constructor
        0x05, // String length.
        'U',
        't',
        'f',
        '-',
        '8'};

    MessageProperties deserialized
        = MessageProperties::Deserialize(testValue.data(), testValue.size());
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    // EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_EQ("Utf-8", deserialized.ContentEncoding.Value());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());

    auto reserialized = MessageProperties::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);
  }
}

TEST_F(PropertySerialization, SerializePropertyAbsoluteExpiryTime)
{
  {
    std::vector<uint8_t> buffer;
    MessageProperties properties;
    properties.AbsoluteExpiryTime = std::chrono::system_clock::from_time_t(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(12345)).count());
    buffer = MessageProperties::Serialize(properties);

    MessageProperties deserialized = MessageProperties::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(properties, deserialized);
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    // EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_EQ(
        deserialized.AbsoluteExpiryTime.Value(),
        std::chrono::system_clock::from_time_t(
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(12345))
                .count()));
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x73, // Descriptor is for a message properties
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-properties).
        0xc0, // List
        0x12, // 0x12 bytes long.
        0x09, // 8 elements.
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x83, // Timestamp constructor
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x2e,
        0xe0};

    MessageProperties deserialized
        = MessageProperties::Deserialize(testValue.data(), testValue.size());
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    // EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_EQ(
        deserialized.AbsoluteExpiryTime.Value(),
        std::chrono::system_clock::from_time_t(
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(12345))
                .count()));
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());

    auto reserialized = MessageProperties::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);
  }
}

TEST_F(PropertySerialization, SerializePropertyCreationTime)
{
  {
    std::vector<uint8_t> buffer;
    MessageProperties properties;
    properties.CreationTime = std::chrono::system_clock::from_time_t(
        std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(12345)).count());
    buffer = MessageProperties::Serialize(properties);

    MessageProperties deserialized = MessageProperties::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(properties, deserialized);
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    // EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_EQ(
        deserialized.CreationTime.Value(),
        std::chrono::system_clock::from_time_t(
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(12345))
                .count()));
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x73, // Descriptor is for a message properties
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-properties).
        0xc0, // List
        0x13, // 0x12 bytes long.
        0x0a, // 8 elements.
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x83, // Timestamp constructor
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x2e,
        0xe0};

    MessageProperties deserialized
        = MessageProperties::Deserialize(testValue.data(), testValue.size());
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    // EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_EQ(
        deserialized.CreationTime.Value(),
        std::chrono::system_clock::from_time_t(
            std::chrono::duration_cast<std::chrono::seconds>(std::chrono::milliseconds(12345))
                .count()));
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());

    auto reserialized = MessageProperties::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);
  }
}

TEST_F(PropertySerialization, SerializePropertyGroupdId)
{
  {
    std::vector<uint8_t> buffer;
    MessageProperties properties;
    properties.GroupId = "GroupId";
    buffer = MessageProperties::Serialize(properties);

    MessageProperties deserialized = MessageProperties::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(properties, deserialized);
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    // EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_EQ("GroupId", deserialized.GroupId.Value());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x73, // Descriptor is for a message properties
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-properties).
        0xc0, // List
        0x14, // 0x14 bytes long.
        0x0b, // 11 elements.
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0xa1, // String constructor
        0x07,
        'G',
        'r',
        'o',
        'u',
        'p',
        'I',
        'd'};

    MessageProperties deserialized
        = MessageProperties::Deserialize(testValue.data(), testValue.size());
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    // EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_EQ("GroupId", deserialized.GroupId.Value());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());

    auto reserialized = MessageProperties::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);
  }
}

TEST_F(PropertySerialization, SerializePropertyGroupSequence)
{
  {
    std::vector<uint8_t> buffer;
    MessageProperties properties;
    properties.GroupSequence = 32767;
    buffer = MessageProperties::Serialize(properties);

    MessageProperties deserialized = MessageProperties::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(properties, deserialized);
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    // EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_EQ(32767, deserialized.GroupSequence.Value());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x73, // Descriptor is for a message properties
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-properties).
        0xc0, // List
        0x11, // 0x14 bytes long.
        0x0c, // 12 elements.
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x70, // int constructor
        0x00,
        0x00,
        0x7f,
        0xff};

    MessageProperties deserialized
        = MessageProperties::Deserialize(testValue.data(), testValue.size());
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    // EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    EXPECT_EQ(32767, deserialized.GroupSequence.Value());
    EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());

    
    auto reserialized = MessageProperties::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);


  }
}

TEST_F(PropertySerialization, SerializePropertyReplyToGroupId)
{
  {
    std::vector<uint8_t> buffer;
    MessageProperties properties;
    properties.ReplyToGroupId = "32767";
    buffer = MessageProperties::Serialize(properties);

    MessageProperties deserialized = MessageProperties::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(properties, deserialized);
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    // EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());
    EXPECT_EQ("32767", deserialized.ReplyToGroupId.Value());
  }
  {
    std::vector<uint8_t> testValue{
        0x00, // Descriptor follows.
        0x53, // Descriptor is small ulong.
        0x73, // Descriptor is for a message properties
              // (http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-properties).
        0xc0, // List
        0x14, // 0x14 bytes long.
        0x0d, // 13 elements.
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0x40, // NIL
        0xa1, // string constructor
        0x05,
        '3',
        '2',
        '7',
        '6',
        '7'};

    MessageProperties deserialized
        = MessageProperties::Deserialize(testValue.data(), testValue.size());
    EXPECT_FALSE(deserialized.MessageId.HasValue());
    EXPECT_FALSE(deserialized.UserId.HasValue());
    EXPECT_FALSE(deserialized.To.HasValue());
    EXPECT_FALSE(deserialized.Subject.HasValue());
    EXPECT_FALSE(deserialized.ReplyTo.HasValue());
    EXPECT_FALSE(deserialized.CorrelationId.HasValue());
    EXPECT_FALSE(deserialized.ContentType.HasValue());
    EXPECT_FALSE(deserialized.ContentEncoding.HasValue());
    EXPECT_FALSE(deserialized.AbsoluteExpiryTime.HasValue());
    EXPECT_FALSE(deserialized.CreationTime.HasValue());
    EXPECT_FALSE(deserialized.GroupId.HasValue());
    EXPECT_FALSE(deserialized.GroupSequence.HasValue());
    // EXPECT_FALSE(deserialized.ReplyToGroupId.HasValue());
    EXPECT_EQ("32767", deserialized.ReplyToGroupId.Value());

    auto reserialized = MessageProperties::Serialize(deserialized);
    EXPECT_EQ(reserialized, testValue);


  }
}
