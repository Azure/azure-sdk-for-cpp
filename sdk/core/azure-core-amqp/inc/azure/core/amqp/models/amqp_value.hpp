// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "amqp_header.hpp"

#include <array>
#include <azure/core/internal/unique_handle.hpp>
#include <azure/core/uuid.hpp>
#include <chrono>
#include <cstdint>
#include <exception>
#include <functional>
#include <list>
#include <map>
#include <string>
#include <vector>

struct AMQP_VALUE_DATA_TAG;

namespace Azure { namespace Core { namespace _internal {

  template <> struct UniqueHandleHelper<AMQP_VALUE_DATA_TAG>
  {
    static void FreeAmqpValue(AMQP_VALUE_DATA_TAG* obj);

    using type = Azure::Core::_internal::BasicUniqueHandle<AMQP_VALUE_DATA_TAG, FreeAmqpValue>;
  };
}}} // namespace Azure::Core::_internal
namespace Azure { namespace Core { namespace Amqp { namespace Models {

    class MessageProperties;

    enum class TerminusDurability : std::uint32_t
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

    class AmqpArray;
    class AmqpMap;
    class AmqpList;
    class AmqpBinaryData;
    class AmqpSymbol;
    class AmqpTimestamp;
    class AmqpComposite;
    class AmqpDescribed;

    using UniqueAmqpValueHandle = Azure::Core::_internal::UniqueHandle<AMQP_VALUE_DATA_TAG>;

    class AmqpValue {
    public:
      /** @brief Construct an AMQP null (empty) value.
       *
       * Defined in [AMQP Core Types
       * section 1.6.1](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-null).
  ).
       *
       */
      AmqpValue() noexcept;
      ~AmqpValue();

      /** @brief Construct an AMQP boolean value.
       *
       * Defined in [AMQP Core Types
       * section 1.6.2](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-boolean).
       *
       * @param value value to be set.
       */
      AmqpValue(bool value);

      /** @brief Construct an AMQP ubyte value, an 8 bit unsigned integer.
       *
       * Defined in [AMQP Core Types
       * section 1.6.3](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-ubyte).
       *
       * @param value value to be set.
       *
       */
      AmqpValue(std::uint8_t value);

      /** @brief Construct an AMQP ushort value.
       *
       * Defined in [AMQP Core Types
       * section 1.6.4](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-ushort).
       *
       * @param value value to be set.
       *
       */
      AmqpValue(std::uint16_t value);

      /** @brief Construct an AMQP uint value.
       *
       * Defined in [AMQP Core Types
       * section 1.6.5](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-ushort).
       *
       * @param value value to be set.
       *
       */
      AmqpValue(std::uint32_t value);

      /** @brief Construct an AMQP ulong value, a 64bit unsigned integer.
       *
       * Defined in [AMQP Core Types
       * section 1.6.6](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-ulong).
       *
       * @param value value to be set.
       *
       */
      AmqpValue(std::uint64_t value);

      /** @brief Construct an AMQP byte value, an 8 bit signed integer.
       *
       * Defined in [AMQP Core Types
       * section 1.6.7](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-byte).
       *
       * @param value value to be set.
       *
       */
      AmqpValue(std::int8_t value);

      /** @brief Construct an AMQP short value, a 16 bit signed integer.
       *
       * Defined in [AMQP Core Types
       * section 1.6.8](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-short).
       *
       * @param value value to be set.
       *
       */
      AmqpValue(std::int16_t value);

      /** @brief Construct an AMQP int value, a 32 bit signed integer.
       *
       * Defined in [AMQP Core Types
       * section 1.6.9](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-int).
       *
       * @param value value to be set.
       *
       */
      AmqpValue(std::int32_t value);

      /** @brief Construct an AMQP long value, a 64 bit signed integer.
       *
       * Defined in [AMQP Core Types
       * section 1.6.10](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-long).
       *
       * @param value value to be set.
       *
       */
      AmqpValue(std::int64_t value);

      /** @brief Construct an AMQP float value, an IEEE 754-2008 value.
       *
       * Defined in [AMQP Core Types
       * section 1.6.11](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-float).
       *
       * @param value value to be set.
       *
       */
      AmqpValue(float value);

      /** @brief Construct an AMQP double value, an IEEE 754-2008 value.
       *
       * Defined in [AMQP Core Types
       * section 1.6.12](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-double).
       *
       * @param value value to be set.
       *
       */
      AmqpValue(double value);

