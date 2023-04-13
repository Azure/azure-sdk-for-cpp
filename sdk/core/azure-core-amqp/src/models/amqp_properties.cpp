// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/models/amqp_properties.hpp"
#include "azure/core/amqp/models/amqp_value.hpp"

#include <azure_uamqp_c/amqp_definitions_sequence_no.h>

#include <azure_uamqp_c/amqp_definitions_properties.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace Azure { namespace Core { namespace _internal {
  void UniqueHandleHelper<PROPERTIES_INSTANCE_TAG>::FreeAmqpProperties(PROPERTIES_HANDLE value)
  {
    properties_destroy(value);
  }
}}} // namespace Azure::Core::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models {

    MessageProperties::MessageProperties() {}

    MessageProperties::~MessageProperties() {}

    MessageProperties::MessageProperties(PROPERTIES_HANDLE properties)
    {
      AMQP_VALUE value;
      // properties_get_message_id returns the value in-place.
      if (!properties_get_message_id(properties, &value))
      {
        MessageId = value;
      }

      if (!properties_get_correlation_id(properties, &value))
      {
        CorrelationId = value;
      }

      {
        amqp_binary binaryValue;

        if (!properties_get_user_id(properties, &binaryValue))
        {
          UserId = std::vector<uint8_t>(
              static_cast<const uint8_t*>(binaryValue.bytes),
              static_cast<const uint8_t*>(binaryValue.bytes) + binaryValue.length);
        }
      }

      if (!properties_get_to(properties, &value))
      {
        To = value;
      }

      const char* stringValue;
      {
        if (!properties_get_subject(properties, &stringValue))
        {
          Subject = stringValue;
        }
      }

      if (!properties_get_reply_to(properties, &value))
      {
        ReplyTo = value;
      }

      if (!properties_get_content_type(properties, &stringValue))
      {
        ContentType = stringValue;
      }

      if (!properties_get_content_encoding(properties, &stringValue))
      {
        ContentEncoding = stringValue;
      }

      timestamp timestampValue;
      if (!properties_get_absolute_expiry_time(properties, &timestampValue))
      {
        std::chrono::milliseconds ms{timestampValue};
        AbsoluteExpiryTime = std::chrono::system_clock::time_point{
            std::chrono::duration_cast<std::chrono::system_clock::duration>(ms)};
      }

      if (!properties_get_creation_time(properties, &timestampValue))
      {
        std::chrono::milliseconds ms{timestampValue};
        CreationTime = std::chrono::system_clock::time_point{
            std::chrono::duration_cast<std::chrono::system_clock::duration>(ms)};
      }
      if (!properties_get_group_id(properties, &stringValue))
      {
        GroupId = stringValue;
      }

      uint32_t uintValue;
      if (!properties_get_group_sequence(properties, &uintValue))
      {
        GroupSequence = uintValue;
      }

      if (!properties_get_reply_to_group_id(properties, &stringValue))
      {
        ReplyToGroupId = stringValue;
      }
    }

    MessageProperties::operator PROPERTIES_HANDLE() const
    {
      Azure::Core::_internal::UniqueHandle<PROPERTIES_INSTANCE_TAG> returnValue(
          properties_create());
      if (MessageId.HasValue())
      {
        if (properties_set_message_id(returnValue.get(), MessageId.Value()))
        {
          throw std::runtime_error("Could not set message id"); // LCOV_EXCL_LINE
        }
      }
      if (CorrelationId.HasValue())
      {
        if (properties_set_correlation_id(returnValue.get(), CorrelationId.Value()))
        {
          throw std::runtime_error("Could not set correlation id"); // LCOV_EXCL_LINE
        }
      }

      if (UserId.HasValue())
      {
        amqp_binary value{UserId.Value().data(), static_cast<uint32_t>(UserId.Value().size())};
        if (properties_set_user_id(returnValue.get(), value))
        {
          throw std::runtime_error("Could not set user id"); // LCOV_EXCL_LINE
        }
      }

      if (To.HasValue())
      {
        if (properties_set_to(returnValue.get(), To.Value()))
        {
          throw std::runtime_error("Could not set to"); // LCOV_EXCL_LINE
        }
      }

      if (Subject.HasValue())
      {
        if (properties_set_subject(returnValue.get(), Subject.Value().data()))
        {
          throw std::runtime_error("Could not set subject");
        }
      }

      if (ReplyTo.HasValue())
      {
        if (properties_set_reply_to(returnValue.get(), ReplyTo.Value()))
        {
          throw std::runtime_error("Could not set reply to"); // LCOV_EXCL_LINE
        }
      }

      if (ContentType.HasValue())
      {
        if (properties_set_content_type(returnValue.get(), ContentType.Value().data()))
        {
          throw std::runtime_error("Could not set content type"); // LCOV_EXCL_LINE
        }
      }

      if (ContentEncoding.HasValue())
      {
        if (properties_set_content_encoding(returnValue.get(), ContentEncoding.Value().data()))
        {
          throw std::runtime_error("Could not set content type");
        }
      }

      if (AbsoluteExpiryTime.HasValue())
      {
        auto timeStamp{std::chrono::duration_cast<std::chrono::milliseconds>(
            AbsoluteExpiryTime.Value().time_since_epoch())};

        if (properties_set_absolute_expiry_time(returnValue.get(), timeStamp.count()))
        {
          throw std::runtime_error("Could not set absolute expiry time"); // LCOV_EXCL_LINE
        }
      }

      if (CreationTime.HasValue())
      {
        auto timeStamp{std::chrono::duration_cast<std::chrono::milliseconds>(
            CreationTime.Value().time_since_epoch())};

        if (properties_set_creation_time(returnValue.get(), timeStamp.count()))
        {
          throw std::runtime_error("Could not set absolute expiry time"); // LCOV_EXCL_LINE
        }
      }

      if (GroupId.HasValue())
      {
        if (properties_set_group_id(returnValue.get(), GroupId.Value().data()))
        {
          throw std::runtime_error("Could not set group id"); // LCOV_EXCL_LINE
        }
      }

      if (GroupSequence.HasValue())
      {
        if (properties_set_group_sequence(returnValue.get(), GroupSequence.Value()))
        {
          throw std::runtime_error("Could not set group sequence"); // LCOV_EXCL_LINE
        }
      }

      if (ReplyToGroupId.HasValue())
      {
        if (properties_set_reply_to_group_id(returnValue.get(), ReplyToGroupId.Value().data()))
        {
          throw std::runtime_error("Could not set reply-to group id");
        }
      }

      return returnValue.release();
    }

    namespace {
      std::string timeToString(std::chrono::system_clock::time_point t)
      {
        std::time_t time = std::chrono::system_clock::to_time_t(t);
        std::string time_str = std::ctime(&time);
        time_str.resize(time_str.size() - 1);
        return time_str;
      }

      size_t LogRawData(std::ostream& os, size_t startOffset, const uint8_t* const pb, size_t cb)
      {
        // scratch buffer which will hold the data being logged.
        std::stringstream ss;

        size_t bytesToWrite = (cb < 0x10 ? cb : 0x10);

        ss << std::hex << std::right << std::setw(8) << std::setfill('0') << startOffset << ": ";

        // Write the buffer data out.
        for (size_t i = 0; i < bytesToWrite; i += 1)
        {
          ss << std::hex << std::right << std::setw(2) << std::setfill('0')
             << static_cast<int>(pb[i]) << " ";
        }

        // Now write the data in string format (similar to what the debugger does).
        // Start by padding partial lines to a fixed end.
        for (size_t i = bytesToWrite; i < 0x10; i += 1)
        {
          ss << "   ";
        }
        ss << "  * ";
        for (size_t i = 0; i < bytesToWrite; i += 1)
        {
          if (isprint(pb[i]))
          {
            ss << pb[i];
          }
          else
          {
            ss << ".";
          }
        }
        for (size_t i = bytesToWrite; i < 0x10; i += 1)
        {
          ss << " ";
        }

        ss << " *";

        os << ss.str();

        return bytesToWrite;
      }
    } // namespace

    std::ostream& operator<<(std::ostream& os, MessageProperties const& properties)
    {
      os << "MessageProperties {";
      {
        if (properties.MessageId.HasValue())
        {
          os << "MessageId: " << properties.MessageId.Value();
        }
        else
        {
          os << "MessageId: <null>";
        }
      }
      {
        if (properties.UserId.HasValue())
        {
          os << ", UserId: ";
          const uint8_t* pb = properties.UserId.Value().data();
          size_t cb = properties.UserId.Value().size();
          size_t currentOffset = 0;
          do
          {
            auto cbLogged = LogRawData(os, currentOffset, pb, cb);
            pb += cbLogged;
            cb -= cbLogged;
            currentOffset += cbLogged;
            if (cb)
            {
              os << std::endl;
            }
          } while (cb);
        }
      }
      if (properties.To.HasValue())
      {
        os << ", To: " << properties.To.Value();
      }

      if (properties.Subject.HasValue())
      {
        os << ", Subject: " << properties.Subject.Value();
      }

      if (properties.ReplyTo.HasValue())
      {
        os << ", ReplyTo: " << properties.ReplyTo.Value();
      }
      if (properties.CorrelationId.HasValue())
      {
        os << ", CorrelationId: " << properties.CorrelationId.Value();
      }

      if (properties.ContentType.HasValue())
      {
        os << ", ContentType: " << properties.ContentType.Value();
      }

      if (properties.ContentEncoding.HasValue())
      {
        os << ", ContentEncoding: " << properties.ContentEncoding.Value();
      }

      if (properties.AbsoluteExpiryTime.HasValue())
      {
        os << ", AbsoluteExpiryTime: " << timeToString(properties.AbsoluteExpiryTime.Value());
      }
      if (properties.CreationTime.HasValue())
      {
        os << ", CreationTime: " << timeToString(properties.CreationTime.Value());
      }
      if (properties.GroupId.HasValue())
      {
        os << ", GroupId: " << properties.GroupId.Value();
      }
      if (properties.GroupSequence.HasValue())
      {
        os << ", GroupSequence: " << properties.GroupSequence.Value();
      }

      if (properties.ReplyToGroupId.HasValue())
      {
        os << ", ReplyToGroupId: " << properties.ReplyToGroupId.Value();
      }
      os << "}";
      return os;
    }
}}}} // namespace Azure::Core::Amqp::Models
