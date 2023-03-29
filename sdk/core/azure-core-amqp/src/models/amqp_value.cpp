// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/models/amqp_value.hpp"
#include "azure/core/amqp/models/amqp_properties.hpp"

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

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  Value::~Value()
  {
    if (m_value)
    {
      amqpvalue_destroy(m_value);
    }
  }
  Value::Value(bool bool_value) : m_value{amqpvalue_create_boolean(bool_value)} {}
  Value::Value(unsigned char byte_value) : m_value{amqpvalue_create_ubyte(byte_value)} {}
  Value::Value(char value) : m_value{amqpvalue_create_byte(value)} {}
  Value::Value(uint16_t value) : m_value{amqpvalue_create_ushort(value)} {}
  Value::Value(int16_t value) : m_value{amqpvalue_create_short(value)} {}
  Value::Value(uint32_t value) : m_value{amqpvalue_create_uint(value)} {}
  Value::Value(int32_t value) : m_value{amqpvalue_create_int(value)} {}
  Value::Value(uint64_t value) : m_value{amqpvalue_create_ulong(value)} {}
  Value::Value(int64_t value) : m_value{amqpvalue_create_long(value)} {}
  Value::Value(float value) : m_value{amqpvalue_create_float(value)} {}
  Value::Value(double value) : m_value{amqpvalue_create_double(value)} {}

  /* ???? */
  //    AMQPValue(uint32_t value) : m_value{amqpvalue_create_char(value)} {}
  //    AMQPValue(timestamp value) : m_value{amqpvalue_create_timestamp(value)} {}
  //    AMQPValue(std::string const& value) : m_value{amqpvalue_create_symbol(value.c_str())} {}

  Value::Value(Uuid value) : m_value{amqpvalue_create_uuid(value.data())} {}
  Value::Value(BinaryData value)
  {
    amqp_binary amqpValue;
    amqpValue.bytes = value.bytes;
    amqpValue.length = static_cast<uint32_t>(value.length);
    m_value = amqpvalue_create_binary(amqpValue);
  }
  Value::Value(std::string value) : m_value{amqpvalue_create_string(value.c_str())} {}
  Value::Value(const char* value) : m_value{amqpvalue_create_string(value)} {}

  Value::Value() : m_value{amqpvalue_create_null()} {}
  Value::Value(Value const& that) throw() : m_value{amqpvalue_clone(that.m_value)} {}
  Value::Value(Value&& that) throw() : m_value{that.m_value} { that.m_value = nullptr; }
  Value::Value(AMQP_VALUE_DATA_TAG* value)
  {
    // We shouldn't take ownership of the incoming value, so instead we clone it.
    // if no value is provided, treat it as null.
    if (value)
    {
      m_value = amqpvalue_clone(value);
    }
    else
    {
      m_value = amqpvalue_create_null();
    }
  }

  Value::operator AMQP_VALUE_DATA_TAG*() const { return m_value; }

  Value& Value::operator=(Value const& that)
  {
    m_value = amqpvalue_clone(that.m_value);
    return *this;
  }
  Value& Value::operator=(Value&& that) throw()
  {
    m_value = that.m_value;
    that.m_value = nullptr;
    return *this;
  }

  Value::operator bool() const
  {
    bool value;
    if (amqpvalue_get_boolean(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator bool()
  {
    bool value;
    if (amqpvalue_get_boolean(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator unsigned char() const
  {
    unsigned char value;
    if (amqpvalue_get_ubyte(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator unsigned char()
  {
    unsigned char value;
    if (amqpvalue_get_ubyte(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator char() const
  {
    char value;
    if (amqpvalue_get_byte(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator char()
  {
    char value;
    if (amqpvalue_get_byte(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator uint16_t() const
  {
    uint16_t value;
    if (amqpvalue_get_ushort(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator uint16_t()
  {
    uint16_t value;
    if (amqpvalue_get_ushort(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator int16_t() const
  {
    int16_t value;
    if (amqpvalue_get_short(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator int16_t()
  {
    int16_t value;
    if (amqpvalue_get_short(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator uint32_t() const
  {
    uint32_t value;
    if (amqpvalue_get_uint(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator uint32_t()
  {
    uint32_t value;
    if (amqpvalue_get_uint(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator int32_t() const
  {
    int32_t value;
    if (amqpvalue_get_int(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator int32_t()
  {
    int32_t value;
    if (amqpvalue_get_int(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator uint64_t() const
  {
    uint64_t value;
    if (amqpvalue_get_ulong(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator uint64_t()
  {
    uint64_t value;
    if (amqpvalue_get_ulong(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator int64_t() const
  {
    int64_t value;
    if (amqpvalue_get_long(m_value, &value) != 0)
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator int64_t()
  {
    int64_t value;
    if (amqpvalue_get_long(m_value, &value))
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator float() const
  {
    float value;
    if (amqpvalue_get_float(m_value, &value))
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator float()
  {
    float value;
    if (amqpvalue_get_float(m_value, &value))
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator double() const
  {
    double value;
    if (amqpvalue_get_double(m_value, &value))
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator double()
  {
    double value;
    if (amqpvalue_get_double(m_value, &value))
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator BinaryData() const
  {
    amqp_binary value;
    if (amqpvalue_get_binary(m_value, &value))
    {
      throw std::runtime_error("Could not retrieve value");
    }
    BinaryData rv;
    rv.bytes = static_cast<const uint8_t*>(value.bytes);
    rv.length = value.length;
    return rv;
  }
  Value::operator BinaryData()
  {
    amqp_binary value;
    if (amqpvalue_get_binary(m_value, &value))
    {
      throw std::runtime_error("Could not retrieve value");
    }
    BinaryData rv;
    rv.bytes = static_cast<const uint8_t*>(value.bytes);
    rv.length = value.length;
    return rv;
  }

  Value::operator std::string() const
  {
    const char* value;
    if (amqpvalue_get_string(m_value, &value))
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }
  Value::operator std::string()
  {
    const char* value;
    if (amqpvalue_get_string(m_value, &value))
    {
      throw std::runtime_error("Could not retrieve value");
    }
    return value;
  }

  bool Value::operator==(Value const& that) const
  {
    return amqpvalue_are_equal(m_value, that.m_value);
  }

  Value Value::CreateList() { return Value(amqpvalue_create_list()); }
  void Value::SetListItemCount(uint32_t count)
  {
    if (amqpvalue_set_list_item_count(m_value, count))
    {
      throw std::runtime_error("Could not set List item count");
    }
  }
  uint32_t Value::GetListItemCount() const
  {
    uint32_t count;
    if (amqpvalue_get_list_item_count(m_value, &count) != 0)
    {
      throw std::runtime_error("Could not get List item count");
    }
    return count;
  }
  void Value::SetListItem(uint32_t index, Value item)
  {
    if (amqpvalue_set_list_item(m_value, index, item))
    {
      throw std::runtime_error("Could not set List item count");
    }
  }
  Value Value::GetListItem(size_t index) const
  {
    AMQP_VALUE item = amqpvalue_get_list_item(m_value, index);
    return item;
  }
  //  AMQPValue AMQPValue::GetListItemInPlace(size_t index) const { return AMQPValue(); }
  Value Value::CreateMap() { return Value(amqpvalue_create_map()); }
  void Value::SetMapValue(Value key, Value value)
  {
    if (amqpvalue_set_map_value(m_value, key, value))
    {
      throw std::runtime_error("Could not set map value.");
    }
  }

  Value Value::GetMapValue(Value key) const { return amqpvalue_get_map_value(m_value, key); }
  std::pair<Value, Value> Value::GetMapKeyAndValue(uint32_t index) const
  {
    AMQP_VALUE key;
    AMQP_VALUE value;
    if (amqpvalue_get_map_key_value_pair(m_value, index, &key, &value))
    {
      throw std::runtime_error("Could not set map value.");
    }
    return std::make_pair(Value{key}, Value{value});
  }

  size_t Value::GetMapValueCount() const
  {
    uint32_t count;
    if (amqpvalue_get_map_pair_count(m_value, &count))
    {
      throw std::runtime_error("Could not get map size.");
    }
    return count;
  }

  Value Value::CreateArray() { return Value(amqpvalue_create_array()); }

  void Value::AddArrayItem(Value itemValue)
  {
    if (amqpvalue_add_array_item(m_value, itemValue))
    {
      throw std::runtime_error("Could not add array item.");
    }
  }

  Value Value::GetArrayItem(uint32_t index) const
  {
    return amqpvalue_get_array_item(m_value, index);
  }

  uint32_t Value::GetArrayItemCount() const
  {
    uint32_t count;
    if (amqpvalue_get_array_item_count(m_value, &count))
    {
      throw std::runtime_error("Could not get array item count.");
    }
    return count;
  }

  Value Value::CreateChar(uint32_t value) { return amqpvalue_create_char(value); }

  uint32_t Value::GetChar() const
  {
    uint32_t value;
    if (amqpvalue_get_char(m_value, &value))
    {
      throw std::runtime_error("Could not get character.");
    }
    return value;
  }

  Value Value::CreateTimestamp(std::chrono::milliseconds value)
  {
    return amqpvalue_create_timestamp(value.count());
  }

  std::chrono::milliseconds Value::GetTimestamp() const
  {
    int64_t ms;
    if (amqpvalue_get_timestamp(m_value, &ms))
    {
      throw std::runtime_error("Could not get timestamp.");
    }
    return std::chrono::milliseconds(ms);
  }

  Value Value::CreateSymbol(std::string const& value)
  {
    return amqpvalue_create_symbol(value.c_str());
  }
  std::string Value::GetSymbol() const
  {
    const char* symbol;
    if (amqpvalue_get_symbol(m_value, &symbol))
    {
      throw std::runtime_error("Could not get symbol.");
    }
    return symbol;
  }

  AmqpValueType Value::GetType() const
  {
    switch (amqpvalue_get_type(m_value))
    {
      case AMQP_TYPE_INVALID: // LCOV_EXCL_LINE
        return AmqpValueType::Invalid; // LCOV_EXCL_LINE
      case AMQP_TYPE_NULL:
        return AmqpValueType::Null;
      case AMQP_TYPE_BOOL:
        return AmqpValueType::Bool;
      case AMQP_TYPE_UBYTE:
        return AmqpValueType::UByte;
      case AMQP_TYPE_USHORT:
        return AmqpValueType::UShort;
      case AMQP_TYPE_UINT:
        return AmqpValueType::UInt;
      case AMQP_TYPE_ULONG:
        return AmqpValueType::ULong;
      case AMQP_TYPE_BYTE:
        return AmqpValueType::Byte;
      case AMQP_TYPE_SHORT:
        return AmqpValueType::Short;
      case AMQP_TYPE_INT:
        return AmqpValueType::Int;
      case AMQP_TYPE_LONG:
        return AmqpValueType::Long;
      case AMQP_TYPE_FLOAT:
        return AmqpValueType::Float;
      case AMQP_TYPE_DOUBLE:
        return AmqpValueType::Double;
      case AMQP_TYPE_CHAR:
        return AmqpValueType::Char;
      case AMQP_TYPE_TIMESTAMP:
        return AmqpValueType::Timestamp;
      case AMQP_TYPE_UUID:
        return AmqpValueType::Uuid;
      case AMQP_TYPE_BINARY:
        return AmqpValueType::Binary;
      case AMQP_TYPE_STRING:
        return AmqpValueType::String;
      case AMQP_TYPE_SYMBOL:
        return AmqpValueType::Symbol;
      case AMQP_TYPE_LIST:
        return AmqpValueType::List;
      case AMQP_TYPE_MAP:
        return AmqpValueType::Map;
      case AMQP_TYPE_ARRAY:
        return AmqpValueType::Array;
      case AMQP_TYPE_DESCRIBED:
        return AmqpValueType::Described;
      case AMQP_TYPE_COMPOSITE:
        return AmqpValueType::Composite;
      case AMQP_TYPE_UNKNOWN:
        return AmqpValueType::Unknown;
    }
    throw std::runtime_error("Unknown AMQP Value Type");
  }

  Value Value::CreateComposite(Value descriptor, uint32_t listSize)
  {
    return amqpvalue_create_composite(descriptor, listSize);
  }
  void Value::SetCompositeItem(uint32_t index, Value itemValue)
  {
    if (amqpvalue_set_composite_item(m_value, index, itemValue))
    {
      throw std::runtime_error("Could not set composite item");
    }
  }
  Value Value::GetCompositeItem(uint32_t index)
  {
    return amqpvalue_get_composite_item(m_value, index);
  }
  //  AMQPValue AMQPValue::GetCompositeItemInPlace(size_t index) const { return AMQPValue(); }
  size_t Value::GetCompositeItemCount() const
  {
    uint32_t size;
    if (amqpvalue_get_composite_item_count(m_value, &size))
    {
      throw std::runtime_error("Could not set composite item");
    }
    return size;
  }
  Value Value::CreateDescribed(Value descriptor, Value value)
  {
    // amqpvalue_create_described takes a reference to the input parameters, we need to stabilize
    // the value of descriptor and value so they don't get accidentally freed.
    return amqpvalue_create_described(amqpvalue_clone(descriptor), amqpvalue_clone(value));
  }

  Value Value::GetDescriptor() const { return amqpvalue_get_inplace_descriptor(m_value); }

  Value Value::GetDescribedValue() const { return amqpvalue_get_inplace_described_value(m_value); }

  Value Value::CreateCompositeWithDescriptor(uint64_t descriptor)
  {
    return amqpvalue_create_composite_with_ulong_descriptor(descriptor);
  }
  bool Value::IsHeaderTypeByDescriptor() const { return is_header_type_by_descriptor(m_value); }
  Header Value::GetHeaderFromValue() const
  {
    HEADER_HANDLE header;
    if (amqpvalue_get_header(m_value, &header))
    {
      throw std::runtime_error("Could not get header from value");
    }
    return header;
  }
  Value Value::CreateHeader(Header const& header) { return amqpvalue_create_header(header); }

  bool Value::IsPropertiesTypeByDescriptor() const
  {
    return is_properties_type_by_descriptor(m_value);
  }

  Properties Value::GetPropertiesFromValue() const
  {
    PROPERTIES_HANDLE properties;
    if (amqpvalue_get_properties(m_value, &properties))
    {
      throw std::runtime_error("Could not get properties from value");
    }
    return properties;
  }

  Value Value::CreateProperties(Properties const& properties)
  {
    return amqpvalue_create_properties(properties);
  }

  std::ostream& operator<<(std::ostream& os, Value const& value)
  {
    char* valueAsString = amqpvalue_to_string(value);
    os << valueAsString;
    free(valueAsString);
    return os;
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

  std::ostream& operator<<(std::ostream& os, BinaryData const& value)
  {
    const uint8_t* pb = value.bytes;
    size_t cb = value.length;
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

  bool Value::IsNull() const
  {
    return (m_value == nullptr) || (amqpvalue_get_type(m_value) == AMQP_TYPE_NULL);
  }

}}}} // namespace Azure::Core::Amqp::Models
