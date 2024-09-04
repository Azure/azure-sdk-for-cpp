// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "amqp_header.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/internal/unique_handle.hpp>
#include <azure/core/uuid.hpp>

#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <list>
#include <map>
#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {
  class AmqpValueImpl;
  class AmqpValueFactory;
}}}}} // namespace Azure::Core::Amqp::Models::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _internal {

  enum class TerminusDurability : std::uint32_t
  {
    None = 0,
    Configuration = 1,
    UnsettledState = 2,
  };

  // Note : Should be an extendable Enumeration.
  enum class TerminusExpiryPolicy
  {
    LinkDetach,
    SessionEnd,
    ConnectionClose,
    Never,
  };
}}}}} // namespace Azure::Core::Amqp::Models::_internal

namespace Azure { namespace Core { namespace Amqp { namespace Models {

  enum class AmqpValueType
  {
    Invalid,
    Null,
    Bool,
    Ubyte,
    Ushort,
    Uint,
    Ulong,
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
    Unknown,
  };

  /**
   * @brief ostream insertion operator for AmqpValueType.
   *
   * @param os - stream to insert to.
   * @param value - value to insert.
   *
   * @returns the input ostream.
   */
  std::ostream& operator<<(std::ostream& os, AmqpValueType value);

  class AmqpArray;
  class AmqpMap;
  class AmqpAnnotations;
  class AmqpList;
  class AmqpBinaryData;
  class AmqpSymbol;
  class AmqpTimestamp;
  class AmqpComposite;
  class AmqpDescribed;

  /** An AMQP value.
   * @details An AMQP value is a polymorphic type that can be used to represent any AMQP type.
   *
   * @see https://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html
   *
   */
  class AmqpValue final {
  public:
    /** @brief Construct an AMQP null (empty) value.
     *
     * Defined in [AMQP Core Types
     * section 1.6.1](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-null).
     *
     */
    AmqpValue() noexcept;
    ~AmqpValue();

    /** @brief Construct an AMQP Value from an existing AMQP Value
     * @param that - source value to copy.
     */
    AmqpValue(AmqpValue const& that) noexcept;

    /** @brief Move an AMQP Value to another existing AMQP Value
     * @param that - source value to move.
     */
    AmqpValue(AmqpValue&& that) noexcept;

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

    /** @brief Construct an AMQP byte value, an 8 bit signed integer.
     *
     * Defined in [AMQP Core Types
     * section 1.6.7](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-byte).
     *
     * @param value value to be set.
     *
     * @remarks This field is a convenience overload to allow clients to declare an AmqpValue with
     * a C++ character.
     *
     */
    AmqpValue(char value);

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

    /** @brief Construct an AMQP Symbol value, a UTF-8 encoded sequence of characters.
     *
     * Defined in [AMQP Core Types
     * section 1.6.21](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-symbol).
     * * @param value to be set.
     *
     */
    AmqpValue(AmqpSymbol const& value);

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

    /** @brief Construct an AMQP Char value, a UTF-32BE encoded Unicode character.
     *
     * Defined in [AMQP Core Types
     * section 1.6.16](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-char).
     *
     * @param value UTF-32 encoded unicode value to be set.
     *
     */
    AmqpValue(char32_t value);

    /** TODO:
     * Decimal32, Decimal64, and Decimal128.
     */

    /** @brief Construct an AMQP Uuid value, an RFC-4122 Universally Unique Identifier.
     *
     * Defined in [AMQP Core Types
     * section 1.6.18](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-uuid).
     *
     * @param value UTF-32 encoded unicode value to be set.
     *
     */
    AmqpValue(Azure::Core::Uuid const& value);

    /** @brief Equality comparison operator.
     * @param that - Value to compare to this value.
     * @returns true if the that is equal to this.
     */
    bool operator==(AmqpValue const& that) const;

    /** @brief Equality comparison operator.
     * @param that - Value to compare to this value.
     * @returns true if the that is equal to this.
     */
    bool operator!=(AmqpValue const& that) const { return !(*this == that); };

