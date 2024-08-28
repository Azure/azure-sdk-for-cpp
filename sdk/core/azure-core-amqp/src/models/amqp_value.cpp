// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/core/amqp/models/amqp_value.hpp"

#include "../src/models/private/value_impl.hpp"
#include "azure/core/amqp/internal/doxygen_pragma.hpp"
#include "azure/core/amqp/internal/models/amqp_protocol.hpp"
#include "azure/core/amqp/models/amqp_properties.hpp"

#include <azure/core/internal/diagnostics/log.hpp>

#if ENABLE_UAMQP
#include <azure_uamqp_c/amqp_definitions_milliseconds.h>
#include <azure_uamqp_c/amqp_definitions_sequence_no.h>

#include <azure_uamqp_c/amqp_definitions_header.h>
#include <azure_uamqp_c/amqp_definitions_properties.h>
#include <azure_uamqp_c/amqpvalue.h>
#include <azure_uamqp_c/amqpvalue_to_string.h>
#endif
#if ENABLE_RUST_AMQP
#include "../../rust_amqp/rust_wrapper/rust_amqp_wrapper.h"

using namespace Azure::Core::Amqp::_detail::RustInterop;
#endif

#include <iomanip>
#include <iostream>
#include <sstream>

using namespace Azure::Core::Amqp::Models::_detail;
using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  /// @cond INTERNAL
  void UniqueHandleHelper<AmqpValueImplementation>::FreeAmqpValue(AmqpValueImplementation* value)
  {
    amqpvalue_destroy(value);
  }
  /// @endcond

#if ENABLE_UAMQP
  /// @cond INTERNAL
  void UniqueHandleHelper<std::remove_pointer<AMQPVALUE_DECODER_HANDLE>::type>::FreeAmqpDecoder(
      AMQPVALUE_DECODER_HANDLE value)
  {
    amqpvalue_decoder_destroy(value);
  }
  /// @endcond
