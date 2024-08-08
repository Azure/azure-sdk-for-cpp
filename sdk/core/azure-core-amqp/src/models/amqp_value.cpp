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

#include <iomanip>
#include <iostream>
#include <sstream>

using namespace Azure::Core::Amqp::Models::_detail;
using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace _detail {
#if ENABLE_UAMQP
  /// @cond INTERNAL
  void UniqueHandleHelper<std::remove_pointer<AMQP_VALUE>::type>::FreeAmqpValue(AMQP_VALUE value)
  {
    amqpvalue_destroy(value);
  }
  /// @endcond

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
#if ENABLE_UAMQP
    std::ostream& operator<<(std::ostream& os, AMQP_TYPE value)
    {
      switch (value)
      {
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
      }
      return os;
    }
    std::ostream& operator<<(std::ostream& os, AMQP_VALUE const value)
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
#endif
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
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_boolean(bool_value)})
  }
#endif
  {
    (void)bool_value;
  }
  AmqpValue::AmqpValue(std::uint8_t byte_value)
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_ubyte(byte_value)})
  }
#endif
  {
    (void)byte_value;
  }
  AmqpValue::AmqpValue(char value)
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_byte(value)})
  }
#endif
  {
    (void)value;
  }
  AmqpValue::AmqpValue(std::int8_t value)
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_byte(value)})
  }
#endif
  {
    (void)value;
  }
  AmqpValue::AmqpValue(std::uint16_t value)
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_ushort(value)})
  }
#endif
  {
    (void)value;
  }
  AmqpValue::AmqpValue(std::int16_t value)
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_short(value)})
  }
#endif
  {
    (void)value;
  }
  AmqpValue::AmqpValue(std::uint32_t value)
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_uint(value)})
  }
#endif
  {
    (void)value;
  }
  AmqpValue::AmqpValue(std::int32_t value)
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_int(value)})
  }
#endif
  {
    (void)value;
  }
  AmqpValue::AmqpValue(std::uint64_t value)
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_ulong(value)})
  }
#endif
  {
    (void)value;
  }
  AmqpValue::AmqpValue(std::int64_t value)
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_long(value)})
  }
#endif
  {
    (void)value;
  }
  AmqpValue::AmqpValue(float value)
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_float(value)})
  }
#endif
  {
    (void)value;
  }
  AmqpValue::AmqpValue(double value)
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_double(value)})
  }
#endif
  {
    (void)value;
  }
  AmqpValue::AmqpValue(char32_t value)
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_char(value)})
  }
#endif
  {
    (void)value;
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
    (void)uuid;
  }

  AmqpValue::AmqpValue(std::string const& value)
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_string(value.c_str())})
  }
#endif
  {
    (void)value;
  }
  AmqpValue::AmqpValue(const char* value)
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_string(value)})
  }
#endif
  {
    (void)value;
  }

  AmqpValue::AmqpValue() noexcept
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_create_null()})
  }
#endif
  {
  }

  AmqpValue::AmqpValue(AmqpValue const& that) noexcept
#if ENABLE_UAMQP
      : m_impl
  {
    std::make_unique<_detail::AmqpValueImpl>(
        _detail::UniqueAmqpValueHandle{amqpvalue_clone(*that.m_impl)})
  }