    /** @brief Less Than comparison operator.
     * @param that - Value to compare to this value.
     * @returns true if the that is less than this.
     *
     * @remark When comparing AMQP values, if the two values are not the same type, this returns
     * if the numeric value of this.GetType() is less than that.GetType(). If the two values are
     * of the same type, this returns if the value of this is less than the value of that.
     */
    bool operator<(AmqpValue const& that) const;

    /** @brief Returns the underlying type of the AMQP value.
     *
     * @returns AmqpValueType of the AMQP value.
     */
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

    /** @brief convert the current AMQP Value to an unsigned 8 bit integer.
     *
     * @returns the value as an unsigned 8 bit integer.
     *
     * @throws std::runtime_error if the underlying AMQP value is not an unsigned 8 bit integer.
     */
    operator std::uint8_t() const;

    /** @brief convert the current AMQP Value to a signed 8 bit integer.
     *
     * @returns the value as an 8 bit integer.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a signed 8 bit integer.
     */
    operator std::int8_t() const;

    /** @brief convert the current AMQP Value to a signed 8 bit integer. Convenience function to
     * allow an AmqpValue to be constructed from a 'char' value.
     *
     * @returns the value as a char.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a signed 8 bit integer.
     */
    operator char() const;

    /** @brief convert the current AMQP Value to an unsigned 16 bit integer.
     *
     * @returns the value as an unsigned 16 bit integer.
     *
     * @throws std::runtime_error if the underlying AMQP value is not an unsigned 16 bit integer.
     */
    operator std::uint16_t() const;

    /** @brief convert the current AMQP Value to a signed 16 bit integer.
     *
     * @returns the value as a signed 16 bit integer.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a signed 16 bit integer.
     */
    operator std::int16_t() const;

    /** @brief convert the current AMQP Value to an unsigned 32 bit integer.
     *
     * @returns the value as an unsigned 32 bit integer.
     *
     * @throws std::runtime_error if the underlying AMQP value is not an unsigned 32 bit integer.
     */
    operator std::uint32_t() const;

    /** @brief convert the current AMQP Value to a signed 32 bit integer.
     *
     * @returns the value as a signed 32 bit integer.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a signed 32 bit integer.
     */
    operator std::int32_t() const;

    /** @brief convert the current AMQP Value to an unsigned 64 bit integer.
     *
     * @returns the value as an unsigned 64 bit integer.
     *
     * @throws std::runtime_error if the underlying AMQP value is not an unsigned 64 bit integer.
     */
    operator std::uint64_t() const;

    /** @brief convert the current AMQP Value to a signed 64 bit integer.
     *
     * @returns the value as a signed 64 bit integer.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a signed 64 bit integer.
     */
    operator std::int64_t() const;

    /** @brief convert the current AMQP Value to a 32 bit IEEE 'float' value..
     *
     * @returns the value as a float.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a float.
     */
    operator float() const;

    /** @brief convert the current AMQP Value to a 64 bit IEEE 'double' value.
     *
     * @returns the value as a double.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a double.
     */
    operator double() const;

    /** @brief convert the current AMQP Value to a 32bit UCS32 value.
     *
     * @returns the value as a 32 bit character.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a 32 bit character.
     */
    operator char32_t() const;

    /** @brief convert the current AMQP Value to a string.
     *
     * @returns the value as a string.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a string.
     */
    explicit operator std::string() const;

    /** @brief convert the current AMQP Value to a UUID.
     *
     * @returns the value as an Azure::Core::Uuid value.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a UUID.
     */
    operator Azure::Core::Uuid() const;

    /** @brief convert the current AMQP Value to an AmqpList.
     *
     * @returns the value as an AmqpList.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a list.
     */
    AmqpList AsList() const;

    /** @brief convert the current AMQP Value to an AmqpMap.
     *
     * @returns the value as an AmqpMap.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a map.
     */
    AmqpMap AsMap() const;

    /** @brief convert the current AMQP Value to an AmqpAnnotations.
     *
     * @returns the value as an AmqpAnnotations.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a map of Symbols to Values.
     */
    AmqpAnnotations AsAnnotations() const;

