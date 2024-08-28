// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/models/amqp_properties.hpp"

#include "azure/core/amqp/models/amqp_value.hpp"
#include "private/properties_impl.hpp"
#include "private/value_impl.hpp"

#if ENABLE_UAMQP

#include <azure_uamqp_c/amqp_definitions_sequence_no.h>

#include <azure_uamqp_c/amqp_definitions_properties.h>

#elif ENABLE_RUST_AMQP
using namespace Azure::Core::Amqp::_detail::RustInterop;
#endif // ENABLE_UAMQP
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace Azure { namespace Core { namespace Amqp { namespace _detail {

  // @cond
  void UniqueHandleHelper<PropertiesImplementation>::FreeAmqpProperties(
      PropertiesImplementation* value)
  {
    properties_destroy(value);
  }
  // @endcond
#if ENABLE_RUST_AMQP

  using PropertiesBuilderImplementation
      = Azure::Core::Amqp::_detail::RustInterop::RustMessagePropertiesBuilder;

  template <> struct UniqueHandleHelper<PropertiesBuilderImplementation>
  {
    static void FreeAmqpPropertiesBuilder(PropertiesBuilderImplementation* obj);

    using type = Core::_internal::
        BasicUniqueHandle<PropertiesBuilderImplementation, FreeAmqpPropertiesBuilder>;
  };
  void UniqueHandleHelper<PropertiesBuilderImplementation>::FreeAmqpPropertiesBuilder(
      PropertiesBuilderImplementation* obj)
  {
    properties_builder_destroy(obj);
  }

#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models {
#if ENABLE_RUST_AMQP
  namespace _detail {
    using UniqueMessagePropertiesBuilderHandle = Azure::Core::Amqp::_detail::UniqueHandle<
        Azure::Core::Amqp::_detail::PropertiesBuilderImplementation>;
  }
#endif
  MessageProperties _detail::MessagePropertiesFactory::FromImplementation(
      UniquePropertiesHandle const& properties)
  {
    MessageProperties rv;
    // properties_get_message_id returns the value in-place.
    Azure::Core::Amqp::_detail::AmqpValueImplementation* value;
    if (!properties_get_message_id(properties.get(), &value))
    {
#if ENABLE_UAMQP
      rv.MessageId = _detail::AmqpValueFactory::FromImplementation(
          _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)});
#elif ENABLE_RUST_AMQP
      rv.MessageId
          = _detail::AmqpValueFactory::FromImplementation(_detail::UniqueAmqpValueHandle{value});
#endif
    }
    if (!properties_get_correlation_id(properties.get(), &value))
    {
#if ENABLE_UAMQP
      rv.CorrelationId = _detail::AmqpValueFactory::FromImplementation(
          _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)});
#elif ENABLE_RUST_AMQP
      rv.CorrelationId
          = _detail::AmqpValueFactory::FromImplementation(_detail::UniqueAmqpValueHandle{value});
#endif
    }

    {
#if ENABLE_UAMQP
      amqp_binary binaryValue;

      if (!properties_get_user_id(properties.get(), &binaryValue))
      {
        rv.UserId = std::vector<uint8_t>(
            static_cast<const uint8_t*>(binaryValue.bytes),
            static_cast<const uint8_t*>(binaryValue.bytes) + binaryValue.length);
      }
#elif ENABLE_RUST_AMQP
      const uint8_t* user_id;
      uint32_t length;
      if (!properties_get_user_id(properties.get(), &user_id, &length))
      {
        rv.UserId = std::vector<uint8_t>(user_id, user_id + length);
      }
#endif
    }

    if (!properties_get_to(properties.get(), &value))
    {
#if ENABLE_UAMQP
      rv.To = _detail::AmqpValueFactory::FromImplementation(
          _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)});
#elif ENABLE_RUST_AMQP
      rv.To = _detail::AmqpValueFactory::FromImplementation(_detail::UniqueAmqpValueHandle{value});
#endif
    }

    const char* stringValue;
    {
      if (!properties_get_subject(properties.get(), &stringValue))
      {
        rv.Subject = stringValue;
      }
    }

    if (!properties_get_reply_to(properties.get(), &value))
    {
      rv.ReplyTo = _detail::AmqpValueFactory::FromImplementation(
          _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)});
    }

    if (!properties_get_content_type(properties.get(), &stringValue))
    {
      rv.ContentType = stringValue;
#if ENABLE_RUST_AMQP
      rust_string_delete(stringValue);
#endif
    }

    if (!properties_get_content_encoding(properties.get(), &stringValue))
    {
      rv.ContentEncoding = stringValue;
    }

#if ENABLE_UAMQP
    timestamp timestampValue;
