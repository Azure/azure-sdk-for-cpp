#include "azure/core/amqp/models/amqp_message.hpp"
#include "azure/core/amqp/models/amqp_header.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"
#include <azure_uamqp_c/message.h>
#include <iostream>

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  Message::Message() : m_message(message_create())
  {
    if (!m_message)
    {
      throw std::runtime_error("Could not create message.");
    }
  }

  Message::Message(MESSAGE_HANDLE message) : m_message(message_clone(message)) {}

  Message::~Message()
  {
    if (m_message)
    {
      message_destroy(m_message);
      m_message = nullptr;
    }
  }

  Message::Message(Message const& that) { m_message = message_clone(that.m_message); }
  Message& Message::operator=(Message const& that)
  {
    m_message = message_clone(that.m_message);
    return *this;
  }

  Message::Message(Message&& that) noexcept : m_message{that.m_message}
  {
    that.m_message = nullptr;
  }
  Message& Message::operator=(Message&& that) noexcept
  {
    m_message = that.m_message;
    that.m_message = nullptr;
    return *this;
  }

  void Message::SetHeader(Header const& header)
  {
    if (message_set_header(m_message, header))
    {
      throw std::runtime_error("Could not set message header");
    }
  }
  Header Message::GetHeader() const
  {
    HEADER_HANDLE header;
    if (message_get_header(m_message, &header))
    {
      throw std::runtime_error("Could not set message header");
    }
    return header;
  }
  void Message::SetFooter(Value const& header)
  {
    if (message_set_footer(m_message, header))
    {
      throw std::runtime_error("Could not set footer");
    }
  }

  Value Message::GetFooter() const
  {
    annotations footer;
    if (message_get_footer(m_message, &footer))
    {
      throw std::runtime_error("Could not get footer");
    }
    return footer;
  }

  void Message::SetDeliveryAnnotations(Value const& annotations)
  {
    if (message_set_delivery_annotations(m_message, annotations))
    {
      throw std::runtime_error("Could not set delivery annotations");
    }
  }
  Value Message::GetDeliveryAnnotations() const
  {
    annotations value;
    if (message_get_delivery_annotations(m_message, &value))
    {
      throw std::runtime_error("Could not get annotations");
    }
    return value;
  }
  void Message::SetFormat(uint32_t messageFormat)
  {
    if (message_set_message_format(m_message, messageFormat))
    {
      throw std::runtime_error("Could not set message format");
    }
  }
  uint32_t Message::GetFormat() const
  {
    uint32_t format;
    if (message_get_message_format(m_message, &format))
    {
      throw std::runtime_error("Could not get message format");
    }
    return format;
  }
  void Message::SetMessageAnnotations(Value const& annotations)
  {
    if (message_set_message_annotations(m_message, annotations))
    {
      throw std::runtime_error("Could not set message format");
    }
  }
  Value Message::GetMessageAnnotations() const
  {
    message_annotations annotations;
    if (message_get_message_annotations(m_message, &annotations))
    {
      throw std::runtime_error("Could not get message annotations");
    }

    return annotations;
  }
  void Message::SetProperties(Properties const& properties)
  {
    if (message_set_properties(m_message, properties))
    {
      throw std::runtime_error("Could not get message annotations");
    }
  }
  Properties Message::GetProperties() const
  {
    PROPERTIES_HANDLE properties;
    if (message_get_properties(m_message, &properties))
    {
      throw std::runtime_error("Could not get message annotations");
    }
    return properties;
  }
  void Message::SetApplicationProperties(Value const& value)
  {
    if (message_set_application_properties(m_message, value))
    {
      throw std::runtime_error("Could not set application properties.");
    }
  }
  Value Message::GetApplicationProperties() const
  {
    AMQP_VALUE properties;
    if (message_get_application_properties(m_message, &properties))
    {
      throw std::runtime_error("Could not get application properties");
    }
    return properties;
  }
  MessageBodyType Message::GetBodyType() const
  {
    MESSAGE_BODY_TYPE messageType;
    if (message_get_body_type(m_message, &messageType))
    {
      throw std::runtime_error("Could not get body type");
    }
    switch (messageType)
    {
      case MESSAGE_BODY_TYPE_NONE:
        return MessageBodyType::None;
      case MESSAGE_BODY_TYPE_DATA:
        return MessageBodyType::Data;
        break;
      case MESSAGE_BODY_TYPE_SEQUENCE:
        return MessageBodyType::Sequence;
      case MESSAGE_BODY_TYPE_VALUE:
        return MessageBodyType::Value;
    }
    return MessageBodyType::None;
  }
  size_t Message::GetBodyAmqpSequenceCount()
  {
    size_t count;
    if (message_get_body_amqp_sequence_count(m_message, &count))
    {
      throw std::runtime_error("Could not get body sequence count.");
    }
    return count;
  }
  Value Message::GetBodyAmqpSequence(uint32_t index)
  {
    AMQP_VALUE value;
    if (message_get_body_amqp_sequence_in_place(m_message, index, &value))
    {
      throw std::runtime_error("Could not get BodySequence.");
    }
    return amqpvalue_clone(value);
  }

  void Message::AddBodyAmqpSequence(Value const& value)
  {
    if (message_add_body_amqp_sequence(m_message, value))
    {
      throw std::runtime_error("Could not Add Body Sequence.");
    }
  }

  void Message::AddBodyAmqpData(BinaryData binaryData)
  {
    BINARY_DATA amqpData;
    amqpData.bytes = binaryData.bytes;
    amqpData.length = binaryData.length;
    if (message_add_body_amqp_data(m_message, amqpData))
    {
      throw std::runtime_error("Could not Add Body Data.");
    }
  }

  BinaryData Message::GetBodyAmqpData(size_t index) const
  {
    BINARY_DATA binaryData;
    if (message_get_body_amqp_data_in_place(m_message, index, &binaryData))
    {
      throw std::runtime_error("Could not get Body Data.");
    }
    return BinaryData{static_cast<const uint8_t*>(binaryData.bytes), binaryData.length};
  }

  size_t Message::GetBodyAmqpDataCount() const
  {
    size_t count;
    if (message_get_body_amqp_data_count(m_message, &count))
    {
      throw std::runtime_error("Could not get AMQP Data Count.");
    }
    return count;
  }
  void Message::SetBodyAmqpValue(Value value)
  {
    if (message_set_body_amqp_value(m_message, value))
    {
      throw std::runtime_error("Could not set body AMQP value");
    }
  }
  std::ostream& operator<<(std::ostream& os, Message const& message)
  {
    if (message.GetHeader())
    {
      os << "Header " << message.GetHeader() << std::endl;
    }
    if (message.GetProperties())
    {
      os << "Properties: " << message.GetProperties();
    }
    if (message.GetBodyAmqpDataCount() != 0)
    {
      os << "AMQP Data: [";
      for (size_t i = 0; i < message.GetBodyAmqpDataCount(); i += 1)
      {
        auto data = message.GetBodyAmqpData(i);
        os << "Data: " << data.length << " bytes: " << data;
        if (i < message.GetBodyAmqpDataCount() - 1)
        {
          os << ", ";
        }
      }
    }
    os << "]";

    if (!message.GetApplicationProperties().IsNull())
    {
      os << std::endl << "Application Properties: " << message.GetApplicationProperties();
    }
    if (!message.GetDeliveryAnnotations().IsNull())
    {
      os << std::endl << "Delivery Annotations: " << message.GetDeliveryAnnotations();
    }
    if (!message.GetMessageAnnotations().IsNull())
    {
      os << std::endl << "Message Annotations: " << message.GetMessageAnnotations();
    }
    if (!message.GetFooter().IsNull())
    {
      os << "Footer: " << message.GetFooter();
    }
    return os;
  }
}}}} // namespace Azure::Core::Amqp::Models