    /** @brief convert the current AMQP Value to an AmqpArray.
     *
     * @returns the value as an AmqpArray.
     *
     * @throws std::runtime_error if the underlying AMQP value is not an array.
     */
    AmqpArray AsArray() const;

    /** @brief convert the current AMQP Value to an AmqpBinaryData.
     *
     * @returns the value as an AmqpBinaryData.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a binary data.
     */
    AmqpBinaryData AsBinary() const;

    /** @brief convert the current AMQP Value to an AmqpTimestamp.
     *
     * @returns the value as an AmqpTimestamp.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a timestamp.
     */
    AmqpTimestamp AsTimestamp() const;

    /** @brief convert the current AMQP Value to an AmqpSymbol.
     *
     * @returns the value as an AmqpSymbol.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a symbol.
     */
    AmqpSymbol AsSymbol() const;

    /** @brief convert the current AMQP Value to an AMQP Composite value.
     *
     * An AMQP Composite value is functionally a list with a defined structure. The structure
     * definition can be found via the GetDescriptor method.
     *
     * @returns the value as an AmqpComposite.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a Composite value.
     */
    AmqpComposite AsComposite() const;

    /** @brief convert the current AMQP Value to an AMQP Described value.
     *
     * An AMQP Described value is a tuple consisting of a Descriptor and Value.
     *
     * @returns the value as an AmqpDescribed.
     *
     * @throws std::runtime_error if the underlying AMQP value is not a Described value.
     */
    AmqpDescribed AsDescribed() const;

    /** @brief Serialize this AMQP value as an array of bytes. */
    static std::vector<uint8_t> Serialize(AmqpValue const& value);

    /** @brief Returns the size (in bytes) of the serialized form of this value */
    static size_t GetSerializedSize(AmqpValue const& value);

    /** @brief Deserialize an AMQP value from an array of bytes.
     *
     * @param[in] data The serialized form of the AMQP value to deserialize.
     * @param[in] size The size of the data parameter to deserialize.
     */
    static AmqpValue Deserialize(uint8_t const* data, size_t size);

  private:
    /** @brief Construct an AMQP value from the internal implementation of an AMQP value. */
    AmqpValue(std::unique_ptr<Azure::Core::Amqp::Models::_detail::AmqpValueImpl>&& value);
    std::unique_ptr<_detail::AmqpValueImpl> m_impl;
    friend class _detail::AmqpValueFactory;
  };
  std::ostream& operator<<(std::ostream& os, AmqpValue const& value);

}}}} // namespace Azure::Core::Amqp::Models

namespace Azure { namespace Core { namespace Amqp { namespace Models { namespace _detail {

  /** @brief Base type for AMQP collection types.
   *
   * Provides convenient conversions for STL collection types to enable classes derived from
   * AmqpCollectionBase to be used as STL containers.
   */
  template <typename T, typename ThisType> class AmqpCollectionBase {
  protected:
    T m_value;

    using initializer_type = std::initializer_list<typename T::value_type>;

    AmqpCollectionBase(initializer_type const& initializer) : m_value{initializer} {}
    AmqpCollectionBase(T initializer) : m_value{initializer} {}
    AmqpCollectionBase(){};

    // Copy constructor
    AmqpCollectionBase(const AmqpCollectionBase& other) = default;

    // Copy assignment operator
    AmqpCollectionBase& operator=(const AmqpCollectionBase& other) = default;

    // Move constructor
    AmqpCollectionBase(AmqpCollectionBase&& other) noexcept = default;

    // Move assignment operator
    AmqpCollectionBase& operator=(AmqpCollectionBase&& other) = default;

    ~AmqpCollectionBase() = default;

  public:
    /** @brief Returns the underlying value.
     */
    explicit operator T const&() const { return m_value; }

    /** @brief Convert this collection type to an AMQP value.*/
    AmqpValue AsAmqpValue() const;

    /** @brief Returns the size of the underlying value.*/
    inline typename T::size_type size() const { return m_value.size(); }

    /**  @brief Array accessor operator.
     * @param pos Position of an element in the container.
     * @return element value.
     */
    const typename T::value_type& operator[](const typename T::size_type pos) const noexcept
    {
      return m_value.operator[](pos);
    }
    void push_back(typename T::value_type&& val) { m_value.push_back(std::move(val)); }
    typename T::const_iterator begin() const noexcept { return m_value.begin(); }
    typename T::const_iterator end() const noexcept { return m_value.end(); }
    size_t find(T const& val) noexcept { return m_value.find(val); }
    typename T::value_type* data() noexcept { return m_value.data(); }
    const typename T::value_type* data() const noexcept { return m_value.data(); }
    const typename T::value_type& at(const typename T::size_type pos) const
    {
      return m_value.at(pos);
    }
    bool operator<(ThisType const& that) const { return m_value < that.m_value; }
    bool operator==(ThisType const& that) const { return m_value == that.m_value; }
    bool operator!=(ThisType const& that) const { return m_value != that.m_value; }
    /** @brief Returns true if the underlying value is empty.*/
    bool empty() const noexcept { return m_value.empty(); }

  protected:
    /**
     * @brief Convert an AmqpCollectionBase instance to a uAMQP AMQP_VALUE.
     *
     * @remarks This is an internal accessor and should never be used by code outside the AMQP
     * implementation.
     *
     */
    virtual operator _detail::AmqpValueImpl() const;
  };
}}}}} // namespace Azure::Core::Amqp::Models::_detail

