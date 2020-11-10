// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Support for date and time standardized string formats.
 */

#pragma once

#include <stdexcept>
#include <string>

namespace Azure { namespace Core {

  /**
   * @brief An exception that gets thrown when @DateTime error occurs.
   */
  class DateTimeException : public std::runtime_error {
  public:
    /**
     * @brief Construct with message string.
     *
     * @param message Message string.
     */
    explicit DateTimeException(std::string const& message) : std::runtime_error(message) {}
  };

  /**
   * @brief Manages date and time in standardized string formats.
   */
  class DateTime {
  public:
    /**
     * @brief Represents time duration.
     */
    class Duration {
      friend class DateTime;

      int64_t m_100nsIntervals;
      constexpr explicit Duration(int64_t intervalsOf100ns) : m_100nsIntervals(intervalsOf100ns) {}

      static constexpr int64_t NanosecondResolution = 100;
      static constexpr int64_t IntervalsOf100nsPerMicrosecond = 10;
      static constexpr int64_t IntervalsOf100nsPerMillisecond
          = 1000 * IntervalsOf100nsPerMicrosecond;
      static constexpr int64_t IntervalsOf100nsPerSecond = 1000 * IntervalsOf100nsPerMillisecond;
      static constexpr int64_t IntervalsOf100nsPerMinute = 60 * IntervalsOf100nsPerSecond;
      static constexpr int64_t IntervalsOf100nsPerHour = 60 * IntervalsOf100nsPerMinute;

    public:
      constexpr Duration() : m_100nsIntervals(0) {}

      static constexpr Duration FromHours(int32_t hours)
      {
        return Duration(hours * IntervalsOf100nsPerHour);
      }

      void constexpr AddHours(int32_t hours)
      {
        m_100nsIntervals += (hours * IntervalsOf100nsPerHour);
      }

      static constexpr Duration FromMinutes(int64_t minutes)
      {
        return Duration(minutes * IntervalsOf100nsPerMinute);
      }

      void constexpr AddMinutes(int64_t minutes)
      {
        m_100nsIntervals += (minutes * IntervalsOf100nsPerMinute);
      }

      static constexpr Duration FromSeconds(int64_t seconds)
      {
        return Duration(seconds * IntervalsOf100nsPerSecond);
      }

      void constexpr AddSeconds(int64_t seconds)
      {
        m_100nsIntervals += (seconds * IntervalsOf100nsPerSecond);
      }

      static constexpr Duration FromMilliseconds(int64_t milliseconds)
      {
        return Duration(milliseconds * IntervalsOf100nsPerMillisecond);
      }

      void constexpr AddMilliseconds(int64_t milliseconds)
      {
        m_100nsIntervals += (milliseconds * IntervalsOf100nsPerMillisecond);
      }

      static constexpr Duration FromMicroseconds(int64_t microseconds)
      {
        return Duration(microseconds * IntervalsOf100nsPerMicrosecond);
      }

      void constexpr AddMicroseconds(int64_t microseconds)
      {
        m_100nsIntervals += (microseconds * IntervalsOf100nsPerMillisecond);
      }

      static constexpr Duration FromNanoseconds(int64_t nanoseconds)
      {
        if (nanoseconds % NanosecondResolution != 0)
        {
          throw DateTimeException("Can't convert from nanoseconds");
        }

        return Duration(nanoseconds / NanosecondResolution);
      }

      void constexpr AddNanoseconds(int64_t nanoseconds)
      {
        if (nanoseconds % NanosecondResolution != 0)
        {
          throw DateTimeException("Can't convert from nanoseconds");
        }

        m_100nsIntervals += (nanoseconds / NanosecondResolution);
      }

      int64_t constexpr GetNanoseconds() const { return m_100nsIntervals * NanosecondResolution; }

      constexpr Duration& operator+=(Duration const& other)
      {
        m_100nsIntervals += other.m_100nsIntervals;
        return *this;
      }

      constexpr Duration& operator-=(Duration const& other)
      {
        m_100nsIntervals -= other.m_100nsIntervals;
        return *this;
      }

      constexpr bool operator==(Duration const& other) const
      {
        return (m_100nsIntervals == other.m_100nsIntervals);
      }

      constexpr bool operator<(Duration const& other) const
      {
        return (m_100nsIntervals < other.m_100nsIntervals);
      }

      constexpr bool operator<=(Duration const& other) const
      {
        return (*this < other) || (*this == other);
      }

      constexpr bool operator!=(Duration const& other) const { return !(*this == other); }

      constexpr bool operator>(Duration const& other) const { return !(*this <= other); }

