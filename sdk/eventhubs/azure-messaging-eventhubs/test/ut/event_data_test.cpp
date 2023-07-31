// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/messaging/eventhubs.hpp"
#include "eventhubs_test_base.hpp"
#include "private/event_data_models_private.hpp"

#include <gtest/gtest.h>

using namespace Azure::Core::Amqp::Models;
using namespace Azure::Messaging::EventHubs::Models;

class EventDataTest : public EventHubsTestBase {};

// Construct an EventData object and convert it to an AMQP message.
// Verify that the resulting AMQP Message has the expected body and data (empty).
TEST_F(EventDataTest, EventDataNew)
{
  Azure::Messaging::EventHubs::Models::EventData eventData;

  auto message
      = Azure::Messaging::EventHubs::_detail::EventDataFactory::EventDataToAmqpMessage(eventData);

  EXPECT_EQ(0ul, message.ApplicationProperties.size());
  EXPECT_FALSE(message.Properties.ContentType.HasValue());
  EXPECT_FALSE(message.Properties.CorrelationId.HasValue());
  EXPECT_FALSE(message.Properties.MessageId.HasValue());

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

TEST_F(EventDataTest, EventData1)
{
  Azure::Messaging::EventHubs::Models::EventData eventData;

  eventData.Body = {1, 2};
  eventData.ContentType = "ct";
  eventData.Properties.emplace("abc", AmqpValue(23));
  eventData.CorrelationId = AmqpValue("ci");
  eventData.MessageId = AmqpValue("mi");

  auto message
      = Azure::Messaging::EventHubs::_detail::EventDataFactory::EventDataToAmqpMessage(eventData);

  EXPECT_EQ(1ul, message.ApplicationProperties.size());
  EXPECT_EQ(eventData.Body, static_cast<std::vector<uint8_t>>(message.GetBodyAsBinary()[0]));
  EXPECT_EQ("ct", message.Properties.ContentType.Value());
  EXPECT_EQ(AmqpValue("ci"), message.Properties.CorrelationId.Value());
  EXPECT_TRUE(message.Properties.MessageId.HasValue());
}

TEST_F(EventDataTest, EventDataStringBody)
{
  Azure::Messaging::EventHubs::Models::EventData eventData{"String Body Message."};

  auto message
      = Azure::Messaging::EventHubs::_detail::EventDataFactory::EventDataToAmqpMessage(eventData);
  EXPECT_FALSE(message.Properties.MessageId.HasValue());
  EXPECT_EQ(message.BodyType, Azure::Core::Amqp::Models::MessageBodyType::Data);
  EXPECT_EQ(message.GetBodyAsBinary().size(), 1ul);
  EXPECT_EQ(
      message.GetBodyAsBinary()[0],
      std::vector<uint8_t>(eventData.Body.begin(), eventData.Body.end()));
}

TEST_F(EventDataTest, EventDataBodyTest)
{
  {
    Azure::Messaging::EventHubs::Models::EventData msg;

    // Note that Data is an AMQP BinaryData value.
    msg.Body = {1, 3, 5, 7, 9};

    auto message
        = Azure::Messaging::EventHubs::_detail::EventDataFactory::EventDataToAmqpMessage(msg);

    EXPECT_EQ(message.GetBodyAsBinary().size(), 1ul);
    EXPECT_EQ(msg.Body, static_cast<std::vector<uint8_t>>(message.GetBodyAsBinary()[0]));
  }
}

TEST_F(EventDataTest, EventDataArrayBody)
{
  Azure::Messaging::EventHubs::Models::EventData eventData{1, 3, 5, 7, 9};

  auto message
      = Azure::Messaging::EventHubs::_detail::EventDataFactory::EventDataToAmqpMessage(eventData);
  EXPECT_FALSE(message.Properties.MessageId.HasValue());
  EXPECT_EQ(message.BodyType, Azure::Core::Amqp::Models::MessageBodyType::Data);
  EXPECT_EQ(message.GetBodyAsBinary().size(), 1ul);
  EXPECT_EQ(
      message.GetBodyAsBinary()[0],
      std::vector<uint8_t>(eventData.Body.begin(), eventData.Body.end()));
}

TEST_F(EventDataTest, EventDataVectorBody)
{
  std::vector<uint8_t> vector{2, 4, 6, 8, 10};
  Azure::Messaging::EventHubs::Models::EventData eventData{vector};

  auto message
      = Azure::Messaging::EventHubs::_detail::EventDataFactory::EventDataToAmqpMessage(eventData);
  EXPECT_FALSE(message.Properties.MessageId.HasValue());
  EXPECT_EQ(message.BodyType, Azure::Core::Amqp::Models::MessageBodyType::Data);
  EXPECT_EQ(message.GetBodyAsBinary().size(), 1ul);
  EXPECT_EQ(
      message.GetBodyAsBinary()[0],
      std::vector<uint8_t>(eventData.Body.begin(), eventData.Body.end()));
}