namespace Azure { namespace Core { namespace Amqp { namespace Models {
  /** @brief Represents an AMQP array.
   *
   * An AMQP array is an aggregate of value types, all of which are of the same type.
   */
  class AmqpArray final : public _detail::AmqpCollectionBase<std::vector<AmqpValue>, AmqpArray> {
  public:
    /** @brief Construct a new AmqpArray object. */
    AmqpArray() : AmqpCollectionBase(){};

    /** @brief Construct a new AmqpArray object with an initializer list. */
    AmqpArray(initializer_type const& values);

    /** @brief Destroy an array. */
    virtual ~AmqpArray() = default;

    /** @brief Copy constructor */
    AmqpArray(const AmqpArray& other) = default;

    /** @brief Copy assignment operator */
    AmqpArray& operator=(const AmqpArray& other) = default;

    /** @brief Move constructor */
    AmqpArray(AmqpArray&& other) noexcept = default;

    /** @brief Move assignment operator */
    AmqpArray& operator=(AmqpArray&& other) noexcept = default;

    /** @brief Construct a new AmqpArray object from an existing uAMQP AMQP_VALUE item
     * @remarks Note that this does NOT capture the passed in AMQP_VALUE object, the caller is
     * responsible for freeing that object.
     *
     * @remarks This is an internal accessor and should never be used by code outside the AMQP
     * implementation.
     *
     * @param value - the AMQP array value to capture.
     */
    AmqpArray(AmqpValue const& value);
  };
  std::ostream& operator<<(std::ostream& os, AmqpArray const& value);

