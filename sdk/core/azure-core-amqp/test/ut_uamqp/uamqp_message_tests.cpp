// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/models/private/message_impl.hpp"
#include "../src/models/private/value_impl.hpp"
#include "azure/core/amqp/internal/models/amqp_protocol.hpp"
#include "azure/core/amqp/models/amqp_message.hpp"

#include <azure_uamqp_c/amqp_definitions_annotations.h>
#include <azure_uamqp_c/amqp_definitions_message_annotations.h>
#include <azure_uamqp_c/amqpvalue.h>
#include <azure_uamqp_c/message.h>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

using namespace Azure::Core::Amqp::Models;

class TestMessageAmqp : public testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(TestMessageAmqp, SimpleCreate)
{
  {
    AmqpMessage message;
  }

  {
    AmqpMessage nullMessage(nullptr);
    EXPECT_FALSE(nullMessage);
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
    auto messageInstance = _detail::AmqpMessageFactory::ToImplementation(message);
    AMQP_VALUE annotationsValue = nullptr;
    auto result = message_get_message_annotations(messageInstance.get(), &annotationsValue);
    _detail::UniqueAmqpValueHandle annotations{annotationsValue};
    ASSERT_EQ(0, result);
    EXPECT_EQ(nullptr, annotations.get());

    // By default, the body type is None, so retrieving the body as any other type should throw.
    EXPECT_EQ(MessageBodyType::None, message.BodyType);
    EXPECT_ANY_THROW(message.GetBodyAsAmqpList());
    EXPECT_ANY_THROW(message.GetBodyAsAmqpValue());
    EXPECT_ANY_THROW(message.GetBodyAsBinary());
  }
}

TEST_F(TestMessageAmqp, TestApplicationProperties)
{
  AmqpMessage message;

  // Ensure that ApplicationProperties values round-trip through uAMQP value serialization.
  message.ApplicationProperties["Blah"] = 19532;

  auto messageInstance = _detail::AmqpMessageFactory::ToImplementation(message);
  auto message2(_detail::AmqpMessageFactory::FromImplementation(messageInstance.get()));

  EXPECT_EQ(message2->ApplicationProperties["Blah"], AmqpValue(19532));

  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessageAmqp, TestDeliveryAnnotations)
{
  AmqpMessage message;
  message.DeliveryAnnotations["12345"] = 19532;

  auto messageInstance = _detail::AmqpMessageFactory::ToImplementation(message);
  auto message2(_detail::AmqpMessageFactory::FromImplementation(messageInstance.get()));
  EXPECT_EQ(AmqpValue{19532}, message2->DeliveryAnnotations["12345"]);
  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessageAmqp, TestAnnotations)
{
  for (bool hasApplicationProperties : {false, true})
  {
    SCOPED_TRACE(testing::Message() << "hasApplicationProperties=" << hasApplicationProperties);
    AmqpMessage message;
    message.MessageAnnotations["12345"] = 19532;
    message.MessageAnnotations[AmqpSymbol{"x-opt-partition-key"}] = "stress-key-0";
    if (hasApplicationProperties)
    {
      message.ApplicationProperties["example"] = "value";
    }

    auto messageInstance = _detail::AmqpMessageFactory::ToImplementation(message);
    AMQP_VALUE annotationsValue = nullptr;
    auto result = message_get_message_annotations(messageInstance.get(), &annotationsValue);
    _detail::UniqueAmqpValueHandle annotations{annotationsValue};
    ASSERT_EQ(0, result);
    ASSERT_NE(nullptr, annotations.get());

    std::vector<uint8_t> encoded;
    auto appendBytes = [](void* context, unsigned char const* bytes, size_t size) {
      auto& output = *static_cast<std::vector<uint8_t>*>(context);
      output.insert(output.end(), bytes, bytes + size);
      return 0;
    };
    ASSERT_EQ(0, amqpvalue_encode(annotations.get(), appendBytes, &encoded));
    ASSERT_GE(encoded.size(), 4U);
    std::vector<uint8_t> const expectedPrefix{0x00, 0x53, 0x72, 0xC1};
    EXPECT_EQ(expectedPrefix, std::vector<uint8_t>(encoded.begin(), encoded.begin() + 4));

    auto message2(_detail::AmqpMessageFactory::FromImplementation(messageInstance.get()));
    EXPECT_EQ(message.MessageAnnotations.AsAmqpValue(), message2->MessageAnnotations.AsAmqpValue());
    EXPECT_EQ(message.ApplicationProperties, message2->ApplicationProperties);
    GTEST_LOG_(INFO) << message;
  }
}

