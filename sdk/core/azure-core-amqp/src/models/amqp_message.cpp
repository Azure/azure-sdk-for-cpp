// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/models/amqp_message.hpp"
#include "azure/core/amqp/models/amqp_header.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include <azure_uamqp_c/message.h>
#include <iostream>

namespace Azure { namespace Core { namespace _internal {
  void UniqueHandleHelper<MESSAGE_INSTANCE_TAG>::FreeAmqpMessage(MESSAGE_HANDLE value)
  {
    message_destroy(value);
  }
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  namespace {

    Azure::Core::_internal::UniqueHandle<HEADER_INSTANCE_TAG> GetHeaderFromMessage(
        MESSAGE_HANDLE message)
    {
      {
        HEADER_HANDLE headerValue;
        if (!message_get_header(message, &headerValue))
        {
          return Azure::Core::_internal::UniqueHandle<HEADER_INSTANCE_TAG>(headerValue);
        }
      }
      return nullptr;
    }
    Azure::Core::_internal::UniqueHandle<PROPERTIES_INSTANCE_TAG> GetPropertiesFromMessage(
        MESSAGE_HANDLE message)
    {
      PROPERTIES_HANDLE propertiesValue;
      if (!message_get_properties(message, &propertiesValue))
      {
        return Azure::Core::_internal::UniqueHandle<PROPERTIES_INSTANCE_TAG>(propertiesValue);
      }
      return nullptr;
    }
  } // namespace

  // AMQP ApplicationProperties descriptor (AMQP Specification 1.0 section 3.2.5)
  // http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-messaging-v1.0-os.html#type-application-properties
  //
  // ApplicationProperties are defined as being a map from string to simple AMQP value types.
  constexpr uint64_t AmqpApplicationPropertiesDescriptor = 116;