  /** @brief An AmqpMap represents an AMQP "map" type.
   *
   * An AMQP Map is a polymorphic map of distinct keys to values.
   *
   */
  class AmqpMap final
      : public _detail::AmqpCollectionBase<std::map<AmqpValue, AmqpValue>, AmqpMap> {

  public:
    /** @brief Construct a new AmqpMap object. */
    AmqpMap() : AmqpCollectionBase(){};

    /** @brief Construct a new AmqpArray object with an initializer list. */
    AmqpMap(std::initializer_list<std::map<AmqpValue, AmqpValue>::value_type> const& values)
        : AmqpCollectionBase(values)
    {
    }

    /** @brief Move Constructor */
    AmqpMap(AmqpMap&& other) noexcept = default;

    /** @brief Move assignment operator */
    AmqpMap& operator=(AmqpMap&& other) noexcept = default;

    /** @brief Copy Constructor */
    AmqpMap(const AmqpMap& other) = default;

    /** @brief Copy assignment operator */
    AmqpMap& operator=(const AmqpMap& other) = default;

    /** @brief Construct a new AmqpMap object from an existing uAMQP AMQP_VALUE item
     * @remarks Note that this does NOT capture the passed in AMQP_VALUE object, the caller is
     * responsible for freeing that object.
     *
     * @remarks This is an internal accessor and should never be used by code outside the AMQP
     * implementation.
     *
     * @param value - the AMQP array value to capture.
     */
    AmqpMap(AmqpValue const& value);

    virtual ~AmqpMap() = default;

    /** @brief Access an element in the map.
     *
     * @return a reference to the value associated with the specified key.
     */
    decltype(m_value)::mapped_type& operator[](decltype(m_value)::key_type&& keyVal)
    {
      return m_value.operator[](std::move(keyVal));
    }

    /**
     * @brief Find a value in the AMQP value map.
     *
     * @param keyVal The key to search for in the map.
     * @returns An iterator to the found element, or the end iterator if the key is not found.
     */
    decltype(m_value)::iterator find(const decltype(m_value)::key_type& keyVal)
    {
      return m_value.find(keyVal);
    }

    /** @brief Insert a new key/value pair into the map.
     *
     * @return std::pair<iterator, bool> - a pair containing an iterator to the inserted element,
     * or an iterator to the element that prevented the insertion, and a bool indicating whether
     * the insertion took place.
     */
    template <class... ValueTypes>
    std::pair<decltype(m_value)::iterator, bool> emplace(ValueTypes&&... values)
    {
      return m_value.emplace(std::forward<ValueTypes>(values)...);
    }
  };
  std::ostream& operator<<(std::ostream& os, AmqpMap const& value);

  /** @brief An AMQP List is a sequence of polymorphic values. It has the behavioral
   * characteristics of an AMQP array, but allows the members to be polymorphic.
   */
  class AmqpList final : public _detail::AmqpCollectionBase<std::vector<AmqpValue>, AmqpList> {
  public:
    AmqpList() : AmqpCollectionBase(){};
    virtual ~AmqpList() = default;
    /** @brief Construct a new AmqpList object with an initializer list. */
    AmqpList(std::initializer_list<std::vector<AmqpValue>::value_type> const& values)
        : AmqpCollectionBase(values)
    {
    }

    /** @brief Copy Constructor */
    AmqpList(const AmqpList& other) = default;

    /** @brief Copy assignment operator */
    AmqpList& operator=(const AmqpList& other) = default;

    /** @brief Move Constructor */
    AmqpList(AmqpList&& other) noexcept = default;

    /** @brief Move assignment operator */
    AmqpList& operator=(AmqpList&& other) noexcept = default;

    /** @brief Construct a new AmqpList object from an existing AmqpValue item
     *
     * @remarks Note that this does NOT capture the passed in AMQP_VALUE object, the caller is
     * responsible for freeing that object.
     *
     * @param value - the AMQP array value to capture.
     */
    AmqpList(AmqpValue const& value);
  };
  std::ostream& operator<<(std::ostream& os, AmqpList const& value);

  /** @brief An AMQP binary value, a sequence of octets
   *
   * Defined in [AMQP Core Types
   * section 1.6.19](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-binary).
   *
   */
  class AmqpBinaryData final
      : public _detail::AmqpCollectionBase<std::vector<std::uint8_t>, AmqpBinaryData> {
  public:
    AmqpBinaryData() : AmqpCollectionBase(){};
    virtual ~AmqpBinaryData() = default;
    /** @brief Construct a new AmqpBinaryData object with an initializer list. */
    AmqpBinaryData(initializer_type const& values) : AmqpCollectionBase(values){};
    /** @brief Construct a new AmqpBinaryData from a vector of bytes. */
    AmqpBinaryData(std::vector<std::uint8_t> const& values) : AmqpCollectionBase(values){};

    /** @brief Copy constructor */
    AmqpBinaryData(const AmqpBinaryData& other) = default;

    /** @brief Copy assignment operator */
    AmqpBinaryData& operator=(const AmqpBinaryData& other) = default;

    /** @brief Move Constructor */
    AmqpBinaryData(AmqpBinaryData&& other) noexcept = default;

    /** @brief Move assignment operator */
    AmqpBinaryData& operator=(AmqpBinaryData&& other) noexcept = default;

    /** @brief Assign a vector of bytes to the current AmqpBinaryData. */
    AmqpBinaryData& operator=(std::vector<std::uint8_t> const& values)
    {
      m_value = values;
      return *this;
    };

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
    AmqpBinaryData(AmqpValue const& value);
  };
  std::ostream& operator<<(std::ostream& os, AmqpBinaryData const& value);

