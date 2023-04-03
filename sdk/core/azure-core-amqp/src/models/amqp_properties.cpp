// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/models/amqp_properties.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"

#include <azure_uamqp_c/amqp_definitions_sequence_no.h>

#include <azure_uamqp_c/amqp_definitions_properties.h>
#include <iostream>
#include <stdexcept>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  Properties::Properties() : m_properties{properties_create()}
  {
    if (m_properties == nullptr)
    {
      throw std::runtime_error("Could not create properties."); // LCOV_EXCL_LINE
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

  AmqpValue Properties::GetMessageId() const
  {
    AMQP_VALUE value;
    if (properties_get_message_id(m_properties, &value))
    {
      throw std::runtime_error("Could not get message id");
    }
    return amqpvalue_clone(value);
  }

  void Properties::SetMessageId(AmqpValue const& messageId)
  {
    if (properties_set_message_id(m_properties, messageId))
    {
      throw std::runtime_error("Could not set message id"); // LCOV_EXCL_LINE
    }
  }

  AmqpValue Properties::GetCorrelationId() const
  {
    AMQP_VALUE value;
    if (properties_get_correlation_id(m_properties, &value))
    {
      return AmqpValue();
    }
    // properties_get_correlation_id returns an in-place value.
    return amqpvalue_clone(value);
  }

  void Properties::SetCorrelationId(AmqpValue const& correlationId)
  {
    if (properties_set_correlation_id(m_properties, correlationId))
    {
      throw std::runtime_error("Could not set correlation id"); // LCOV_EXCL_LINE
    }
  }

  BinaryData Properties::GetUserId() const
  {
    amqp_binary value;
    if (properties_get_user_id(m_properties, &value))
    {
      throw std::runtime_error("Could not get user id"); // LCOV_EXCL_LINE
    }
    return BinaryData{reinterpret_cast<const uint8_t*>(value.bytes), value.length};
  }

  void Properties::SetUserId(BinaryData const& userId)
  {
    amqp_binary value{userId.bytes, static_cast<uint32_t>(userId.length)};
    if (properties_set_user_id(m_properties, value))
    {
      throw std::runtime_error("Could not set user id"); // LCOV_EXCL_LINE
    }
  }

  AmqpValue Properties::GetTo() const
  {
    AMQP_VALUE value;
    if (properties_get_to(m_properties, &value))
    {
      throw std::runtime_error("Could not get to"); // LCOV_EXCL_LINE
    }
    return amqpvalue_clone(value);
  }

  void Properties::SetTo(AmqpValue replyTo)
  {
    if (properties_set_to(m_properties, replyTo))
    {
      throw std::runtime_error("Could not set to"); // LCOV_EXCL_LINE
    }
  }

  std::string Properties::GetSubject() const
  {
    const char* value;
    if (properties_get_subject(m_properties, &value))
    {
      throw std::runtime_error("Could not get subject");
    }
    return value;
  }

  void Properties::SetSubject(std::string const& subject)
  {
    if (properties_set_subject(m_properties, subject.data()))
    {
      throw std::runtime_error("Could not set subject");
    }
  }

  AmqpValue Properties::GetReplyTo() const
  {
    AMQP_VALUE value;
    if (properties_get_reply_to(m_properties, &value))
    {
      throw std::runtime_error("Could not get reply to");
    }
    return amqpvalue_clone(value);
  }

  void Properties::SetReplyTo(AmqpValue replyTo)
  {
    if (properties_set_reply_to(m_properties, replyTo))
    {
      throw std::runtime_error("Could not set reply to"); // LCOV_EXCL_LINE
    }
  }

  std::string Properties::GetContentType() const
  {
    const char* value;
    if (properties_get_content_type(m_properties, &value))
    {
      throw std::runtime_error("Could not get content type");
    }
    return value;
  }

  void Properties::SetContentType(std::string const& contentType)
  {
    if (properties_set_content_type(m_properties, contentType.data()))
    {
      throw std::runtime_error("Could not set content type"); // LCOV_EXCL_LINE
    }
  }

  std::string Properties::GetContentEncoding() const
  {
    const char* value;
    if (properties_get_content_encoding(m_properties, &value))
    {
      throw std::runtime_error("Could not get content encoding");
    }
    return value;
  }

  void Properties::SetContentEncoding(std::string const& contentEncoding)
  {
    if (properties_set_content_encoding(m_properties, contentEncoding.data()))
    {
      throw std::runtime_error("Could not set content type");
    }
  }

  std::chrono::system_clock::time_point Properties::GetAbsoluteExpiryTime() const
  {
    timestamp expiryTime;
    if (properties_get_absolute_expiry_time(m_properties, &expiryTime))
    {
      throw std::runtime_error("Could not get absolute expiry time");
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
      throw std::runtime_error("Could not set absolute expiry time"); // LCOV_EXCL_LINE
    }
  }

  std::chrono::system_clock::time_point Properties::GetCreationTime() const
  {
    timestamp creationTime;
    if (properties_get_creation_time(m_properties, &creationTime))
    {
      throw std::runtime_error("Could not get creation time");
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
      throw std::runtime_error("Could not set creation time"); // LCOV_EXCL_LINE
    }
  }

  std::string Properties::GetGroupId() const
  {
    const char* value;
    if (properties_get_group_id(m_properties, &value))
    {
      throw std::runtime_error("Could not get group id");
    }
    return value;
  }

  void Properties::SetGroupId(std::string const& groupId)
  {
    if (properties_set_group_id(m_properties, groupId.data()))
    {
      throw std::runtime_error("Could not set group id"); // LCOV_EXCL_LINE
    }
  }

  uint32_t Properties::GetGroupSequence() const
  {
    uint32_t sequence;
    if (properties_get_group_sequence(m_properties, &sequence))
    {
      throw std::runtime_error("Could not get group sequence");
    }

    return sequence;
  }

  void Properties::SetGroupSequence(uint32_t groupSequence)
  {
    if (properties_set_group_sequence(m_properties, groupSequence))
    {
      throw std::runtime_error("Could not set group sequence"); // LCOV_EXCL_LINE
    }
  }

  std::string Properties::GetReplyToGroupId() const
  {
    const char* value;
    if (properties_get_reply_to_group_id(m_properties, &value))
    {
      throw std::runtime_error("Could not get reply-to group id");
    }
    return value;
  }

  void Properties::SetReplyToGroupId(std::string const& replyToGroupId)
  {
    if (properties_set_reply_to_group_id(m_properties, replyToGroupId.data()))
    {
      throw std::runtime_error("Could not set reply-to group id");
    }
  }

  namespace {
    std::string timeToString(std::chrono::system_clock::time_point t)
    {
      std::time_t time = std::chrono::system_clock::to_time_t(t);
      std::string time_str = std::ctime(&time);
      time_str.resize(time_str.size() - 1);
      return time_str;
    }
  } // namespace
  std::ostream& operator<<(std::ostream& os, Properties const& properties)
  {
    os << "Properties {";
    {
      AMQP_VALUE messageId;
      if (!properties_get_message_id(properties, &messageId))
      {
        os << "MessageId: " << properties.GetMessageId();
      }
      else
      {
        os << "MessageId: <null>";
      }
    }
    {
      amqp_binary binary;
      if (!properties_get_user_id(properties.m_properties, &binary))
      {
        os << ", UserId: "
           << std::string(
                  static_cast<const char*>(binary.bytes),
                  static_cast<const char*>(binary.bytes) + binary.length);
      }
    }
    {
      AMQP_VALUE value; // AmqpValue is returned in-place, so doesn't need to be freed.
      if (!properties_get_to(properties.m_properties, &value))
      {
        os << ", To: " << AmqpValue(value);
      }
    }

    {
      const char* value; // AmqpValue is returned in-place, so doesn't need to be freed.
      if (!properties_get_subject(properties.m_properties, &value))
      {
        os << ", Subject: " << value;
      }
    }

    {
      AMQP_VALUE value; // AmqpValue is returned in-place, so doesn't need to be freed.
      if (!properties_get_reply_to(properties.m_properties, &value))
      {
        os << ", ReplyTo: " << AmqpValue(value);
      }
    }
    {
      AMQP_VALUE value; // AmqpValue is returned in-place, so doesn't need to be freed.
      if (!properties_get_correlation_id(properties.m_properties, &value))
      {
        os << ", CorrelationId: " << AmqpValue(value);
      }
    }

    {
      const char* value; // AmqpValue is returned in-place, so doesn't need to be freed.
      if (!properties_get_content_type(properties.m_properties, &value))
      {
        os << ", ContentType: " << value;
      }
    }

    {
      const char* value; // AmqpValue is returned in-place, so doesn't need to be freed.
      if (!properties_get_content_encoding(properties.m_properties, &value))
      {
        os << ", ContentEncoding: " << value;
      }
    }

    {
      timestamp expiryTime;
      if (!properties_get_absolute_expiry_time(properties.m_properties, &expiryTime))
      {
        std::chrono::milliseconds ms{expiryTime};

        os << ", AbsoluteExpiryTime: "
           << timeToString(std::chrono::system_clock::from_time_t(0) + ms);
      }
    }

    {
      timestamp creationTime;
      if (!properties_get_creation_time(properties.m_properties, &creationTime))
      {
        std::chrono::milliseconds ms{creationTime};
        os << ", CreationTime: " << timeToString(std::chrono::system_clock::from_time_t(0) + ms);
      }
    }

    {
      const char* value; // AmqpValue is returned in-place, so doesn't need to be freed.
      if (!properties_get_group_id(properties.m_properties, &value))
      {
        os << ", GroupId: " << value;
      }
    }

    {
      uint32_t sequence;
      if (!properties_get_group_sequence(properties.m_properties, &sequence))
      {
        os << ", "
           << "GroupSequence: " << sequence;
      }
    }

    {
      const char* value; // AmqpValue is returned in-place, so doesn't need to be freed.
      if (!properties_get_reply_to_group_id(properties.m_properties, &value))
      {
        os << ", "
           << "ReplyToGroupId: " << value;
      }
    }
    os << "}";
    return os;
  }
}}}} // namespace Azure::Core::Amqp::Models