TEST_F(TestMessageAmqp, TestDescribedMessageAnnotations)
{
  AmqpAnnotations expected{{AmqpSymbol{"x-opt-partition-key"}, "stress-key-0"}};
  _detail::UniqueMessageHandle messageInstance{message_create()};
  ASSERT_NE(nullptr, messageInstance.get());
  _detail::UniqueAmqpValueHandle annotations{amqpvalue_create_message_annotations(
      _detail::AmqpValueFactory::ToImplementation(expected.AsAmqpValue()))};
  ASSERT_NE(nullptr, annotations.get());
  ASSERT_EQ(0, message_set_message_annotations(messageInstance.get(), annotations.get()));

  std::shared_ptr<AmqpMessage> message;
  ASSERT_NO_THROW(message = _detail::AmqpMessageFactory::FromImplementation(messageInstance.get()));
  ASSERT_NE(nullptr, message);
  EXPECT_EQ(expected.AsAmqpValue(), message->MessageAnnotations.AsAmqpValue());
}

TEST_F(TestMessageAmqp, TestRawMessageAnnotations)
{
  AmqpAnnotations expected{{AmqpSymbol{"x-opt-partition-key"}, "stress-key-0"}};
  _detail::UniqueMessageHandle messageInstance{message_create()};
  ASSERT_NE(nullptr, messageInstance.get());
  ASSERT_EQ(
      0,
      message_set_message_annotations(
          messageInstance.get(),
          _detail::AmqpValueFactory::ToImplementation(expected.AsAmqpValue())));

  std::shared_ptr<AmqpMessage> message;
  ASSERT_NO_THROW(message = _detail::AmqpMessageFactory::FromImplementation(messageInstance.get()));
  ASSERT_NE(nullptr, message);
  EXPECT_EQ(expected.AsAmqpValue(), message->MessageAnnotations.AsAmqpValue());
}

TEST_F(TestMessageAmqp, TestUnexpectedMessageAnnotationsDescriptor)
{
  AmqpAnnotations expected{{AmqpSymbol{"x-opt-partition-key"}, "stress-key-0"}};
  _detail::UniqueMessageHandle messageInstance{message_create()};
  ASSERT_NE(nullptr, messageInstance.get());
  AmqpDescribed annotations(
      static_cast<uint64_t>(Azure::Core::Amqp::_detail::AmqpDescriptors::DeliveryAnnotations),
      expected.AsAmqpValue());
  ASSERT_EQ(
      0,
      message_set_message_annotations(
          messageInstance.get(),
          _detail::AmqpValueFactory::ToImplementation(annotations.AsAmqpValue())));

  EXPECT_THROW(
      _detail::AmqpMessageFactory::FromImplementation(messageInstance.get()), std::runtime_error);
}

TEST_F(TestMessageAmqp, TestFooter)
{
  AmqpMessage message;
  message.Footer["12345"] = 37.2;

  auto messageInstance = _detail::AmqpMessageFactory::ToImplementation(message);
  std::shared_ptr<AmqpMessage> message2(
      _detail::AmqpMessageFactory::FromImplementation(messageInstance.get()));
  EXPECT_EQ(AmqpValue{37.2}, message2->Footer["12345"]);

  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessageAmqp, TestHeader)
{
  AmqpMessage message;
  message.Header.DeliveryCount = 1;

  auto messageInstance = _detail::AmqpMessageFactory::ToImplementation(message);
  std::shared_ptr<AmqpMessage> message2(
      _detail::AmqpMessageFactory::FromImplementation(messageInstance.get()));

  // Ensure that message values survive across round-trips through MESSAGE.
  EXPECT_EQ(message2->Header.DeliveryCount, 1);
  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessageAmqp, TestProperties)
{
  AmqpMessage message;
  MessageProperties properties;
  properties.Subject = "Message subject.";
  message.Properties = properties;

  auto messageInstance = _detail::AmqpMessageFactory::ToImplementation(message);
  std::shared_ptr<AmqpMessage> message2(
      _detail::AmqpMessageFactory::FromImplementation(messageInstance.get()));

  auto newProperties{message2->Properties};
  EXPECT_EQ(newProperties.Subject.Value(), properties.Subject.Value());
  GTEST_LOG_(INFO) << message;
}

