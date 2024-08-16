// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/models/private/message_impl.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"

#include <gtest/gtest.h>

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
    AmqpMessage nullMessage(nullptr);
    EXPECT_FALSE(nullMessage);

    auto nativeMessage = _detail::AmqpMessageFactory::ToUamqp(nullMessage);
    auto round_trip_message = _detail::AmqpMessageFactory::FromUamqp(nativeMessage.get());
    EXPECT_EQ(nullMessage, *round_trip_message.get());
  }

  {
    AmqpMessage message1;
    message1.Properties.MessageId = 12345;
    message1.SetBody("Hello world");
    AmqpMessage message2(std::move(message1));
    AmqpMessage message3(message2);
    AmqpMessage message4;
    message4 = message2;
    EXPECT_EQ(message4, message2);
    GTEST_LOG_(INFO) << message4;
    AmqpMessage message5 = std::move(message3);
    GTEST_LOG_(INFO) << message5;
    EXPECT_NE(message5, message3);
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

  auto message2{message};

  EXPECT_EQ(message2.ApplicationProperties["Blah"], AmqpValue(19532));

  GTEST_LOG_(INFO) << message;

  auto nativeMessage = _detail::AmqpMessageFactory::ToUamqp(message);
  auto round_trip_message = _detail::AmqpMessageFactory::FromUamqp(nativeMessage.get());
  EXPECT_EQ(message, *round_trip_message.get());
}

TEST_F(TestMessage, TestDeliveryAnnotations)
{
  AmqpMessage message;
  message.DeliveryAnnotations["12345"] = 19532;

  auto message2(message);
  EXPECT_EQ(AmqpValue{19532}, message2.DeliveryAnnotations["12345"]);
  GTEST_LOG_(INFO) << message;

  auto nativeMessage = _detail::AmqpMessageFactory::ToUamqp(message);
  auto round_trip_message = _detail::AmqpMessageFactory::FromUamqp(nativeMessage.get());
  EXPECT_EQ(message, *round_trip_message.get());
}

TEST_F(TestMessage, TestAnnotations)
{
  AmqpMessage message;
  message.MessageAnnotations["12345"] = 19532;

  auto message2(message);
  EXPECT_EQ(AmqpValue{19532}, message2.MessageAnnotations["12345"]);
  GTEST_LOG_(INFO) << message;
  auto nativeMessage = _detail::AmqpMessageFactory::ToUamqp(message);
  auto round_trip_message = _detail::AmqpMessageFactory::FromUamqp(nativeMessage.get());
  EXPECT_EQ(message, *round_trip_message.get());
}

TEST_F(TestMessage, TestFooter)
{
  AmqpMessage message;
  message.Footer["12345"] = 37.2;

  auto message2(message);
  EXPECT_EQ(AmqpValue{37.2}, message2.Footer["12345"]);

  GTEST_LOG_(INFO) << message;
  auto nativeMessage = _detail::AmqpMessageFactory::ToUamqp(message);
  auto round_trip_message = _detail::AmqpMessageFactory::FromUamqp(nativeMessage.get());
  EXPECT_EQ(message, *round_trip_message.get());
}

TEST_F(TestMessage, TestHeader)
{
  AmqpMessage message;
  message.Header.DeliveryCount = 1;

  std::shared_ptr<AmqpMessage> message2(std::make_shared<AmqpMessage>(message));

  // Ensure that message values survive across round-trips through MESSAGE.
  EXPECT_EQ(message2->Header.DeliveryCount, 1);
  GTEST_LOG_(INFO) << message;
  auto nativeMessage = _detail::AmqpMessageFactory::ToUamqp(message);
  auto round_trip_message = _detail::AmqpMessageFactory::FromUamqp(nativeMessage.get());
  EXPECT_EQ(message, *round_trip_message.get());
}

