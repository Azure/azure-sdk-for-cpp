// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/messaging/eventhubs/models/event_data.hpp"

#include "azure/messaging/eventhubs/eventhub_constants.hpp"
#include "private/event_data_models_private.hpp"

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

  ReceivedEventData::ReceivedEventData(Azure::Core::Amqp::Models::AmqpMessage const& message)
      : EventData(), m_message{message}
  {
    // Promote the specific message properties into ReceivedEventData.
    Properties = message.ApplicationProperties;
    ContentType = message.Properties.ContentType;
    CorrelationId = message.Properties.CorrelationId;

    // If the message's body type is a single value, capture it in the ReceivedEventData.Body.
    // Otherwise we can't express the message body as a single value, so we'll leave
    // ReceivedEventData.Body as null.
    switch (message.BodyType)
    {
      case Azure::Core::Amqp::Models::MessageBodyType::Value:
        Body.Value = message.GetBodyAsAmqpValue();
        break;
      case Azure::Core::Amqp::Models::MessageBodyType::Sequence: {

        auto sequence = message.GetBodyAsAmqpList();
        if (sequence.size() == 1)
        {
          Body.Sequence = sequence[0];
        }
        break;
      }
      case Azure::Core::Amqp::Models::MessageBodyType::Data: {

        auto binaryData = message.GetBodyAsBinary();
        if (binaryData.size() == 1)
        {
          Body.Data = binaryData[0];
        }
        break;
      }
      default:
        break;
    }

    // Copy the message annotations into the ReceivedEventData.SystemProperties. There are 3
    // eventhubs specific annotations which are promoted in the ReceivedEventData, so promote them
    // as well.
    for (auto const& item : message.MessageAnnotations)
    {
      // Ignore any annotations where the key isn't an Amqp Symbols.
      if ((item.first.GetType() != Azure::Core::Amqp::Models::AmqpValueType::Symbol))
      {
        continue;
      }
      auto key = item.first.AsSymbol();
      if (key == _detail::EnqueuedTimeAnnotation)
      {
        EnqueuedTime = Azure::DateTime{std::chrono::system_clock::time_point{
            static_cast<std::chrono::milliseconds>(item.second.AsTimestamp())}};
      }
      else if (key == _detail::OffsetNumberAnnotation)
      {
        switch (item.second.GetType())
        {
          case Azure::Core::Amqp::Models::AmqpValueType::Ulong:
            Offset = item.second;
            break;
          case Azure::Core::Amqp::Models::AmqpValueType::Long:
            Offset = static_cast<int64_t>(item.second);
            break;
          case Azure::Core::Amqp::Models::AmqpValueType::Uint:
            Offset = static_cast<uint32_t>(item.second);
            break;
          case Azure::Core::Amqp::Models::AmqpValueType::Int:
            Offset = static_cast<int32_t>(item.second);
            break;
          case Azure::Core::Amqp::Models::AmqpValueType::String:
            Offset = std::strtoul(static_cast<std::string>(item.second).c_str(), nullptr, 10);
            break;
          default:
            break;
        }
      }
      else if (key == _detail::PartitionKeyAnnotation)
      {
        PartitionKey = static_cast<std::string>(item.second);
      }
      else if (key == _detail::SequenceNumberAnnotation)
      {
        SequenceNumber = item.second;
      }
      else
      {
        // The Key in MessageAnnotations is normally an AmqpSymbol, cast it to a string Key when
        // placing in SystemProperties.
        SystemProperties.emplace(static_cast<std::string>(key), item.second);
      }
    }
  }
}}}} // namespace Azure::Messaging::EventHubs::Models
namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {

  Azure::Core::Amqp::Models::AmqpMessage EventDataFactory::EventDataToAmqpMessage(
      Models::EventData const& eventData)
  {
    Azure::Core::Amqp::Models::AmqpMessage rv;
    rv.Properties.ContentType = eventData.ContentType;
    rv.Properties.CorrelationId = eventData.CorrelationId;

    rv.ApplicationProperties = eventData.Properties;
    EventBodyToAmqpMessageBody(eventData.Body, rv);
    return rv;
  }

  /** @brief Set the body of the AMQP message.
   *
   * Throws an exception if the caller has set more than one of Value, Sequence, or Data.
   *
   * @param message The AMQP message to set the body on.
   */
  void EventDataFactory::EventBodyToAmqpMessageBody(
      Models::EventDataBody const& body,
      Azure::Core::Amqp::Models::AmqpMessage& message)
  {
    if (!body.Data.empty())
    {
      if (!body.Sequence.empty() || !body.Value.IsNull())
      {
        throw std::runtime_error("Message body cannot contain both data and value/sequence.");
      }
      message.SetBody(body.Data);
    }
    else if (!body.Sequence.empty())
    {
      if (!body.Value.IsNull() || !body.Data.empty())
      {
        throw std::runtime_error("Message body cannot contain both sequence and data/value.");
      }
      message.SetBody(body.Sequence);
    }
    else
    {
      if (!body.Sequence.empty() || !body.Data.empty())
      {
        throw std::runtime_error("Message body cannot contain both value and data/sequence.");
      }

      message.SetBody(body.Value);
    }
  }

}}}} // namespace Azure::Messaging::EventHubs::_detail
