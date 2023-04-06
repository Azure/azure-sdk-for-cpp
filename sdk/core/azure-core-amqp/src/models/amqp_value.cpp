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

namespace Azure { namespace Core { namespace _internal {
  void UniqueHandleHelper<AMQP_VALUE_DATA_TAG>::FreeAmqpValue(AMQP_VALUE value)
  {
    amqpvalue_destroy(value);
  }
}}} // namespace Azure::Core::_internal
namespace Azure { namespace Core { namespace Amqp { namespace Models {

    AmqpValue::~AmqpValue() {}
    AmqpValue::AmqpValue(bool bool_value) : m_value{amqpvalue_create_boolean(bool_value)} {}
    AmqpValue::AmqpValue(unsigned char byte_value) : m_value{amqpvalue_create_ubyte(byte_value)} {}
    AmqpValue::AmqpValue(std::int8_t value) : m_value{amqpvalue_create_byte(value)} {}
    AmqpValue::AmqpValue(uint16_t value) : m_value{amqpvalue_create_ushort(value)} {}
    AmqpValue::AmqpValue(int16_t value) : m_value{amqpvalue_create_short(value)} {}
    AmqpValue::AmqpValue(std::uint32_t value) : m_value{amqpvalue_create_uint(value)} {}
    AmqpValue::AmqpValue(int32_t value) : m_value{amqpvalue_create_int(value)} {}
    AmqpValue::AmqpValue(uint64_t value) : m_value{amqpvalue_create_ulong(value)} {}
    AmqpValue::AmqpValue(int64_t value) : m_value{amqpvalue_create_long(value)} {}
    AmqpValue::AmqpValue(float value) : m_value{amqpvalue_create_float(value)} {}
    AmqpValue::AmqpValue(double value) : m_value{amqpvalue_create_double(value)} {}

    /* ???? */
    //    AMQPValue(std::uint32_t value) : m_value{amqpvalue_create_char(value)} {}
    //    AMQPValue(timestamp value) : m_value{amqpvalue_create_timestamp(value)} {}
    //    AMQPValue(std::string const& value) : m_value{amqpvalue_create_symbol(value.c_str())} {}

    //  AmqpValue::AmqpValue(Azure::Core::Uuid value) : m_value{amqpvalue_create_uuid(value.data())}
    AmqpValue::AmqpValue(std::string value) : m_value{amqpvalue_create_string(value.c_str())} {}
    AmqpValue::AmqpValue(const char* value) : m_value{amqpvalue_create_string(value)} {}

    AmqpValue::AmqpValue() : m_value{amqpvalue_create_null()} {}
    AmqpValue::AmqpValue(AmqpValue const& that) noexcept
        : m_value{amqpvalue_clone(that.m_value.get())}
    {
    }
    AmqpValue::AmqpValue(AmqpValue&& that) noexcept : m_value{that.m_value.release()}
    {
      that.m_value = nullptr;
    }
    AmqpValue::AmqpValue(AMQP_VALUE_DATA_TAG* value)
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

