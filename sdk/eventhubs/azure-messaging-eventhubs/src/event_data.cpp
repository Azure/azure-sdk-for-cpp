// Copyright (c) Microsoft Corporation.
// SPDX-Licence-Identifier: MIT

#include "azure/messaging/eventhubs/models/event_data.hpp"

#include "private/eventhubs_constants.hpp"
#include "private/eventhubs_utilities.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <iostream>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Messaging { namespace EventHubs { namespace Models {

  EventData::EventData(std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage const> const& message)
      : // Promote the specific message properties into ReceivedEventData.
        ContentType{message->Properties.ContentType},
        CorrelationId{message->Properties.CorrelationId}, MessageId{message->Properties.MessageId},
        Properties{message->ApplicationProperties}, m_message{message}
  {
    // If the message's body type is a single binary value, capture it in the
    // EventData.Body. Otherwise we can't express the message body as a single value, so
    // we'll leave EventData.Body as null.
    if (message->BodyType == Azure::Core::Amqp::Models::MessageBodyType::Data)
    {
      auto& binaryData = message->GetBodyAsBinary();
      if (binaryData.size() == 1)
      {
        Body = std::vector<uint8_t>(binaryData[0]);
      }
    }
  }

  ReceivedEventData::ReceivedEventData(
      std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage const> const& message)
      : EventData(message)
  {
    // Copy the message annotations into the ReceivedEventData.SystemProperties. There are 3
    // eventhubs specific annotations which are promoted in the ReceivedEventData, so promote them
    // as well.
    for (auto const& item : message->MessageAnnotations)
    {
      // Ignore any annotations where the key isn't an Amqp Symbols.
      if ((item.first.GetType() != Azure::Core::Amqp::Models::AmqpValueType::Symbol))
      {
        continue;
      }
      auto key = item.first.AsSymbol();
      if (key == _detail::EnqueuedTimeAnnotation)
      {
        auto timePoint = static_cast<std::chrono::milliseconds>(item.second.AsTimestamp());
        auto dateTime = Azure::DateTime{Azure::DateTime::time_point{timePoint}};
        EnqueuedTime = dateTime;
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
        std::string keyName = static_cast<std::string>(key);
        auto result{SystemProperties.emplace(keyName, item.second)};
        if (!result.second)
        {
          // If the key already exists, log a warning.
          Log::Stream(Logger::Level::Warning)
              << "Duplicate key in MessageAnnotations: " << key << std::endl;
        }
      }
    }
  }

  std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage const> EventData::GetRawAmqpMessage() const
  {
    // If the underlying message is already populated, return it. This will typically happen when a
    // client attempts to send a raw AMQP message.
    if (m_message)
    {
      return m_message;
    }
    std::shared_ptr<Azure::Core::Amqp::Models::AmqpMessage> rv{
        std::make_shared<Azure::Core::Amqp::Models::AmqpMessage>()};
    rv->Properties.ContentType = ContentType;
    rv->Properties.CorrelationId = CorrelationId;
    rv->Properties.MessageId = MessageId;

    rv->ApplicationProperties = Properties;
    if (!Body.empty())
    {
      rv->SetBody(Body);
    }
    return rv;
  }

  std::ostream& operator<<(std::ostream& os, EventData const& data)
  {
    os << "EventData: [" << std::endl;
    os << "  Body: " << data.Body.size() << std::endl;
    if (!data.Properties.empty())
    {
      os << "  Properties: [";
      for (auto const& item : data.Properties)
      {
        os << "    " << item.first << ": " << item.second << std::endl;
      }
      os << "  ]" << std::endl;
    }
    if (data.ContentType.HasValue())
    {
      os << "  ContentType: " << data.ContentType.Value() << std::endl;
    }
    if (data.CorrelationId.HasValue())
    {
      os << "  CorrelationId: " << data.CorrelationId.Value() << std::endl;
    }
    if (data.MessageId.HasValue())
    {
      os << "  MessageId: " << data.MessageId.Value() << std::endl;
    }

    os << "]" << std::endl;

    return os;
  }
  std::ostream& operator<<(std::ostream& os, ReceivedEventData const& data)
  {
    os << "EventData: [" << std::endl;
    os << "  Body: " << data.Body.size() << " bytes" << std::endl;
    if (!data.Properties.empty())
    {
      os << "  Properties: [";
      for (auto const& item : data.Properties)
      {
        os << "    " << item.first << ": " << item.second << std::endl;
      }
      os << "  ]" << std::endl;
    }
    if (!data.SystemProperties.empty())
    {
      os << "  SystemProperties: [" << std::endl;
      for (auto const& item : data.SystemProperties)
      {
        os << "    " << item.first << ": " << item.second << std::endl;
      }
      os << "  ]" << std::endl;
    }
    if (data.ContentType.HasValue())
    {
      os << "  ContentType: " << data.ContentType.Value() << std::endl;
    }
    if (data.CorrelationId.HasValue())
    {
      os << "  CorrelationId: " << data.CorrelationId.Value() << std::endl;
    }
    if (data.PartitionKey.HasValue())
    {
      os << "  PartitionKey: " << data.PartitionKey.Value() << std::endl;
    }
    if (data.SequenceNumber.HasValue())
    {
      os << "  SequenceNumber: " << data.SequenceNumber.Value() << std::endl;
    }
    if (data.MessageId.HasValue())
    {
      os << "  MessageId: " << data.MessageId.Value() << std::endl;
    }
    if (data.Offset.HasValue())
    {
      os << "  Offset: " << data.Offset.Value() << std::endl;
    }
    if (data.EnqueuedTime.HasValue())
    {
      os << "  EnqueuedTime: " << data.EnqueuedTime.Value().ToString() << std::endl;
    }
    os << "Raw Message" << *data.GetRawAmqpMessage();
    os << "]" << std::endl;

    return os;
  }

}}}} // namespace Azure::Messaging::EventHubs::Models
