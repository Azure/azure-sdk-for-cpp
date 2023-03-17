// Copyright(c) Microsoft Corporation.All rights reserved.
// SPDX - License - Identifier : MIT

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

  class Value {
    AMQP_VALUE_DATA_TAG* m_value;

  public:
    ~Value();

    Value(bool bool_value);
    Value(unsigned char byte_value);
    Value(char value);
    Value(uint16_t value);
    Value(int16_t value);
    Value(uint32_t value);
    Value(int32_t value);
    Value(uint64_t value);
    Value(int64_t value);
    Value(float value);
    Value(double value);

    /* ???? */
    //    AMQPValue(uint32_t value) : m_value{amqpvalue_create_char(value)} {}
    //    AMQPValue(timestamp value) : m_value{amqpvalue_create_timestamp(value)} {}
    //    AMQPValue(std::string const& value) : m_value{amqpvalue_create_symbol(value.c_str())} {}

    Value(Uuid value);
    Value(BinaryData value);
    Value(std::string value);
    Value(const char* value);

    Value();
    Value(Value const& that) throw();
    Value(Value&& that) throw();

    // Interoperability functions for uAMQP
    operator AMQP_VALUE_DATA_TAG*() const;
    Value(AMQP_VALUE_DATA_TAG* value);

    Value& operator=(Value const& that);
    Value& operator=(Value const& that) const;
    Value& operator=(Value&& that) throw();

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

    bool operator==(Value const& that) const;
    AmqpValueType GetType() const;

    // List Operations.
    static Value CreateList();
    void SetListItemCount(uint32_t count);
    uint32_t GetListItemCount() const;
    void SetListItem(uint32_t index, Value item);
    Value GetListItem(size_t index) const;
    //    AMQPValue GetListItemInPlace(size_t index) const;

    // Map operations.
    static Value CreateMap();
    void SetMapValue(Value key, Value value);
    Value GetMapValue(Value key) const;
    std::pair<Value, Value> GetMapKeyAndValue(uint32_t index) const;
    size_t GetMapValueCount() const;

    // Array operations - note that all array items must be of the same type.
    static Value CreateArray();
    void AddArrayItem(Value itemValue);
    Value GetArrayItem(uint32_t index) const;
    uint32_t GetArrayItemCount() const;

    // Char
    static Value CreateChar(uint32_t value);
    uint32_t GetChar() const;

    // Timestamps
    static Value CreateTimestamp(std::chrono::milliseconds value);
    std::chrono::milliseconds GetTimestamp() const;

    // Symbols
    static Value CreateSymbol(std::string const& value);
    std::string GetSymbol() const;

    // Composite values - A composite value is functionally a list with a fixed size.
    static Value CreateComposite(Value descriptor, uint32_t listSize);
    void SetCompositeItem(uint32_t index, Value itemValue);
    Value GetCompositeItem(uint32_t index);
    //    AMQPValue GetCompositeItemInPlace(size_t index) const;
    size_t GetCompositeItemCount() const;

    static Value CreateDescribed(Value descriptor, Value value);
    static Value CreateCompositeWithDescriptor(uint64_t descriptor);

    // Headers.
    bool IsHeaderTypeByDescriptor() const;
    Header GetHeaderFromValue() const;

    static Value CreateHeader(Header const& header);

    // Properties.
    bool IsPropertiesTypeByDescriptor() const;
    Properties GetPropertiesFromValue() const;

    static Value CreateProperties(Properties const& properties);

    friend std::ostream& operator<<(std::ostream& os, Value const& value);
  };

#if 0
  /* type handling */

  /* encoding */
  typedef int (*AMQPVALUE_ENCODER_OUTPUT)(void* context, const unsigned char* bytes, size_t length);

  MOCKABLE_FUNCTION(
      ,
      int,
      amqpvalue_encode,
      AMQP_VALUE,
      value,
      AMQPVALUE_ENCODER_OUTPUT,
      encoder_output,
      void*,
      context);
  MOCKABLE_FUNCTION(, int, amqpvalue_get_encoded_size, AMQP_VALUE, value, size_t*, encoded_size);

  /* decoding */
  typedef struct AMQPVALUE_DECODER_HANDLE_DATA_TAG* AMQPVALUE_DECODER_HANDLE;
  typedef void (*ON_VALUE_DECODED)(void* context, AMQP_VALUE decoded_value);

  MOCKABLE_FUNCTION(
      ,
      AMQPVALUE_DECODER_HANDLE,
      amqpvalue_decoder_create,
      ON_VALUE_DECODED,
      on_value_decoded,
      void*,
      callback_context);
  MOCKABLE_FUNCTION(, void, amqpvalue_decoder_destroy, AMQPVALUE_DECODER_HANDLE, handle);
  MOCKABLE_FUNCTION(
      ,
      int,
      amqpvalue_decode_bytes,
      AMQPVALUE_DECODER_HANDLE,
      handle,
      const unsigned char*,
      buffer,
      size_t,
      size);

#endif
}}}} // namespace Azure::Core::Amqp::Models