  /** @brief An AMQP symbol value. An AMQP Symbol is a string valued from a constrained domain,
   * whose content is sequence of ASCII encoded characters.
   *
   * Defined in [AMQP Core Types
   * section 1.6.19](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-symbol).
   *
   */
  class AmqpSymbol final : public _detail::AmqpCollectionBase<std::string, AmqpSymbol> {
  public:
    AmqpSymbol() : AmqpCollectionBase(){};
    virtual ~AmqpSymbol() = default;

    /** @brief Construct a new AmqpSymbol object with an initializer list. */
    AmqpSymbol(std::string const& values) : AmqpCollectionBase(values){};

    /** @brief Construct a new AmqpSymbol object with an initializer list. */
    AmqpSymbol(initializer_type const& initializer) : AmqpCollectionBase(initializer) {}

    /** @brief Construct a new AmqpSymbol object from a constant string value. */
    AmqpSymbol(const char* const values) : AmqpCollectionBase(values){};

    /** @brief Copy constructor */
    AmqpSymbol(const AmqpSymbol& other) = default;

    /** @brief Copy Assignment operator */
    AmqpSymbol& operator=(const AmqpSymbol& other) = default;

    /** @brief Move constructor */
    AmqpSymbol(AmqpSymbol&& other) noexcept = default;

    /** @brief Move Assignment operator */
    AmqpSymbol& operator=(AmqpSymbol&& other) noexcept = default;

    AmqpValueType GetType() const { return AmqpValueType::Symbol; }

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
    AmqpSymbol(AmqpValue const& value);

    /** @brief Compare two AMQP symbols for equality.
     *
     * @param that - the AMQP symbol to compare to.
     * @returns true if the two symbols are equal, false otherwise.
     */
    bool operator==(AmqpSymbol const& that) const { return m_value == that.m_value; }
    /** @brief Compare two AMQP symbols for equality.
     *
     * @param that - the AMQP symbol to compare to.
     * @returns true if the two symbols are equal, false otherwise.
     */
    bool operator==(const decltype(m_value)::value_type* const that) const
    {
      return m_value == that;
    }
  };
  std::ostream& operator<<(std::ostream& os, AmqpSymbol const& value);

  /** @brief An AmqpAnnotations represents AMQP Message Annotations.
   *
   * An AMQP Map is a polymorphic map of AMQP Symbol keys to values.
   *
   */
  class AmqpAnnotations final
      : public _detail::AmqpCollectionBase<std::map<AmqpSymbol, AmqpValue>, AmqpAnnotations> {

  public:
    /** @brief Construct a new AmqpMap object. */
    AmqpAnnotations() : AmqpCollectionBase(){};

    /** @brief Construct a new AmqpArray object with an initializer list. */
    AmqpAnnotations(
        std::initializer_list<std::map<AmqpSymbol, AmqpValue>::value_type> const& values)
        : AmqpCollectionBase(values)
    {
    }

    /** @brief Move Constructor */
    AmqpAnnotations(AmqpAnnotations&& other) noexcept = default;

    /** @brief Move assignment operator */
    AmqpAnnotations& operator=(AmqpAnnotations&& other) noexcept = default;

    /** @brief Copy Constructor */
    AmqpAnnotations(const AmqpAnnotations& other) = default;

    /** @brief Copy assignment operator */
    AmqpAnnotations& operator=(const AmqpAnnotations& other) = default;

    /** @brief Construct a new AmqpMap object from an existing uAMQP AMQP_VALUE item
     * @remarks Note that this does NOT capture the passed in AMQP_VALUE object, the caller is
     * responsible for freeing that object.
     *
     * @remarks This is an internal accessor and should never be used by code outside the AMQP
     * implementation.
     *
     * @param value - the AMQP array value to capture.
     */
    AmqpAnnotations(AmqpValue const& value);

    virtual ~AmqpAnnotations() = default;

    /** @brief Access an element in the map.
     *
     * @return a reference to the value associated with the specified key.
     */
    decltype(m_value)::mapped_type& operator[](decltype(m_value)::key_type&& keyVal)
    {
      return m_value.operator[](std::move(keyVal));
    }

    /**
     * @brief Find a value in the AMQP value map.
     *
     * @param keyVal The key to search for in the map.
     * @returns An iterator to the found element, or the end iterator if the key is not found.
     */
    decltype(m_value)::iterator find(const decltype(m_value)::key_type& keyVal)
    {
      return m_value.find(keyVal);
    }

    /** @brief Insert a new key/value pair into the map.
     *
     * @return std::pair<iterator, bool> - a pair containing an iterator to the inserted element,
     * or an iterator to the element that prevented the insertion, and a bool indicating whether
     * the insertion took place.
     */
    template <class... ValueTypes>
    std::pair<decltype(m_value)::iterator, bool> emplace(ValueTypes&&... values)
    {
      return m_value.emplace(std::forward<ValueTypes>(values)...);
    }

    bool operator==(AmqpAnnotations const& that) const { return m_value == that.m_value; }
  };
  std::ostream& operator<<(std::ostream& os, AmqpAnnotations const& value);

