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

namespace Azure { namespace Core {
  namespace _internal {

    template <> struct UniqueHandleHelper<AMQP_VALUE_DATA_TAG>
    {
      static void FreeAmqpValue(AMQP_VALUE_DATA_TAG* obj);

      using type = Azure::Core::_internal::BasicUniqueHandle<AMQP_VALUE_DATA_TAG, FreeAmqpValue>;
    };
  } // namespace _internal

  namespace Amqp { namespace Models {

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
        AmqpValue();
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
        explicit AmqpValue(std::string value);

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

        /** @brief Construct an AMQP timestamp value, an absolute point in time with a precision of
         * milliseconds.
         *
         * Defined in [AMQP Core Types
         * section 1.6.20](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-timestamp).
         *
         * @param value to be set.
         *
         */
//        AmqpValue(std::chrono::milliseconds const& value);

        /** TODO:
         * Decimal32, Decimal64, and Decimal128.
         */

        /** @brief Construct an AMQP Char value, a UTF-32BE encoded Unicode character.
         *
         * Defined in [AMQP Core Types
         * section 1.6.16](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-char).
         *
         * @param UTF-32 encoded unicode value to be set.
         *
         */
        static AmqpValue CreateChar(std::uint32_t value);

        /** @brief Construct an AMQP Uuid value, an RFC-4122 Universally Unique Identifier.
         *
         * Defined in [AMQP Core Types
         * section 1.6.18](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-uuid).
         *
         * @param UTF-32 encoded unicode value to be set.
         *
         */
        AmqpValue(Azure::Core::Uuid value);

        AmqpValue(AmqpValue const& that) throw();
        AmqpValue(AmqpValue&& that) throw();

        // Interoperability functions for uAMQP
        operator AMQP_VALUE_DATA_TAG*() const;
        AmqpValue(AMQP_VALUE_DATA_TAG* value);

        AmqpValue& operator=(AmqpValue const& that);
        AmqpValue& operator=(AmqpValue&& that) throw();

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

        operator bool() const;
        operator bool();
        operator std::uint8_t() const;
        operator std::uint8_t();
        operator std::int8_t() const;
        operator std::int8_t();
        operator std::uint16_t() const;
        operator std::uint16_t();
        operator std::int16_t() const;
        operator std::int16_t();
        operator std::uint32_t() const;
        operator std::uint32_t();
        operator std::int32_t() const;
        operator std::int32_t();
        operator std::uint64_t() const;
        operator std::uint64_t();
        operator std::int64_t() const;
        operator std::int64_t();
        operator float() const;
        operator float();
        operator double() const;
        operator double();
        explicit operator std::string() const;
        explicit operator std::string();
        operator Uuid();
        operator Uuid() const;
//        operator std::chrono::milliseconds const() const;

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
        //        static AmqpValue CreateSymbol(std::string const& value);
        //        std::string GetSymbol() const;

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
        MessageProperties GetPropertiesFromValue() const;

        static AmqpValue CreateProperties(MessageProperties const& properties);

        friend std::ostream& operator<<(std::ostream& os, AmqpValue const& value);

      protected:
        UniqueAmqpValueHandle m_value;
      };

      /** @brief An AmqpArray represents a sequentially ordered list of values. The values of the
       * AmqpArray MUST all have the same underlying type.
       *
       */
      class AmqpArray : public std::vector<AmqpValue> {
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
        operator AMQP_VALUE_DATA_TAG*() const;
      };

      /** @brief An AmqpMap represents an AMQP "map" type.
       *
       * An AMQP Map is a polymorphic map of distinct keys to values.
       *
       */
      class AmqpMap : public std::map<AmqpValue, AmqpValue> {
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
        operator const AmqpValue() const;

        /**
         * @brief Convert an AmqpMap instance to a uAMQP AMQP_VALUE.
         *
         * @remarks This is an internal accessor and should never be used by code outside the AMQP
         * implementation.
         *
         * @remarks Note that this returns a newly allocated AMQP_VALUE object which must be freed
         * by the caller.
         */
        operator AMQP_VALUE_DATA_TAG*() const;
      };

      /** @brief An AMQP List is a sequence of polymorphic values. It has the behavioral
       * characteristics of an AMQP array, but allows the members to be polymorphic.
       */
      class AmqpList : public std::vector<AmqpValue> {
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
        operator AMQP_VALUE_DATA_TAG*() const;
      };

      /** @brief An AMQP binary value, a sequence of octets
       *
       * Defined in [AMQP Core Types
       * section 1.6.19](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-binary).
       *
       */

      class AmqpBinaryData : public std::vector<std::uint8_t> {
      public:
        AmqpBinaryData();
        /** @brief Construct a new AmqpList object with an initializer list. */
        AmqpBinaryData(std::initializer_list<std::vector<std::uint8_t>::value_type> const& values);

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
        operator AMQP_VALUE_DATA_TAG*() const;
      };

      class AmqpSymbol : public std::string {
      public:
        AmqpSymbol();
        /** @brief Construct a new AmqpSymbol object with an initializer list. */
        AmqpSymbol(std::string const& values);

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
        operator AMQP_VALUE_DATA_TAG*() const;
      };

            class AmqpTimestamp : public std::chrono::milliseconds {
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
        operator AMQP_VALUE_DATA_TAG*() const;
      };


  }} // namespace Amqp::Models
}} // namespace Azure::Core