#else
    uint64_t timestampValue;
#endif

    if (!properties_get_absolute_expiry_time(properties.get(), &timestampValue))
    {
      std::chrono::milliseconds ms{timestampValue};
      rv.AbsoluteExpiryTime = std::chrono::system_clock::time_point{
          std::chrono::duration_cast<std::chrono::system_clock::duration>(ms)};
    }

    if (!properties_get_creation_time(properties.get(), &timestampValue))
    {
      std::chrono::milliseconds ms{timestampValue};
      rv.CreationTime = std::chrono::system_clock::time_point{
          std::chrono::duration_cast<std::chrono::system_clock::duration>(ms)};
    }
    if (!properties_get_group_id(properties.get(), &stringValue))
    {
      rv.GroupId = stringValue;
    }

    uint32_t uintValue;
    if (!properties_get_group_sequence(properties.get(), &uintValue))
    {
      rv.GroupSequence = uintValue;
    }

    if (!properties_get_reply_to_group_id(properties.get(), &stringValue))
    {
      rv.ReplyToGroupId = stringValue;
#if ENABLE_RUST_AMQP
      rust_string_delete(stringValue);
#endif
    }
    return rv;
  }

  _detail::UniquePropertiesHandle _detail::MessagePropertiesFactory::ToImplementation(
      MessageProperties const& properties)
  {
#if ENABLE_UAMQP
    UniquePropertiesHandle returnValue(properties_create());
#elif ENABLE_RUST_AMQP
    _detail::UniqueMessagePropertiesBuilderHandle returnValue{properties_builder_create()};
#endif

    if (properties.MessageId.HasValue())
    {
      if (properties_set_message_id(
              returnValue.get(),
              _detail::AmqpValueFactory::ToImplementation(properties.MessageId.Value())))
      {
        throw std::runtime_error("Could not set message id");
      }
    }
    if (properties.CorrelationId.HasValue())
    {
      if (properties_set_correlation_id(
              returnValue.get(),
              _detail::AmqpValueFactory::ToImplementation(properties.CorrelationId.Value())))
      {
        throw std::runtime_error("Could not set correlation id");
      }
    }

    if (properties.UserId.HasValue())
    {
#if ENABLE_UAMQP
      amqp_binary value{
          properties.UserId.Value().data(),
          static_cast<uint32_t>(properties.UserId.Value().size())};
      if (properties_set_user_id(returnValue.get(), value))
      {
        throw std::runtime_error("Could not set user id");
      }
#elif ENABLE_RUST_AMQP
      if (properties_set_user_id(
              returnValue.get(),
              properties.UserId.Value().data(),
              static_cast<uint32_t>(properties.UserId.Value().size())))
      {
        throw std::runtime_error("Could not set user id");
      }
#endif
    }

    if (properties.To.HasValue())
    {
      if (properties_set_to(
              returnValue.get(),
              _detail::AmqpValueFactory::ToImplementation(properties.To.Value())))
      {
        throw std::runtime_error("Could not set to");
      }
    }

    if (properties.Subject.HasValue())
    {
      if (properties_set_subject(returnValue.get(), properties.Subject.Value().data()))
      {
        throw std::runtime_error("Could not set subject");
      }
    }

    if (properties.ReplyTo.HasValue())
    {
      if (properties_set_reply_to(
              returnValue.get(),
              _detail::AmqpValueFactory::ToImplementation(properties.ReplyTo.Value())))
      {
        throw std::runtime_error("Could not set reply to");
      }
    }

    if (properties.ContentType.HasValue())
    {
      if (properties_set_content_type(returnValue.get(), properties.ContentType.Value().data()))
      {
        throw std::runtime_error("Could not set content type");
      }
    }

    if (properties.ContentEncoding.HasValue())
    {
      if (properties_set_content_encoding(
              returnValue.get(), properties.ContentEncoding.Value().data()))
      {
        throw std::runtime_error("Could not set content type");
      }
    }

    if (properties.AbsoluteExpiryTime.HasValue())
    {
      auto timeStamp{std::chrono::duration_cast<std::chrono::milliseconds>(
          properties.AbsoluteExpiryTime.Value().time_since_epoch())};

      if (properties_set_absolute_expiry_time(returnValue.get(), timeStamp.count()))
      {
        throw std::runtime_error("Could not set absolute expiry time");
      }
    }

    if (properties.CreationTime.HasValue())
    {
      auto timeStamp{std::chrono::duration_cast<std::chrono::milliseconds>(
          properties.CreationTime.Value().time_since_epoch())};

      if (properties_set_creation_time(returnValue.get(), timeStamp.count()))
      {
        throw std::runtime_error("Could not set absolute expiry time");
      }
    }

    if (properties.GroupId.HasValue())
    {
      if (properties_set_group_id(returnValue.get(), properties.GroupId.Value().data()))
      {
        throw std::runtime_error("Could not set group id");
      }
    }

    if (properties.GroupSequence.HasValue())
    {
      if (properties_set_group_sequence(returnValue.get(), properties.GroupSequence.Value()))
      {
        throw std::runtime_error("Could not set group sequence");
      }
    }

    if (properties.ReplyToGroupId.HasValue())
    {
      if (properties_set_reply_to_group_id(
              returnValue.get(), properties.ReplyToGroupId.Value().data()))
      {
        throw std::runtime_error("Could not set reply-to group id");
      }
    }

#if ENABLE_RUST_AMQP
    // Now that we've set all the builder parameters, actually build the header.
    Azure::Core::Amqp::_detail::PropertiesImplementation* implementation;
    if (properties_build(returnValue.get(), &implementation))
    {
      throw std::runtime_error("Could not build header.");
    }
    return _detail::UniquePropertiesHandle{implementation};
#elif ENABLE_UAMQP
    return returnValue;
#endif
  }

  namespace {

    template <typename T> bool CompareNullable(T const& left, T const& right)
    {
      if (left.HasValue() != right.HasValue())
      {
        return false;
      }
      if (left.HasValue())
      {
        return left.Value() == right.Value();
      }
      return true;
    }
  } // namespace

  bool MessageProperties::operator==(MessageProperties const& that) const noexcept
  {
    return (
        CompareNullable(MessageId, that.MessageId)
        && CompareNullable(CorrelationId, that.CorrelationId)
        && CompareNullable(UserId, that.UserId) && CompareNullable(To, that.To)
        && CompareNullable(Subject, that.Subject) && CompareNullable(ReplyTo, that.ReplyTo)
        && CompareNullable(ContentType, that.ContentType)
        && CompareNullable(ContentEncoding, that.ContentEncoding)
        && CompareNullable(AbsoluteExpiryTime, that.AbsoluteExpiryTime)
        && CompareNullable(CreationTime, that.CreationTime)
        && CompareNullable(GroupId, that.GroupId)
        && CompareNullable(GroupSequence, that.GroupSequence)
        && CompareNullable(ReplyToGroupId, that.ReplyToGroupId));
  }

  bool MessageProperties::ShouldSerialize() const noexcept
  {
    return (
        MessageId.HasValue() || CorrelationId.HasValue() || UserId.HasValue() || To.HasValue()
        || Subject.HasValue() || ReplyTo.HasValue() || ContentType.HasValue()
        || ContentEncoding.HasValue() || AbsoluteExpiryTime.HasValue() || CreationTime.HasValue()
        || GroupId.HasValue() || GroupSequence.HasValue() || ReplyToGroupId.HasValue());
  }

  std::vector<uint8_t> MessageProperties::Serialize(MessageProperties const& properties)
  {
    auto handle = _detail::MessagePropertiesFactory::ToImplementation(properties);
    Models::_detail::UniqueAmqpValueHandle propertiesAsuAMQPValue{
        amqpvalue_create_properties(handle.get())};
    AmqpValue propertiesAsValue{
        _detail::AmqpValueFactory::FromImplementation(propertiesAsuAMQPValue)};
    return Models::AmqpValue::Serialize(propertiesAsValue);
  }

  MessageProperties MessageProperties::Deserialize(uint8_t const* data, size_t size)
  {
    AmqpValue value{AmqpValue::Deserialize(data, size)};
    Azure::Core::Amqp::_detail::PropertiesImplementation* handle;
    if (amqpvalue_get_properties(_detail::AmqpValueFactory::ToImplementation(value), &handle))
    {
      throw std::runtime_error("Could not convert value to AMQP Properties.");
    }
    _detail::UniquePropertiesHandle uniqueHandle{handle};
    handle = nullptr;
    return _detail::MessagePropertiesFactory::FromImplementation(uniqueHandle);
  }

  namespace {
    std::string timeToString(std::chrono::system_clock::time_point t)
    {
      std::time_t time = std::chrono::system_clock::to_time_t(t);
      char buf[26]{};
#ifdef _MSC_VER
#pragma warning(push)
// warning C4996: 'localtime': This function or variable may be unsafe. Consider using localtime_s
// instead.
#pragma warning(disable : 4996)
#endif
          std::strftime(buf, std::extent<decltype(buf)>::value, "%c", std::localtime(&time));
#ifdef _MSC_VER
#pragma warning(pop)
#endif
          return buf;
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
