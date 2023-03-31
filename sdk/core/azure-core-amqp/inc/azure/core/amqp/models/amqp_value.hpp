// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_header.hpp"

#include <array>
#include <chrono>
#include <exception>
#include <functional>
#include <string>
struct AMQP_VALUE_DATA_TAG;

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  class Properties;
  struct BinaryData
  {
    const uint8_t* bytes;
    size_t length;

    friend std::ostream& operator<<(std::ostream& os, BinaryData const& value);
  };

  enum class TerminusDurability : uint32_t
  {
    None = 0,
    Configuration = 1,
    UnsettledState = 2
  };

  // Note : Should be an extendable Enumeration.
  enum class TerminusExpiryPolicy
  {
    LinkDetach,
    SessionEnd,
    ConnectionClose,
    Never
  };

  using Uuid = std::array<unsigned char, 16>;

  enum class AmqpValueType
  {
    Invalid,
    Null,
    Bool,
    UByte,
    UShort,
    UInt,
    ULong,
    Byte,
    Short,
    Int,
    Long,
    Float,
    Double,
    Char,
    Timestamp,
    Uuid,
    Binary,
    String,
    Symbol,
    List,
    Map,
    Array,
    Described,
    Composite,
    Unknown
  };

  class AmqpValue {
  public:
    ~AmqpValue();

    AmqpValue(bool bool_AmqpValue);
    AmqpValue(unsigned char byte_value);
    AmqpValue(char value);
    AmqpValue(uint16_t value);
    AmqpValue(int16_t value);
    AmqpValue(uint32_t value);
    AmqpValue(int32_t value);
    AmqpValue(uint64_t value);
    AmqpValue(int64_t value);
    AmqpValue(float value);
    AmqpValue(double value);

    /* ???? */
    //    AmqpValue(uint32_t value) : m_value{amqpvalue_create_char(value)} {}
    //    AmqpValue(timestamp value) : m_value{amqpvalue_create_timestamp(value)} {}
    //    AmqpValue(std::string const& value) : m_value{amqpvalue_create_symbol(value.c_str())} {}

    AmqpValue(Uuid value);
    AmqpValue(BinaryData value);
    explicit AmqpValue(std::string value);
    AmqpValue(const char* value);

    AmqpValue();
    AmqpValue(AmqpValue const& that) throw();
    AmqpValue(AmqpValue&& that) throw();

    // Interoperability functions for uAMQP
    operator AMQP_VALUE_DATA_TAG*() const;
    AmqpValue(AMQP_VALUE_DATA_TAG* value);

    AmqpValue& operator=(AmqpValue const& that);
    AmqpValue& operator=(AmqpValue&& that) throw();

    bool IsNull() const;

    operator bool() const;
    operator bool();
    operator unsigned char() const;
    operator unsigned char();
    operator char() const;
    operator char();
    operator uint16_t() const;
    operator uint16_t();
    operator int16_t() const;
    operator int16_t();
    operator uint32_t() const;
    operator uint32_t();
    operator int32_t() const;
    operator int32_t();
    operator uint64_t() const;
    operator uint64_t();
    operator int64_t() const;
    operator int64_t();
    operator float() const;
    operator float();
    operator double() const;
    operator double();
    operator BinaryData() const;
    operator BinaryData();
    explicit operator std::string() const;
    explicit operator std::string();
    operator Uuid();
    operator Uuid() const;

    bool operator==(AmqpValue const& that) const;
    AmqpValueType GetType() const;

    // List Operations.
    static AmqpValue CreateList();
    void SetListItemCount(uint32_t count);
    uint32_t GetListItemCount() const;
    void SetListItem(uint32_t index, AmqpValue item);
    AmqpValue GetListItem(size_t index) const;
    //    AMQPAmqpValue GetListItemInPlace(size_t index) const;

    // Map operations.
    static AmqpValue CreateMap();
    void SetMapValue(AmqpValue key, AmqpValue value);
    AmqpValue GetMapValue(AmqpValue key) const;
    std::pair<AmqpValue, AmqpValue> GetMapKeyAndValue(uint32_t index) const;
    size_t GetMapValueCount() const;

    // Array operations - note that all array items must be of the same type.
    static AmqpValue CreateArray();
    void AddArrayItem(AmqpValue itemAmqpValue);
    AmqpValue GetArrayItem(uint32_t index) const;
    uint32_t GetArrayItemCount() const;

    // Char
    static AmqpValue CreateChar(uint32_t value);
    uint32_t GetChar() const;

    // Timestamps
    static AmqpValue CreateTimestamp(std::chrono::milliseconds value);
    std::chrono::milliseconds GetTimestamp() const;

    // Symbols
    static AmqpValue CreateSymbol(std::string const& value);
    std::string GetSymbol() const;

    // Composite values - A composite value is functionally a list with a fixed size.
    static AmqpValue CreateComposite(AmqpValue descriptor, uint32_t listSize);
    void SetCompositeItem(uint32_t index, AmqpValue itemAmqpValue);
    AmqpValue GetCompositeItem(uint32_t index);
    //    AMQPAmqpValue GetCompositeItemInPlace(size_t index) const;
    size_t GetCompositeItemCount() const;

    static AmqpValue CreateDescribed(AmqpValue descriptor, AmqpValue value);
    static AmqpValue CreateCompositeWithDescriptor(uint64_t descriptor);

    // Descriptors
    AmqpValue GetDescriptor() const;
    AmqpValue GetDescribedValue() const;

    // Headers.
    bool IsHeaderTypeByDescriptor() const;
    Header GetHeaderFromValue() const;

    static AmqpValue CreateHeader(Header const& header);

    // Properties.
    bool IsPropertiesTypeByDescriptor() const;
    Properties GetPropertiesFromValue() const;

    static AmqpValue CreateProperties(Properties const& properties);

    friend std::ostream& operator<<(std::ostream& os, AmqpValue const& value);

  protected:
    AMQP_VALUE_DATA_TAG* m_value;
  };

  /** @brief An AmqpMap represents an AMQP "map" type. which maps a key to a value.
   *
   */
  class AmqpMap : public AmqpValue {};

}}}} // namespace Azure::Core::Amqp::Models