      /** @brief Construct an AMQP string value, a UTF-8 encoded sequence of characters.
       *
       * Defined in [AMQP Core Types
       * section 1.6.20](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-string).
       *
       * @param value to be set.
       *
       */
      explicit AmqpValue(std::string const& value);

      /** @brief Construct an AMQP string value, a UTF-8 encoded sequence of characters.
       *
       * Defined in [AMQP Core Types
       * section 1.6.20](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-string).
       *
       * @param value to be set.
       *
       * @remarks This is a convenience constructor to allow callers to specify:
       * ```cpp
       * AmqpValue myValue("This is some text");
       * ```
       *
       */
      AmqpValue(const char* value);

      /** TODO:
       * Decimal32, Decimal64, and Decimal128.
       */

      /** @brief Construct an AMQP Char value, a UTF-32BE encoded Unicode character.
       *
       * Defined in [AMQP Core Types
       * section 1.6.16](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-char).
       *
       * @param value UTF-32 encoded unicode value to be set.
       *
       */
      static AmqpValue CreateChar(std::uint32_t value);

      /** @brief Construct an AMQP Uuid value, an RFC-4122 Universally Unique Identifier.
       *
       * Defined in [AMQP Core Types
       * section 1.6.18](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-uuid).
       *
       * @param value UTF-32 encoded unicode value to be set.
       *
       */
      AmqpValue(Azure::Core::Uuid const& value);

      /** @brief Construct an AMQP Value from an existing AMQP Value
       * @param that - source value to copy.
       */
      AmqpValue(AmqpValue const& that) noexcept;

      /** @brief Move an AMQP Value to another existing AMQP Value
       * @param that - source value to move.
       */
      AmqpValue(AmqpValue&& that) noexcept;

      // Interoperability functions for uAMQP

      /** @brief Interoperability helper function which converts an AmqpValue to a uAMQP
       * AMQP_VALUE object.
       *
       * @returns uAMQP AMQP_VALUE object.
       *
       * @remarks This is an internal operator which should not be called by customers.
       */
      operator AMQP_VALUE_DATA_TAG*() const;

      /** @brief Interoperability helper function which creates an AmqpValue from a uAMQP
       * AMQP_VALUE object.
       *
       * @param value source uAMQP AMQP_VALUE object.
       *
       * @remarks This is an internal operator which should not be called by customers.
       */
      AmqpValue(AMQP_VALUE_DATA_TAG* value);

      /** @brief Copy an AMQP value to the current AMQP value.
       *
       * @param that the other AMQP Value to copy.
       * @returns "this".
       */
      AmqpValue& operator=(AmqpValue const& that);
      /** @brief Move an AMQP value to the current AMQP value.
       *
       * @param that the other AMQP Value to move.
       * @returns "this".
       */
      AmqpValue& operator=(AmqpValue&& that) noexcept;

      /** @brief Equality comparison operator.
       * @param that - Value to compare to this value.
       * @returns true if the that is equal to this.
       */
      bool operator==(AmqpValue const& that) const;
      /** @brief Less Than comparison operator.
       * @param that - Value to compare to this value.
       * @returns true if the that is less than this.
       */
      bool operator<(AmqpValue const& that) const;
      AmqpValueType GetType() const;

      /** @brief Returns 'true' if the AMQP value is "null".
       *
       * @returns true if the AmqpValue is null.
       */
      bool IsNull() const;

      /** @brief convert the current AMQP Value to a boolean.
       *
       * @returns bool true if the AMQP value is true.
       *
       * @throws std::runtime_error if the underlying AMQP value is not a boolean.
       */
      operator bool() const;
      operator std::uint8_t() const;
      operator std::int8_t() const;
      operator std::uint16_t() const;
      operator std::int16_t() const;
      operator std::uint32_t() const;
      operator std::int32_t() const;
      operator std::uint64_t() const;
      operator std::int64_t() const;
      operator float() const;
      operator double() const;
      explicit operator std::string() const;
      operator Uuid() const;

      // List Operations.
      AmqpList AsList() const;

      // Map operations.
      AmqpMap AsMap() const;

      // Array operations - note that all array items must be of the same type.
      AmqpArray AsArray() const;

      AmqpBinaryData AsBinary() const;

      AmqpTimestamp AsTimestamp() const;

      // Get Char value.
      std::uint32_t GetChar() const;

      // Symbols
      AmqpSymbol AsSymbol() const;

