// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/private/eventhubs_constants.hpp"
#include "azure/messaging/eventhubs.hpp"
#include "eventhubs_test_base.hpp"

#include <gtest/gtest.h>

using namespace Azure::Core::Amqp::Models;
using namespace Azure::Messaging::EventHubs::Models;

class EventDataTest : public EventHubsTestBase {
};

// Construct an EventData object and convert it to an AMQP message.
// Verify that the resulting AMQP Message has the expected body and data (empty).
TEST_F(EventDataTest, EventDataNew)
{
  Azure::Messaging::EventHubs::Models::EventData eventData;

  auto message{eventData.GetRawAmqpMessage()};

  EXPECT_EQ(0ul, message->ApplicationProperties.size());
  EXPECT_FALSE(message->Properties.ContentType.HasValue());
  EXPECT_TRUE(message->Properties.CorrelationId.IsNull());
  EXPECT_TRUE(message->Properties.MessageId.IsNull());

  {
    EventData newData;
    newData.ContentType = "application/xml";

    {
      EventData copyData{newData};
      EXPECT_EQ(copyData.ContentType.Value(), newData.ContentType.Value());
    }
    {
      EventData moveData{std::move(newData)};
      // The contents of newData should be moved to moveData. The state of newData is undefined.
      EXPECT_TRUE(moveData.ContentType.HasValue());
      EXPECT_EQ(moveData.ContentType.Value(), "application/xml");
    }
  }
  {
    EventData newData;
    newData.ContentType = "application/json";
    {
      EventData copyData;
      copyData = newData;
      EXPECT_EQ(copyData.ContentType.Value(), newData.ContentType.Value());
    }
    {
      EventData moveData;
      moveData = std::move(newData);
      EXPECT_TRUE(moveData.ContentType.HasValue());
    }
  }
}

TEST_F(EventDataTest, EventData1)
{
  Azure::Messaging::EventHubs::Models::EventData eventData;

  eventData.Body = {1, 2};
  eventData.ContentType = "ct";
  eventData.Properties.emplace("abc", AmqpValue(23));
  eventData.CorrelationId = AmqpValue("ci");
  eventData.MessageId = AmqpValue("mi");

  GTEST_LOG_(INFO) << "Message: " << eventData;

  auto message{eventData.GetRawAmqpMessage()};

  EXPECT_EQ(1ul, message->ApplicationProperties.size());
  EXPECT_EQ(eventData.Body, static_cast<std::vector<uint8_t>>(message->GetBodyAsBinary()[0]));
  EXPECT_EQ("ct", message->Properties.ContentType.Value());
  EXPECT_EQ(AmqpValue("ci"), message->Properties.CorrelationId);
  EXPECT_FALSE(message->Properties.MessageId.IsNull());

  Azure::Messaging::EventHubs::Models::ReceivedEventData receivedEventData(message);
  EXPECT_EQ(eventData.Body, receivedEventData.Body);
  EXPECT_EQ(eventData.ContentType.HasValue(), receivedEventData.ContentType.HasValue());
  if (eventData.ContentType.HasValue())
  {
    EXPECT_EQ(eventData.ContentType.Value(), receivedEventData.ContentType.Value());
  }
  EXPECT_EQ(eventData.Properties, receivedEventData.Properties);
  EXPECT_EQ(eventData.CorrelationId, receivedEventData.CorrelationId);
  EXPECT_EQ(eventData.MessageId, receivedEventData.MessageId);
  GTEST_LOG_(INFO) << "Received message: " << receivedEventData;
}

TEST_F(EventDataTest, EventDataStringBody)
{
  Azure::Messaging::EventHubs::Models::EventData eventData{"String Body Message."};

  auto message{eventData.GetRawAmqpMessage()};
  EXPECT_TRUE(message->Properties.MessageId.IsNull());
  EXPECT_EQ(message->BodyType, Azure::Core::Amqp::Models::MessageBodyType::Data);
  EXPECT_EQ(message->GetBodyAsBinary().size(), 1ul);
  EXPECT_EQ(
      message->GetBodyAsBinary()[0],
      std::vector<uint8_t>(eventData.Body.begin(), eventData.Body.end()));
}

TEST_F(EventDataTest, EventDataBodyTest)
{
  {
    Azure::Messaging::EventHubs::Models::EventData msg;

    // Note that Data is an AMQP BinaryData value.
    msg.Body = {1, 3, 5, 7, 9};

    auto message{msg.GetRawAmqpMessage()};

    EXPECT_EQ(message->GetBodyAsBinary().size(), 1ul);
    EXPECT_EQ(msg.Body, static_cast<std::vector<uint8_t>>(message->GetBodyAsBinary()[0]));
  }
}

TEST_F(EventDataTest, EventDataArrayBody)
{
  Azure::Messaging::EventHubs::Models::EventData eventData{1, 3, 5, 7, 9};

  auto message{eventData.GetRawAmqpMessage()};
  EXPECT_TRUE(message->Properties.MessageId.IsNull());
  EXPECT_EQ(message->BodyType, Azure::Core::Amqp::Models::MessageBodyType::Data);
  EXPECT_EQ(message->GetBodyAsBinary().size(), 1ul);
  EXPECT_EQ(
      message->GetBodyAsBinary()[0],
      std::vector<uint8_t>(eventData.Body.begin(), eventData.Body.end()));
}

TEST_F(EventDataTest, EventDataVectorBody)
{
  std::vector<uint8_t> vector{2, 4, 6, 8, 10};
  Azure::Messaging::EventHubs::Models::EventData eventData{vector};

  auto message{eventData.GetRawAmqpMessage()};
  EXPECT_TRUE(message->Properties.MessageId.IsNull());
  EXPECT_EQ(message->BodyType, Azure::Core::Amqp::Models::MessageBodyType::Data);
  EXPECT_EQ(message->GetBodyAsBinary().size(), 1ul);
  EXPECT_EQ(
      message->GetBodyAsBinary()[0],
      std::vector<uint8_t>(eventData.Body.begin(), eventData.Body.end()));
}