TEST_F(TestMessage, TestProperties)
{
  AmqpMessage message;
  MessageProperties properties;
  properties.Subject = "Message subject.";
  message.Properties = properties;

  std::shared_ptr<AmqpMessage> message2(std::make_shared<AmqpMessage>(message));

  auto newProperties{message2->Properties};
  EXPECT_EQ(newProperties.Subject.Value(), properties.Subject.Value());
  GTEST_LOG_(INFO) << message;

  auto nativeMessage = _detail::AmqpMessageFactory::ToUamqp(message);
  auto round_trip_message = _detail::AmqpMessageFactory::FromUamqp(nativeMessage.get());
  EXPECT_EQ(message, *round_trip_message.get());
}

TEST_F(TestMessage, TestBodyAmqpSequence)
{
  {
    AmqpMessage message;

    message.SetBody({"Test", 95, AmqpMap{{3, 5}, {4, 9}}.AsAmqpValue()});

    EXPECT_EQ(1, message.GetBodyAsAmqpList().size());
    EXPECT_EQ("Test", static_cast<std::string>(message.GetBodyAsAmqpList()[0].at(0)));
    EXPECT_EQ(95, static_cast<int32_t>(message.GetBodyAsAmqpList()[0].at(1)));
    EXPECT_EQ(message.BodyType, MessageBodyType::Sequence);

    std::shared_ptr<AmqpMessage> message2(std::make_shared<AmqpMessage>(message));
    EXPECT_EQ(1, message2->GetBodyAsAmqpList().size());
    EXPECT_EQ(message, *message2);
    EXPECT_EQ("Test", static_cast<std::string>(message2->GetBodyAsAmqpList()[0].at(0)));
    EXPECT_EQ(95, static_cast<int32_t>(message2->GetBodyAsAmqpList()[0].at(1)));
    EXPECT_EQ(message2->BodyType, MessageBodyType::Sequence);

    GTEST_LOG_(INFO) << message;

    auto nativeMessage = _detail::AmqpMessageFactory::ToUamqp(message);
    auto round_trip_message = _detail::AmqpMessageFactory::FromUamqp(nativeMessage.get());
    EXPECT_EQ(message, *round_trip_message.get());
  }
  {
    AmqpMessage message;
    message.SetBody({{1}, {"Test", 3}, {"Test", 95, AmqpMap{{3, 5}, {4, 9}}.AsAmqpValue()}});
    EXPECT_EQ(3, message.GetBodyAsAmqpList().size());
    EXPECT_EQ("Test", static_cast<std::string>(message.GetBodyAsAmqpList()[1].at(0)));
    EXPECT_EQ(95, static_cast<int32_t>(message.GetBodyAsAmqpList()[2].at(1)));
    EXPECT_EQ(message.BodyType, MessageBodyType::Sequence);
    std::shared_ptr<AmqpMessage> message2(std::make_shared<AmqpMessage>(message));
    EXPECT_EQ(3, message2->GetBodyAsAmqpList().size());
    EXPECT_EQ("Test", static_cast<std::string>(message2->GetBodyAsAmqpList()[2].at(0)));
    EXPECT_EQ(95, static_cast<int32_t>(message2->GetBodyAsAmqpList()[2].at(1)));
    EXPECT_EQ(message2->BodyType, MessageBodyType::Sequence);
    GTEST_LOG_(INFO) << message;

    auto nativeMessage = _detail::AmqpMessageFactory::ToUamqp(message);
    auto round_trip_message = _detail::AmqpMessageFactory::FromUamqp(nativeMessage.get());
    EXPECT_EQ(message, *round_trip_message.get());
  }
}