      constexpr bool operator>=(Duration const& rhs) const { return !(*this < rhs); }

      constexpr Duration operator+(Duration const& other) const
      {
        return Duration(m_100nsIntervals + other.m_100nsIntervals);
      }

      constexpr Duration operator-(Duration const& other) const
      {
        return Duration(m_100nsIntervals - other.m_100nsIntervals);
      }
    };

    /**
     * @brief Defines the format applied to the fraction part from any @DateFormat
     *
     */
    enum class TimeFractionFormat
    {
      /// Decimals are not included when there are no decimals in the source Datetime and any zeros
      /// from the right are also removed.
      DropTrailingZeros,

      /// Decimals are included for any Datetime.
      AllDigits,

      /// Decimals are removed for any Datetime.
      Truncate
    };

    /**
     * @brief Defines the supported date and time string formats.
     */
    enum class DateFormat
    {
      /// RFC 1123.
      Rfc1123,

      /// ISO 8601.
      Iso8601,
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
     * @throw DateTimeException If any parameter is invalid.
     */
    DateTime(int16_t year, int8_t month = 1, int8_t day = 1, int8_t hour = 0, int8_t minute = 0, int8_t second = 0);

    /**
     * @brief Create @DateTime from a string representing time in UTC in the specified format.
     *
     * @param dateTime A string with the date and time.
     * @param format A format to which /p dateTime string adheres to.
     *
     * @return @DateTime that was constructed from the \p dateTime string.
     *
     * @throw DateTimeException If \p format is not recognized, or if parsing error.
     */
    static DateTime Parse(std::string const& dateTime, DateFormat format);

  private:
    /**
     * @brief Get a string representation of the @DateTime.
     *
     * @param format The representation format to use.
     * @param fractionFormat The format for the fraction part of the Datetime. Only supported by
     * ISO 8601.
     *
     * @throw DateTimeException If year exceeds 9999, or if \p format is not recognized.
     */
    std::string GetString(DateFormat format, TimeFractionFormat fractionFormat) const;

  public:
    /**
     * @brief Get a string representation of the @DateTime.
     *
     * @param format The representation format to use.
     *
     * @throw DateTimeException If year exceeds 9999, or if \p format is not recognized.
     */
    std::string GetString(DateFormat format) const
    {
      return GetString(format, TimeFractionFormat::DropTrailingZeros);
    };

    /**
     * @brief Get a string representation of the @DateTime formatted with ISO 8601.
     *
     * @param fractionFormat The format that is applied to the fraction part from the ISO 8601 date.
     *
     * @throw DateTimeException If year exceeds 9999, or if \p fractionFormat is not recognized.
     */
    std::string GetIso8601String(TimeFractionFormat fractionFormat) const
    {
      return GetString(DateFormat::Iso8601, fractionFormat);
    };

    DateTime& operator+=(Duration const& value)
    {
      m_since1601 += value;
      return *this;
    }

    DateTime& operator-=(Duration const& value)
    {
      m_since1601 -= value;
      return *this;
    }

    /// Subtract an interval from @DateTime.
    DateTime operator-(Duration const& value) const { return DateTime(m_since1601 - value); }

    /// Add an interval to @DateTime.
    DateTime operator+(Duration const& value) const { return DateTime(m_since1601 + value); }

    /// Get duration between two instances of @DateTime.
    Duration operator-(DateTime const& other) const { return m_since1601 - other.m_since1601; }

    /// Compare two instances of @DateTime for equality.
    constexpr bool operator==(DateTime const& dt) const { return m_since1601 == dt.m_since1601; }

    /// Compare two instances of @DateTime for inequality.
    constexpr bool operator!=(const DateTime& dt) const { return !(*this == dt); }

    /// Compare the chronological order of two @DateTime instances.
    constexpr bool operator>(const DateTime& dt) const { return !(*this <= dt); }

    /// Compare the chronological order of two @DateTime instances.
    constexpr bool operator<(const DateTime& dt) const
    {
      return this->m_since1601 < dt.m_since1601;
    }

    /// Compare the chronological order of two @DateTime instances.
    constexpr bool operator>=(const DateTime& dt) const { return !(*this < dt); }

    /// Compare the chronological order of two @DateTime instances.
    constexpr bool operator<=(const DateTime& dt) const { return (*this == dt) || (*this < dt); }

  private:
    // Private constructor. Use static methods to create an instance.
    DateTime(Duration const& since1601) : m_since1601(since1601) {}
    Duration m_since1601;
  };
}} // namespace Azure::Core