TEST_F(TestMessageAmqp, TestBodyAmqpSequence)
{
  {
    AmqpMessage message;

    message.SetBody({"Test", 95, AmqpMap{{3, 5}, {4, 9}}.AsAmqpValue()});

    EXPECT_EQ(1, message.GetBodyAsAmqpList().size());
    EXPECT_EQ("Test", static_cast<std::string>(message.GetBodyAsAmqpList()[0].at(0)));
    EXPECT_EQ(95, static_cast<int32_t>(message.GetBodyAsAmqpList()[0].at(1)));
    EXPECT_EQ(message.BodyType, MessageBodyType::Sequence);

    auto messageInstance = _detail::AmqpMessageFactory::ToImplementation(message);
    std::shared_ptr<AmqpMessage> message2(
        _detail::AmqpMessageFactory::FromImplementation(messageInstance.get()));
    EXPECT_EQ(1, message2->GetBodyAsAmqpList().size());
    EXPECT_EQ(message, *message2);
    EXPECT_EQ("Test", static_cast<std::string>(message2->GetBodyAsAmqpList()[0].at(0)));
    EXPECT_EQ(95, static_cast<int32_t>(message2->GetBodyAsAmqpList()[0].at(1)));
    EXPECT_EQ(message2->BodyType, MessageBodyType::Sequence);

    GTEST_LOG_(INFO) << message;
  }
  {
    AmqpMessage message;
    message.SetBody({{1}, {"Test", 3}, {"Test", 95, AmqpMap{{3, 5}, {4, 9}}.AsAmqpValue()}});
    EXPECT_EQ(3, message.GetBodyAsAmqpList().size());
    EXPECT_EQ("Test", static_cast<std::string>(message.GetBodyAsAmqpList()[1].at(0)));
    EXPECT_EQ(95, static_cast<int32_t>(message.GetBodyAsAmqpList()[2].at(1)));
    EXPECT_EQ(message.BodyType, MessageBodyType::Sequence);
    auto messageInstance = _detail::AmqpMessageFactory::ToImplementation(message);
    std::shared_ptr<AmqpMessage> message2(
        _detail::AmqpMessageFactory::FromImplementation(messageInstance.get()));
    EXPECT_EQ(3, message2->GetBodyAsAmqpList().size());
    EXPECT_EQ("Test", static_cast<std::string>(message2->GetBodyAsAmqpList()[2].at(0)));
    EXPECT_EQ(95, static_cast<int32_t>(message2->GetBodyAsAmqpList()[2].at(1)));
    EXPECT_EQ(message2->BodyType, MessageBodyType::Sequence);
    GTEST_LOG_(INFO) << message;
  }
}

TEST_F(TestMessageAmqp, TestBodyAmqpData)
{
  AmqpMessage message;
  uint8_t testBody[] = "Test body";
  message.SetBody(AmqpBinaryData{'T', 'e', 's', 't', ' ', 'b', 'o', 'd', 'y', 0});
  EXPECT_EQ(message.GetBodyAsBinary().size(), 1);

  auto const& body = message.GetBodyAsBinary()[0];
  EXPECT_EQ(body.size(), sizeof(testBody));
  EXPECT_EQ(memcmp(body.data(), testBody, sizeof(testBody)), 0);

  EXPECT_EQ(message.BodyType, MessageBodyType::Data);

  auto messageInstance = _detail::AmqpMessageFactory::ToImplementation(message);
  std::shared_ptr<AmqpMessage> message2(
      _detail::AmqpMessageFactory::FromImplementation(messageInstance.get()));
  EXPECT_EQ(message2->GetBodyAsBinary().size(), 1);

  auto const& body2 = message2->GetBodyAsBinary()[0];
  EXPECT_EQ(body2.size(), sizeof(testBody));
  EXPECT_EQ(memcmp(body2.data(), testBody, sizeof(testBody)), 0);

  EXPECT_EQ(message2->BodyType, MessageBodyType::Data);

  GTEST_LOG_(INFO) << message;
}
