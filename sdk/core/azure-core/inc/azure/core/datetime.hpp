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
   * @brief Manages date and time in standardized string formats.
   */
  class DateTime {
  public:
    /// A type that represents tick spans.
    typedef uint64_t IntervalType;

  private:
    //  Number of seconds between 01-01-1970 and 01-01-1601.
    static constexpr IntervalType WindowsToPosixOffsetSeconds = 11644473600LL;

  public:
    /**
     * @brief Defines the supported date and time string formats.
     */
    enum class DateFormat
    {
      /// RFC 1123.
      Rfc1123,

      /// ISO 8601.
      Iso8601
    };

    /**
     * @brief Get the current UTC time.
     */
    static DateTime UtcNow();

    /// An invalid UTC timestamp value.
    static constexpr IntervalType UtcTimestampInvalid = static_cast<IntervalType>(-1);

    /**
     * @brief Get seconds since Unix/POSIX time epoch at `01-01-1970 00:00:00`.
     * If time is before epoch, @UtcTimestampInvalid is returned.
     */
    static IntervalType UtcTimestamp()
    {
      auto const seconds = UtcNow().ToInterval() / WindowsToPosixOffsetSeconds;
      return (seconds >= WindowsToPosixOffsetSeconds) ? (seconds - WindowsToPosixOffsetSeconds)
                                                      : UtcTimestampInvalid;
    }

    /**
     * @brief Construct an uninitialized (!@IsInitialized()) instance of @DateTime.
     */
    DateTime() : m_interval(0) {}

    /**
     * @brief Create @DateTime from a string representing time in UTC in the specified format.
     *
     * @param timeString A string with the date and time.
     * @param format A format to which /p timeString adheres to.
     *
     * @return @DateTime that was constructed from the \p timeString; Uninitialized
     * (!@IsInitialized()) @DateTime if parsing \p timeString was not successful.
     *
     * @throw DateTimeException If \p format is not recognized.
     */
    static DateTime FromString(
        std::string const& timeString,
        DateFormat format = DateFormat::Rfc1123);

    /**
     * @brief Get a string representation of the @DateTime.
     *
     * @param format The representation format to use.
     *
     * @throw DateTimeException If year exceeds 9999, or if \p format is not recognized.
     */
    std::string ToString(DateFormat format = DateFormat::Rfc1123) const;

    /// Get the integral time value.
    IntervalType ToInterval() const { return m_interval; }

    /// Subtract an interval from @DateTime.
    DateTime operator-(IntervalType value) const { return DateTime(m_interval - value); }

    /// Add an interval to @DateTime.
    DateTime operator+(IntervalType value) const { return DateTime(m_interval + value); }

    /// Compare two instances of @DateTime for equality.
    bool operator==(DateTime dt) const { return m_interval == dt.m_interval; }

    /// Compare two instances of @DateTime for inequality.
    bool operator!=(const DateTime& dt) const { return !(*this == dt); }

    /// Compare the chronological order of two @DateTime instances.
    bool operator>(const DateTime& dt) const { return this->m_interval > dt.m_interval; }

    /// Compare the chronological order of two @DateTime instances.
    bool operator<(const DateTime& dt) const { return this->m_interval < dt.m_interval; }

    /// Compare the chronological order of two @DateTime instances.
    bool operator>=(const DateTime& dt) const { return this->m_interval >= dt.m_interval; }

    /// Compare the chronological order of two @DateTime instances.
    bool operator<=(const DateTime& dt) const { return this->m_interval <= dt.m_interval; }

    /// Create an interval from milliseconds.
    static IntervalType FromMilliseconds(unsigned int milliseconds)
    {
      return milliseconds * TicksPerMillisecond;
    }

    /// Create an interval from seconds.
    static IntervalType FromSeconds(unsigned int seconds) { return seconds * TicksPerSecond; }

    /// Create an interval from minutes.
    static IntervalType FromMinutes(unsigned int minutes) { return minutes * TicksPerMinute; }

    /// Create an interval from hours.
    static IntervalType FromHours(unsigned int hours) { return hours * TicksPerHour; }

    /// Create an interval from days.
    static IntervalType FromDays(unsigned int days) { return days * TicksPerDay; }

    /// Checks whether this instance of @DateTime is initialized.
    bool IsInitialized() const { return m_interval != 0; }

  private:
    friend IntervalType operator-(DateTime t1, DateTime t2);

    static constexpr IntervalType TicksPerMillisecond = static_cast<IntervalType>(10000);
    static constexpr IntervalType TicksPerSecond = 1000 * TicksPerMillisecond;
    static constexpr IntervalType TicksPerMinute = 60 * TicksPerSecond;
    static constexpr IntervalType TicksPerHour = 60 * TicksPerMinute;
    static constexpr IntervalType TicksPerDay = 24 * TicksPerHour;

    // Private constructor. Use static methods to create an instance.
    DateTime(IntervalType interval) : m_interval(interval) {}

    // Storing as hundreds of nanoseconds 10e-7, i.e. 1 here equals 100ns.
    IntervalType m_interval;
  };

  inline DateTime::IntervalType operator-(DateTime t1, DateTime t2)
  {
    auto diff = (t1.m_interval - t2.m_interval);

    // Round it down to seconds
    diff /= DateTime::TicksPerSecond;

    return static_cast<DateTime::IntervalType>(diff);
  }

  /**
   * @brief An exception that gets thrown when @DateTime error occurs.
   */
  class DateTimeException : public std::runtime_error {
  public:
    /**
     * @brief Construct with message string.
     *
     * @param msg Message string.
     */
    explicit DateTimeException(std::string const& msg) : std::runtime_error(msg) {}
  };
}} // namespace Azure::Core
