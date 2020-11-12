// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Support for date and time standardized string formats.
 */

#pragma once

#include <chrono>
#include <string>

namespace Azure { namespace Core {
  /**
   * @brief Manages date and time in standardized string formats.
   */
  class DateTime {
  public:
    /**
     * @brief Units of measurement the difference between instances of @DateTime.
     */
    // 1 == 100 ns (1 / 10,000,000 of a second, 7 fractional digits).
    typedef std::chrono::duration<int64_t, std::ratio<1, 10000000>> Duration;

    /**
     * @brief Defines the format applied to the fraction part of any @DateTime.
     */
    enum class TimeFractionFormat
    {
      /// Include only meaningful fractional time digits, up to and excluding trailing zeroes.
      DropTrailingZeros,

      /// Include all the fractional time digits up to maximum precision, even if the entire value
      /// is zero.
      AllDigits,

      /// Drop all the fractional time digits.
      Truncate
    };

    /**
     * @brief Defines the supported date and time string formats.
     */
    enum class DateFormat
    {
      /// RFC 1123.
      Rfc1123,

      /// RFC 3339.
      Rfc3339,
    };

    /**
     * @brief Get the current UTC time.
     */
    static DateTime Now();

    /**
     * @brief Construct an instance of @DateTime.
     *
     * @param year Year.
     * @param month Month.
     * @param day Day.
     * @param hour Hour.
     * @param minute Minute.
     * @param second Seconds.
     *
     * @throw std::invalid_argument If any parameter is invalid.
     */
    explicit DateTime(
        int16_t year,
        int8_t month = 1,
        int8_t day = 1,
        int8_t hour = 0,
        int8_t minute = 0,
        int8_t second = 0);

    /**
     * @brief Create @DateTime from a string representing time in UTC in the specified format.
     *
     * @param dateTime A string with the date and time.
     * @param format A format to which \p dateTime string adheres to.
     *
     * @return @DateTime that was constructed from the \p dateTime string.
     *
     * @throw std::invalid_argument If \p format is not recognized, or if parsing error.
     */
    static DateTime Parse(std::string const& dateTime, DateFormat format);

  private:
    /**
     * @brief Get a string representation of the @DateTime.
     *
     * @param format The representation format to use.
     * @param fractionFormat The format for the fraction part of the Datetime. Only supported by
     * RFC 3339.
     *
     * @throw std::invalid_argument If year exceeds 9999, or if \p format is not recognized.
     */
    std::string GetString(DateFormat format, TimeFractionFormat fractionFormat) const;

  public:
    /**
     * @brief Get a string representation of the @DateTime.
     *
     * @param format The representation format to use.
     *
     * @throw std::invalid_argument If year exceeds 9999, or if \p format is not recognized.
     */
    std::string GetString(DateFormat format) const
    {
      return GetString(format, TimeFractionFormat::DropTrailingZeros);
    };

    /**
     * @brief Get a string representation of the @DateTime formatted with RFC 3339.
     *
     * @param fractionFormat The format that is applied to the fraction part from the RFC 3339 date.
     *
     * @throw std::invalid_argument If year exceeds 9999, or if \p format is not recognized.
     */
    std::string GetRfc3339String(TimeFractionFormat fractionFormat) const
    {
      return GetString(DateFormat::Rfc3339, fractionFormat);
    };

    /**
     * @brief Add \p duration to this @DateTime.
     * @param duration @Duration to add.
     * @return Reference to this @DateTime.
     */
    DateTime& operator+=(Duration const& duration)
    {
      m_since1601 += duration;
      return *this;
    }

    /**
     * @brief Subtract \p duration from this @DateTime.
     * @param duration @Duration to subtract from this @DateTime.
     * @return Reference to this @DateTime.
     */
    DateTime& operator-=(Duration const& duration)
    {
      m_since1601 -= duration;
      return *this;
    }

    /**
     * @brief Subtract @Duration from @DateTime.
     * @param duration @Duration to subtract from this @DateTime.
     * @return New DateTime representing subtraction result.
     */
    DateTime operator-(Duration const& duration) const { return DateTime(m_since1601 - duration); }

    /**
     * @brief Add @Duration to @DateTime.
     * @param duration @Duration to add to this @DateTime.
     * @return New DateTime representing addition result.
     */
    DateTime operator+(Duration const& duration) const { return DateTime(m_since1601 + duration); }

    /**
     * @brief Get @Duration between two instances of @DateTime.
     * @param other @DateTime to subtract from this @DateTime.
     * @return @Duration between this @DateTime and the \p other.
     */
    Duration operator-(DateTime const& other) const { return m_since1601 - other.m_since1601; }

    /**
     * @brief Compare with \p other @DateTime for equality.
     * @param other Other @DateTime to compare with.
     * @return `true` if @DateTime instances are equal, `false` otherwise.
     */
    constexpr bool operator==(DateTime const& other) const
    {
      return m_since1601 == other.m_since1601;
    }

    /**
     * @brief Compare with \p other @DateTime for inequality.
     * @param other Other @DateTime to compare with.
     * @return `true` if @DateTime instances are not equal, `false` otherwise.
     */
    constexpr bool operator!=(const DateTime& other) const { return !(*this == other); }

    /**
     * @brief Check if \p other @DateTime precedes this @DateTime chronologically.
     * @param other @DateTime to compare with.
     * @return `true` if the \p other @DateTime precedes this, `false` otherwise.
     */
    constexpr bool operator>(const DateTime& other) const { return !(*this <= other); }

    /**
     * @brief Check if \p other @DateTime is chronologically after this @DateTime.
     * @param other @DateTime to compare with.
     * @return `true` if the \p other @DateTime is chonologically after this @DateTime, `false`
     * otherwise.
     */
    constexpr bool operator<(const DateTime& other) const
    {
      return this->m_since1601 < other.m_since1601;
    }

    /**
     * @brief Check if \p other @DateTime precedes this @DateTime chronologically, or is equal to
     * it.
     * @param other @DateTime to compare with.
     * @return `true` if the \p other @DateTime precedes or is equal to this @DateTime, `false`
     * otherwise.
     */
    constexpr bool operator>=(const DateTime& other) const { return !(*this < other); }

    /**
     * @brief Check if \p other @DateTime is chronologically after or equal to this @DateTime.
     * @param other @DateTime to compare with.
     * @return `true` if the \p other @DateTime is chonologically after or equal to this @DateTime,
     * `false` otherwise.
     */
    constexpr bool operator<=(const DateTime& other) const
    {
      return (*this == other) || (*this < other);
    }

    /**
     * @brief Get this @DateTime representation as a @Duration from the start of the
     * implementation-defined epoch.
     * @return @Duration since the start of the implementation-defined epoch.
     */
    constexpr explicit operator Duration() const { return m_since1601; }

  private:
    // Private constructor. Use static methods to create an instance.
    explicit DateTime(Duration const& since1601) : m_since1601(since1601) {}
    Duration m_since1601;
  };
}} // namespace Azure::Core