    AmqpValue::operator AMQP_VALUE_DATA_TAG*() const { return m_value.get(); }

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
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator bool()
    {
      bool value;
      if (amqpvalue_get_boolean(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator unsigned char() const
    {
      unsigned char value;
      if (amqpvalue_get_ubyte(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator unsigned char()
    {
      unsigned char value;
      if (amqpvalue_get_ubyte(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator std::int8_t() const
    {
      char value;
      if (amqpvalue_get_byte(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator std::int8_t()
    {
      char value;
      if (amqpvalue_get_byte(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator uint16_t() const
    {
      uint16_t value;
      if (amqpvalue_get_ushort(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator uint16_t()
    {
      uint16_t value;
      if (amqpvalue_get_ushort(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator int16_t() const
    {
      int16_t value;
      if (amqpvalue_get_short(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator int16_t()
    {
      int16_t value;
      if (amqpvalue_get_short(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator std::uint32_t() const
    {
      std::uint32_t value;
      if (amqpvalue_get_uint(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator std::uint32_t()
    {
      std::uint32_t value;
      if (amqpvalue_get_uint(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator int32_t() const
    {
      int32_t value;
      if (amqpvalue_get_int(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator int32_t()
    {
      int32_t value;
      if (amqpvalue_get_int(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator uint64_t() const
    {
      uint64_t value;
      if (amqpvalue_get_ulong(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator uint64_t()
    {
      uint64_t value;
      if (amqpvalue_get_ulong(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator int64_t() const
    {
      int64_t value;
      if (amqpvalue_get_long(m_value.get(), &value) != 0)
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator int64_t()
    {
      int64_t value;
      if (amqpvalue_get_long(m_value.get(), &value))
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator float() const
    {
      float value;
      if (amqpvalue_get_float(m_value.get(), &value))
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator float()
    {
      float value;
      if (amqpvalue_get_float(m_value.get(), &value))
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator double() const
    {
      double value;
      if (amqpvalue_get_double(m_value.get(), &value))
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator double()
    {
      double value;
      if (amqpvalue_get_double(m_value.get(), &value))
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }

    AmqpValue::operator std::string() const
    {
      const char* value;
      if (amqpvalue_get_string(m_value.get(), &value))
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
    }
    AmqpValue::operator std::string()
    {
      const char* value;
      if (amqpvalue_get_string(m_value.get(), &value))
      {
        throw std::runtime_error("Could not retrieve value");
      }
      return value;
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
        case AmqpValueType::UByte:
          return static_cast<uint8_t>(*this) < static_cast<uint8_t>(that);
        case AmqpValueType::UShort:
          return static_cast<uint16_t>(*this) < static_cast<uint16_t>(that);
        case AmqpValueType::UInt:
          return static_cast<uint32_t>(*this) < static_cast<uint32_t>(that);
        case AmqpValueType::ULong:
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
          return GetChar() < that.GetChar();
        case AmqpValueType::Timestamp:
          return static_cast<std::chrono::milliseconds>(*this)
              < static_cast<std::chrono::milliseconds>(that);
        case AmqpValueType::String:
          return static_cast<std::string>(*this) < static_cast<std::string>(that);
        case AmqpValueType::Symbol:
          return AsSymbol() < that.AsSymbol();

        case AmqpValueType::Map:
          return AsMap() < that.AsMap();
        case AmqpValueType::Array:
          return AsArray() < that.AsArray();

        case AmqpValueType::Uuid:
        case AmqpValueType::Binary:
        case AmqpValueType::List:
        case AmqpValueType::Described:
        case AmqpValueType::Composite:
        default:
          throw std::logic_error("Unknown Amqp Value type in operator<");
      }
    }

    // AmqpValue AmqpValue::CreateList() { return AmqpValue(amqpvalue_create_list()); }
    // void AmqpValue::SetListItemCount(std::uint32_t count)
    //{
    //   if (amqpvalue_set_list_item_count(m_value.get(), count))
    //   {
    //     throw std::runtime_error("Could not set List item count");
    //   }
    // }
    // std::uint32_t AmqpValue::GetListItemCount() const
    //{
    //   std::uint32_t count;
    //   if (amqpvalue_get_list_item_count(m_value.get(), &count) != 0)
    //   {
    //     throw std::runtime_error("Could not get List item count");
    //   }
    //   return count;
    // }
    // void AmqpValue::SetListItem(std::uint32_t index, AmqpValue item)
    //{
    //   if (amqpvalue_set_list_item(m_value.get(), index, item))
    //   {
    //     throw std::runtime_error("Could not set List item count");
    //   }
    // }
    // AmqpValue AmqpValue::GetListItem(size_t index) const
    //{
    //   AMQP_VALUE item = amqpvalue_get_list_item(m_value.get(), index);
    //   return item;
    // }

    ////  AMQPValue AMQPValue::GetListItemInPlace(size_t index) const { return AMQPValue(); }

    AmqpMap AmqpValue::AsMap() const { return AmqpMap(m_value.get()); }

    AmqpArray AmqpValue::AsArray() const { return AmqpArray(m_value.get()); }

    AmqpSymbol AmqpValue::AsSymbol() const { return AmqpSymbol(m_value.get()); }

    AmqpValue AmqpValue::CreateChar(std::uint32_t value) { return amqpvalue_create_char(value); }

    std::uint32_t AmqpValue::GetChar() const
    {
      std::uint32_t value;
      if (amqpvalue_get_char(m_value.get(), &value))
      {
        throw std::runtime_error("Could not get character.");
      }
      return value;
    }

    // AmqpValue::AmqpValue(std::chrono::milliseconds const& value)
    //     : m_value{amqpvalue_create_timestamp(value.count())}
    //{
    // }

    // AmqpValue::operator std::chrono::milliseconds const() const
    //{
    //   int64_t ms;
    //   if (amqpvalue_get_timestamp(m_value.get(), &ms))
    //   {
    //     throw std::runtime_error("Could not get timestamp.");
    //   }
    //   return std::chrono::milliseconds(ms);
    // }

    // AmqpValue AmqpValue::CreateSymbol(std::string const& value)
    //{
    //   return amqpvalue_create_symbol(value.c_str());
    // }
    // std::string AmqpValue::GetSymbol() const
    //{
    //   const char* symbol;
    //   if (amqpvalue_get_symbol(m_value.get(), &symbol))
    //   {
    //     throw std::runtime_error("Could not get symbol.");
    //   }
    //   return symbol;
    // }

    AmqpValueType AmqpValue::GetType() const
    {
      switch (amqpvalue_get_type(m_value.get()))
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
      throw std::runtime_error("Unknown AMQP AmqpValue Type");
    }

    AmqpValue AmqpValue::CreateComposite(AmqpValue descriptor, std::uint32_t listSize)
    {
      return amqpvalue_create_composite(descriptor, listSize);
    }
    void AmqpValue::SetCompositeItem(std::uint32_t index, AmqpValue itemValue)
    {
      if (amqpvalue_set_composite_item(m_value.get(), index, itemValue))
      {
        throw std::runtime_error("Could not set composite item");
      }
    }
    AmqpValue AmqpValue::GetCompositeItem(std::uint32_t index)
    {
      return amqpvalue_get_composite_item(m_value.get(), index);
    }
    //  AMQPValue AMQPValue::GetCompositeItemInPlace(size_t index) const { return AMQPValue(); }
    size_t AmqpValue::GetCompositeItemCount() const
    {
      std::uint32_t size;
      if (amqpvalue_get_composite_item_count(m_value.get(), &size))
      {
        throw std::runtime_error("Could not set composite item");
      }
      return size;
    }
    AmqpValue AmqpValue::CreateDescribed(AmqpValue descriptor, AmqpValue value)
    {
      // amqpvalue_create_described takes a reference to the input parameters, we need to stabilize
      // the value of descriptor and value so they don't get accidentally freed.
      return amqpvalue_create_described(amqpvalue_clone(descriptor), amqpvalue_clone(value));
    }

    AmqpValue AmqpValue::GetDescriptor() const
    {
      return amqpvalue_get_inplace_descriptor(m_value.get());
    }

    AmqpValue AmqpValue::GetDescribedValue() const
    {
      return amqpvalue_get_inplace_described_value(m_value.get());
    }

    AmqpValue AmqpValue::CreateCompositeWithDescriptor(uint64_t descriptor)
    {
      return amqpvalue_create_composite_with_ulong_descriptor(descriptor);
    }
    bool AmqpValue::IsHeaderTypeByDescriptor() const
    {
      return is_header_type_by_descriptor(m_value.get());
    }
    Header AmqpValue::GetHeaderFromValue() const
    {
      HEADER_HANDLE header;
      if (amqpvalue_get_header(m_value.get(), &header))
      {
        throw std::runtime_error("Could not get header from value");
      }
      return header;
    }
    AmqpValue AmqpValue::CreateHeader(Header const& header)
    {
      return amqpvalue_create_header(header);
    }

    bool AmqpValue::IsPropertiesTypeByDescriptor() const
    {
      return is_properties_type_by_descriptor(m_value.get());
    }

    MessageProperties AmqpValue::GetPropertiesFromValue() const
    {
      PROPERTIES_HANDLE properties;
      if (amqpvalue_get_properties(m_value.get(), &properties))
      {
        throw std::runtime_error("Could not get properties from value");
      }
      return properties;
    }

    AmqpValue AmqpValue::CreateProperties(MessageProperties const& properties)
    {
      return amqpvalue_create_properties(properties);
    }

    std::ostream& operator<<(std::ostream& os, AmqpValue const& value)
    {
      char* valueAsString = amqpvalue_to_string(value);
      os << valueAsString;
      free(valueAsString);
      return os;
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
      reserve(arraySize);
      for (std::uint32_t i = 0; i < arraySize; i += 1)
      {
        push_back(amqpvalue_get_array_item(value, i));
      }
    }

    AmqpArray::AmqpArray(std::initializer_list<AmqpValue> const& initializer)
        : std::vector<AmqpValue>(initializer)
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
    AmqpArray::AmqpArray() {}

    AmqpArray::operator AMQP_VALUE_DATA_TAG*() const
    {
      Azure::Core::_internal::UniqueHandle<AMQP_VALUE_DATA_TAG> array{amqpvalue_create_array()};
      for (const auto& val : *this)
      {
        if (amqpvalue_add_array_item(array.get(), val))
        {
          throw(std::runtime_error("Could not add value to array."));
        }
      }
      return array.release();
    }
    AmqpArray::operator AmqpValue() const { return static_cast<AMQP_VALUE>(*this); }

    AmqpMap::AmqpMap(AMQP_VALUE const value)
    {
      if (amqpvalue_get_type(value) != AMQP_TYPE_MAP)
      {
        throw std::runtime_error("Input AMQP value MUST be an array.");
      }
      std::uint32_t mapSize;
      if (amqpvalue_get_map_pair_count(value, &mapSize))
      {
        throw std::runtime_error("Could not get array size from AMQP_VALUE");
      }
      for (std::uint32_t i = 0; i < mapSize; i += 1)
      {
        Azure::Core::_internal::UniqueHandle<AMQP_VALUE_DATA_TAG> key;
        Azure::Core::_internal::UniqueHandle<AMQP_VALUE_DATA_TAG> val;

        {
          AMQP_VALUE kv, vv;
          amqpvalue_get_map_key_value_pair(value, i, &kv, &vv);
          key.reset(kv);
          val.reset(vv);
        }
        emplace(std::make_pair(AmqpValue(key.get()), AmqpValue(val.get())));
      }
    }

    AmqpMap::AmqpMap(
        std::initializer_list<std::map<AmqpValue, AmqpValue>::value_type> const& initializer)
        : std::map<AmqpValue, AmqpValue>(initializer)
    {
    }
    AmqpMap::AmqpMap() {}

    AmqpMap::operator AMQP_VALUE_DATA_TAG*() const
    {
      Azure::Core::_internal::UniqueHandle<AMQP_VALUE_DATA_TAG> value{amqpvalue_create_map()};
      for (const auto& val : *this)
      {
        if (amqpvalue_set_map_value(value.get(), val.first, val.second))
        {
          throw(std::runtime_error("Could not add value to array."));
        }
      }
      return value.release();
    }
    AmqpMap::operator const AmqpValue() const { return static_cast<AMQP_VALUE>(*this); }

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

    AmqpList::AmqpList(std::initializer_list<std::vector<AmqpValue>::value_type> const& initializer)
        : std::vector<AmqpValue>(initializer)
    {
    }
    AmqpList::AmqpList() {}

    AmqpList::operator AMQP_VALUE_DATA_TAG*() const
    {
      Azure::Core::_internal::UniqueHandle<AMQP_VALUE_DATA_TAG> list{amqpvalue_create_list()};
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
      return list.release();
    }

    AmqpList::operator const AmqpValue() const { return static_cast<AMQP_VALUE>(*this); }

    AmqpBinaryData::operator AMQP_VALUE_DATA_TAG*() const
    {
      Azure::Core::_internal::UniqueHandle<AMQP_VALUE_DATA_TAG> binary{
          amqpvalue_create_binary({data(), static_cast<uint32_t>(size())})};
      return binary.release();
    }

    AmqpBinaryData::operator const AmqpValue() const { return static_cast<AMQP_VALUE>(*this); }

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
      assign(
          static_cast<const uint8_t*>(binaryData.bytes),
          static_cast<const uint8_t*>(binaryData.bytes) + binaryData.length);
    }

    AmqpBinaryData::AmqpBinaryData(
        std::initializer_list<std::vector<uint8_t>::value_type> const& initializer)
        : std::vector<uint8_t>(initializer)
    {
    }
    AmqpBinaryData::AmqpBinaryData() {}

    AmqpSymbol::operator AMQP_VALUE_DATA_TAG*() const
    {
      Azure::Core::_internal::UniqueHandle<AMQP_VALUE_DATA_TAG> symbol{
          amqpvalue_create_symbol(c_str())};
      return symbol.release();
    }

    AmqpSymbol::operator const AmqpValue() const { return static_cast<AMQP_VALUE>(*this); }

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
      assign(binaryData);
    }

    AmqpSymbol::AmqpSymbol(std::string const& initializer) : std::string(initializer) {}
    AmqpSymbol::AmqpSymbol() {}

    AmqpTimestamp::operator AMQP_VALUE_DATA_TAG*() const
    {
      Azure::Core::_internal::UniqueHandle<AMQP_VALUE_DATA_TAG> symbol{
          amqpvalue_create_timestamp(count())};
      return symbol.release();
    }

    AmqpTimestamp::operator const AmqpValue() const { return static_cast<AMQP_VALUE>(*this); }

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
    AmqpTimestamp::AmqpTimestamp(AMQP_VALUE const value)
        : std::chrono::milliseconds(GetMillisecondsFromAmqp(value))
    {
    }

    AmqpTimestamp::AmqpTimestamp(std::chrono::milliseconds const& initializer)
        : std::chrono::milliseconds(initializer)
    {
    }
    AmqpTimestamp::AmqpTimestamp() {}

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

    bool AmqpValue::IsNull() const
    {
      return (m_value == nullptr) || (amqpvalue_get_type(m_value.get()) == AMQP_TYPE_NULL);
    }

}}}} // namespace Azure::Core::Amqp::Models
