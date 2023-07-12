// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

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
    EXPECT_FALSE(newData.ContentType.HasValue());
    EXPECT_TRUE(moveData.ContentType.HasValue());
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
    EXPECT_FALSE(newData.ContentType.HasValue());
    EXPECT_TRUE(moveData.ContentType.HasValue());
  }
}

TEST_F(EventDataTest, EventData1)
{
  Azure::Messaging::EventHubs::Models::EventData eventData;

  eventData.Body.Value = Azure::Core::Amqp::Models::AmqpValue(5);
  eventData.ContentType = "ct";
  eventData.Properties.emplace("abc", AmqpValue(23));
  eventData.CorrelationId = AmqpValue("ci");
  eventData.MessageId = AmqpValue("mi");

  auto message
      = Azure::Messaging::EventHubs::_detail::EventDataFactory::EventDataToAmqpMessage(eventData);

  EXPECT_EQ(1ul, message.ApplicationProperties.size());
  EXPECT_EQ(eventData.Body.Value, message.GetBodyAsAmqpValue());
  EXPECT_EQ("ct", message.Properties.ContentType.Value());
  EXPECT_EQ(AmqpValue("ci"), message.Properties.CorrelationId.Value());
  EXPECT_FALSE(message.Properties.MessageId.HasValue());
}

TEST_F(EventDataTest, EventDataBodyTest)
{
  {
    Azure::Messaging::EventHubs::Models::EventData msg;
    msg.Body.Value = AmqpValue("3");
    auto message
        = Azure::Messaging::EventHubs::_detail::EventDataFactory::EventDataToAmqpMessage(msg);

    EXPECT_EQ(msg.Body.Value, message.GetBodyAsAmqpValue());
  }
  {
    Azure::Messaging::EventHubs::Models::EventData msg;

    // Note that Data is an AMQP BinaryData value.
    msg.Body.Data = AmqpBinaryData{1, 3, 5, 7, 9};

    auto message
        = Azure::Messaging::EventHubs::_detail::EventDataFactory::EventDataToAmqpMessage(msg);

    EXPECT_EQ(message.GetBodyAsBinary().size(), 1);
    EXPECT_EQ(msg.Body.Data, message.GetBodyAsBinary()[0]);
  }

  {
    Azure::Messaging::EventHubs::Models::EventData msg;

    // Note that Sequence is an array of AmqpLists.
    msg.Body.Sequence = {3, "Foo", AmqpValue{AmqpBinaryData{1, 3, 5, 7, 9}}};

    auto message
        = Azure::Messaging::EventHubs::_detail::EventDataFactory::EventDataToAmqpMessage(msg);
    EXPECT_EQ(message.GetBodyAsAmqpList().size(), 1);
    EXPECT_EQ(msg.Body.Sequence, message.GetBodyAsAmqpList()[0]);
  }

  // Data and Value set - error.
  {
    Azure::Messaging::EventHubs::Models::EventData msg;

    msg.Body.Data = AmqpBinaryData{1, 3, 5, 7};
    msg.Body.Value = 27;

    EXPECT_ANY_THROW(
        Azure::Messaging::EventHubs::_detail::EventDataFactory::EventDataToAmqpMessage(msg));
  }
  // Data and Sequence set - error.
  {
    Azure::Messaging::EventHubs::Models::EventData msg;

    msg.Body.Data = AmqpBinaryData{1, 3, 5, 7};
    msg.Body.Sequence = {27, 3.25};

    EXPECT_ANY_THROW(
        Azure::Messaging::EventHubs::_detail::EventDataFactory::EventDataToAmqpMessage(msg));
  }

  // Value and Sequence set - error.
  {
    Azure::Messaging::EventHubs::Models::EventData msg;

    msg.Body.Value = "AmqpBinaryData{1, 3, 5, 7}";
    msg.Body.Sequence = {27, 3.25};

    EXPECT_ANY_THROW(
        Azure::Messaging::EventHubs::_detail::EventDataFactory::EventDataToAmqpMessage(msg));
  }

  // Value, Data, and Sequence set - error.
  {
    Azure::Messaging::EventHubs::Models::EventData msg;

    msg.Body.Data = AmqpBinaryData{1, 7, 32, 127, 255};
    msg.Body.Value = "AmqpBinaryData{1, 3, 5, 7}";
    msg.Body.Sequence = {27, 3.25};

    EXPECT_ANY_THROW(
        Azure::Messaging::EventHubs::_detail::EventDataFactory::EventDataToAmqpMessage(msg));
  }
}