#endif
}}}} // namespace Azure::Core::Amqp::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models {
  namespace _detail {
    std::ostream& operator<<(
        std::ostream& os,
        Azure::Core::Amqp::_detail::AmqpValueImplementationType value)
    {
      switch (value)
      {
#if ENABLE_UAMQP
        case AMQP_TYPE_INVALID:
          os << "AMQP_TYPE_INVALID";
          break;
        case AMQP_TYPE_NULL:
          os << "AMQP_TYPE_NULL";
          break;
        case AMQP_TYPE_BOOL:
          os << "AMQP_TYPE_BOOL";
          break;
        case AMQP_TYPE_UBYTE:
          os << "AMQP_TYPE_UBYTE";
          break;
        case AMQP_TYPE_USHORT:
          os << "AMQP_TYPE_USHORT";
          break;
        case AMQP_TYPE_UINT:
          os << "AMQP_TYPE_UINT";
          break;
        case AMQP_TYPE_ULONG:
          os << "AMQP_TYPE_ULONG";
          break;
        case AMQP_TYPE_BYTE:
          os << "AMQP_TYPE_BYTE";
          break;
        case AMQP_TYPE_SHORT:
          os << "AMQP_TYPE_SHORT";
          break;
        case AMQP_TYPE_INT:
          os << "AMQP_TYPE_INT";
          break;
        case AMQP_TYPE_LONG:
          os << "AMQP_TYPE_LONG";
          break;
        case AMQP_TYPE_FLOAT:
          os << "AMQP_TYPE_FLOAT";
          break;
        case AMQP_TYPE_DOUBLE:
          os << "AMQP_TYPE_DOUBLE";
          break;
        case AMQP_TYPE_CHAR:
          os << "AMQP_TYPE_CHAR";
          break;
        case AMQP_TYPE_TIMESTAMP:
          os << "AMQP_TYPE_TIMESTAMP";
          break;
        case AMQP_TYPE_UUID:
          os << "AMQP_TYPE_UUID";
          break;
        case AMQP_TYPE_BINARY:
          os << "AMQP_TYPE_BINARY";
          break;
        case AMQP_TYPE_STRING:
          os << "AMQP_TYPE_STRING";
          break;
        case AMQP_TYPE_SYMBOL:
          os << "AMQP_TYPE_SYMBOL";
          break;
        case AMQP_TYPE_LIST:
          os << "AMQP_TYPE_LIST";
          break;
        case AMQP_TYPE_MAP:
          os << "AMQP_TYPE_MAP";
          break;
        case AMQP_TYPE_ARRAY:
          os << "AMQP_TYPE_ARRAY";
          break;
        case AMQP_TYPE_DESCRIBED:
          os << "AMQP_TYPE_DESCRIBED";
          break;
        case AMQP_TYPE_COMPOSITE:
          os << "AMQP_TYPE_COMPOSITE";
          break;
        case AMQP_TYPE_UNKNOWN:
          os << "AMQP_TYPE_UNKNOWN";
          break;
#elif ENABLE_RUST_AMQP
        case RustAmqpValueType::AmqpValueInvalid:
          os << "AMQP_TYPE_INVALID";
          break;
        case RustAmqpValueType::AmqpValueNull:
          os << "AMQP_TYPE_NULL";
          break;
        case RustAmqpValueType::AmqpValueBoolean:
          os << "AMQP_TYPE_BOOL";
          break;
        case RustAmqpValueType::AmqpValueUByte:
          os << "AMQP_TYPE_UBYTE";
          break;
        case RustAmqpValueType::AmqpValueUShort:
          os << "AMQP_TYPE_USHORT";
          break;
        case RustAmqpValueType::AmqpValueUint:
          os << "AMQP_TYPE_UINT";
          break;
        case RustAmqpValueType::AmqpValueUlong:
          os << "AMQP_TYPE_ULONG";
          break;
        case RustAmqpValueType::AmqpValueByte:
          os << "AMQP_TYPE_BYTE";
          break;
        case RustAmqpValueType::AmqpValueShort:
          os << "AMQP_TYPE_SHORT";
          break;
        case RustAmqpValueType::AmqpValueInt:
          os << "AMQP_TYPE_INT";
          break;
        case RustAmqpValueType::AmqpValueLong:
          os << "AMQP_TYPE_LONG";
          break;
        case RustAmqpValueType::AmqpValueFloat:
          os << "AMQP_TYPE_FLOAT";
          break;
        case RustAmqpValueType::AmqpValueDouble:
          os << "AMQP_TYPE_DOUBLE";
          break;
        case RustAmqpValueType::AmqpValueChar:
          os << "AMQP_TYPE_CHAR";
          break;
        case RustAmqpValueType::AmqpValueTimestamp:
          os << "AMQP_TYPE_TIMESTAMP";
          break;
        case RustAmqpValueType::AmqpValueUuid:
          os << "AMQP_TYPE_UUID";
          break;
        case RustAmqpValueType::AmqpValueBinary:
          os << "AMQP_TYPE_BINARY";
          break;
        case RustAmqpValueType::AmqpValueString:
          os << "AMQP_TYPE_STRING";
          break;
        case RustAmqpValueType::AmqpValueSymbol:
          os << "AMQP_TYPE_SYMBOL";
          break;
        case RustAmqpValueType::AmqpValueList:
          os << "AMQP_TYPE_LIST";
          break;
        case RustAmqpValueType::AmqpValueMap:
          os << "AMQP_TYPE_MAP";
          break;
        case RustAmqpValueType::AmqpValueArray:
          os << "AMQP_TYPE_ARRAY";
          break;
        case RustAmqpValueType::AmqpValueDescribed:
          os << "AMQP_TYPE_DESCRIBED";
          break;
        case RustAmqpValueType::AmqpValueComposite:
          os << "AMQP_TYPE_COMPOSITE";
          break;
        case RustAmqpValueType::AmqpValueUnknown:
          os << "AMQP_TYPE_UNKNOWN";
          break;
#endif
      }
      return os;
    }

    std::ostream& operator<<(
        std::ostream& os,
        Azure::Core::Amqp::_detail::AmqpValueImplementation* const value)
    {
      if (value != nullptr)
      {
        os << "AMQP_VALUE: " << static_cast<void*>(value) << " " << amqpvalue_get_type(value)
           << ": ";
        char* valueAsString{amqpvalue_to_string(value)};
        os << valueAsString;
        free(valueAsString);
      }
      else
      {
        os << "AMQP_VALUE: nullptr";
      }
      return os;
    }
  } // namespace _detail

  std::ostream& operator<<(std::ostream& os, AmqpValueType value)
  {
    switch (value)
    {
      case AmqpValueType::Invalid:
        os << "Invalid";
        break;
      case AmqpValueType::Null:
        os << "Null";
        break;
      case AmqpValueType::Bool:
        os << "Bool";
        break;
      case AmqpValueType::Ubyte:
        os << "Ubyte";
        break;
      case AmqpValueType::Ushort:
        os << "Ushort";
        break;
      case AmqpValueType::Uint:
        os << "Uint";
        break;
      case AmqpValueType::Ulong:
        os << "Ulong";
        break;
      case AmqpValueType::Byte:
        os << "Byte";
        break;
      case AmqpValueType::Short:
        os << "Short";
        break;
      case AmqpValueType::Int:
        os << "Int";
        break;
      case AmqpValueType::Long:
        os << "Long";
        break;
      case AmqpValueType::Float:
        os << "Float";
        break;
      case AmqpValueType::Double:
        os << "Double";
        break;
      case AmqpValueType::Char:
        os << "Char";
        break;
      case AmqpValueType::Timestamp:
        os << "Timestamp";
        break;
      case AmqpValueType::Uuid:
        os << "Uuid";
        break;
      case AmqpValueType::Binary:
        os << "Binary";
        break;
      case AmqpValueType::String:
        os << "String";
        break;
      case AmqpValueType::Symbol:
        os << "Symbol";
        break;
      case AmqpValueType::List:
        os << "List";
        break;
      case AmqpValueType::Map:
        os << "Map";
        break;
      case AmqpValueType::Array:
        os << "Array";
        break;
      case AmqpValueType::Described:
        os << "Described";
        break;
      case AmqpValueType::Composite:
        os << "Composite";
        break;
      case AmqpValueType::Unknown:
        os << "Unknown";
        break;
    }

    return os;
  }

  AmqpValue::~AmqpValue() {}

  AmqpValue::AmqpValue(bool bool_value)
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_boolean(bool_value)})}
  {
  }
  AmqpValue::AmqpValue(std::uint8_t byte_value)
      : m_impl{std::make_unique<AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_ubyte(byte_value)})}
  {
  }
  AmqpValue::AmqpValue(char value)
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_byte(value)})}
  {
  }
  AmqpValue::AmqpValue(std::int8_t value)
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_byte(value)})}
  {
  }
  AmqpValue::AmqpValue(std::uint16_t value)
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_ushort(value)})}
  {
  }
  AmqpValue::AmqpValue(std::int16_t value)
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_short(value)})}
  {
  }
  AmqpValue::AmqpValue(std::uint32_t value)
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_uint(value)})}
  {
  }
  AmqpValue::AmqpValue(std::int32_t value)
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_int(value)})}
  {
  }
  AmqpValue::AmqpValue(std::uint64_t value)
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_ulong(value)})}
  {
  }
  AmqpValue::AmqpValue(std::int64_t value)
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_long(value)})}
  {
  }
  AmqpValue::AmqpValue(float value)
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_float(value)})}
  {
  }
  AmqpValue::AmqpValue(double value)
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_double(value)})}
  {
  }
  AmqpValue::AmqpValue(char32_t value)
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_char(value)})}
  {
  }
  AmqpValue::AmqpValue(Azure::Core::Uuid const& uuid)
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(_detail::UniqueAmqpValueHandle{amqpvalue_create_uuid(
        const_cast<unsigned char*>(static_cast<const unsigned char*>(uuid.AsArray().data())))})
  }
#endif
  {
#if ENABLE_RUST_AMQP
    uint8_t uuidArray[16];
    std::copy(uuid.AsArray().begin(), uuid.AsArray().end(), uuidArray);
    m_impl = std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_uuid(&uuidArray)});