  Message::Message(MESSAGE_HANDLE message)
      : Header{GetHeaderFromMessage(message).get()}, Properties{
                                                         GetPropertiesFromMessage(message).get()}
  {
    uint32_t uint32Value;

    // Copy the Message Format from the source message. It will eventually be transferred to the
    // Transfer performative when the message is sent.
    if (!message_get_message_format(message, &uint32Value))
    {
      MessageFormat = uint32Value;
    }

    {
      AMQP_VALUE annotationsVal;
      // message_get_delivery_annotations returns a clone of the message annotations.
      if (!message_get_delivery_annotations(message, &annotationsVal) && annotationsVal != nullptr)
      {
        UniqueAmqpValueHandle deliveryAnnotations(annotationsVal);
        auto deliveryMap = AmqpValue{deliveryAnnotations.get()}.AsMap();
        DeliveryAnnotations = deliveryMap;
      }
    }
    {
      // message_get_message_annotations returns a clone of the message annotations.
      AMQP_VALUE annotationVal;
      if (!message_get_message_annotations(message, &annotationVal) && annotationVal)
      {
        UniqueAmqpValueHandle messageAnnotations(annotationVal);
        if (messageAnnotations)
        {
          auto messageMap = AmqpValue{messageAnnotations.get()}.AsMap();
          MessageAnnotations = messageMap;
        }
      }
    }
    {
      /*
       * The ApplicationProperties field in an AMQP message for uAMQP expects that the map value is
       * wrapped as a described value. A described value has a ULONG descriptor value and a value
       * type.
       *
       * Making things even more interesting, the ApplicationProperties field in an uAMQP message
       * is asymmetric.
       *
       * The MessageSender class will wrap ApplicationProperties in a described value, so when
       * setting application properties, the described value must NOT be present, but when
       * decoding an application properties, the GetApplicationProperties method has to be able to
       * handle both when the described value is present or not.
       */
      AMQP_VALUE properties;
      if (!message_get_application_properties(message, &properties) && properties)
      {
        UniqueAmqpValueHandle describedProperties(properties);
        properties = nullptr;
        if (describedProperties)
        {
          AMQP_VALUE value;
          if (amqpvalue_get_type(describedProperties.get()) == AMQP_TYPE_DESCRIBED)
          {
            auto describedType = amqpvalue_get_inplace_descriptor(describedProperties.get());
            uint64_t describedTypeValue;
            if (amqpvalue_get_ulong(describedType, &describedTypeValue))
            {
              throw std::runtime_error("Could not retrieve application properties described type.");
            }
            if (describedTypeValue != AmqpApplicationPropertiesDescriptor)
            {
              throw std::runtime_error("Application Properties are not the corect described type.");
            }

            value = amqpvalue_get_inplace_described_value(describedProperties.get());
          }
          else
          {
            value = describedProperties.get();
          }
          if (amqpvalue_get_type(value) != AMQP_TYPE_MAP)
          {
            throw std::runtime_error("Application Properties must be a map?!");
          }
          auto appProperties = AmqpMap(value);
          for (auto const& val : appProperties)
          {
            if (val.first.GetType() != AmqpValueType::String)
            {
              throw std::runtime_error("Key of Application Properties must be a string.");
            }
            ApplicationProperties.emplace(
                std::make_pair(static_cast<std::string>(val.first), val.second));
          }
        }
      }
    }
    {
      annotations footerVal;
      if (!message_get_footer(message, &footerVal) && footerVal)
      {
        UniqueAmqpValueHandle footerAnnotations(footerVal);
        footerVal = nullptr;
        auto footerMap = AmqpValue{footerAnnotations.get()}.AsMap();
        Footer = footerMap;
      }
    }
    {
      MESSAGE_BODY_TYPE bodyType;

      if (!message_get_body_type(message, &bodyType))
      {
        switch (bodyType)
        {
          case MESSAGE_BODY_TYPE_NONE:
            BodyType = MessageBodyType::None;
            break;
          case MESSAGE_BODY_TYPE_DATA: {
            size_t dataCount;
            if (!message_get_body_amqp_data_count(message, &dataCount))
            {
              for (auto i = 0ul; i < dataCount; i += 1)
              {
                BINARY_DATA binaryValue;
                if (!message_get_body_amqp_data_in_place(message, i, &binaryValue))
                {
                  m_binaryDataBody.push_back(AmqpBinaryData(std::vector<std::uint8_t>(
                      binaryValue.bytes, binaryValue.bytes + binaryValue.length)));
                }
              }
            }
            BodyType = MessageBodyType::Data;
          }
          break;
          case MESSAGE_BODY_TYPE_SEQUENCE: {

            size_t sequenceCount;
            if (!message_get_body_amqp_sequence_count(message, &sequenceCount))
            {
              for (auto i = 0ul; i < sequenceCount; i += 1)
              {
                AMQP_VALUE sequence;
                if (!message_get_body_amqp_sequence_in_place(message, i, &sequence))
                {
                  m_amqpSequenceBody.push_back(sequence);
                }
              }
            }
            BodyType = MessageBodyType::Sequence;
          }
          break;
          case MESSAGE_BODY_TYPE_VALUE: {
            AMQP_VALUE bodyValue;
            if (!message_get_body_amqp_value_in_place(message, &bodyValue))
            {
              m_amqpValueBody = bodyValue;
            }
            BodyType = MessageBodyType::Value;
          }
          break;
          case MESSAGE_BODY_TYPE_INVALID:
          default:
            throw std::runtime_error("Unknown body type.");
        }
      }
    }
  }
  Message::operator MESSAGE_HANDLE() const
  {
    Azure::Core::_internal::UniqueHandle<MESSAGE_INSTANCE_TAG> rv(message_create());

    if (message_set_header(rv.get(), Header))
    {
      throw std::runtime_error("Could not set message header.");
    }
    if (message_set_properties(rv.get(), Properties))
    {
      throw std::runtime_error("Could not set message properties.");
    }

    if (MessageFormat.HasValue())
    {
      if (message_set_message_format(rv.get(), MessageFormat.Value()))
      {
        throw std::runtime_error("Could not set destination message format.");
      }
    }
    if (!DeliveryAnnotations.empty())
    {
      if (message_set_delivery_annotations(rv.get(), static_cast<UniqueAmqpValueHandle>(DeliveryAnnotations).get()))
      {
        throw std::runtime_error("Could not set delivery annotations.");
      }
    }
    if (!MessageAnnotations.empty())
    {
      if (message_set_message_annotations(rv.get(), static_cast<UniqueAmqpValueHandle>(MessageAnnotations).get()))
      {
        throw std::runtime_error("Could not set message annotations.");
      }
    }
    if (!ApplicationProperties.empty())
    {
      AmqpMap appProperties;
      for (auto const& val : ApplicationProperties)
      {
        if ((val.second.GetType() == AmqpValueType::List)
            || (val.second.GetType() == AmqpValueType::Map)
            || (val.second.GetType() == AmqpValueType::Composite)
            || (val.second.GetType() == AmqpValueType::Described))
        {
          throw std::runtime_error(
              "Message Application Property values must be simple value types");
        }
        appProperties.emplace(val);
      }
      if (message_set_application_properties(rv.get(), static_cast<UniqueAmqpValueHandle>(appProperties).get()))
      {
        throw std::runtime_error("Could not set application properties.");
      }
    }
    if (!Footer.empty())
    {
      if (message_set_footer(rv.get(), static_cast<UniqueAmqpValueHandle>(Footer).get()))
      {
        throw std::runtime_error("Could not set message annotations.");
      }
    }
    switch (BodyType)
    {
      case MessageBodyType::None:
        break;
      case MessageBodyType::Data:
        for (auto const& binaryVal : m_binaryDataBody)
        {
          BINARY_DATA valueData;
          valueData.bytes = binaryVal.data();
          valueData.length = static_cast<uint32_t>(binaryVal.size());
          if (message_add_body_amqp_data(rv.get(), valueData))
          {
            throw std::runtime_error("Could not set message body AMQP sequence value.");
          }
        }
        break;
      case MessageBodyType::Sequence:
        for (auto const& sequenceVal : m_amqpSequenceBody)
        {
          if (message_add_body_amqp_sequence(rv.get(), sequenceVal))
          {
            throw std::runtime_error("Could not set message body AMQP sequence value.");
          }
        }
        break;
      case MessageBodyType::Value:
        if (message_set_body_amqp_value(rv.get(), m_amqpValueBody))
        {
          throw std::runtime_error("Could not set message body AMQP value.");
        }
        break;
      case MessageBodyType::Invalid: // LCOV_EXCL_LINE
      default: // LCOV_EXCL_LINE
        throw std::runtime_error("Unknown message body type."); // LCOV_EXCL_LINE
    }
    return rv.release();
  }