TEST_F(TestMessage, TestBodyAmqpData)
{
  AmqpMessage message;
  uint8_t testBody[] = "Test body";
  message.SetBody(AmqpBinaryData{'T', 'e', 's', 't', ' ', 'b', 'o', 'd', 'y', 0});
  EXPECT_EQ(message.GetBodyAsBinary().size(), 1);

  auto const& body = message.GetBodyAsBinary()[0];
  EXPECT_EQ(body.size(), sizeof(testBody));
  EXPECT_EQ(memcmp(body.data(), testBody, sizeof(testBody)), 0);

  EXPECT_EQ(message.BodyType, MessageBodyType::Data);

  std::shared_ptr<AmqpMessage> message2(std::make_shared<AmqpMessage>(message));
  EXPECT_EQ(message2->GetBodyAsBinary().size(), 1);

  auto const& body2 = message2->GetBodyAsBinary()[0];
  EXPECT_EQ(body2.size(), sizeof(testBody));
  EXPECT_EQ(memcmp(body2.data(), testBody, sizeof(testBody)), 0);

  EXPECT_EQ(message2->BodyType, MessageBodyType::Data);

  GTEST_LOG_(INFO) << message;

  message.SetBody({AmqpBinaryData{1, 3, 5, 7, 9, 10}, AmqpBinaryData{2, 4, 6, 8}});
  GTEST_LOG_(INFO) << message;

  auto nativeMessage = _detail::AmqpMessageFactory::ToUamqp(message);
  auto round_trip_message = _detail::AmqpMessageFactory::FromUamqp(nativeMessage.get());
  EXPECT_EQ(message, *round_trip_message.get());
}

class MessageSerialization : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(MessageSerialization, SerializeMessageBodyValue)
{ // Body as a single AMQP value.
  {
    std::vector<uint8_t> buffer;
    AmqpMessage message;
    message.Properties.MessageId = "12345";
    message.SetBody("String Value Body.");
    buffer = AmqpMessage::Serialize(message);

    AmqpMessage deserialized = AmqpMessage::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(message, deserialized);
  }
}
// Body as a single BinaryData value.
TEST_F(MessageSerialization, SerializeMessageBodyBinary)
{
  {
    std::vector<uint8_t> buffer;
    AmqpMessage message;
    message.Properties.MessageId = "12345";
    message.SetBody(AmqpBinaryData{'T', 'e', 's', 't', ' ', 'b', 'o', 'd', 'y', 0});
    buffer = AmqpMessage::Serialize(message);
    AmqpMessage deserialized = AmqpMessage::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(message, deserialized);
  }

  // Body as AMQP Value.
  {
    std::vector<uint8_t> buffer;
    AmqpMessage message;
    message.Properties.MessageId = "12345";
    message.SetBody(AmqpMap{{"key1", "value1"}, {"key2", "value2"}}.AsAmqpValue());
    buffer = AmqpMessage::Serialize(message);
    AmqpMessage deserialized = AmqpMessage::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(message, deserialized);
  }
  // Body as a vector of BinaryData values.
  {
    std::vector<uint8_t> buffer;
    AmqpMessage message;
    message.Properties.MessageId = "12345";
    message.SetBody(std::vector<AmqpBinaryData>{
        AmqpBinaryData{'T', 'e', 's', 't', ' ', 'b', 'o', 'd', 'y', 0},
        AmqpBinaryData{1, 3, 5, 7, 9, 10}});
    buffer = AmqpMessage::Serialize(message);
    AmqpMessage deserialized = AmqpMessage::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(message, deserialized);
  }
  // Body as a vector of BinaryData values, v2.
  {
    std::vector<uint8_t> buffer;
    AmqpMessage message;
    message.Properties.MessageId = "12345";
    message.SetBody(AmqpBinaryData{'T', 'e', 's', 't', ' ', 'b', 'o', 'd', 'y', 0});
    message.SetBody(AmqpBinaryData{1, 3, 5, 7, 9, 10});
    buffer = AmqpMessage::Serialize(message);
    AmqpMessage deserialized = AmqpMessage::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(deserialized.GetBodyAsBinary().size(), 2);
    EXPECT_EQ(message, deserialized);
  }
}

