// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/models/amqp_value.hpp"

#include "azure/core/amqp/models/amqp_properties.hpp"
#include "azure/core/amqp/models/amqp_protocol.hpp"

// Note: These blank lines are significant because clang-format orders includes alphabetically, but
// there are dependencies in the uAMQP headers which require this ordering.
#include <azure_uamqp_c/amqp_definitions_milliseconds.h>
#include <azure_uamqp_c/amqp_definitions_sequence_no.h>

#include <azure_uamqp_c/amqp_definitions_header.h>
#include <azure_uamqp_c/amqp_definitions_properties.h>
#include <azure_uamqp_c/amqpvalue.h>
#include <azure_uamqp_c/amqpvalue_to_string.h>

#include <iomanip>
#include <iostream>
#include <sstream>

using namespace Azure::Core::Amqp::Models::_detail;

void Azure::Core::_internal::UniqueHandleHelper<AMQP_VALUE_DATA_TAG>::FreeAmqpValue(
    AMQP_VALUE value)
{
  amqpvalue_destroy(value);
}
void Azure::Core::_internal::UniqueHandleHelper<AMQPVALUE_DECODER_HANDLE_DATA_TAG>::FreeAmqpDecoder(
    AMQPVALUE_DECODER_HANDLE value)
{
  amqpvalue_decoder_destroy(value);
}

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  AmqpValue::~AmqpValue() {}

  AmqpValue::AmqpValue(bool bool_value) : m_value{amqpvalue_create_boolean(bool_value)} {}
  AmqpValue::AmqpValue(unsigned char byte_value) : m_value{amqpvalue_create_ubyte(byte_value)} {}
  AmqpValue::AmqpValue(char value) : m_value{amqpvalue_create_byte(value)} {}
  AmqpValue::AmqpValue(std::int8_t value) : m_value{amqpvalue_create_byte(value)} {}
  AmqpValue::AmqpValue(uint16_t value) : m_value{amqpvalue_create_ushort(value)} {}
  AmqpValue::AmqpValue(int16_t value) : m_value{amqpvalue_create_short(value)} {}
  AmqpValue::AmqpValue(std::uint32_t value) : m_value{amqpvalue_create_uint(value)} {}
  AmqpValue::AmqpValue(int32_t value) : m_value{amqpvalue_create_int(value)} {}
  AmqpValue::AmqpValue(uint64_t value) : m_value{amqpvalue_create_ulong(value)} {}
  AmqpValue::AmqpValue(int64_t value) : m_value{amqpvalue_create_long(value)} {}
  AmqpValue::AmqpValue(float value) : m_value{amqpvalue_create_float(value)} {}
  AmqpValue::AmqpValue(double value) : m_value{amqpvalue_create_double(value)} {}
  AmqpValue::AmqpValue(char32_t value) : m_value{amqpvalue_create_char(value)} {}
  AmqpValue::AmqpValue(Azure::Core::Uuid const& uuid)
      : m_value{amqpvalue_create_uuid(
          const_cast<unsigned char*>(static_cast<const unsigned char*>(uuid.AsArray().data())))}
  {
  }

  AmqpValue::AmqpValue(std::string const& value) : m_value{amqpvalue_create_string(value.c_str())}
  {
  }
  AmqpValue::AmqpValue(const char* value) : m_value{amqpvalue_create_string(value)} {}

  AmqpValue::AmqpValue() noexcept : m_value{amqpvalue_create_null()} {}
  AmqpValue::AmqpValue(AmqpValue const& that) noexcept
      : m_value{amqpvalue_clone(that.m_value.get())}
  {
  }
  AmqpValue::AmqpValue(AmqpValue&& that) noexcept : m_value{that.m_value.release()}
  {
    that.m_value = nullptr;
  }
  AmqpValue::AmqpValue(AMQP_VALUE value)
  {
    // We shouldn't take ownership of the incoming value, so instead we clone it.
    // if no value is provided, treat it as null.
    if (value)
    {
      m_value.reset(amqpvalue_clone(value));
    }
    else
    {
      m_value.reset(amqpvalue_create_null());
    }
  }

  AmqpValue::operator AMQP_VALUE() const { return m_value.get(); }

  AmqpValue& AmqpValue::operator=(AmqpValue const& that)
  {
    m_value.reset(amqpvalue_clone(that.m_value.get()));
    return *this;
  }
  AmqpValue& AmqpValue::operator=(AmqpValue&& that) noexcept
  {
    m_value.reset(that.m_value.release());
    return *this;
  }

  AmqpValue::operator bool() const
  {
    bool value;
    if (amqpvalue_get_boolean(m_value.get(), &value) != 0)
    {
      throw std::runtime_error("Could not retrieve boolean value");
    }
    return value;
  }

  AmqpValue::operator unsigned char() const
  {
    unsigned char value;
    if (amqpvalue_get_ubyte(m_value.get(), &value) != 0)
    {
      throw std::runtime_error("Could not retrieve ubyte value");
    }
    return value;
  }

  AmqpValue::operator std::int8_t() const
  {
    char value;
    if (amqpvalue_get_byte(m_value.get(), &value) != 0)
    {
      throw std::runtime_error("Could not retrieve byte value");
    }
    return value;
  }

  AmqpValue::operator char() const
  {
    char value;
    if (amqpvalue_get_byte(m_value.get(), &value) != 0)
    {
      throw std::runtime_error("Could not retrieve byte value");
    }
    return value;
  }

  AmqpValue::operator uint16_t() const
  {
    uint16_t value;
    if (amqpvalue_get_ushort(m_value.get(), &value) != 0)
    {
      throw std::runtime_error("Could not retrieve ushort value");
    }
    return value;
  }

  AmqpValue::operator int16_t() const
  {
    int16_t value;
    if (amqpvalue_get_short(m_value.get(), &value) != 0)
    {
      throw std::runtime_error("Could not retrieve short value");
    }
    return value;
  }

  AmqpValue::operator std::uint32_t() const
  {
    std::uint32_t value;
    if (amqpvalue_get_uint(m_value.get(), &value) != 0)
    {
      throw std::runtime_error("Could not retrieve uint value");
    }
    return value;
  }

  AmqpValue::operator std::int32_t() const
  {
    int32_t value;
    if (amqpvalue_get_int(m_value.get(), &value) != 0)
    {
      throw std::runtime_error("Could not retrieve int value");
    }
    return value;
  }

  AmqpValue::operator uint64_t() const
  {
    uint64_t value;
    if (amqpvalue_get_ulong(m_value.get(), &value) != 0)
    {
      throw std::runtime_error("Could not retrieve ulong value");
    }
    return value;
  }

  AmqpValue::operator int64_t() const
  {
    int64_t value;
    if (amqpvalue_get_long(m_value.get(), &value) != 0)
    {
      throw std::runtime_error("Could not retrieve long value");
    }
    return value;
  }

  AmqpValue::operator float() const
  {
    float value;
    if (amqpvalue_get_float(m_value.get(), &value))
    {
      throw std::runtime_error("Could not retrieve float value");
    }
    return value;
  }

  AmqpValue::operator double() const
  {
    double value;
    if (amqpvalue_get_double(m_value.get(), &value))
    {
      throw std::runtime_error("Could not retrieve double value");
    }
    return value;
  }

  AmqpValue::operator char32_t() const
  {
    std::uint32_t value;
    if (amqpvalue_get_char(m_value.get(), &value))
    {
      throw std::runtime_error("Could not get character.");
    }
    return value;
  }

  AmqpValue::operator std::string() const
  {
    const char* value;
    if (amqpvalue_get_string(m_value.get(), &value))
    {
      throw std::runtime_error("Could not retrieve string value");
    }
    return value;
  }

  AmqpValue::operator Azure::Core::Uuid() const
  {
    uuid value;
    if (amqpvalue_get_uuid(m_value.get(), &value))
    {
      throw std::runtime_error("Could not retrieve uuid value");
    }
    std::array<uint8_t, 16> uuidArray;
    memcpy(uuidArray.data(), value, 16);
    return Azure::Core::Uuid::CreateFromArray(uuidArray);
  }

  bool AmqpValue::operator==(AmqpValue const& that) const
  {
    return amqpvalue_are_equal(m_value.get(), that.m_value.get());
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

  AmqpMap AmqpValue::AsMap() const { return AmqpMap(m_value.get()); }

  AmqpArray AmqpValue::AsArray() const { return AmqpArray(m_value.get()); }

  AmqpSymbol AmqpValue::AsSymbol() const { return AmqpSymbol(m_value.get()); }

  AmqpComposite AmqpValue::AsComposite() const { return AmqpComposite(m_value.get()); }
  AmqpList AmqpValue::AsList() const { return AmqpList(m_value.get()); }
  AmqpBinaryData AmqpValue::AsBinary() const { return AmqpBinaryData(m_value.get()); }
  AmqpDescribed AmqpValue::AsDescribed() const { return AmqpDescribed(m_value.get()); }
  AmqpTimestamp AmqpValue::AsTimestamp() const { return AmqpTimestamp(m_value.get()); }

  namespace {

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
  } // namespace

  AmqpValueType AmqpValue::GetType() const
  {
    auto val{UamqpToAmqpTypeMap.find(amqpvalue_get_type(m_value.get()))};
    if (val == UamqpToAmqpTypeMap.end())
    {
      throw std::runtime_error("Unknown AMQP AmqpValue Type");
    }
    return val->second;
  }

  std::ostream& operator<<(std::ostream& os, AmqpValue const& value)
  {
    char* valueAsString = amqpvalue_to_string(value);
    os << valueAsString;
    free(valueAsString);
    return os;
  }

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
    Azure::Core::Amqp::_detail::UniqueAmqpDecoderHandle m_decoder;
    AmqpValue m_decodedValue;

    static void OnAmqpValueDecoded(void* context, AMQP_VALUE value)
    {
      auto deserializer = static_cast<AmqpValueDeserializer*>(context);
      deserializer->m_decodedValue = value;
    }
  };

  AmqpValue AmqpValue::Deserialize(uint8_t const* data, size_t size)
  {
    return AmqpValueDeserializer{}(data, size);
  }

  class AmqpValueSerializer final {
  public:
    AmqpValueSerializer() = default;

    std::vector<uint8_t> operator()(AmqpValue const& value)
    {
      if (amqpvalue_encode(value, OnAmqpValueEncoded, this))
      {
        throw std::runtime_error("Could not encode object");
      }

      return m_encodedValue;
    }

  private:
    std::vector<uint8_t> m_encodedValue;

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
  };

  std::vector<uint8_t> AmqpValue::Serialize(AmqpValue const& value)
  {
    return AmqpValueSerializer{}(value);
  }
  size_t AmqpValue::GetSerializedSize(AmqpValue const& value)
  {
    size_t encodedSize;
    if (amqpvalue_get_encoded_size(value, &encodedSize))
    {
      throw std::runtime_error("Could not get encoded size for value.");
    }
    return encodedSize;
  }

  AmqpArray::AmqpArray(AMQP_VALUE const value)
  {
    if (amqpvalue_get_type(value) != AMQP_TYPE_ARRAY)
    {
      throw std::runtime_error("Input AMQP value MUST be an array.");
    }
    std::uint32_t arraySize;
    if (amqpvalue_get_array_item_count(value, &arraySize))
    {
      throw std::runtime_error("Could not get array size from AMQP_VALUE");
    }
    m_value.reserve(arraySize);
    for (std::uint32_t i = 0; i < arraySize; i += 1)
    {
      m_value.push_back(amqpvalue_get_array_item(value, i));
    }
  }
  AmqpArray::AmqpArray(std::initializer_list<AmqpValue> const& initializer)
      : AmqpCollectionBase(initializer)
  {
    if (initializer.size())
    {
      AmqpValueType expectedType = initializer.begin()->GetType();
      for (auto v : initializer)
      {
        if (v.GetType() != expectedType)
        {
          throw std::runtime_error("Type mismatch creating a new AMQP array.");
        }
      }
    }
  }
  template <>
  _detail::AmqpCollectionBase<std::vector<AmqpValue>, AmqpArray>::operator UniqueAmqpValueHandle()
      const
  {
    UniqueAmqpValueHandle array{amqpvalue_create_array()};
    for (const auto& val : *this)
    {
      if (amqpvalue_add_array_item(array.get(), val))
      {
        throw(std::runtime_error("Could not add value to array."));
      }
    }
    return array;
  }

  AmqpMap::AmqpMap(AMQP_VALUE const value)
  {
    if (amqpvalue_get_type(value) != AMQP_TYPE_MAP)
    {
      throw std::runtime_error("Input AMQP value MUST be a map.");
    }
    std::uint32_t mapSize;
    if (amqpvalue_get_map_pair_count(value, &mapSize))
    {
      throw std::runtime_error("Could not get array size from AMQP_VALUE");
    }
    for (std::uint32_t i = 0; i < mapSize; i += 1)
    {
      UniqueAmqpValueHandle key;
      UniqueAmqpValueHandle val;

      {
        AMQP_VALUE kv, vv;
        amqpvalue_get_map_key_value_pair(value, i, &kv, &vv);
        key.reset(kv);
        val.reset(vv);
      }
      m_value.emplace(std::make_pair(AmqpValue(key.get()), AmqpValue(val.get())));
    }
  }

  template <>
  _detail::AmqpCollectionBase<std::map<AmqpValue, AmqpValue>, AmqpMap>::
  operator UniqueAmqpValueHandle() const
  {
    UniqueAmqpValueHandle value{amqpvalue_create_map()};
    for (const auto& val : *this)
    {
      if (amqpvalue_set_map_value(value.get(), val.first, val.second))
      {
        throw(std::runtime_error("Could not add value to array."));
      }
    }
    return value;
  }

  AmqpList::AmqpList(AMQP_VALUE const value)
  {
    if (amqpvalue_get_type(value) != AMQP_TYPE_LIST)
    {
      throw std::runtime_error("Input AMQP value MUST be an array.");
    }
    std::uint32_t listSize;
    if (amqpvalue_get_list_item_count(value, &listSize))
    {
      throw std::runtime_error("Could not get array size from AMQP_VALUE");
    }
    for (std::uint32_t i = 0; i < listSize; i += 1)
    {
      push_back(amqpvalue_get_list_item(value, i));
    }
  }

  template <>
  _detail::AmqpCollectionBase<std::vector<AmqpValue>, AmqpList>::operator UniqueAmqpValueHandle()
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
      if (amqpvalue_set_list_item(list.get(), i, val))
      {
        throw(std::runtime_error("Could not add value to list."));
      }
      i += 1;
    }
    return list;
  }

  template <>
  _detail::AmqpCollectionBase<std::vector<uint8_t>, AmqpBinaryData>::
  operator UniqueAmqpValueHandle() const
  {
    UniqueAmqpValueHandle binary{amqpvalue_create_binary({data(), static_cast<uint32_t>(size())})};
    return binary;
  }

  AmqpBinaryData::AmqpBinaryData(AMQP_VALUE const value)
  {
    if (amqpvalue_get_type(value) != AMQP_TYPE_BINARY)
    {
      throw std::runtime_error("Input AMQP value MUST be binary.");
    }
    amqp_binary binaryData;
    if (amqpvalue_get_binary(value, &binaryData))
    {
      throw std::runtime_error("Could not retrieve binary data.");
    }
    // Copy the binary data to our storage.
    m_value.assign(
        static_cast<const uint8_t*>(binaryData.bytes),
        static_cast<const uint8_t*>(binaryData.bytes) + binaryData.length);
  }

  template <>
  _detail::AmqpCollectionBase<std::string, AmqpSymbol>::operator UniqueAmqpValueHandle() const
  {
    UniqueAmqpValueHandle symbol{amqpvalue_create_symbol(m_value.c_str())};
    return symbol;
  }

  AmqpSymbol::AmqpSymbol(AMQP_VALUE const value)
  {
    if (amqpvalue_get_type(value) != AMQP_TYPE_SYMBOL)
    {
      throw std::runtime_error("Input AMQP value MUST be a symbol.");
    }
    const char* binaryData;
    if (amqpvalue_get_symbol(value, &binaryData))
    {
      throw std::runtime_error("Could not retrieve binary data.");
    }
    // Copy the binary data to our storage.
    m_value.assign(binaryData);
  }

  AmqpTimestamp::operator UniqueAmqpValueHandle() const
  {
    UniqueAmqpValueHandle symbol{amqpvalue_create_timestamp(m_value.count())};
    return symbol;
  }

  AmqpTimestamp::operator AmqpValue() const
  {
    return static_cast<UniqueAmqpValueHandle>(*this).get();
  }

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
  AmqpTimestamp::AmqpTimestamp(AMQP_VALUE const value) : m_value(GetMillisecondsFromAmqp(value)) {}

  AmqpTimestamp::AmqpTimestamp(std::chrono::milliseconds const& initializer) : m_value(initializer)
  {
  }
  AmqpTimestamp::AmqpTimestamp() : m_value{} {}

  AmqpComposite::AmqpComposite(AMQP_VALUE const value)
  {
    if (amqpvalue_get_type(value) != AMQP_TYPE_COMPOSITE)
    {
      throw std::runtime_error("Input AMQP value MUST be a composite value.");
    }

    std::uint32_t compositeSize;
    if (amqpvalue_get_composite_item_count(value, &compositeSize))
    {
      throw std::runtime_error("Could not get composite size from AMQP_VALUE");
    }
    for (std::uint32_t i = 0; i < compositeSize; i += 1)
    {
      push_back(amqpvalue_get_composite_item_in_place(value, i));
    }

    m_descriptor = amqpvalue_get_inplace_descriptor(value);
    if (m_descriptor.IsNull())
    {
      throw std::runtime_error("Could not read descriptor for composite value.");
    }
  }

  AmqpComposite::AmqpComposite(
      AmqpValue const& descriptor,
      std::initializer_list<std::vector<AmqpValue>::value_type> const& initializer)
      : AmqpCollectionBase{initializer}, m_descriptor{descriptor}
  {
  }

  AmqpComposite::operator UniqueAmqpValueHandle() const
  {
    UniqueAmqpValueHandle composite{
        amqpvalue_create_composite(m_descriptor, static_cast<std::uint32_t>(size()))};
    std::uint32_t i = 0;
    for (const auto& val : *this)
    {
      if (amqpvalue_set_composite_item(composite.get(), i, val))
      {
        throw(std::runtime_error("Could not add value to list."));
      }
      i += 1;
    }
    return composite;
  }

  AmqpDescribed::AmqpDescribed(AMQP_VALUE const value)
  {
    if (amqpvalue_get_type(value) != AMQP_TYPE_DESCRIBED)
    {
      throw std::runtime_error("Input AMQP value MUST be a described value.");
    }

    m_descriptor = amqpvalue_get_inplace_descriptor(value);
    if (m_descriptor.IsNull())
    {
      throw std::runtime_error("Could not read descriptor for described value.");
    }

    m_value = amqpvalue_get_inplace_described_value(value);
    if (m_value.IsNull())
    {
      throw std::runtime_error("Could not read descriptor for described value.");
    }
  }

  AmqpDescribed::AmqpDescribed(AmqpSymbol const& descriptor, AmqpValue const& value)
      : m_descriptor(static_cast<UniqueAmqpValueHandle>(descriptor).get()), m_value(value)
  {
  }
  AmqpDescribed::AmqpDescribed(uint64_t descriptor, AmqpValue const& value)
      : m_descriptor(descriptor), m_value(value)
  {
  }

  AmqpDescribed::operator UniqueAmqpValueHandle() const
  {
    // For <reasons>, amqpvalue_create_described does not clone the provided descriptor or value,
    // but amqpvalue_destroy on a described destroys the underlying value. That means we need to
    // manually clone the input descriptors to ensure that the reference counts work out.
    UniqueAmqpValueHandle composite{
        amqpvalue_create_described(amqpvalue_clone(m_descriptor), amqpvalue_clone(m_value))};
    return composite;
  }

  AmqpDescribed::operator AmqpValue const() const
  {
    return static_cast<UniqueAmqpValueHandle>(*this).get();
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
    AmqpValue arrayValue(value);
    os << arrayValue;
    return os;
  }

  std::ostream& operator<<(std::ostream& os, AmqpList const& value)
  {
    // Let the AmqpValue specialization handle serialization of the list.
    AmqpValue arrayValue(value);
    os << arrayValue;
    return os;
  }

  std::ostream& operator<<(std::ostream& os, AmqpMap const& value)
  {
    // Let the AmqpValue specialization handle serialization of the map.
    AmqpValue mapValue(value);
    os << mapValue;
    return os;
  }
  std::ostream& operator<<(std::ostream& os, AmqpSymbol const& value)
  {
    // Let the AmqpValue specialization handle serialization of the array.
    AmqpValue arrayValue(value);
    os << arrayValue;
    return os;
  }

  bool AmqpValue::IsNull() const
  {
    return (m_value == nullptr) || (amqpvalue_get_type(m_value.get()) == AMQP_TYPE_NULL);
  }
}}}} // namespace Azure::Core::Amqp::Models