  /** @brief An AMQP timestamp value.
   *
   * Defined in [AMQP Core Types
   * section 1.6.19](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#type-timestamp).
   *
   */
  class AmqpTimestamp final {
    std::chrono::milliseconds m_value;

  public:
    AmqpTimestamp();
    ~AmqpTimestamp() = default;

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
    AmqpTimestamp(AmqpValue const& value);

    /** @brief Copy constructor */
    AmqpTimestamp(const AmqpTimestamp& other) = default;

    /** @brief Copy Assignment operator */
    AmqpTimestamp& operator=(const AmqpTimestamp& other) = default;

    /** @brief Move constructor */
    AmqpTimestamp(AmqpTimestamp&& other) noexcept = default;

    /** @brief Move Assignment operator */
    AmqpTimestamp& operator=(AmqpTimestamp&& other) noexcept = default;

    /**
     * @brief Convert an AmqpTimestamp instance to an AmqpValue.
     *
     * @remarks This is an internal accessor and should never be used by code outside the AMQP
     * implementation.
     *
     * @remarks Note that this returns a newly allocated AMQP_VALUE object which must be freed
     * by the caller.
     */
    operator _detail::AmqpValueImpl() const;

    /** @brief Convert this AmqpTimestamp to an AmqpValue.
     *
     * @return An AmqpValue containing this AmqpTimestamp.
     */
    AmqpValue AsAmqpValue() const;

    /**
     * @brief Convert an AmqpTimestamp instance to a std::chrono::milliseconds.
     */
    operator std::chrono::milliseconds() const { return m_value; }