#endif
  {
    (void)that;
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
#if ENABLE_UAMQP
    if (amqpvalue_get_boolean(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve boolean value");
    }
#endif
    return value;
  }

  AmqpValue::operator std::uint8_t() const
  {
    unsigned char value = {};
#if ENABLE_UAMQP
    if (amqpvalue_get_ubyte(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve ubyte value");
    }
#endif
    return value;
  }

  AmqpValue::operator std::int8_t() const
  {
    char value = {};
#if ENABLE_UAMQP
    if (amqpvalue_get_byte(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve byte value");
    }
#endif
    return value;
  }

  AmqpValue::operator char() const
  {
    char value = {};
#if ENABLE_UAMQP
    if (amqpvalue_get_byte(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve byte value");
    }
#endif
    return value;
  }

  AmqpValue::operator std::uint16_t() const
  {
    uint16_t value = {};
#if ENABLE_UAMQP
    if (amqpvalue_get_ushort(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve ushort value");
    }
#endif
    return value;
  }

  AmqpValue::operator std::int16_t() const
  {
    int16_t value = {};
#if ENABLE_UAMQP
    if (amqpvalue_get_short(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve short value");
    }
#endif
    return value;
  }

  AmqpValue::operator std::uint32_t() const
  {
    std::uint32_t value = {};
#if ENABLE_UAMQP
    if (amqpvalue_get_uint(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve uint value");
    }
#endif
    return value;
  }

  AmqpValue::operator std::int32_t() const
  {
    int32_t value = {};
#if ENABLE_UAMQP
    if (amqpvalue_get_int(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve int value");
    }
#endif
    return value;
  }

  AmqpValue::operator std::uint64_t() const
  {
    uint64_t value = {};
#if ENABLE_UAMQP
    if (amqpvalue_get_ulong(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve ulong value");
    }
#endif
    return value;
  }

  AmqpValue::operator std::int64_t() const
  {
    int64_t value = {};
#if ENABLE_UAMQP
    if (amqpvalue_get_long(*m_impl, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve long value");
    }
#endif
    return value;
  }

  AmqpValue::operator float() const
  {
    float value = {};
#if ENABLE_UAMQP
    if (amqpvalue_get_float(*m_impl, &value))
    {
      throw std::runtime_error("Could not retrieve float value");
    }
#endif
    return value;
  }

  AmqpValue::operator double() const
  {
    double value = {};
#if ENABLE_UAMQP
    if (amqpvalue_get_double(*m_impl, &value))
    {
      throw std::runtime_error("Could not retrieve double value");
    }
#endif
    return value;
  }

  AmqpValue::operator char32_t() const
  {
    std::uint32_t value = {};
#if ENABLE_UAMQP
    if (amqpvalue_get_char(*m_impl, &value))
    {
      throw std::runtime_error("Could not get character.");
    }
#endif
    return value;
  }

  AmqpValue::operator std::string() const
  {
    const char* value = {};
#if ENABLE_UAMQP
    if (amqpvalue_get_string(*m_impl, &value))
    {
      throw std::runtime_error("Could not retrieve string value");
    }
#endif
    return value;
  }

  AmqpValue::operator Azure::Core::Uuid() const
  {
#if ENABLE_UAMQP
    uuid value;
    if (amqpvalue_get_uuid(*m_impl, &value))
    {
      throw std::runtime_error("Could not retrieve uuid value");
    }
#endif
    std::array<uint8_t, 16> uuidArray{};
#if ENABLE_UAMQP
    memcpy(uuidArray.data(), value, 16);
#endif
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
    // The uAMQP function `amqpvalue_are_equal` does not work for all types, so we need to do some
    // special handling for those which are not handled properly.
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
#if ENABLE_UAMQP
    return amqpvalue_are_equal(*m_impl, *that.m_impl);
#else
    return false;
#endif
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
    const std::map<AMQP_TYPE, AmqpValueType> UamqpToAmqpTypeMap{
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
#endif

    class AmqpValueDeserializer final {
    public:
#if ENABLE_UAMQP
      AmqpValueDeserializer() : m_decoder{amqpvalue_decoder_create(OnAmqpValueDecoded, this)} {}

      AmqpValue operator()(std::uint8_t const* data, size_t size) const
      {
        if (amqpvalue_decode_bytes(m_decoder.get(), data, size))
        {
          throw std::runtime_error("Could not decode object");
        }
        return m_decodedValue;
      }
#endif
    private:
#if ENABLE_UAMQP
      _detail::UniqueAmqpDecoderHandle m_decoder;
      AmqpValue m_decodedValue;
      static void OnAmqpValueDecoded(void* context, AMQP_VALUE value)
      {
        auto deserializer = static_cast<AmqpValueDeserializer*>(context);
        deserializer->m_decodedValue = _detail::AmqpValueFactory::FromUamqp(
            _detail::UniqueAmqpValueHandle{amqpvalue_clone(value)});
      }
#endif
    };
    class AmqpValueSerializer final {
    public:
      AmqpValueSerializer() = default;

      std::vector<uint8_t> operator()(AmqpValue const& value)
      {
#if ENABLE_UAMQP
        if (amqpvalue_encode(_detail::AmqpValueFactory::ToUamqp(value), OnAmqpValueEncoded, this))
        {
          throw std::runtime_error("Could not encode object");
        }
#endif
        (void)value;
        return m_encodedValue;
      }

    private:
      std::vector<uint8_t> m_encodedValue;
#if ENABLE_UAMQP

      // The OnAmqpValueEncoded callback appends the array provided to the existing encoded value,
      // extending as needed.
      //
      // Returns 0 if successful, 1 otherwise.
      static int OnAmqpValueEncoded(void* context, unsigned char const* bytes, size_t length)
      {
        auto serializer = static_cast<AmqpValueSerializer*>(context);
        serializer->m_encodedValue.insert(serializer->m_encodedValue.end(), bytes, bytes + length);
        return 0;
      }
#endif
    };

  } // namespace
  AmqpValueType AmqpValue::GetType() const
  {
#if ENABLE_UAMQP
    auto val{UamqpToAmqpTypeMap.find(amqpvalue_get_type(*m_impl))};
    if (val == UamqpToAmqpTypeMap.end())
    {
      throw std::runtime_error("Unknown AMQP AmqpValue Type");
    }
    return val->second;
#else
    return AmqpValueType::Invalid;
#endif
  }

  std::ostream& operator<<(std::ostream& os, AmqpValue const& value)
  {
#if ENABLE_UAMQP
    char* valueAsString{amqpvalue_to_string(_detail::AmqpValueFactory::ToUamqp(value))};
    os << valueAsString;
    free(valueAsString);
#else
    (void)value;
#endif
    return os;
  }

  AmqpValue AmqpValue::Deserialize(uint8_t const* data, size_t size)
  {
#if ENABLE_UAMQP
    return AmqpValueDeserializer{}(data, size);
#else
    (void)data;
    (void)size;
    return AmqpValue{};
#endif
  }

  std::vector<uint8_t> AmqpValue::Serialize(AmqpValue const& value)
  {
#if ENABLE_UAMQP
    return AmqpValueSerializer{}(value);
#else
    (void)value;
    return {};
#endif
  }
  size_t AmqpValue::GetSerializedSize(AmqpValue const& value)
  {
#if ENABLE_UAMQP
    size_t encodedSize;
    if (amqpvalue_get_encoded_size(_detail::AmqpValueFactory::ToUamqp(value), &encodedSize))
    {
      throw std::runtime_error("Could not get encoded size for value.");
    }
    return encodedSize;
#else
    (void)value;
    return {};
#endif
  }

#if ENABLE_UAMQP
  AmqpValue _detail::AmqpValueFactory::FromUamqp(UniqueAmqpValueHandle const& value)
  {
    return AmqpValue{
        std::make_unique<AmqpValueImpl>(UniqueAmqpValueHandle{amqpvalue_clone(value.get())})};
  }
  AmqpValue _detail::AmqpValueFactory::FromUamqp(UniqueAmqpValueHandle&& value)
  {
    return AmqpValue(std::make_unique<AmqpValueImpl>(std::move(value)));
  }

  AmqpValue _detail::AmqpValueFactory::FromUamqp(AmqpValueImpl&& value)
  {
    return AmqpValue(std::make_unique<AmqpValueImpl>(std::move(value)));
  }

  AMQP_VALUE _detail::AmqpValueFactory::ToUamqp(AmqpValue const& value) { return *value.m_impl; }
#endif

  AmqpValueImpl::AmqpValueImpl(AmqpValueImpl const& other)
#if ENABLE_UAMQP
      : m_value
  {
    amqpvalue_clone(other.m_value.get())
  }
#endif
  {
    (void)other;
  }
  AmqpValueImpl::AmqpValueImpl(AmqpValueImpl&& other) noexcept
#if ENABLE_UAMQP
      : m_value
  {
    std::move(other.m_value)
  }
#endif
  {
    (void)other;
  }

  AmqpArray::AmqpArray(AmqpValue const& value)
  {
    if (value.GetType() != AmqpValueType::Array)
    {
      throw std::runtime_error("Input AMQP value MUST be an array.");
    }
#if ENABLE_UAMQP
    std::uint32_t arraySize;
    if (amqpvalue_get_array_item_count(_detail::AmqpValueFactory::ToUamqp(value), &arraySize))
    {
      throw std::runtime_error("Could not get array size from AMQP_VALUE");
    }
    m_value.reserve(arraySize);
    for (std::uint32_t i = 0; i < arraySize; i += 1)
    {
      // amqpvalue_get_array_item clones the value. We don't need to clone it again.
      UniqueAmqpValueHandle item{
          amqpvalue_get_array_item(_detail::AmqpValueFactory::ToUamqp(value), i)};
      m_value.push_back(_detail::AmqpValueFactory::FromUamqp(item));
    }
#endif
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
#if (ENABLE_UAMQP)
    UniqueAmqpValueHandle array{amqpvalue_create_array()};
    for (const auto& val : *this)
    {
      if (amqpvalue_add_array_item(array.get(), _detail::AmqpValueFactory::ToUamqp(val)))
      {
        throw(std::runtime_error("Could not add value to array."));
      }
    }
    return array;
#else
    return {};
#endif
  }

  template <>
  AmqpValue _detail::AmqpCollectionBase<std::vector<AmqpValue>, AmqpArray>::AsAmqpValue() const
  {
#if ENABLE_UAMQP
    return _detail::AmqpValueFactory::FromUamqp(_detail::AmqpValueImpl{*this});
#else
    return {};
#endif
  }

  AmqpMap::AmqpMap(AmqpValue const& value)
  {

    if (value.GetType() != AmqpValueType::Map)
    {
      throw std::runtime_error("Input AMQP value MUST be a map.");
    }
#if ENABLE_UAMQP
    std::uint32_t mapSize;
    if (amqpvalue_get_map_pair_count(_detail::AmqpValueFactory::ToUamqp(value), &mapSize))
    {
      throw std::runtime_error("Could not get array size from AMQP_VALUE");
    }
    for (std::uint32_t i = 0; i < mapSize; i += 1)
    {
      AMQP_VALUE key{}, val{};
      amqpvalue_get_map_key_value_pair(_detail::AmqpValueFactory::ToUamqp(value), i, &key, &val);
      m_value.emplace(std::make_pair(
          _detail::AmqpValueFactory::FromUamqp(UniqueAmqpValueHandle{key}),
          _detail::AmqpValueFactory::FromUamqp(UniqueAmqpValueHandle{val})));
    }
#endif
  }

  template <>
  _detail::AmqpCollectionBase<std::map<AmqpValue, AmqpValue>, AmqpMap>::operator _detail::
      AmqpValueImpl() const
  {
#if ENABLE_UAMQP
    UniqueAmqpValueHandle value{amqpvalue_create_map()};
    for (const auto& val : *this)
    {
      if (amqpvalue_set_map_value(
              value.get(),
              _detail::AmqpValueFactory::ToUamqp(val.first),
              _detail::AmqpValueFactory::ToUamqp(val.second)))
      {
        throw(std::runtime_error("Could not add value to array."));
      }
    }
    return value;
#else
    return {};
#endif
  }
  template <>
  AmqpValue _detail::AmqpCollectionBase<std::map<AmqpValue, AmqpValue>, AmqpMap>::AsAmqpValue()
      const
  {
#if ENABLE_UAMQP
    return _detail::AmqpValueFactory::FromUamqp(_detail::AmqpValueImpl{*this});
#else
    return {};
#endif
  }
  AmqpList::AmqpList(AmqpValue const& value)
  {
    if (value.GetType() != AmqpValueType::List)
    {
      throw std::runtime_error("Input AMQP value MUST be a list.");
    }
#if ENABLE_UAMQP
    std::uint32_t listSize;
    if (amqpvalue_get_list_item_count(_detail::AmqpValueFactory::ToUamqp(value), &listSize))
    {
      throw std::runtime_error("Could not get array size from AMQP_VALUE");
    }
    for (std::uint32_t i = 0; i < listSize; i += 1)
    {
      UniqueAmqpValueHandle item{
          amqpvalue_get_list_item(_detail::AmqpValueFactory::ToUamqp(value), i)};
      push_back(_detail::AmqpValueFactory::FromUamqp(item));
    }
#endif
  }

  template <>
  _detail::AmqpCollectionBase<std::vector<AmqpValue>, AmqpList>::operator _detail::AmqpValueImpl()
      const
  {
#if ENABLE_UAMQP
    UniqueAmqpValueHandle list{amqpvalue_create_list()};
    if (amqpvalue_set_list_item_count(list.get(), static_cast<std::uint32_t>(size())))
    {
      throw(std::runtime_error("Could not set list size."));
    }
    std::uint32_t i = 0;
    for (const auto& val : *this)
    {
      if (amqpvalue_set_list_item(list.get(), i, _detail::AmqpValueFactory::ToUamqp(val)))
      {
        throw(std::runtime_error("Could not add value to list."));
      }
      i += 1;
    }
    return list;
#else
    return {};
#endif
  }
  template <>
  AmqpValue _detail::AmqpCollectionBase<std::vector<AmqpValue>, AmqpList>::AsAmqpValue() const
  {
#if ENABLE_UAMQP
    return _detail::AmqpValueFactory::FromUamqp(_detail::AmqpValueImpl{*this});
#else
    return {};
#endif
  }

  template <>
  _detail::AmqpCollectionBase<std::vector<uint8_t>, AmqpBinaryData>::operator _detail::
      AmqpValueImpl() const
  {
#if ENABLE_UAMQP
    UniqueAmqpValueHandle binary{amqpvalue_create_binary({data(), static_cast<uint32_t>(size())})};
    return binary;
#else
    return {};
#endif
  }

  template <>
  AmqpValue _detail::AmqpCollectionBase<std::vector<std::uint8_t>, AmqpBinaryData>::AsAmqpValue()
      const
  {
#if ENABLE_UAMQP
    return _detail::AmqpValueFactory::FromUamqp(_detail::AmqpValueImpl{*this});
#else
    return {};
#endif
  }
  AmqpBinaryData::AmqpBinaryData(AmqpValue const& value)
  {
    if (value.GetType() != AmqpValueType::Binary)
    {
      throw std::runtime_error("Input AMQP value MUST be binary.");
    }
#if ENABLE_UAMQP
    amqp_binary binaryData;
    if (amqpvalue_get_binary(_detail::AmqpValueFactory::ToUamqp(value), &binaryData))
    {
      throw std::runtime_error("Could not retrieve binary data.");
    }
    // Copy the binary data to our storage.
    m_value.assign(
        static_cast<const uint8_t*>(binaryData.bytes),
        static_cast<const uint8_t*>(binaryData.bytes) + binaryData.length);
#endif
  }

  template <>
  _detail::AmqpCollectionBase<std::string, AmqpSymbol>::operator _detail::AmqpValueImpl() const
  {
#if ENABLE_UAMQP
    UniqueAmqpValueHandle symbol{amqpvalue_create_symbol(m_value.c_str())};
    return symbol;
#else
    return {};
#endif
  }
  template <> AmqpValue _detail::AmqpCollectionBase<std::string, AmqpSymbol>::AsAmqpValue() const
  {
#if ENABLE_UAMQP
    return _detail::AmqpValueFactory::FromUamqp(_detail::AmqpValueImpl{*this});
#else
    return {};
#endif
  }

  AmqpSymbol::AmqpSymbol(AmqpValue const& value)
  {
    if (value.GetType() != AmqpValueType::Symbol)
    {
      throw std::runtime_error("Input AMQP value MUST be a symbol.");
    }

#if ENABLE_UAMQP
    const char* binaryData;
    if (amqpvalue_get_symbol(_detail::AmqpValueFactory::ToUamqp(value), &binaryData))
    {
      throw std::runtime_error("Could not retrieve binary data.");
    }
    // Copy the binary data to our storage.
    m_value.assign(binaryData);
#endif
  }

  AmqpTimestamp::operator _detail::AmqpValueImpl() const
  {
#if ENABLE_UAMQP
    UniqueAmqpValueHandle symbol{amqpvalue_create_timestamp(m_value.count())};
    return symbol;
#else
    return {};
#endif
  }

  AmqpValue AmqpTimestamp::AsAmqpValue() const
  {
#if ENABLE_UAMQP
    return _detail::AmqpValueFactory::FromUamqp(static_cast<_detail::AmqpValueImpl>(*this));
#else
    return {};
#endif
  }

#if ENABLE_UAMQP
  namespace {
    std::chrono::milliseconds GetMillisecondsFromAmqp(AMQP_VALUE value)
    {
      if (amqpvalue_get_type(value) != AMQP_TYPE_TIMESTAMP)
      {
        throw std::runtime_error("Input AMQP value MUST be a timestamp.");
      }
      timestamp stamp;
      if (amqpvalue_get_timestamp(value, &stamp))
      {
        throw std::runtime_error("Could not retrieve binary data.");
      }
      return std::chrono::milliseconds(stamp);
    }
  } // namespace
#endif

  AmqpTimestamp::AmqpTimestamp(AmqpValue const& value)
#if ENABLE_UAMQP
      : m_value(GetMillisecondsFromAmqp(_detail::AmqpValueFactory::ToUamqp(value)))
#else
      : m_value
  {
  }
#endif
  {
    (void)value;
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
#if ENABLE_UAMQP
    std::uint32_t compositeSize;
    if (amqpvalue_get_composite_item_count(
            _detail::AmqpValueFactory::ToUamqp(value), &compositeSize))
    {
      throw std::runtime_error("Could not get composite size from AMQP_VALUE");
    }
    for (std::uint32_t i = 0; i < compositeSize; i += 1)
    {
      push_back(_detail::AmqpValueFactory::FromUamqp(_detail::UniqueAmqpValueHandle{amqpvalue_clone(
          amqpvalue_get_composite_item_in_place(_detail::AmqpValueFactory::ToUamqp(value), i))}));
    }

    m_descriptor
        = _detail::AmqpValueFactory::FromUamqp(_detail::UniqueAmqpValueHandle{amqpvalue_clone(
            amqpvalue_get_inplace_descriptor(_detail::AmqpValueFactory::ToUamqp(value)))});
    if (m_descriptor.IsNull())
    {
      throw std::runtime_error("Could not read descriptor for composite value.");
    }
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
#if ENABLE_UAMQP
    return _detail::AmqpValueImpl(_detail::UniqueAmqpValueHandle{});
#else
    return {};
#endif
  }

  template <>
  AmqpValue _detail::AmqpCollectionBase<std::vector<AmqpValue>, AmqpComposite>::AsAmqpValue() const
  {
#if ENABLE_UAMQP
    return _detail::AmqpValueFactory::FromUamqp(_detail::AmqpValueImpl{*this});
#else
    return {};
#endif
  }

  AmqpComposite::operator _detail::AmqpValueImpl() const
  {
#if ENABLE_UAMQP
    UniqueAmqpValueHandle composite{amqpvalue_create_composite(
        _detail::AmqpValueFactory::ToUamqp(m_descriptor), static_cast<std::uint32_t>(size()))};
    std::uint32_t i = 0;
    for (const auto& val : *this)
    {
      if (amqpvalue_set_composite_item(composite.get(), i, _detail::AmqpValueFactory::ToUamqp(val)))
      {
        throw(std::runtime_error("Could not add value to list."));
      }
      i += 1;
    }
    return composite;
#else
    return {};
#endif
  }

  AmqpDescribed::AmqpDescribed(AmqpValue const& value)
  {
    if (value.GetType() != AmqpValueType::Described)
    {
      throw std::runtime_error("Input AMQP value MUST be a described value.");
    }
#if ENABLE_UAMQP
    m_descriptor
        = _detail::AmqpValueFactory::FromUamqp(_detail::UniqueAmqpValueHandle{amqpvalue_clone(
            amqpvalue_get_inplace_descriptor(_detail::AmqpValueFactory::ToUamqp(value)))});
    if (m_descriptor.IsNull())
    {
      throw std::runtime_error("Could not read descriptor for described value.");
    }

    m_value = _detail::AmqpValueFactory::FromUamqp(_detail::UniqueAmqpValueHandle{amqpvalue_clone(
        amqpvalue_get_inplace_described_value(_detail::AmqpValueFactory::ToUamqp(value)))});
    if (m_value.IsNull())
    {
      throw std::runtime_error("Could not read descriptor for described value.");
    }
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
#if ENABLE_UAMQP
    // For <reasons>, amqpvalue_create_described does not clone the provided descriptor or value,
    // but amqpvalue_destroy on a described destroys the underlying value. That means we need to
    // manually clone the input descriptors to ensure that the reference counts work out.
    UniqueAmqpValueHandle composite{amqpvalue_create_described(
        amqpvalue_clone(_detail::AmqpValueFactory::ToUamqp(m_descriptor)),
        amqpvalue_clone(_detail::AmqpValueFactory::ToUamqp(m_value)))};
    return composite;
#else
    return {};
#endif
  }

  AmqpValue AmqpDescribed::AsAmqpValue() const
  {
#if ENABLE_UAMQP
    return _detail::AmqpValueFactory::FromUamqp(static_cast<_detail::AmqpValueImpl>(*this));
#else
    return {};
#endif
  }

  AmqpDescribed::operator AmqpValue const() const
  {
#if ENABLE_UAMQP
    return _detail::AmqpValueFactory::FromUamqp(static_cast<_detail::AmqpValueImpl>(*this));
#else
    return {};
#endif
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
#if ENABLE_UAMQP
    return !m_impl || !m_impl->m_value || (amqpvalue_get_type(*m_impl) == AMQP_TYPE_NULL);
#else
    return false;
#endif
  }
}}}} // namespace Azure::Core::Amqp::Models