#endif
  }

  AmqpValue::AmqpValue(std::string const& value)
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_string(value.c_str())})}
  {
  }

  AmqpValue::AmqpValue(const char* value)
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_string(value)})}
  {
  }

  AmqpValue::AmqpValue() noexcept
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_create_null()})}
  {
  }

  AmqpValue::AmqpValue(AmqpValue const& that) noexcept
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_clone(*that.m_impl)})}
  {
  }

  AmqpValue::AmqpValue(AmqpValue&& that) noexcept : m_impl{std::move(that.m_impl)} {}

  // Construct a new AmqpValueImpl and move the value from the input into it.

  AmqpValue::AmqpValue(std::unique_ptr<AmqpValueImpl>&& impl) : m_impl{std::move(impl)} {}

  AmqpValue& AmqpValue::operator=(AmqpValue const& that)
  {
    m_impl = std::make_unique<_detail::AmqpValueImpl>(*that.m_impl);
    return *this;
  }
  AmqpValue& AmqpValue::operator=(AmqpValue&& that) noexcept
  {
    m_impl = std::move(that.m_impl);
    return *this;
  }

  AmqpValue::operator bool() const
  {
    bool value = {};
    if (amqpvalue_get_boolean(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve boolean value");
    }
    return value;
  }

  AmqpValue::operator std::uint8_t() const
  {
    unsigned char value = {};
    if (amqpvalue_get_ubyte(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve ubyte value");
    }
    return value;
  }

  AmqpValue::operator std::int8_t() const
  {
#if ENABLE_UAMQP
    char value;
#elif ENABLE_RUST_AMQP
    int8_t value;
#endif
    if (amqpvalue_get_byte(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve byte value");
    }
    return value;
  }

  AmqpValue::operator char() const
  {
#if ENABLE_UAMQP
    char value;
#elif ENABLE_RUST_AMQP
    int8_t value;
#endif
    if (amqpvalue_get_byte(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve byte value");
    }
    return value;
  }

  AmqpValue::operator std::uint16_t() const
  {
    uint16_t value = {};
    if (amqpvalue_get_ushort(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve ushort value");
    }
    return value;
  }

  AmqpValue::operator std::int16_t() const
  {
    int16_t value = {};
    if (amqpvalue_get_short(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve short value");
    }
    return value;
  }

  AmqpValue::operator std::uint32_t() const
  {
    std::uint32_t value = {};
    if (amqpvalue_get_uint(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve uint value");
    }
    return value;
  }

  AmqpValue::operator std::int32_t() const
  {
    int32_t value = {};
    if (amqpvalue_get_int(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve int value");
    }
    return value;
  }

  AmqpValue::operator std::uint64_t() const
  {
    uint64_t value = {};
    if (amqpvalue_get_ulong(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve ulong value");
    }
    return value;
  }

  AmqpValue::operator std::int64_t() const
  {
    int64_t value = {};
    if (amqpvalue_get_long(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve long value");
    }
    return value;
  }

  AmqpValue::operator float() const
  {
    float value = {};
    if (amqpvalue_get_float(*m_impl, &value))
    {
      throw std::runtime_error("Could not retrieve float value");
    }
    return value;
  }

  AmqpValue::operator double() const
  {
    double value = {};
    if (amqpvalue_get_double(*m_impl, &value))
    {
      throw std::runtime_error("Could not retrieve double value");
    }
    return value;
  }

  AmqpValue::operator char32_t() const
  {
    std::uint32_t value = {};
    if (amqpvalue_get_char(*m_impl, &value))
    {
      throw std::runtime_error("Could not get character.");
    }
    return value;
  }

  AmqpValue::operator std::string() const
  {
    const char* value;
    if (amqpvalue_get_string(*m_impl, &value))
    {
      throw std::runtime_error("Could not retrieve string value");
    }
#if ENABLE_UAMQP
    return value;

#elif ENABLE_RUST_AMQP
    std::string rv = value;
    rust_string_delete(value);
    return rv;
#endif
  }

  AmqpValue::operator Azure::Core::Uuid() const
  {
#if ENABLE_UAMQP
    uuid value;
    if (amqpvalue_get_uuid(*m_impl, &value))
    {
      throw std::runtime_error("Could not retrieve uuid value");
    }
#elif ENABLE_RUST_AMQP
    uint8_t value[16];
    if (amqpvalue_get_uuid(*m_impl, &value))
    {
      throw std::runtime_error("Could not retrieve uuid value");
    }
#endif
    std::array<uint8_t, 16> uuidArray{};
    memcpy(uuidArray.data(), value, 16);
    return Azure::Core::Uuid::CreateFromArray(uuidArray);
  }

  bool AmqpValue::operator==(AmqpValue const& that) const
  {
    // If both values are null, they are equal.
    if (IsNull() && that.IsNull())
    {
      return true;
    }
    // If only one of the values is null, they are not equal.
    if (IsNull() || that.IsNull())
    {
      return false;
    }
    // If the types are not equal, they are not equal.
    if (GetType() != that.GetType())
    {
      return false;
    }
    // The uAMQP function `amqpvalue_are_equal` does not work for all types, so we need to do
    // some special handling for those which are not handled properly.
    if (GetType() == AmqpValueType::Composite || GetType() == AmqpValueType::Described)
    {
      if (GetType() == AmqpValueType::Composite)
      {
        AmqpComposite thisComposite{this->AsComposite()};
        AmqpComposite thatComposite{that.AsComposite()};
        return (thisComposite == thatComposite);
      }
      else
      {
        AmqpDescribed thisDescribed{this->AsDescribed()};
        AmqpDescribed thatDescribed{that.AsDescribed()};
        return (thisDescribed == thatDescribed);
      }
    }
    return amqpvalue_are_equal(*m_impl, *that.m_impl);
  }

  bool AmqpValue::operator<(AmqpValue const& that) const
  {
    if (GetType() != that.GetType())
    {
      // If the types don't match, use the numeric type ordering to compare the types.
      return GetType() < that.GetType();
    }
    switch (GetType())
    {
      case AmqpValueType::Null:
        return false;
      case AmqpValueType::Bool:
        return static_cast<bool>(*this) < static_cast<bool>(that);
      case AmqpValueType::Ubyte:
        return static_cast<uint8_t>(*this) < static_cast<uint8_t>(that);
      case AmqpValueType::Ushort:
        return static_cast<uint16_t>(*this) < static_cast<uint16_t>(that);
      case AmqpValueType::Uint:
        return static_cast<uint32_t>(*this) < static_cast<uint32_t>(that);
      case AmqpValueType::Ulong:
        return static_cast<uint64_t>(*this) < static_cast<uint64_t>(that);
      case AmqpValueType::Byte:
        return static_cast<std::int8_t>(*this) < static_cast<std::int8_t>(that);
      case AmqpValueType::Short:
        return static_cast<int16_t>(*this) < static_cast<int16_t>(that);
      case AmqpValueType::Int:
        return static_cast<int32_t>(*this) < static_cast<int32_t>(that);
      case AmqpValueType::Long:
        return static_cast<int64_t>(*this) < static_cast<int64_t>(that);
      case AmqpValueType::Float:
        return static_cast<float>(*this) < static_cast<float>(that);
      case AmqpValueType::Double:
        return static_cast<double>(*this) < static_cast<double>(that);
      case AmqpValueType::Char:
        return static_cast<char32_t>(*this) < static_cast<char32_t>(that);
      case AmqpValueType::String:
        return static_cast<std::string>(*this) < static_cast<std::string>(that);
      case AmqpValueType::Symbol:
        return AsSymbol() < that.AsSymbol();

      case AmqpValueType::Map:
        return AsMap() < that.AsMap();
      case AmqpValueType::Array:
        return AsArray() < that.AsArray();
      case AmqpValueType::Timestamp:
        return AsTimestamp() < that.AsTimestamp();
      case AmqpValueType::Uuid:
        return static_cast<Azure::Core::Uuid>(*this).AsArray()
            < static_cast<Azure::Core::Uuid>(that).AsArray();
      case AmqpValueType::Binary:
        return AsBinary() < that.AsBinary();
      case AmqpValueType::List:
        return AsList() < that.AsList();
      case AmqpValueType::Described:
        return AsDescribed() < that.AsDescribed();
      case AmqpValueType::Composite:
        return AsComposite() < that.AsComposite();
      default:
        throw std::logic_error("Unknown Amqp Value type in operator<");
    }
  }

  AmqpMap AmqpValue::AsMap() const { return AmqpMap(*this); }

  AmqpArray AmqpValue::AsArray() const { return AmqpArray(*this); }

  AmqpSymbol AmqpValue::AsSymbol() const { return AmqpSymbol(*this); }

  AmqpComposite AmqpValue::AsComposite() const { return AmqpComposite(*this); }
  AmqpList AmqpValue::AsList() const { return AmqpList(*this); }
  AmqpBinaryData AmqpValue::AsBinary() const { return AmqpBinaryData(*this); }
  AmqpDescribed AmqpValue::AsDescribed() const { return AmqpDescribed(*this); }
  AmqpTimestamp AmqpValue::AsTimestamp() const { return AmqpTimestamp(*this); }

  namespace {
#if ENABLE_UAMQP
    const std::map<AMQP_TYPE, AmqpValueType> ImplTypeToAmqpTypeMap{
        {AMQP_TYPE_INVALID, AmqpValueType::Invalid},
        {AMQP_TYPE_NULL, AmqpValueType::Null},
        {AMQP_TYPE_BOOL, AmqpValueType::Bool},
        {AMQP_TYPE_UBYTE, AmqpValueType::Ubyte},
        {AMQP_TYPE_USHORT, AmqpValueType::Ushort},
        {AMQP_TYPE_UINT, AmqpValueType::Uint},
        {AMQP_TYPE_ULONG, AmqpValueType::Ulong},
        {AMQP_TYPE_BYTE, AmqpValueType::Byte},
        {AMQP_TYPE_SHORT, AmqpValueType::Short},
        {AMQP_TYPE_INT, AmqpValueType::Int},
        {AMQP_TYPE_LONG, AmqpValueType::Long},
        {AMQP_TYPE_FLOAT, AmqpValueType::Float},
        {AMQP_TYPE_DOUBLE, AmqpValueType::Double},
        {AMQP_TYPE_CHAR, AmqpValueType::Char},
        {AMQP_TYPE_TIMESTAMP, AmqpValueType::Timestamp},
        {AMQP_TYPE_UUID, AmqpValueType::Uuid},
        {AMQP_TYPE_BINARY, AmqpValueType::Binary},
        {AMQP_TYPE_STRING, AmqpValueType::String},
        {AMQP_TYPE_SYMBOL, AmqpValueType::Symbol},
        {AMQP_TYPE_LIST, AmqpValueType::List},
        {AMQP_TYPE_MAP, AmqpValueType::Map},
        {AMQP_TYPE_ARRAY, AmqpValueType::Array},
        {AMQP_TYPE_COMPOSITE, AmqpValueType::Composite},
        {AMQP_TYPE_DESCRIBED, AmqpValueType::Described},
        {AMQP_TYPE_UNKNOWN, AmqpValueType::Unknown},
    };
#elif ENABLE_RUST_AMQP
    const std::map<RustAmqpValueType, AmqpValueType> ImplTypeToAmqpTypeMap{
        {RustAmqpValueType::AmqpValueInvalid, AmqpValueType::Invalid},
        {RustAmqpValueType::AmqpValueNull, AmqpValueType::Null},
        {RustAmqpValueType::AmqpValueBoolean, AmqpValueType::Bool},
        {RustAmqpValueType::AmqpValueUByte, AmqpValueType::Ubyte},
        {RustAmqpValueType::AmqpValueUShort, AmqpValueType::Ushort},
        {RustAmqpValueType::AmqpValueUint, AmqpValueType::Uint},
        {RustAmqpValueType::AmqpValueUlong, AmqpValueType::Ulong},
        {RustAmqpValueType::AmqpValueByte, AmqpValueType::Byte},
        {RustAmqpValueType::AmqpValueShort, AmqpValueType::Short},
        {RustAmqpValueType::AmqpValueInt, AmqpValueType::Int},
        {RustAmqpValueType::AmqpValueLong, AmqpValueType::Long},
        {RustAmqpValueType::AmqpValueFloat, AmqpValueType::Float},
        {RustAmqpValueType::AmqpValueDouble, AmqpValueType::Double},
        {RustAmqpValueType::AmqpValueChar, AmqpValueType::Char},
        {RustAmqpValueType::AmqpValueTimestamp, AmqpValueType::Timestamp},
        {RustAmqpValueType::AmqpValueUuid, AmqpValueType::Uuid},
        {RustAmqpValueType::AmqpValueBinary, AmqpValueType::Binary},
        {RustAmqpValueType::AmqpValueString, AmqpValueType::String},
        {RustAmqpValueType::AmqpValueSymbol, AmqpValueType::Symbol},
        {RustAmqpValueType::AmqpValueList, AmqpValueType::List},
        {RustAmqpValueType::AmqpValueMap, AmqpValueType::Map},
        {RustAmqpValueType::AmqpValueArray, AmqpValueType::Array},
        {RustAmqpValueType::AmqpValueComposite, AmqpValueType::Composite},
        {RustAmqpValueType::AmqpValueDescribed, AmqpValueType::Described},
        {RustAmqpValueType::AmqpValueUnknown, AmqpValueType::Unknown},
    };
#endif

#if ENABLE_UAMQP
    class AmqpValueDeserializer final {
    public:
      AmqpValueDeserializer() : m_decoder{amqpvalue_decoder_create(OnAmqpValueDecoded, this)} {}

      AmqpValue operator()(std::uint8_t const* data, size_t size) const
      {
        if (amqpvalue_decode_bytes(m_decoder.get(), data, size))
        {
          throw std::runtime_error("Could not decode object");
        }
        return m_decodedValue;
      }

    private:
      AmqpValue m_decodedValue;
      _detail::UniqueAmqpDecoderHandle m_decoder;
      static void OnAmqpValueDecoded(void* context, AMQP_VALUE value)
      {
        auto deserializer = static_cast<AmqpValueDeserializer*>(context);
        deserializer->m_decodedValue = _detail::AmqpValueFactory::FromImplementation(
            _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)});
      }
    };

    class AmqpValueSerializer final {
    public:
      AmqpValueSerializer() = default;

      std::vector<uint8_t> operator()(AmqpValue const& value)
      {
        if (amqpvalue_encode(
                _detail::AmqpValueFactory::ToImplementation(value), OnAmqpValueEncoded, this))
        {
          throw std::runtime_error("Could not encode object");
        }
        (void)value;
        return m_encodedValue;
      }

    private:
      std::vector<uint8_t> m_encodedValue;

      // The OnAmqpValueEncoded callback appends the array provided to the existing encoded
      // value, extending as needed.
      //
      // Returns 0 if successful, 1 otherwise.
      static int OnAmqpValueEncoded(void* context, unsigned char const* bytes, size_t length)
      {
        auto serializer = static_cast<AmqpValueSerializer*>(context);
        serializer->m_encodedValue.insert(serializer->m_encodedValue.end(), bytes, bytes + length);
        return 0;
      }
    };
#endif

  } // namespace
  AmqpValueType AmqpValue::GetType() const
  {
    auto val{ImplTypeToAmqpTypeMap.find(amqpvalue_get_type(*m_impl))};
    if (val == ImplTypeToAmqpTypeMap.end())
    {
      throw std::runtime_error("Unknown AMQP AmqpValue Type");
    }
    return val->second;
  }

  std::ostream& operator<<(std::ostream& os, AmqpValue const& value)
  {
    char* valueAsString{amqpvalue_to_string(_detail::AmqpValueFactory::ToImplementation(value))};
    os << valueAsString;
#if ENABLE_UAMQP
    free(valueAsString);
#elif ENABLE_RUST_AMQP
    rust_string_delete(valueAsString);
#endif
    return os;
  }

  AmqpValue AmqpValue::Deserialize(uint8_t const* data, size_t size)
  {
#if ENABLE_UAMQP
    return AmqpValueDeserializer{}(data, size);
#elif ENABLE_RUST_AMQP
    Azure::Core::Amqp::_detail::AmqpValueImplementation* value;
    if (amqpvalue_decode_bytes(data, size, &value))
    {
      throw std::runtime_error("Could not decode object");
    }
    return _detail::AmqpValueFactory::FromImplementation(UniqueAmqpValueHandle{value});
#endif
  }

  std::vector<uint8_t> AmqpValue::Serialize(AmqpValue const& value)
  {
#if ENABLE_UAMQP
    return AmqpValueSerializer{}(value);
#elif ENABLE_RUST_AMQP
    size_t encodedSize;
    if (amqpvalue_get_encoded_size(
            _detail::AmqpValueFactory::ToImplementation(value), &encodedSize))
    {
      throw std::runtime_error("Could not get encoded size for value.");
    }
    std::vector<uint8_t> encodedValue(encodedSize);
    if (amqpvalue_encode(
            _detail::AmqpValueFactory::ToImplementation(value),
            encodedValue.data(),
            encodedValue.size()))
    {
      throw std::runtime_error("Could not encode object");
    }
    return encodedValue;
#endif
  }

  size_t AmqpValue::GetSerializedSize(AmqpValue const& value)
  {
    size_t encodedSize;
    if (amqpvalue_get_encoded_size(
            _detail::AmqpValueFactory::ToImplementation(value), &encodedSize))
    {
      throw std::runtime_error("Could not get encoded size for value.");
    }
    return encodedSize;
  }

  AmqpValue _detail::AmqpValueFactory::FromImplementation(UniqueAmqpValueHandle const& value)
  {
    return AmqpValue{
        std::make_unique<AmqpValueImpl>(UniqueAmqpValueHandle{amqpvalue_clone(value.get())})};
  }
  AmqpValue _detail::AmqpValueFactory::FromImplementation(UniqueAmqpValueHandle&& value)
  {
    return AmqpValue(std::make_unique<AmqpValueImpl>(std::move(value)));
  }

  AmqpValue _detail::AmqpValueFactory::FromImplementation(AmqpValueImpl&& value)
  {
    return AmqpValue(std::make_unique<AmqpValueImpl>(std::move(value)));
  }

  Azure::Core::Amqp::_detail::AmqpValueImplementation* _detail::AmqpValueFactory::ToImplementation(
      AmqpValue const& value)
  {
    return *value.m_impl;
  }

  AmqpValueImpl::AmqpValueImpl(AmqpValueImpl const& other)
      : m_value{amqpvalue_clone(other.m_value.get())}
  {
    (void)other;
  }
  AmqpValueImpl::AmqpValueImpl(AmqpValueImpl&& other) noexcept : m_value{std::move(other.m_value)}
  {
    (void)other;
  }

  AmqpArray::AmqpArray(AmqpValue const& value)
  {
    if (value.GetType() != AmqpValueType::Array)
    {
      throw std::runtime_error("Input AMQP value MUST be an array.");
    }
    std::uint32_t arraySize;
    if (amqpvalue_get_array_item_count(
            _detail::AmqpValueFactory::ToImplementation(value), &arraySize))
    {
      throw std::runtime_error("Could not get array size from AMQP_VALUE");
    }
    m_value.reserve(arraySize);
    for (std::uint32_t i = 0; i < arraySize; i += 1)
    {
      // amqpvalue_get_array_item clones the value. We don't need to clone it again.
      UniqueAmqpValueHandle item{
          amqpvalue_get_array_item(_detail::AmqpValueFactory::ToImplementation(value), i)};
      m_value.push_back(_detail::AmqpValueFactory::FromImplementation(item));
    }
  }
  AmqpArray::AmqpArray(initializer_type const& initializer) : AmqpCollectionBase(initializer)
  {
    if (initializer.size())
    {
      AmqpValueType expectedType = initializer.begin()->GetType();
      for (auto const& v : initializer)
      {
        if (v.GetType() != expectedType)
        {
          throw std::runtime_error("Type mismatch creating a new AMQP array.");
        }
      }
    }
  }

  template <>
  _detail::AmqpCollectionBase<std::vector<AmqpValue>, AmqpArray>::operator _detail::AmqpValueImpl()
      const
  {
    UniqueAmqpValueHandle array{amqpvalue_create_array()};
    for (const auto& val : *this)
    {
      if (amqpvalue_add_array_item(array.get(), _detail::AmqpValueFactory::ToImplementation(val)))
      {
        throw(std::runtime_error("Could not add value to array."));
      }
    }
    return array;
  }

  template <>
  AmqpValue _detail::AmqpCollectionBase<std::vector<AmqpValue>, AmqpArray>::AsAmqpValue() const
  {
    return _detail::AmqpValueFactory::FromImplementation(_detail::AmqpValueImpl{*this});
  }

  AmqpMap::AmqpMap(AmqpValue const& value)
  {

    if (value.GetType() != AmqpValueType::Map)
    {
      throw std::runtime_error("Input AMQP value MUST be a map.");
    }
    std::uint32_t mapSize;
    if (amqpvalue_get_map_pair_count(_detail::AmqpValueFactory::ToImplementation(value), &mapSize))
    {
      throw std::runtime_error("Could not get array size from AMQP_VALUE");
    }
    for (std::uint32_t i = 0; i < mapSize; i += 1)
    {
      Azure::Core::Amqp::_detail::AmqpValueImplementation *key{}, *val{};
      amqpvalue_get_map_key_value_pair(
          _detail::AmqpValueFactory::ToImplementation(value), i, &key, &val);
      m_value.emplace(std::make_pair(
          _detail::AmqpValueFactory::FromImplementation(UniqueAmqpValueHandle{key}),
          _detail::AmqpValueFactory::FromImplementation(UniqueAmqpValueHandle{val})));
    }
  }

  template <>
  _detail::AmqpCollectionBase<std::map<AmqpValue, AmqpValue>, AmqpMap>::operator _detail::
      AmqpValueImpl() const
  {
    UniqueAmqpValueHandle value{amqpvalue_create_map()};
    for (const auto& val : *this)
    {
      if (amqpvalue_set_map_value(
              value.get(),
              _detail::AmqpValueFactory::ToImplementation(val.first),
              _detail::AmqpValueFactory::ToImplementation(val.second)))
      {
        throw(std::runtime_error("Could not add value to array."));
      }
    }
    return value;
  }
  template <>
  AmqpValue _detail::AmqpCollectionBase<std::map<AmqpValue, AmqpValue>, AmqpMap>::AsAmqpValue()
      const
  {
    return _detail::AmqpValueFactory::FromImplementation(_detail::AmqpValueImpl{*this});
  }
  AmqpList::AmqpList(AmqpValue const& value)
  {
    if (value.GetType() != AmqpValueType::List)
    {
      throw std::runtime_error("Input AMQP value MUST be a list.");
    }
    std::uint32_t listSize;
    if (amqpvalue_get_list_item_count(
            _detail::AmqpValueFactory::ToImplementation(value), &listSize))
    {
      throw std::runtime_error("Could not get array size from AMQP_VALUE");
    }
    for (std::uint32_t i = 0; i < listSize; i += 1)
    {
      UniqueAmqpValueHandle item{
          amqpvalue_get_list_item(_detail::AmqpValueFactory::ToImplementation(value), i)};
      push_back(_detail::AmqpValueFactory::FromImplementation(item));
    }
  }

  template <>
  _detail::AmqpCollectionBase<std::vector<AmqpValue>, AmqpList>::operator _detail::AmqpValueImpl()
      const
  {
    UniqueAmqpValueHandle list{amqpvalue_create_list()};
    if (amqpvalue_set_list_item_count(list.get(), static_cast<std::uint32_t>(size())))
    {
      throw(std::runtime_error("Could not set list size."));
    }
    std::uint32_t i = 0;
    for (const auto& val : *this)
    {
      if (amqpvalue_set_list_item(list.get(), i, _detail::AmqpValueFactory::ToImplementation(val)))
      {
        throw(std::runtime_error("Could not add value to list."));
      }
      i += 1;
    }
    return list;
  }
  template <>
  AmqpValue _detail::AmqpCollectionBase<std::vector<AmqpValue>, AmqpList>::AsAmqpValue() const
  {
    return _detail::AmqpValueFactory::FromImplementation(_detail::AmqpValueImpl{*this});
  }

  template <>
  _detail::AmqpCollectionBase<std::vector<uint8_t>, AmqpBinaryData>::operator _detail::
      AmqpValueImpl() const
  {
#if ENABLE_UAMQP
    UniqueAmqpValueHandle binary{amqpvalue_create_binary({data(), static_cast<uint32_t>(size())})};
#elif ENABLE_RUST_AMQP
    UniqueAmqpValueHandle binary{amqpvalue_create_binary(data(), static_cast<uint32_t>(size()))};
#endif
    return binary;
  }

  template <>
  AmqpValue _detail::AmqpCollectionBase<std::vector<std::uint8_t>, AmqpBinaryData>::AsAmqpValue()
      const
  {
    return _detail::AmqpValueFactory::FromImplementation(_detail::AmqpValueImpl{*this});
  }
  AmqpBinaryData::AmqpBinaryData(AmqpValue const& value)
  {
    if (value.GetType() != AmqpValueType::Binary)
    {
      throw std::runtime_error("Input AMQP value MUST be binary.");
    }
#if ENABLE_UAMQP
    amqp_binary binaryData;
    if (amqpvalue_get_binary(_detail::AmqpValueFactory::ToImplementation(value), &binaryData))
    {
      throw std::runtime_error("Could not retrieve binary data.");
    }
    // Copy the binary data to our storage.
    m_value.assign(
        static_cast<const uint8_t*>(binaryData.bytes),
        static_cast<const uint8_t*>(binaryData.bytes) + binaryData.length);
#elif ENABLE_RUST_AMQP
    uint32_t size;
    uint8_t const* data;
    if (amqpvalue_get_binary(_detail::AmqpValueFactory::ToImplementation(value), &data, &size) != 0)
    {
      throw std::runtime_error("Could not retrieve binary data.");
    }
    // Copy the binary data to our storage.
    m_value.assign(static_cast<const uint8_t*>(data), static_cast<const uint8_t*>(data) + size);

#endif
  }

  template <>
  _detail::AmqpCollectionBase<std::string, AmqpSymbol>::operator _detail::AmqpValueImpl() const
  {
    UniqueAmqpValueHandle symbol{amqpvalue_create_symbol(m_value.c_str())};
    return symbol;
  }

  template <> AmqpValue _detail::AmqpCollectionBase<std::string, AmqpSymbol>::AsAmqpValue() const
  {
    return _detail::AmqpValueFactory::FromImplementation(_detail::AmqpValueImpl{*this});
  }

  // Moved below AmqpSymbol::AsAmqpValue.
  AmqpValue::AmqpValue(AmqpSymbol const& value)
      : m_impl{std::make_unique<_detail::AmqpValueImpl>(
          _detail::UniqueAmqpValueHandle{amqpvalue_clone(*value.AsAmqpValue().m_impl)})}
  {
  }

  AmqpSymbol::AmqpSymbol(AmqpValue const& value)
  {
    if (value.GetType() != AmqpValueType::Symbol)
    {
      throw std::runtime_error("Input AMQP value MUST be a symbol.");
    }

    const char* binaryData;
    if (amqpvalue_get_symbol(_detail::AmqpValueFactory::ToImplementation(value), &binaryData))
    {
      throw std::runtime_error("Could not retrieve binary data.");
    }
    // Copy the binary data to our storage.
    m_value.assign(binaryData);
  }

  AmqpTimestamp::operator _detail::AmqpValueImpl() const
  {
    UniqueAmqpValueHandle timestamp{amqpvalue_create_timestamp(m_value.count())};
    return timestamp;
  }

  AmqpValue AmqpTimestamp::AsAmqpValue() const
  {
    return _detail::AmqpValueFactory::FromImplementation(
        static_cast<_detail::AmqpValueImpl>(*this));
  }

  namespace {
    std::chrono::milliseconds GetMillisecondsFromAmqp(
        Azure::Core::Amqp::_detail::AmqpValueImplementation* value)
    {
#if ENABLE_UAMQP
      if (amqpvalue_get_type(value) != AMQP_TYPE_TIMESTAMP)
#elif ENABLE_RUST_AMQP
      if (amqpvalue_get_type(value) != RustAmqpValueType::AmqpValueTimestamp)
#endif
      {
        throw std::runtime_error("Input AMQP value MUST be a timestamp.");
      }

      int64_t stamp;
      if (amqpvalue_get_timestamp(value, &stamp))
      {
        throw std::runtime_error("Could not retrieve binary data.");
      }
      return std::chrono::milliseconds(stamp);
    }
  } // namespace

  AmqpTimestamp::AmqpTimestamp(AmqpValue const& value)
      : m_value(GetMillisecondsFromAmqp(_detail::AmqpValueFactory::ToImplementation(value)))
  {
  }

  AmqpTimestamp::AmqpTimestamp(std::chrono::milliseconds const& initializer) : m_value(initializer)
  {
  }
  AmqpTimestamp::AmqpTimestamp() : m_value{} {}

  AmqpComposite::AmqpComposite(AmqpValue const& value)
  {
    if (value.GetType() != AmqpValueType::Composite)
    {
      throw std::runtime_error("Input AMQP value MUST be a composite value.");
    }
    std::uint32_t compositeSize;
    if (amqpvalue_get_composite_item_count(
            _detail::AmqpValueFactory::ToImplementation(value), &compositeSize))
    {
      throw std::runtime_error("Could not get composite size from AMQP_VALUE");
    }
    for (std::uint32_t i = 0; i < compositeSize; i += 1)
    {
#if ENABLE_UAMQP
      push_back(_detail::AmqpValueFactory::FromImplementation(
          _detail::UniqueAmqpValueHandle{amqpvalue_clone(amqpvalue_get_composite_item_in_place(
              _detail::AmqpValueFactory::ToImplementation(value), i))}));
#elif ENABLE_RUST_AMQP
      Azure::Core::Amqp::_detail::AmqpValueImplementation* item;
      if (amqpvalue_get_composite_item_in_place(
              _detail::AmqpValueFactory::ToImplementation(value), i, &item))
      {
        throw std::runtime_error("Could not get composite item.");
      }
      push_back(
          _detail::AmqpValueFactory::FromImplementation(_detail::UniqueAmqpValueHandle{item}));

#endif
    }

#if ENABLE_UAMQP
    m_descriptor = _detail::AmqpValueFactory::FromImplementation(
        _detail::UniqueAmqpValueHandle{amqpvalue_clone(
            amqpvalue_get_inplace_descriptor(_detail::AmqpValueFactory::ToImplementation(value)))});
    if (m_descriptor.IsNull())
    {
      throw std::runtime_error("Could not read descriptor for composite value.");
    }
#elif ENABLE_RUST_AMQP
    Azure::Core::Amqp::_detail::AmqpValueImplementation* item;
    if (amqpvalue_get_inplace_descriptor(_detail::AmqpValueFactory::ToImplementation(value), &item))
    {
      throw std::runtime_error("Could not get composite descriptor.");
    }
    m_descriptor
        = _detail::AmqpValueFactory::FromImplementation(_detail::UniqueAmqpValueHandle{item});
#endif
  }

  AmqpComposite::AmqpComposite(
      AmqpValue const& descriptor,
      std::initializer_list<AmqpValue> const& initializer)
      : AmqpCollectionBase{initializer}, m_descriptor{descriptor}
  {
  }

  template <>
  _detail::AmqpCollectionBase<std::vector<AmqpValue>, AmqpComposite>::operator _detail::
      AmqpValueImpl() const
  {
    return _detail::AmqpValueImpl(_detail::UniqueAmqpValueHandle{});
  }

  template <>
  AmqpValue _detail::AmqpCollectionBase<std::vector<AmqpValue>, AmqpComposite>::AsAmqpValue() const
  {
    return _detail::AmqpValueFactory::FromImplementation(_detail::AmqpValueImpl{*this});
  }

  AmqpComposite::operator _detail::AmqpValueImpl() const
  {
    UniqueAmqpValueHandle composite{amqpvalue_create_composite(
        _detail::AmqpValueFactory::ToImplementation(m_descriptor),
        static_cast<std::uint32_t>(size()))};
    std::uint32_t i = 0;
    for (const auto& val : *this)
    {
      if (amqpvalue_set_composite_item(
              composite.get(), i, _detail::AmqpValueFactory::ToImplementation(val)))
      {
        throw(std::runtime_error("Could not add value to list."));
      }
      i += 1;
    }
    return composite;
  }

  AmqpDescribed::AmqpDescribed(AmqpValue const& value)
  {
    if (value.GetType() != AmqpValueType::Described)
    {
      throw std::runtime_error("Input AMQP value MUST be a described value.");
    }
#if ENABLE_UAMQP
    m_descriptor = _detail::AmqpValueFactory::FromImplementation(
        _detail::UniqueAmqpValueHandle{amqpvalue_clone(
            amqpvalue_get_inplace_descriptor(_detail::AmqpValueFactory::ToImplementation(value)))});
    if (m_descriptor.IsNull())
    {
      throw std::runtime_error("Could not read descriptor for described value.");
    }
    m_value = _detail::AmqpValueFactory::FromImplementation(
        _detail::UniqueAmqpValueHandle{amqpvalue_clone(amqpvalue_get_inplace_described_value(
            _detail::AmqpValueFactory::ToImplementation(value)))});
    if (m_value.IsNull())
    {
      throw std::runtime_error("Could not read descriptor for described value.");
    }

#elif ENABLE_RUST_AMQP
    Azure::Core::Amqp::_detail::AmqpValueImplementation* item;
    if (amqpvalue_get_inplace_descriptor(_detail::AmqpValueFactory::ToImplementation(value), &item))
    {
      throw std::runtime_error("Could not get composite item.");
    }
    m_descriptor
        = _detail::AmqpValueFactory::FromImplementation(_detail::UniqueAmqpValueHandle{item});

    if (amqpvalue_get_inplace_described_value(
            _detail::AmqpValueFactory::ToImplementation(value), &item))
    {
      throw std::runtime_error("Could not get composite item.");
    }
    m_value = _detail::AmqpValueFactory::FromImplementation(_detail::UniqueAmqpValueHandle{item});
#endif
  }

  AmqpDescribed::AmqpDescribed(AmqpSymbol const& descriptor, AmqpValue const& value)
      : m_descriptor{descriptor.AsAmqpValue()}, m_value{value}
  {
  }
  AmqpDescribed::AmqpDescribed(uint64_t descriptor, AmqpValue const& value)
      : m_descriptor(descriptor), m_value(value)
  {
  }

  AmqpDescribed::operator _detail::AmqpValueImpl() const
  {
    // For <reasons>, amqpvalue_create_described does not clone the provided descriptor or
    // value, but amqpvalue_destroy on a described destroys the underlying value. That means we
    // need to manually clone the input descriptors to ensure that the reference counts work
    // out.
    UniqueAmqpValueHandle composite{amqpvalue_create_described(
        amqpvalue_clone(_detail::AmqpValueFactory::ToImplementation(m_descriptor)),
        amqpvalue_clone(_detail::AmqpValueFactory::ToImplementation(m_value)))};
    return composite;
  }

  AmqpValue AmqpDescribed::AsAmqpValue() const
  {
    return _detail::AmqpValueFactory::FromImplementation(
        static_cast<_detail::AmqpValueImpl>(*this));
  }

  AmqpDescribed::operator AmqpValue const() const
  {
    return _detail::AmqpValueFactory::FromImplementation(
        static_cast<_detail::AmqpValueImpl>(*this));
  }

  namespace {

    size_t LogRawData(std::ostream& os, size_t startOffset, const uint8_t* const pb, size_t cb)
    {
      // scratch buffer which will hold the data being logged.
      std::stringstream ss;

      size_t bytesToWrite = (cb < 0x10 ? cb : 0x10);

      ss << std::hex << std::right << std::setw(8) << std::setfill('0') << startOffset << ": ";

      // Write the buffer data out.
      for (size_t i = 0; i < bytesToWrite; i += 1)
      {
        ss << std::hex << std::right << std::setw(2) << std::setfill('0') << static_cast<int>(pb[i])
           << " ";
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

  std::ostream& operator<<(std::ostream& os, AmqpBinaryData const& value)
  {
    const uint8_t* pb = value.data();
    size_t cb = value.size();
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
    return os;
  }

  std::ostream& operator<<(std::ostream& os, AmqpArray const& value)
  {
    // Let the AmqpValue specialization handle serialization of the array.
    AmqpValue arrayValue(value.AsAmqpValue());
    os << arrayValue;
    return os;
  }

  std::ostream& operator<<(std::ostream& os, AmqpList const& value)
  {
    // Let the AmqpValue specialization handle serialization of the list.
    AmqpValue arrayValue(value.AsAmqpValue());
    os << arrayValue;
    return os;
  }

  std::ostream& operator<<(std::ostream& os, AmqpMap const& value)
  {
    // Let the AmqpValue specialization handle serialization of the map.
    AmqpValue mapValue(value.AsAmqpValue());
    os << mapValue;
    return os;
  }
  std::ostream& operator<<(std::ostream& os, AmqpSymbol const& value)
  {
    // Let the AmqpValue specialization handle serialization of the array.
    AmqpValue arrayValue(value.AsAmqpValue());
    os << arrayValue;
    return os;
  }

  bool AmqpValue::IsNull() const
  {
    return !m_impl || !m_impl->m_value || (GetType() == AmqpValueType::Null);
  }

}}}} // namespace Azure::Core::Amqp::Models