      // Composite values - A composite value is functionally a list with a fixed size.
      AmqpComposite AsComposite() const;

      AmqpDescribed AsDescribed() const;

      friend std::ostream& operator<<(std::ostream& os, AmqpValue const& value);

    protected:
      UniqueAmqpValueHandle m_value;
    };

    /** @brief An AmqpArray represents a sequentially ordered list of values. The values of the
     * AmqpArray MUST all have the same underlying type.
     *
     */
    class AmqpArray {
      std::vector<AmqpValue> m_value;

    public:
      /** @brief Construct a new AmqpArray object. */
      AmqpArray();

      /** @brief Construct a new AmqpArray object with an initializer list. */
      AmqpArray(std::initializer_list<AmqpValue> const& values);

      /** @brief Construct a new AmqpArray object from an existing uAMQP AMQP_VALUE item
       * @remarks Note that this does NOT capture the passed in AMQP_VALUE object, the caller is
       * responsible for freeing that object.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @param value - the AMQP array value to capture.
       */
      AmqpArray(AMQP_VALUE_DATA_TAG* const value);

      /**
       * @brief Convert an existing AmqpArray to an AmqpValue.
       */
      operator AmqpValue() const;

      /**
       * @brief Convert an AmqpArray instance to a uAMQP AMQP_VALUE.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @remarks Note that this returns a newly allocated AMQP_VALUE object which must be freed
       * by the caller.
       */
      operator UniqueAmqpValueHandle() const;

      bool operator<(AmqpArray const& that) const { return m_value < that.m_value; }
      inline decltype(m_value)::size_type size() const { return m_value.size(); }
      const decltype(m_value)::value_type& operator[](
          const decltype(m_value)::size_type pos) const noexcept
      {
        return m_value.operator[](pos);
      }
      void push_back(decltype(m_value)::value_type&& val) { m_value.push_back(val); }
      decltype(m_value)::const_iterator begin() const noexcept { return m_value.begin(); }
      decltype(m_value)::const_iterator end() const noexcept { return m_value.end(); }
      decltype(m_value)::value_type* data() noexcept { return m_value.data(); }
      const decltype(m_value)::value_type* data() const noexcept { return m_value.data(); }
      const decltype(m_value)::value_type& at(const decltype(m_value)::size_type pos) const
      {
        return m_value.at(pos);
      }
      friend std::ostream& operator<<(std::ostream& os, AmqpArray const& value);
    };

    /** @brief An AmqpMap represents an AMQP "map" type.
     *
     * An AMQP Map is a polymorphic map of distinct keys to values.
     *
     */
    class AmqpMap //;: public std::map<AmqpValue, AmqpValue>
    {
      std::map<AmqpValue, AmqpValue> m_value;

    public:
      /** @brief Construct a new AmqpMap object. */
      AmqpMap();

      /** @brief Construct a new AmqpArray object with an initializer list. */
      AmqpMap(std::initializer_list<std::map<AmqpValue, AmqpValue>::value_type> const& values);

      /** @brief Construct a new AmqpMap object from an existing uAMQP AMQP_VALUE item
       * @remarks Note that this does NOT capture the passed in AMQP_VALUE object, the caller is
       * responsible for freeing that object.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @param value - the AMQP array value to capture.
       */
      AmqpMap(AMQP_VALUE_DATA_TAG* const value);

      /**
       * @brief Convert an existing AmqpMap to an AmqpValue.
       */
      operator AmqpValue() const;

      /**
       * @brief Convert an AmqpMap instance to a uAMQP AMQP_VALUE.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @remarks Note that this returns a newly allocated AMQP_VALUE object which must be freed
       * by the caller.
       */
      operator UniqueAmqpValueHandle() const;

      bool operator<(AmqpMap const& that) const { return m_value < that.m_value; }
      inline decltype(m_value)::size_type size() const { return m_value.size(); }
      decltype(m_value)::const_iterator begin() const noexcept { return m_value.begin(); }
      decltype(m_value)::const_iterator end() const noexcept { return m_value.end(); }
      decltype(m_value)::mapped_type& operator[](decltype(m_value)::key_type&& keyVal)
      {
        return m_value.operator[](keyVal);
      }

      template <class... ValueTypes>
      std::pair<decltype(m_value)::iterator, bool> emplace(ValueTypes&&... values)
      {
        return m_value.emplace(values...);
      }
      bool empty() const noexcept { return m_value.empty(); }
      friend std::ostream& operator<<(std::ostream& os, AmqpMap const& value);
    };