TEST_F(EventDataTest, ReceivedEventData)
{
  {
    std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> message{
        std::make_shared<Azure::Core::Amqp::Models::AmqpMessage>()};
    message->MessageAnnotations[Azure::Core::Amqp::Models::AmqpSymbol{
        Azure::Messaging::EventHubs::_detail::PartitionKeyAnnotation}
                                    .AsAmqpValue()]
        = "PartitionKey";
    Azure::Messaging::EventHubs::Models::ReceivedEventData receivedEventData(message);
    ASSERT_TRUE(receivedEventData.PartitionKey);
    EXPECT_EQ(receivedEventData.PartitionKey.Value(), "PartitionKey");
    EXPECT_FALSE(receivedEventData.EnqueuedTime);
    EXPECT_FALSE(receivedEventData.Offset);
    EXPECT_FALSE(receivedEventData.SequenceNumber);
  }
  {
    std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> message{
        std::make_shared<Azure::Core::Amqp::Models::AmqpMessage>()};

    Azure::DateTime timeNow{
        std::chrono::time_point_cast<std::chrono::milliseconds>(Azure::DateTime::clock::now())};

    GTEST_LOG_(INFO) << "timeNow: " << timeNow.ToString();

    message->MessageAnnotations[Azure::Core::Amqp::Models::AmqpSymbol{
        Azure::Messaging::EventHubs::_detail::EnqueuedTimeAnnotation}
                                    .AsAmqpValue()]
        = Azure::Core::Amqp::Models::AmqpTimestamp{std::chrono::duration_cast<
                                                       std::chrono::milliseconds>(
                                                       timeNow.time_since_epoch())}
              .AsAmqpValue();
    Azure::Messaging::EventHubs::Models::ReceivedEventData receivedEventData(message);
    ASSERT_TRUE(receivedEventData.EnqueuedTime.HasValue());
    GTEST_LOG_(INFO) << "EnqueuedTime: " << receivedEventData.EnqueuedTime.Value().ToString();
    EXPECT_EQ(receivedEventData.EnqueuedTime.Value(), timeNow);
    EXPECT_FALSE(receivedEventData.PartitionKey);
    EXPECT_FALSE(receivedEventData.Offset);
    EXPECT_FALSE(receivedEventData.SequenceNumber);
  }

  {
    std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> message{
        std::make_shared<Azure::Core::Amqp::Models::AmqpMessage>()};
    message->MessageAnnotations[Azure::Core::Amqp::Models::AmqpSymbol{
        Azure::Messaging::EventHubs::_detail::SequenceNumberAnnotation}
                                    .AsAmqpValue()]
        = static_cast<int64_t>(235);
    Azure::Messaging::EventHubs::Models::ReceivedEventData receivedEventData(message);
    ASSERT_TRUE(receivedEventData.SequenceNumber);
    EXPECT_EQ(receivedEventData.SequenceNumber.Value(), 235);
    EXPECT_FALSE(receivedEventData.EnqueuedTime);
    EXPECT_FALSE(receivedEventData.PartitionKey);
    EXPECT_FALSE(receivedEventData.Offset);
  }
  {
    std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> message{
        std::make_shared<Azure::Core::Amqp::Models::AmqpMessage>()};
    message->MessageAnnotations[Azure::Core::Amqp::Models::AmqpSymbol{
        Azure::Messaging::EventHubs::_detail::OffsetAnnotation}
                                    .AsAmqpValue()]
        = 54644;
    Azure::Messaging::EventHubs::Models::ReceivedEventData receivedEventData(message);
    ASSERT_FALSE(receivedEventData.Offset); // Offset must be a string value, not a numeric value.
    EXPECT_FALSE(receivedEventData.SequenceNumber);
    EXPECT_FALSE(receivedEventData.EnqueuedTime);
    EXPECT_FALSE(receivedEventData.PartitionKey);
  }
  {
    std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> message{
        std::make_shared<Azure::Core::Amqp::Models::AmqpMessage>()};
    message->MessageAnnotations[Azure::Core::Amqp::Models::AmqpSymbol{
        Azure::Messaging::EventHubs::_detail::OffsetAnnotation}
                                    .AsAmqpValue()]
        = "54644";
    Azure::Messaging::EventHubs::Models::ReceivedEventData receivedEventData(message);
    ASSERT_TRUE(receivedEventData.Offset);
    EXPECT_EQ(receivedEventData.Offset.Value(), "54644");
    EXPECT_FALSE(receivedEventData.SequenceNumber);
    EXPECT_FALSE(receivedEventData.EnqueuedTime);
    EXPECT_FALSE(receivedEventData.PartitionKey);
  }
  {
    std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> message{
        std::make_shared<Azure::Core::Amqp::Models::AmqpMessage>()};
    message->MessageAnnotations[Azure::Core::Amqp::Models::AmqpSymbol{
        Azure::Messaging::EventHubs::_detail::OffsetAnnotation}
                                    .AsAmqpValue()]
        = "53";
    Azure::Messaging::EventHubs::Models::ReceivedEventData receivedEventData(message);
    ASSERT_TRUE(receivedEventData.Offset);
    EXPECT_EQ(receivedEventData.Offset.Value(), "53");
    EXPECT_FALSE(receivedEventData.SequenceNumber);
    EXPECT_FALSE(receivedEventData.EnqueuedTime);
    EXPECT_FALSE(receivedEventData.PartitionKey);
  }
}