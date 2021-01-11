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
  namespace Details {
    class Clock {
    public:
      using rep = int64_t;
      using period = std::ratio<1, 10000000>;
      using duration = std::chrono::duration<rep, period>;
      using time_point = std::chrono::time_point<Clock>;
      static constexpr bool is_steady = true;

      static time_point now() noexcept;
    };
  } // namespace Details

  /**
   * @brief Manages date and time in standardized string formats.
   * FIXME: update, put more details
   * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   */
  class DateTime : public Details::Clock::time_point {
    static DateTime const SystemClockEpoch;

  public:
    /**
     * @brief Construct a default instance of @DateTime (00:00:00.0000000 on Janualy 1st, 0001).
     */
    constexpr DateTime() : time_point() {}

  private:
    DateTime(
        int16_t year,
        int8_t month,
        int8_t day,
        int8_t hour,
        int8_t minute,
        int8_t second,
        int32_t fracSec,
        int8_t dayOfWeek,
        int8_t localDiffHours,
        int8_t localDiffMinutes,
        bool roundFracSecUp = false);

  public:
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
        int8_t second = 0)
        : DateTime(year, month, day, hour, minute, second, 0, -1, 0, 0)
    {
    }

    /**
     * @brief Construct an instance of @DateTime from base class.
     */
    constexpr DateTime(time_point const& timePoint) : time_point(timePoint) {}

    /**
     * @brief Construct an instance of @DateTime from `std::chrono::system_clock::time_point`.
     * @param systemTime A value of `std::chrono::system_clock::time_point`.
     */
    DateTime(std::chrono::system_clock::time_point const& systemTime)
        : DateTime(SystemClockEpoch + systemTime.time_since_epoch())
    {
    }

    /**
     * @brief Convert an instance of @DateTime to `std::chrono::system_clock::time_point`.
     * @throw std::invalid_argument if @DateTime is outside of the range that can be represented.
     */
    explicit operator std::chrono::system_clock::time_point() const;

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
  };

  inline Details::Clock::time_point Details::Clock::now() noexcept
  {
    return DateTime(std::chrono::system_clock::now());
  }

  inline bool operator==(DateTime const& dt, std::chrono::system_clock::time_point const& tp)
  {
    return dt == DateTime(tp);
  }

  inline bool operator<(DateTime const& dt, std::chrono::system_clock::time_point const& tp)
  {
    return dt < DateTime(tp);
  }

  inline bool operator<=(DateTime const& dt, std::chrono::system_clock::time_point const& tp)
  {
    return dt <= DateTime(tp);
  }

  inline bool operator!=(DateTime const& dt, std::chrono::system_clock::time_point const& tp)
  {
    return !(dt == tp);
  }

  inline bool operator>(DateTime const& dt, std::chrono::system_clock::time_point const& tp)
  {
    return !(dt <= tp);
  }

  inline bool operator>=(DateTime const& dt, std::chrono::system_clock::time_point const& tp)
  {
    return !(dt < tp);
  }

  inline bool operator==(std::chrono::system_clock::time_point const& tp, DateTime const& dt)
  {
    return dt == tp;
  }

  inline bool operator!=(std::chrono::system_clock::time_point const& tp, DateTime const& dt)
  {
    return dt != tp;
  }

  inline bool operator<(std::chrono::system_clock::time_point const& tp, DateTime const& dt)
  {
    return !(dt >= tp);
  }

  inline bool operator<=(std::chrono::system_clock::time_point const& tp, DateTime const& dt)
  {
    return !(dt > tp);
  }

  inline bool operator>(std::chrono::system_clock::time_point const& tp, DateTime const& dt)
  {
    return !(dt <= tp);
  }

  inline bool operator>=(std::chrono::system_clock::time_point const& tp, DateTime const& dt)
  {
    return !(dt < tp);
  }
}} // namespace Azure::Core
