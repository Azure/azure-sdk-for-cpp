// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

#include "azure/core/amqp/models/amqp_properties.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"

#include <azure_uamqp_c/amqp_definitions_sequence_no.h>

#include <azure_uamqp_c/amqp_definitions_properties.h>
#include <iostream>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  Properties::Properties() : m_properties{properties_create()}
  {
    if (m_properties == nullptr)
    {
      throw std::runtime_error("Could not create properties.");
    }
  }

  Properties::~Properties()
  {
    if (m_properties)
    {
      properties_destroy(m_properties);
      m_properties = nullptr;
    }
  }

  Value Properties::GetMessageId() const
  {
    AMQP_VALUE value;
    if (properties_get_message_id(m_properties, &value))
    {
      throw std::exception("Could not set message id");
    }
    return amqpvalue_clone(value);
  }

  void Properties::SetMessageId(Value const& messageId)
  {
    if (properties_set_message_id(m_properties, messageId))
    {
      throw std::exception("Could not set message id");
    }
  }

  Value Properties::GetCorrelationId() const
  {
    AMQP_VALUE value;
    if (properties_get_correlation_id(m_properties, &value))
    {
      throw std::exception("Could not set correlation id");
    }
    // properties_get_correlation_id returns an in-place value.
    return amqpvalue_clone(value);
  }

  void Properties::SetCorrelationId(Value const& correlationId)
  {
    if (properties_set_correlation_id(m_properties, correlationId))
    {
      throw std::exception("Could not set correlation id");
    }
  }

  BinaryData Properties::GetUserId() const
  {
    amqp_binary value;
    if (properties_get_user_id(m_properties, &value))
    {
      throw std::exception("Could not get user id");
    }
    return BinaryData{reinterpret_cast<const uint8_t*>(value.bytes), value.length};
  }

  void Properties::SetUserId(BinaryData const& userId)
  {
    amqp_binary value{userId.bytes, static_cast<uint32_t>(userId.length)};
    if (properties_set_user_id(m_properties, value))
    {
      throw std::exception("Could not set user id");
    }
  }

  Value Properties::GetTo() const
  {
    AMQP_VALUE value;
    if (properties_get_to(m_properties, &value))
    {
      throw std::exception("Could not get to");
    }
    return amqpvalue_clone(value);
  }

  void Properties::SetTo(Value replyTo)
  {
    if (properties_set_to(m_properties, replyTo))
    {
      throw std::exception("Could not set to");
    }
  }

  std::string Properties::GetSubject() const
  {
    const char* value;
    if (properties_get_subject(m_properties, &value))
    {
      throw std::exception("Could not get subject");
    }
    return value;
  }

  void Properties::SetSubject(std::string const& subject)
  {
    if (properties_set_subject(m_properties, subject.data()))
    {
      throw std::exception("Could not set subject");
    }
  }

  Value Properties::GetReplyTo() const
  {
    AMQP_VALUE value;
    if (properties_get_reply_to(m_properties, &value))
    {
      throw std::exception("Could not get reply to");
    }
    return amqpvalue_clone(value);
  }

  void Properties::SetReplyTo(Value replyTo)
  {
    if (properties_set_reply_to(m_properties, replyTo))
    {
      throw std::exception("Could not set reply to");
    }
  }

  std::string Properties::GetContentType() const
  {
    const char* value;
    if (properties_get_content_type(m_properties, &value))
    {
      throw std::exception("Could not get content type");
    }
    return value;
  }

  void Properties::SetContentType(std::string const& contentType)
  {
    if (properties_set_content_type(m_properties, contentType.data()))
    {
      throw std::exception("Could not set content type");
    }
  }

  std::string Properties::GetContentEncoding() const
  {
    const char* value;
    if (properties_get_content_encoding(m_properties, &value))
    {
      throw std::exception("Could not get content encoding");
    }
    return value;
  }

  void Properties::SetContentEncoding(std::string const& contentEncoding)
  {
    if (properties_set_content_encoding(m_properties, contentEncoding.data()))
    {
      throw std::exception("Could not set content type");
    }
  }

  std::chrono::system_clock::time_point Properties::GetAbsoluteExpiryTime() const
  {
    timestamp expiryTime;
    if (properties_get_absolute_expiry_time(m_properties, &expiryTime))
    {
      throw std::exception("Could not get absolute expiry time");
    }

    std::chrono::milliseconds ms{expiryTime};
    return std::chrono::system_clock::time_point{
        std::chrono::duration_cast<std::chrono::system_clock::duration>(ms)};
  }
  void Properties::SetAbsoluteExpiryTime(std::chrono::system_clock::time_point const& expiryTime)
  {
    auto timeStamp{
        std::chrono::duration_cast<std::chrono::milliseconds>(expiryTime.time_since_epoch())};

    if (properties_set_absolute_expiry_time(m_properties, timeStamp.count()))
    {
      throw std::exception("Could not set absolute expiry time");
    }
  }

  std::chrono::system_clock::time_point Properties::GetCreationTime() const
  {
    timestamp creationTime;
    if (properties_get_creation_time(m_properties, &creationTime))
    {
      throw std::exception("Could not get absolute expiry time");
    }

    std::chrono::milliseconds ms{creationTime};
    return std::chrono::system_clock::from_time_t(0) + ms;
  }

  void Properties::SetCreationTime(std::chrono::system_clock::time_point const& creationTime)
  {
    auto timeStamp{
        std::chrono::duration_cast<std::chrono::milliseconds>(creationTime.time_since_epoch())};

    if (properties_set_creation_time(m_properties, timeStamp.count()))
    {
      throw std::exception("Could not set absolute expiry time");
    }
  }

  std::string Properties::GetGroupId() const
  {
    const char* value;
    if (properties_get_group_id(m_properties, &value))
    {
      throw std::exception("Could not get group id");
    }
    return value;
  }

  void Properties::SetGroupId(std::string const& groupId)
  {
    if (properties_set_group_id(m_properties, groupId.data()))
    {
      throw std::exception("Could not set group id");
    }
  }

  uint32_t Properties::GetGroupSequence() const
  {
    uint32_t sequence;
    if (properties_get_group_sequence(m_properties, &sequence))
    {
      throw std::exception("Could not get group sequence");
    }

    return sequence;
  }

  void Properties::SetGroupSequence(uint32_t groupSequence)
  {
    if (properties_set_group_sequence(m_properties, groupSequence))
    {
      throw std::exception("Could not set group sequence");
    }
  }

  std::string Properties::GetReplyToGroupId() const
  {
    const char* value;
    if (properties_get_reply_to_group_id(m_properties, &value))
    {
      throw std::exception("Could not get reply-to group id");
    }
    return value;
  }

  void Properties::SetReplyToGroupId(std::string const& replyToGroupId)
  {
    if (properties_set_reply_to_group_id(m_properties, replyToGroupId.data()))
    {
      throw std::exception("Could not set reply-to group id");
    }
  }
  std::string timeToString(std::chrono::system_clock::time_point t)
  {
    std::time_t time = std::chrono::system_clock::to_time_t(t);
    std::string time_str = std::ctime(&time);
    time_str.resize(time_str.size() - 1);
    return time_str;
  }
  std::ostream& operator<<(std::ostream& os, Properties const& properties)
  {
    os << "Properties {";
    os << "MessageId: " << properties.GetMessageId() << ", ";
    os << "UserId: " << properties.GetUserId() << ", ";
    os << "To: " << properties.GetTo() << ", ";
    os << "Subject: " << properties.GetSubject() << ", ";
    os << "ReplyTo: " << properties.GetReplyTo() << ", ";
    os << "ContentType: " << properties.GetContentType() << ", ";
    os << "ContentEncoding: " << properties.GetContentEncoding() << ", ";
    os << "AbsoluteExpiryTime: " << timeToString(properties.GetAbsoluteExpiryTime()) << ", ";
    os << "CreationTime: " << timeToString(properties.GetCreationTime()) << ", ";
    os << "GroupId: " << properties.GetGroupId() << ", ";
    os << "GroupSequence: " << properties.GetGroupSequence() << ", ";
    os << "ReplyToGroupId: " << properties.GetReplyToGroupId() << ", ";
    os << "}";
    return os;
  }
}}}} // namespace Azure::Core::Amqp::Models