    /** @brief An AMQP List is a sequence of polymorphic values. It has the behavioral
     * characteristics of an AMQP array, but allows the members to be polymorphic.
     */
    class AmqpList {
      std::vector<AmqpValue> m_value;

    public:
      AmqpList();
      /** @brief Construct a new AmqpList object with an initializer list. */
      AmqpList(std::initializer_list<std::vector<AmqpValue>::value_type> const& values);

      /** @brief Construct a new AmqpList object from an existing uAMQP AMQP_VALUE item
       *
       * @remarks Note that this does NOT capture the passed in AMQP_VALUE object, the caller is
       * responsible for freeing that object.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @param value - the AMQP array value to capture.
       */
      AmqpList(AMQP_VALUE_DATA_TAG* const value);

      /**
       * @brief Convert an existing AmqpList to an AmqpValue.
       */
      operator const AmqpValue() const;

      /**
       * @brief Convert an AmqpList instance to a uAMQP AMQP_VALUE.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @remarks Note that this returns a newly allocated AMQP_VALUE object which must be freed
       * by the caller.
       */
      operator UniqueAmqpValueHandle() const;

      bool operator<(AmqpList const& that) const { return m_value < that.m_value; }
      inline decltype(m_value)::size_type size() const { return m_value.size(); }
      const decltype(m_value)::value_type& operator[](
          const decltype(m_value)::size_type pos) const noexcept
      {
        return m_value.operator[](pos);
      }
      void push_back(decltype(m_value)::value_type&& val) { m_value.push_back(val); }
      decltype(m_value)::const_iterator begin() const noexcept { return m_value.begin(); }
      decltype(m_value)::const_iterator end() const noexcept { return m_value.end(); }
      decltype(m_value)::value_type* data() noexcept { return m_value.data(); }
      const decltype(m_value)::value_type* data() const noexcept { return m_value.data(); }
      const decltype(m_value)::value_type& at(const decltype(m_value)::size_type pos) const
      {
        return m_value.at(pos);
      }
      friend std::ostream& operator<<(std::ostream& os, AmqpList const& value);
    };

    /** @brief An AMQP binary value, a sequence of octets
     *
     * Defined in [AMQP Core Types
     * section 1.6.19](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-binary).
     *
     */

    class AmqpBinaryData //: private std::vector<std::uint8_t>
    {
    private:
      std::vector<std::uint8_t> m_value;

    public:
      AmqpBinaryData();
      /** @brief Construct a new AmqpList object with an initializer list. */
      AmqpBinaryData(std::initializer_list<std::vector<std::uint8_t>::value_type> const& values);
      AmqpBinaryData(std::vector<std::uint8_t> const& values);

      /** @brief Construct a new AmqpBinaryData object from an existing uAMQP AMQP_VALUE item
       *
       * @remarks Note that this does NOT capture the passed in AMQP_VALUE object, the caller is
       * responsible for freeing that object.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @param value - the AMQP "binary" value to capture.
       */
      AmqpBinaryData(AMQP_VALUE_DATA_TAG* const value);

      /**
       * @brief Convert an existing AmqpBinaryData to an AmqpValue.
       */
      operator const AmqpValue() const;

      /**
       * @brief Convert an AmqpBinaryData instance to a uAMQP AMQP_VALUE.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @remarks Note that this returns a newly allocated AMQP_VALUE object which must be freed
       * by the caller.
       */
      operator UniqueAmqpValueHandle() const;

      bool operator<(AmqpBinaryData const& that) const { return m_value < that.m_value; }
      inline decltype(m_value)::size_type size() const { return m_value.size(); }
      const decltype(m_value)::value_type& operator[](
          const decltype(m_value)::size_type pos) const noexcept
      {
        return m_value.operator[](pos);
      }
      void push_back(decltype(m_value)::value_type&& val) { m_value.push_back(val); }
      decltype(m_value)::const_iterator begin() const noexcept { return m_value.begin(); }
      decltype(m_value)::const_iterator end() const noexcept { return m_value.end(); }
      decltype(m_value)::value_type* data() noexcept { return m_value.data(); }
      const decltype(m_value)::value_type* data() const noexcept { return m_value.data(); }
      const decltype(m_value)::value_type& at(const decltype(m_value)::size_type pos) const
      {
        return m_value.at(pos);
      }
      friend std::ostream& operator<<(std::ostream& os, AmqpBinaryData const& value);
    };