  AmqpList Message::GetBodyAsAmqpList() const
  {
    if (BodyType != MessageBodyType::Sequence)
    {
      throw std::runtime_error("Invalid body type, should be MessageBodyType::Sequence.");
    }
    return m_amqpSequenceBody;
  }

  void Message::SetBody(AmqpBinaryData const& value)
  {
    BodyType = MessageBodyType::Data;
    m_binaryDataBody.push_back(value);
  }
  void Message::SetBody(std::vector<AmqpBinaryData> const& value)
  {
    BodyType = MessageBodyType::Data;
    m_binaryDataBody = value;
  }
  void Message::SetBody(AmqpValue const& value)
  {
    BodyType = MessageBodyType::Value;
    m_amqpValueBody = value;
  }
  void Message::SetBody(AmqpList const& value)
  {
    BodyType = MessageBodyType::Sequence;
    m_amqpSequenceBody = value;
  }

  AmqpValue Message::GetBodyAsAmqpValue() const
  {
    if (BodyType != MessageBodyType::Value)
    {
      throw std::runtime_error("Invalid body type, should be MessageBodyType::Value.");
    }
    return m_amqpValueBody;
  }
  std::vector<AmqpBinaryData> Message::GetBodyAsBinary() const
  {
    if (BodyType != MessageBodyType::Data)
    {
      throw std::runtime_error("Invalid body type, should be MessageBodyType::Value.");
    }
    return m_binaryDataBody;
  }

  std::ostream& operator<<(std::ostream& os, Message const& message)
  {
    os << "Message: " << std::endl;
    os << "Header " << message.Header << std::endl;
    os << "Properties: " << message.Properties;
    os << "Body: [" << std::endl;
    switch (message.BodyType)
    {
      case MessageBodyType::Invalid: // LCOV_EXCL_LINE
        os << "Invalid"; // LCOV_EXCL_LINE
        break; // LCOV_EXCL_LINE
      case MessageBodyType::None:
        os << "None";
        break;
      case MessageBodyType::Data: {
        os << "AMQP Data: [";
        auto bodyBinary = message.GetBodyAsBinary();
        uint8_t i = 0;
        for (auto const& val : bodyBinary)
        {
          os << "Data: " << val << std::endl;
          if (i < bodyBinary.size() - 1)
          {
            os << ", ";
          }
          i += 1;
        }
        os << "]";
      }
      break;
      case MessageBodyType::Sequence: {
        os << "AMQP Sequence: [";
        auto bodySequence = message.GetBodyAsAmqpList();
        uint8_t i = 0;
        for (auto const& val : bodySequence)
        {
          os << "Sequence: " << val << std::endl;
          if (i < bodySequence.size() - 1)
          {
            os << ", ";
          }
          i += 1;
        }
        os << "]";
      }
      break;
      case MessageBodyType::Value:
        os << "AmqpValue: " << message.GetBodyAsAmqpValue();
        break;
    }
    os << "]";

    {
      if (!message.ApplicationProperties.empty())
      {
        os << std::endl << "Application Properties: ";
        for (auto const& val : message.ApplicationProperties)
        {
          os << "{" << val.first << ", " << val.second << "}";
        }
      }
    }
    if (!message.DeliveryAnnotations.empty())
    {
      os << std::endl << "Delivery Annotations: ";
      for (auto const& val : message.DeliveryAnnotations)
      {
        os << "{" << val.first << ", " << val.second << "}";
      }
    }
    if (!message.MessageAnnotations.empty())
    {
      os << std::endl << "Message Annotations: ";
      for (auto const& val : message.MessageAnnotations)
      {
        os << "{" << val.first << ", " << val.second << "}";
      }
    }
    if (!message.Footer.empty())
    {
      os << "Footer: ";
      for (auto const& val : message.Footer)
      {
        os << "{" << val.first << ", " << val.second << "}";
      }
    }
    return os;
  }
}}}} // namespace Azure::Core::Amqp::Models