TEST_F(MessageSerialization, SerializeMessageBodySequence)
{
  // Body as a single AMQP Sequence.
  {
    std::vector<uint8_t> buffer;
    AmqpMessage message;
    message.Properties.MessageId = "12345";
    message.Properties.ContentType = "application/binary";
    message.Footer["footer1"] = "value1";
    message.SetBody(AmqpList{1, 3, 5, 7});
    buffer = AmqpMessage::Serialize(message);
    AmqpMessage deserialized = AmqpMessage::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(message, deserialized);
  }

  // Body as a vector of Amqp List values.
  {
    std::vector<uint8_t> buffer;
    AmqpMessage message;
    message.Properties.MessageId = "12345";
    message.SetBody(std::vector<AmqpList>{
        AmqpList{'T', 'e', 's', 't', ' ', 'b', 'o', 'd', 'y', 0}, AmqpList{1, 3, 5, 7, 9, 10}});
    buffer = AmqpMessage::Serialize(message);
    AmqpMessage deserialized = AmqpMessage::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(message, deserialized);
  }

  // Body as a vector of Amqp List values, added one at a time.
  {
    std::vector<uint8_t> buffer;
    AmqpMessage message;
    message.Properties.MessageId = "12345";
    message.SetBody(AmqpList{'T', 'e', 's', 't', ' ', 'b', 'o', 'd', 'y', 0});
    message.SetBody(AmqpList{1, 3, 5, 7, 9, 10});
    buffer = AmqpMessage::Serialize(message);
    AmqpMessage deserialized = AmqpMessage::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(message, deserialized);
  }
}

TEST_F(MessageSerialization, SerializeMessageWithHeader)
{ // Body as a single AMQP value, with message header..
  {
    std::vector<uint8_t> buffer;
    AmqpMessage message;
    message.Header.Priority = 5;
    message.Properties.MessageId = "12345";
    message.SetBody("String Value Body.");
    buffer = AmqpMessage::Serialize(message);

    AmqpMessage deserialized = AmqpMessage::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(message, deserialized);
  }
}

TEST_F(MessageSerialization, SerializeMessageWithDeliveryAnnotations)
{ // Body as a single AMQP value, with message header..
  {
    std::vector<uint8_t> buffer;
    AmqpMessage message;
    message.Header.Priority = 5;
    message.Properties.MessageId = "12345";
    message.DeliveryAnnotations["key1"] = "value1";
    message.DeliveryAnnotations["key2"] = "value2";

    message.SetBody("String Value Body.");
    buffer = AmqpMessage::Serialize(message);

    AmqpMessage deserialized = AmqpMessage::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(message, deserialized);
  }
}

TEST_F(MessageSerialization, SerializeMessageWithMessageAnnotations)
{ // Body as a single AMQP value, with message header..
  {
    std::vector<uint8_t> buffer;
    AmqpMessage message;
    message.Header.Priority = 5;
    message.Properties.MessageId = "12345";
    message.MessageAnnotations["key1"] = "value1";
    message.MessageAnnotations["key2"] = "value2";

    message.SetBody("String Value Body.");
    buffer = AmqpMessage::Serialize(message);

    AmqpMessage deserialized = AmqpMessage::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(message, deserialized);
  }
}

TEST_F(MessageSerialization, SerializeMessageWithApplicationProperties)
{ // Body as a single AMQP value, with message header..
  {
    std::vector<uint8_t> buffer;
    AmqpMessage message;
    message.Header.Priority = 5;
    message.Properties.MessageId = "12345";
    message.Properties.ContentEncoding = "utf-8";
    message.MessageAnnotations["key1"] = "value1";
    message.MessageAnnotations["key2"] = "value2";
    message.ApplicationProperties["key1"] = "value1";
    message.ApplicationProperties["key2"] = 37;

    message.SetBody("String Value Body.");
    buffer = AmqpMessage::Serialize(message);

    AmqpMessage deserialized = AmqpMessage::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(message, deserialized);
  }
}
TEST_F(MessageSerialization, SerializeMessageWithFooter)
{ // Body as a single AMQP value, with message header..
  {
    std::vector<uint8_t> buffer;
    AmqpMessage message;
    message.Header.Priority = 5;
    message.Properties.MessageId = "12345";
    message.Properties.ContentEncoding = "utf-8";
    message.Footer["footer1"] = "value1";
    message.Footer["footer2"] = 37;

    message.SetBody("String Value Body.");
    buffer = AmqpMessage::Serialize(message);

    AmqpMessage deserialized = AmqpMessage::Deserialize(buffer.data(), buffer.size());
    EXPECT_EQ(message, deserialized);
  }
}
