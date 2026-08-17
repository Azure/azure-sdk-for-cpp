// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/private/eventhubs_constants.hpp"
#include "../src/private/eventhubs_utilities.hpp"
#include "azure/messaging/eventhubs.hpp"
#include "eventhubs_test_base.hpp"

#include <limits>

#include <gtest/gtest.h>

using namespace Azure::Core::Amqp::Models;
using namespace Azure::Messaging::EventHubs::Models;

class EventDataTest : public EventHubsTestBase {
};

class EventDataBatchTest : public EventHubsTestBase {
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

namespace {
Azure::Nullable<std::string> OffsetFromAnnotation(
    Azure::Core::Amqp::Models::AmqpValue const& annotationValue)
{
  std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> message{
      std::make_shared<Azure::Core::Amqp::Models::AmqpMessage>()};
  message->MessageAnnotations[Azure::Core::Amqp::Models::AmqpSymbol{
      Azure::Messaging::EventHubs::_detail::OffsetAnnotation}
                                  .AsAmqpValue()]
      = annotationValue;
  Azure::Messaging::EventHubs::Models::ReceivedEventData receivedEventData(message);
  return receivedEventData.Offset;
}
} // namespace

// The service always sends the offset as a string, and every other Event Hubs client reads a
// string only. An integer of any width is a service contract break, so the offset stays empty.
TEST_F(EventDataTest, OffsetAnnotationAcceptsAStringOnly)
{
  {
    auto offset{OffsetFromAnnotation(Azure::Core::Amqp::Models::AmqpValue{"54644"})};
    ASSERT_TRUE(offset);
    EXPECT_EQ(offset.Value(), "54644");
  }
  {
    auto offset{OffsetFromAnnotation(Azure::Core::Amqp::Models::AmqpValue{"@latest"})};
    ASSERT_TRUE(offset);
    EXPECT_EQ(offset.Value(), "@latest");
  }

  using Azure::Core::Amqp::Models::AmqpValue;
  EXPECT_FALSE(OffsetFromAnnotation(AmqpValue{static_cast<std::uint8_t>(12)}));
  EXPECT_FALSE(OffsetFromAnnotation(AmqpValue{static_cast<std::uint16_t>(4096)}));
  EXPECT_FALSE(OffsetFromAnnotation(AmqpValue{static_cast<std::uint32_t>(4294967295u)}));
  EXPECT_FALSE(OffsetFromAnnotation(AmqpValue{(std::numeric_limits<std::uint64_t>::max)()}));
  EXPECT_FALSE(OffsetFromAnnotation(AmqpValue{static_cast<std::int8_t>(-12)}));
  EXPECT_FALSE(OffsetFromAnnotation(AmqpValue{static_cast<std::int16_t>(-4096)}));
  EXPECT_FALSE(OffsetFromAnnotation(AmqpValue{static_cast<std::int32_t>(-2147483648LL)}));
  EXPECT_FALSE(OffsetFromAnnotation(AmqpValue{(std::numeric_limits<std::int64_t>::max)()}));
}

TEST_F(EventDataTest, OffsetAnnotationIgnoresAnUnexpectedType)
{
  Azure::Nullable<std::string> offset;
  EXPECT_NO_THROW(offset = OffsetFromAnnotation(Azure::Core::Amqp::Models::AmqpValue{true}));
  EXPECT_FALSE(offset);
}

// The Event Hubs service routes on the message annotations only. Make sure that the batch envelope
// and every message in the batch carry the partition key there, and that the delivery annotations
// stay empty.
TEST_F(EventDataBatchTest, PartitionKeyIsInMessageAnnotations)
{
  constexpr const char* partitionKey = "test-partition-key";

  Azure::Messaging::EventHubs::EventDataBatchOptions options;
  options.MaxBytes = static_cast<std::uint64_t>((std::numeric_limits<uint16_t>::max)());
  options.PartitionKey = partitionKey;

  Azure::Messaging::EventHubs::EventDataBatch batch{
      Azure::Messaging::EventHubs::_detail::EventDataBatchFactory::CreateEventDataBatch(options)};

  EXPECT_TRUE(batch.TryAdd(EventData{"First message."}));
  EXPECT_TRUE(batch.TryAdd(EventData{"Second message."}));

  auto batchMessage{batch.ToAmqpMessage()};

  AmqpSymbol const partitionKeyAnnotation{
      Azure::Messaging::EventHubs::_detail::PartitionKeyAnnotation};

  auto envelopeAnnotation = batchMessage.MessageAnnotations.find(partitionKeyAnnotation);
  ASSERT_NE(envelopeAnnotation, batchMessage.MessageAnnotations.end())
      << "The batch envelope has no partition key message annotation.";
  EXPECT_EQ(partitionKey, static_cast<std::string>(envelopeAnnotation->second));

  // Delivery annotations stop at the first hop, so the partition key must not be there.
  EXPECT_EQ(
      batchMessage.DeliveryAnnotations.find(partitionKeyAnnotation),
      batchMessage.DeliveryAnnotations.end());

  EXPECT_EQ(0x80013700u, batchMessage.MessageFormat);

  auto const& batchedMessages = batchMessage.GetBodyAsBinary();
  ASSERT_EQ(2ul, batchedMessages.size());
  for (auto const& batchedMessage : batchedMessages)
  {
    AmqpMessage innerMessage{
        AmqpMessage::Deserialize(batchedMessage.data(), batchedMessage.size())};
    auto innerAnnotation = innerMessage.MessageAnnotations.find(partitionKeyAnnotation);
    ASSERT_NE(innerAnnotation, innerMessage.MessageAnnotations.end())
        << "A message in the batch has no partition key message annotation.";
    EXPECT_EQ(partitionKey, static_cast<std::string>(innerAnnotation->second));
  }

  // The batch builds the envelope from the annotated copy of the first message. That copy also
  // carries the generated message ID, so the envelope must carry the same message ID. This pins
  // the source of the envelope, which the partition key assertions above cannot show on their own.
  AmqpMessage firstMessage{
      AmqpMessage::Deserialize(batchedMessages[0].data(), batchedMessages[0].size())};
  ASSERT_FALSE(batchMessage.Properties.MessageId.IsNull())
      << "The batch envelope did not come from the annotated copy of the first message.";
  EXPECT_EQ(firstMessage.Properties.MessageId, batchMessage.Properties.MessageId);
}

// The partition key of the batch is the routing key for every message in the batch. Make sure that
// it replaces a partition key annotation that the caller already set on a raw AMQP message.
TEST_F(EventDataBatchTest, BatchPartitionKeyReplacesCallerAnnotation)
{
  constexpr const char* batchPartitionKey = "batch-partition-key";

  Azure::Messaging::EventHubs::EventDataBatchOptions options;
  options.MaxBytes = static_cast<std::uint64_t>((std::numeric_limits<uint16_t>::max)());
  options.PartitionKey = batchPartitionKey;

  Azure::Messaging::EventHubs::EventDataBatch batch{
      Azure::Messaging::EventHubs::_detail::EventDataBatchFactory::CreateEventDataBatch(options)};

  AmqpSymbol const partitionKeyAnnotation{
      Azure::Messaging::EventHubs::_detail::PartitionKeyAnnotation};

  auto callerMessage{std::make_shared<AmqpMessage>()};
  callerMessage->SetBody(AmqpBinaryData{'a', 'b', 'c'});
  callerMessage
      ->MessageAnnotations[AmqpSymbol{Azure::Messaging::EventHubs::_detail::PartitionKeyAnnotation}]
      = AmqpValue("caller-partition-key");

  EXPECT_TRUE(batch.TryAdd(callerMessage));

  auto batchMessage{batch.ToAmqpMessage()};

  auto envelopeAnnotation = batchMessage.MessageAnnotations.find(partitionKeyAnnotation);
  ASSERT_NE(envelopeAnnotation, batchMessage.MessageAnnotations.end());
  EXPECT_EQ(batchPartitionKey, static_cast<std::string>(envelopeAnnotation->second));

  auto const& batchedMessages = batchMessage.GetBodyAsBinary();
  ASSERT_EQ(1ul, batchedMessages.size());
  AmqpMessage innerMessage{
      AmqpMessage::Deserialize(batchedMessages[0].data(), batchedMessages[0].size())};
  auto innerAnnotation = innerMessage.MessageAnnotations.find(partitionKeyAnnotation);
  ASSERT_NE(innerAnnotation, innerMessage.MessageAnnotations.end());
  EXPECT_EQ(batchPartitionKey, static_cast<std::string>(innerAnnotation->second));

  // The message that the caller supplied must not change.
  auto callerAnnotation = callerMessage->MessageAnnotations.find(partitionKeyAnnotation);
  ASSERT_NE(callerAnnotation, callerMessage->MessageAnnotations.end());
  EXPECT_EQ("caller-partition-key", static_cast<std::string>(callerAnnotation->second));
}

// A batch without a partition key must not add a partition key annotation anywhere.
TEST_F(EventDataBatchTest, NoPartitionKeyAddsNoAnnotation)
{
  Azure::Messaging::EventHubs::EventDataBatchOptions options;
  options.MaxBytes = static_cast<std::uint64_t>((std::numeric_limits<uint16_t>::max)());

  Azure::Messaging::EventHubs::EventDataBatch batch{
      Azure::Messaging::EventHubs::_detail::EventDataBatchFactory::CreateEventDataBatch(options)};

  EXPECT_TRUE(batch.TryAdd(EventData{"First message."}));

  auto batchMessage{batch.ToAmqpMessage()};

  AmqpSymbol const partitionKeyAnnotation{
      Azure::Messaging::EventHubs::_detail::PartitionKeyAnnotation};

  EXPECT_EQ(
      batchMessage.MessageAnnotations.find(partitionKeyAnnotation),
      batchMessage.MessageAnnotations.end());
  EXPECT_EQ(
      batchMessage.DeliveryAnnotations.find(partitionKeyAnnotation),
      batchMessage.DeliveryAnnotations.end());

  auto const& batchedMessages = batchMessage.GetBodyAsBinary();
  ASSERT_EQ(1ul, batchedMessages.size());
  AmqpMessage innerMessage{
      AmqpMessage::Deserialize(batchedMessages[0].data(), batchedMessages[0].size())};
  EXPECT_EQ(
      innerMessage.MessageAnnotations.find(partitionKeyAnnotation),
      innerMessage.MessageAnnotations.end());
}