    /**    @brief Compare this AMQP Timestamp with another.
     * @param that - the other AMQP Timestamp to compare with.
     * @return true if this AMQP Timestamp is less than the other.
     */
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
  class AmqpComposite final
      : public _detail::AmqpCollectionBase<std::vector<AmqpValue>, AmqpComposite> {
  public:
    /** @brief Construct a new AmqpComposite object. */
    AmqpComposite() : AmqpCollectionBase(){};
    virtual ~AmqpComposite() = default;

    /** @brief Construct a new AmqpComposite object with an initializer list. */
    AmqpComposite(AmqpValue const& descriptor, std::initializer_list<AmqpValue> const& values);

    /** @brief Construct a new AmqpComposite object from another */
    AmqpComposite(const AmqpComposite& other) = default;

    /** @brief Copy assignment operator.
     * @param other composite value to copy.
     * @returns reference to this composite value.
     */
    AmqpComposite& operator=(const AmqpComposite& other) = default;

    /** @brief Move constructor.
     * @param other composite value to move.
     */
    AmqpComposite(AmqpComposite&& other) noexcept = default;

    /** @brief Move assignment operator.
     * @param other composite value to move.
     * @returns reference to this composite value.
     */
    AmqpComposite& operator=(AmqpComposite&& other) noexcept = default;

    /** @brief Construct a new AmqpComposite object from an existing uAMQP AMQP_VALUE item
     * @remarks Note that this does NOT capture the passed in AMQP_VALUE object, the caller is
     * responsible for freeing that object.
     *
     * @remarks This is an internal accessor and should never be used by code outside the AMQP
     * implementation.
     *
     * @param value - the AMQP array value to capture.
     */
    AmqpComposite(AmqpValue const& value);

    /** @brief Compare this AmqpComposite value with another.
     *
     * @param that - the AmqpComposite to compare with.
     */
    bool operator==(AmqpComposite const& that) const
    {
      if (GetDescriptor() == that.GetDescriptor())
      {
        return m_value == that.m_value;
      }
      return false;
    }

    /** @brief Compare this AmqpComposite value with another.
     *
     * @param that - the AmqpComposite to compare with.
     */
    bool operator!=(AmqpComposite const& that) const { return !(*this == that); }

    /** @brief Returns the descriptor for this composite type.
     *
     * @returns The descriptor for this composite type.
     */
    AmqpValue const& GetDescriptor() const { return m_descriptor; }

  protected:
    /**
     * @brief Convert an AmqpComposite instance to a uAMQP AMQP_VALUE.
     *
     * @remarks This is an internal accessor and should never be used by code outside the AMQP
     * implementation.
     *
     * @remarks Note that this returns a newly allocated AMQP_VALUE object which must be freed
     * by the caller.
     */
    operator _detail::AmqpValueImpl() const override;

  private:
    AmqpValue m_descriptor;
  };

  /** @brief An AmqpDescribed represents an AMQP described type.
   *
   * @remarks An [AMQP Described
   * type](http://docs.oasis-open.org/amqp/core/v1.0/os/amqp-core-types-v1.0-os.html#doc-idp38080)
   * is a tuple consisting of a type and a "descriptor" for that type. The "descriptor"
   * indicates that the AMQP object is a representation of the type.
   *
   */
  class AmqpDescribed final {
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

    /** @brief Construct a new AmqpDescribed object by moving the contents of another AmqpDescribed
     * object.
     *
     * @param other - the AmqpDescribed object to move.
     */
    AmqpDescribed(AmqpDescribed&& other) noexcept = default;

    /** @brief Copy constructor for AmqpDescribed.
     *
     * @param other - the AmqpDescribed object to copy.
     */
    AmqpDescribed(const AmqpDescribed& other) = default;

    /** @brief Move assignment operator for AmqpDescribed.
     *
     * @param other - the AmqpDescribed object to move.
     * @returns reference to this AmqpDescribed object.
     */
    AmqpDescribed& operator=(AmqpDescribed&& other) noexcept = default;

    /** @brief Copy assignment operator for AmqpDescribed.
     *
     * @param other - the AmqpDescribed object to copy.
     * @returns reference to this AmqpDescribed object.
     */
    AmqpDescribed& operator=(const AmqpDescribed& other) = default;

    ~AmqpDescribed() = default;

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
    AmqpDescribed(AmqpValue const& value);

    /** @brief Compare this AmqpDescribed value with another.
     *
     * @param that - the AmqpDescribed to compare with.
     */
    bool operator==(AmqpDescribed const& that) const
    {
      if (GetDescriptor() == that.GetDescriptor())
      {
        return GetValue() == that.GetValue();
      }
      return false;
    }

    /** @brief Compare this AmqpDescribed value with another.
     *
     * @param that - the AmqpDescribed to compare with.
     */
    bool operator!=(AmqpDescribed const& that) const { return !(*this == that); }
    /**
     * @brief Convert an existing AmqpComposite to an AmqpValue.
     */
    explicit operator AmqpValue const() const;

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
    operator _detail::AmqpValueImpl() const;

    /** @brief Convert this AmqpDescribed to an AmqpValue.
     *
     * @return An AmqpValue containing this AmqpDescribed.
     */
    AmqpValue AsAmqpValue() const;

    /**
     * @brief Compare this AmqpDescribed value with another.
     *
     * @param that - the AmqpDescribed to compare with.
     * @returns true if this AmqpDescribed is less than the other.
     */
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