    class AmqpSymbol {
      std::string m_value;

    public:
      AmqpSymbol();
      /** @brief Construct a new AmqpSymbol object with an initializer list. */
      AmqpSymbol(std::string const& values);
      AmqpSymbol(const char* const values);

      /** @brief Construct a new AmqpSymbol object from an existing uAMQP AMQP_VALUE item
       *
       * @remarks Note that this does NOT capture the passed in AMQP_VALUE object, the caller is
       * responsible for freeing that object.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @param value - the AMQP "binary" value to capture.
       */
      AmqpSymbol(AMQP_VALUE_DATA_TAG* const value);

      /**
       * @brief Convert an existing AmqpSymbol to an AmqpValue.
       */
      operator AmqpValue() const;

      /**
       * @brief Convert an AmqpSymbol instance to a uAMQP AMQP_VALUE.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @remarks Note that this returns a newly allocated AMQP_VALUE object which must be freed
       * by the caller.
       */
      operator UniqueAmqpValueHandle() const;

      bool operator<(AmqpSymbol const& that) const { return m_value < that.m_value; }
      inline decltype(m_value)::size_type size() const { return m_value.size(); }
      const decltype(m_value)::value_type& operator[](
          const decltype(m_value)::size_type pos) const noexcept
      {
        return m_value.operator[](pos);
      }
      void push_back(decltype(m_value)::value_type&& val) { m_value.push_back(val); }
      decltype(m_value)::const_iterator begin() const noexcept { return m_value.begin(); }
      decltype(m_value)::const_iterator end() const noexcept { return m_value.end(); }
      const decltype(m_value)::value_type* data() const noexcept { return m_value.data(); }
      const decltype(m_value)::value_type& at(const decltype(m_value)::size_type pos) const
      {
        return m_value.at(pos);
      }

      std::string& assign(std::string&& that) { return m_value.assign(that); }
      decltype(m_value)::value_type const* c_str() const noexcept { return m_value.c_str(); }
      operator const std::string() const { return m_value; }
      operator std::string&() { return m_value; }
      bool operator==(AmqpSymbol const& that) const { return m_value == that.m_value; }
      bool operator==(const decltype(m_value)::value_type* const that) const
      {
        return m_value == that;
      }
      bool equals(AmqpSymbol const& that) const { return m_value == that.m_value; }
    };

    class AmqpTimestamp {
      std::chrono::milliseconds m_value;

    public:
      AmqpTimestamp();
      /** @brief Construct a new AmqpTimestamp object . */
      AmqpTimestamp(std::chrono::milliseconds const& values);

      /** @brief Construct a new AmqpSymbol object from an existing uAMQP AMQP_VALUE item
       *
       * @remarks Note that this does NOT capture the passed in AMQP_VALUE object, the caller is
       * responsible for freeing that object.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @param value - the AMQP "binary" value to capture.
       */
      AmqpTimestamp(AMQP_VALUE_DATA_TAG* const value);

      /**
       * @brief Convert an existing AmqpSymbol to an AmqpValue.
       */
      operator const AmqpValue() const;

      /**
       * @brief Convert an AmqpSymbol instance to a uAMQP AMQP_VALUE.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @remarks Note that this returns a newly allocated AMQP_VALUE object which must be freed
       * by the caller.
       */
      operator UniqueAmqpValueHandle() const;

      operator std::chrono::milliseconds const() const { return m_value; }

      bool operator<(AmqpTimestamp const& that) { return m_value < that.m_value; }
    };

    /** @brief An AmqpComposite represents a sequentially ordered list of values. The values of
     * the composite may have different types.
     *
     * @remarks An [AMQP Composite
     * type](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#doc-idp42752)
     * is a composite value where each value consists of an ordered sequence of fields, each with
     * a specified name, type and multiplicity. They roughly correspond to a C or C++ "struct"
     * type.
     *
     * @note The AMQP Composite type representation does NOT include the underlying field names,
     * just the field values.
     *
     */
    class AmqpComposite {
      std::vector<AmqpValue> m_value;
      AmqpValue m_descriptor;

    public:
      /** @brief Construct a new AmqpArray object. */
      AmqpComposite();

      /** @brief Construct a new AmqpArray object with an initializer list. */
      AmqpComposite(AmqpValue const& descriptor, std::initializer_list<AmqpValue> const& values);

      /** @brief Construct a new AmqpComposite object from an existing uAMQP AMQP_VALUE item
       * @remarks Note that this does NOT capture the passed in AMQP_VALUE object, the caller is
       * responsible for freeing that object.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @param value - the AMQP array value to capture.
       */
      AmqpComposite(AMQP_VALUE_DATA_TAG* const value);

      /**
       * @brief Convert an existing AmqpComposite to an AmqpValue.
       */
      operator AmqpValue const() const;

      /** @brief Returns the descriptor for this composite type.
       *
       * @returns The descriptor for this composite type.
       */
      AmqpValue const& GetDescriptor() const { return m_descriptor; }

      /**
       * @brief Convert an AmqpComposite instance to a uAMQP AMQP_VALUE.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @remarks Note that this returns a newly allocated AMQP_VALUE object which must be freed
       * by the caller.
       */
      operator UniqueAmqpValueHandle() const;

      bool operator<(AmqpComposite const& that) const { return m_value < that.m_value; }
      inline decltype(m_value)::size_type size() const { return m_value.size(); }
      const decltype(m_value)::value_type& operator[](
          const decltype(m_value)::size_type pos) const noexcept
      {
        return m_value.operator[](pos);
      }
      void push_back(decltype(m_value)::value_type&& val) { m_value.push_back(val); }
      decltype(m_value)::const_iterator begin() const noexcept { return m_value.begin(); }
      decltype(m_value)::const_iterator end() const noexcept { return m_value.end(); }
      decltype(m_value)::value_type* data() noexcept { return m_value.data(); }
      const decltype(m_value)::value_type* data() const noexcept { return m_value.data(); }
      const decltype(m_value)::value_type& at(const decltype(m_value)::size_type pos) const
      {
        return m_value.at(pos);
      }
    };

    /** @brief An AmqpDescribed represents an AMQP described type.
     *
     * @remarks An [AMQP Described
     * type](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#doc-idp38080)
     * is a tuple consisting of a type and a "descriptor" for that type. The "descriptor"
     * indicates that the AMQP object is a representation of the type.
     *
     */
    class AmqpDescribed {
    public:
      /** @brief Construct a new AmqpDescribed object.
       *
       * By convention, AMQP Descriptor values are either symbolic or numeric. Other types are
       * reserved.
       *
       * @param descriptor - the Descriptor for the described value.
       * @param value - the Value for the described value.
       *
       */
      AmqpDescribed(AmqpSymbol const& descriptor, AmqpValue const& value);

      /** @brief Construct a new AmqpDescribed object with a 64bit descriptor.
       *
       * @param descriptor - the Descriptor for the described value. The descriptor value SHOULD
       * be one of the values from the AMQP specification.
       * @param value - the Value for the described value.
       *
       */
      AmqpDescribed(uint64_t descriptor, AmqpValue const& value);

      /** @brief Construct a new AmqpDescribed object from an existing uAMQP AMQP_VALUE item
       * @remarks Note that this does NOT capture the passed in AMQP_VALUE object, the caller is
       * responsible for freeing that object.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @param value - the AMQP array value to capture.
       */
      AmqpDescribed(AMQP_VALUE_DATA_TAG* const value);

      /**
       * @brief Convert an existing AmqpComposite to an AmqpValue.
       */
      operator AmqpValue const() const;

      /** @brief Returns the descriptor for this composite type.
       *
       * @returns The descriptor for this composite type.
       */
      AmqpValue const& GetDescriptor() const { return m_descriptor; }

      /** @brief Returns the descriptor for this composite type.
       *
       * @returns The descriptor for this composite type.
       */
      AmqpValue const& GetValue() const { return m_value; }

      /**
       * @brief Convert an AmqpDescriptor instance to a uAMQP AMQP_VALUE.
       *
       * @remarks This is an internal accessor and should never be used by code outside the AMQP
       * implementation.
       *
       * @remarks Note that this returns a newly allocated AMQP_VALUE object which must be freed
       * by the caller.
       */
      operator UniqueAmqpValueHandle() const;

      bool operator<(AmqpDescribed const& that) const
      {
        return m_descriptor < that.m_descriptor ? true
            : m_descriptor == that.m_descriptor ? m_value < that.m_value
                                                : false;
      }

    private:
      AmqpValue m_descriptor;
      AmqpValue m_value;
    };

}}}} // namespace Azure::Core::Amqp::Models
