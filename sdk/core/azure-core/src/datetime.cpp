// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/datetime.hpp"

#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/time.h>
#endif

using namespace Azure::Core;

DateTime DateTime::Now()
{
#ifdef _WIN32
  FILETIME fileTime = {};
  GetSystemTimeAsFileTime(&fileTime);

  ULARGE_INTEGER largeInt = {};
  largeInt.LowPart = fileTime.dwLowDateTime;
  largeInt.HighPart = fileTime.dwHighDateTime;

  return DateTime(Duration(largeInt.QuadPart));
#else
  static constexpr int64_t WindowsToPosixOffsetSeconds = 11644473600LL;

  struct timeval time = {};
  if (gettimeofday(&time, nullptr) != 0)
  {
    return DateTime(Duration());
  }

  int64_t result = WindowsToPosixOffsetSeconds + time.tv_sec;
  result *= DateTime::Duration::period::den; // convert to 10e-7
  result += time.tv_usec * 10; // convert and add microseconds, 10e-6 to 10e-7

  return DateTime(Duration(result));
#endif
}

namespace {
struct ComputeYearResult
{
  int Year;
  int SecondsLeftThisYear;
};

constexpr auto MinYear = 1601;
constexpr auto MaxYear = 9999;

constexpr int SecondsInMinute = 60;
constexpr int SecondsInHour = SecondsInMinute * 60;
constexpr int SecondsInDay = 24 * SecondsInHour;

constexpr int DaysInYear = 365;

constexpr ComputeYearResult ComputeYear(int64_t secondsSince1601)
{
  constexpr int DaysIn4Years = DaysInYear * 4 + 1;

  constexpr int DaysIn100Years = DaysIn4Years * 25 - 1;
  constexpr int DaysIn400Years = DaysIn100Years * 4 + 1;

  constexpr int SecondsInYear = SecondsInDay * DaysInYear;
  constexpr int SecondsIn4Years = SecondsInDay * DaysIn4Years;

  constexpr int64_t SecondsIn100Years = static_cast<int64_t>(SecondsInDay) * DaysIn100Years;
  constexpr int64_t SecondsIn400Years = static_cast<int64_t>(SecondsInDay) * DaysIn400Years;

  int64_t secondsLeft = secondsSince1601;

  int year400 = static_cast<int>(secondsLeft / SecondsIn400Years);
  secondsLeft -= year400 * SecondsIn400Years;

  int year100 = static_cast<int>(secondsLeft / SecondsIn100Years);
  secondsLeft -= year100 * SecondsIn100Years;

  int year4 = static_cast<int>(secondsLeft / SecondsIn4Years);
  int secondsInt = static_cast<int>(secondsLeft - static_cast<int64_t>(year4) * SecondsIn4Years);

  int year1 = secondsInt / SecondsInYear;
  secondsInt -= year1 * SecondsInYear;

  return {year400 * 400 + year100 * 100 + year4 * 4 + year1, secondsInt};
}

constexpr bool IsLeapYear(int year)
{
  return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

// The following table assumes no leap year; leap year is added separately
constexpr unsigned short const CumulativeDaysToMonth[12] = {
    0, // Jan
    31, // Feb
    59, // Mar
    90, // Apr
    120, // May
    151, // Jun
    181, // Jul
    212, // Aug
    243, // Sep
    273, // Oct
    304, // Nov
    334 // Dec
};

constexpr unsigned short const CumulativeDaysToMonthLeap[12] = {
    0, // Jan
    31, // Feb
    60, // Mar
    91, // Apr
    121, // May
    152, // Jun
    182, // Jul
    213, // Aug
    244, // Sep
    274, // Oct
    305, // Nov
    335 // Dec
};

constexpr char const dayNames[] = "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat";
constexpr char const monthNames[] = "Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec";

} // namespace

std::string DateTime::GetString(DateFormat format, TimeFractionFormat fractionFormat) const
{
  static constexpr auto const EndOfYear9999 = 2650467743999999999LL;
  if (m_since1601.count() > EndOfYear9999)
  {
    throw std::invalid_argument("The requested year exceeds the year 9999.");
  }

  int64_t const epochAdjusted = m_since1601.count();
  int64_t const secondsSince1601 = epochAdjusted / Duration::period::den; // convert to seconds
  int const fracSec = static_cast<int>(epochAdjusted % Duration::period::den);

  auto const yearData = ComputeYear(secondsSince1601);
  int const year = yearData.Year + MinYear;
  int const yearDay = yearData.SecondsLeftThisYear / SecondsInDay;

  int leftover = yearData.SecondsLeftThisYear % SecondsInDay;
  int const hour = leftover / SecondsInHour;

  leftover = leftover % SecondsInHour;
  int const minute = leftover / SecondsInMinute;

  leftover = leftover % SecondsInMinute;

  auto const& monthTable = IsLeapYear(year) ? CumulativeDaysToMonthLeap : CumulativeDaysToMonth;
  int month = 0;
  while (month < 11 && monthTable[month + 1] <= yearDay)
  {
    ++month;
  }

  auto const monthDay = yearDay - monthTable[month] + 1;
  auto const weekday = static_cast<int>((secondsSince1601 / SecondsInDay + 1) % 7);

  char outBuffer[38]{}; // Thu, 01 Jan 1970 00:00:00 GMT\0
                        // 1970-01-01T00:00:00.1234567Z\0

  char* outCursor = outBuffer;
  switch (format)
  {
    case DateFormat::Rfc1123:
#ifdef _MSC_VER
      sprintf_s(
#else
      std::sprintf(
#endif
          outCursor,
#ifdef _MSC_VER
          26,
#endif
          "%s, %02d %s %04d %02d:%02d:%02d",
          dayNames + 4ULL * static_cast<uint64_t>(weekday),
          monthDay,
          monthNames + 4ULL * static_cast<uint64_t>(month),
          year,
          hour,
          minute,
          leftover);

      outCursor += 25;
      memcpy(outCursor, " GMT", 4);
      outCursor += 4;
      return std::string(outBuffer, outCursor);

    case DateFormat::Rfc3339:
#ifdef _MSC_VER
      sprintf_s(
#else
      std::sprintf(
#endif
          outCursor,
#ifdef _MSC_VER
          20,
#endif
          "%04d-%02d-%02dT%02d:%02d:%02d",
          year,
          month + 1,
          monthDay,
          hour,
          minute,
          leftover);

      outCursor += 19;
      if ((fracSec != 0 && fractionFormat != TimeFractionFormat::Truncate)
          || fractionFormat == TimeFractionFormat::AllDigits)
      {
        // Append fractional second, which is a 7-digit value with no trailing zeros
        // This way, '1200' becomes '00012'
        size_t appended =
#ifdef _MSC_VER
            sprintf_s(
#else
            std::sprintf(
#endif
                outCursor,
#ifdef _MSC_VER
                9,
#endif
                ".%07d",
                fracSec);

        if (fractionFormat != TimeFractionFormat::AllDigits)
        {
          while (outCursor[appended - 1] == '0')
          {
            --appended; // trim trailing zeros
          }
        }
        outCursor += appended;
      }

      *outCursor = 'Z';
      ++outCursor;
      return std::string(outBuffer, outCursor);

    default:
      throw std::invalid_argument("Unrecognized date format.");
  }
}

namespace {
constexpr bool StringStartsWith(char const* haystack, char const* needle)
{
  while (*needle)
  {
    if (*haystack != *needle)
    {
      return false;
    }

    ++haystack;
    ++needle;
  }

  return true;
}

template <int N = 9> constexpr bool IsDigit(char c)
{
  return ((unsigned char)((unsigned char)(c) - '0') <= N);
}

constexpr int StringToDoubleDigitInt(char const* str)
{
  return (static_cast<unsigned char>(str[0]) - '0') * 10
      + (static_cast<unsigned char>(str[1]) - '0');
}

constexpr bool ValidateDay(int day, int month, int year)
{
  if (day < 1)
  {
    return false;
  }

  // Month is 0-based
  switch (month)
  {
    case 1: // Feb
      return day <= (28 + IsLeapYear(year));
      break;
    case 3: // Apr
    case 5: // Jun
    case 8: // Sep
    case 10: // Nov
      return day <= 30;
      break;
    default:
      return day <= 31;
  }
}

constexpr int GetYearDay(int month, int monthDay, int year)
{
  return CumulativeDaysToMonth[month] + monthDay + (IsLeapYear(year) && month > 1) - 1;
}

constexpr int CountLeapYears(int yearsSince1601)
{
  int tmpYears = yearsSince1601;

  int const year400 = tmpYears / 400;
  tmpYears -= year400 * 400;

  int result = year400 * 97;

  int const year100 = tmpYears / 100;
  tmpYears -= year100 * 100;
  result += year100 * 24;

  result += tmpYears / 4;

  return result;
}

constexpr int64_t AdjustTimezone(
    int64_t result,
    unsigned char chSign,
    int adjustHours,
    int adjustMinutes)
{
  if (adjustHours > 23)
  {
    return -1;
  }

  // adjustMinutes > 59 is impossible due to digit 5 check
  int const tzAdjust = adjustMinutes * 60 + adjustHours * 60 * 60;
  if (chSign == '-')
  {
    if (std::numeric_limits<int64_t>::max() - result < tzAdjust)
    {
      return -1;
    }

    result += tzAdjust;
  }
  else
  {
    if (tzAdjust > result)
    {
      return -1;
    }

    result -= tzAdjust;
  }

  return result;
}
} // namespace

/*
https://tools.ietf.org/html/rfc822
https://tools.ietf.org/html/rfc1123

date-time   =  [ day "," ] date time        ; dd mm yy
                                            ;  hh:mm:ss zzz

day         =  "Mon"  / "Tue" /  "Wed"  / "Thu"
            /  "Fri"  / "Sat" /  "Sun"

date        =  1*2DIGIT month 2DIGIT        ; day month year
                                            ;  e.g. 20 Jun 82
RFC1123 changes this to:
date        =  1*2DIGIT month 2*4DIGIT        ; day month year
                                              ;  e.g. 20 Jun 1982
This implementation only accepts 4 digit years.

month       =  "Jan"  /  "Feb" /  "Mar"  /  "Apr"
            /  "May"  /  "Jun" /  "Jul"  /  "Aug"
            /  "Sep"  /  "Oct" /  "Nov"  /  "Dec"

time        =  hour zone                    ; ANSI and Military

hour        =  2DIGIT ":" 2DIGIT [":" 2DIGIT]
                                            ; 00:00:00 - 23:59:59

zone        =  "UT"  / "GMT"                ; Universal Time
                                            ; North American : UT
            /  "EST" / "EDT"                ;  Eastern:  - 5/ - 4
            /  "CST" / "CDT"                ;  Central:  - 6/ - 5
            /  "MST" / "MDT"                ;  Mountain: - 7/ - 6
            /  "PST" / "PDT"                ;  Pacific:  - 8/ - 7

// military time deleted by RFC 1123

            / ( ("+" / "-") 4DIGIT )        ; Local differential
                                            ;  hours+min. (HHMM)
*/

DateTime DateTime::Parse(std::string const& dateString, DateFormat format)
{
  int64_t secondsSince1601 = 0;
  uint64_t fracSec = 0;

  auto str = dateString.c_str();
  if (format == DateFormat::Rfc1123)
  {
    int parsedWeekday = 0;
    for (; parsedWeekday < 7; ++parsedWeekday)
    {
      if (StringStartsWith(str, dayNames + static_cast<uint64_t>(parsedWeekday) * 4ULL)
          && str[3] == ',' && str[4] == ' ')
      {
        str += 5; // parsed day of week
        break;
      }
    }

    int monthDay;
    if (IsDigit<3>(str[0]) && IsDigit(str[1]) && str[2] == ' ')
    {
      monthDay = StringToDoubleDigitInt(str); // validity checked later
      str += 3; // parsed day
    }
    else if (IsDigit(str[0]) && str[1] == ' ')
    {
      monthDay = str[0] - '0';
      str += 2; // parsed day
    }
    else
    {
      throw std::invalid_argument("Error parsing DateTime: Day of the month.");
    }

    if (monthDay == 0)
    {
      throw std::invalid_argument("Error parsing DateTime: Invalid month day number (0).");
    }

    int month = 0;
    for (;;)
    {
      if (StringStartsWith(str, monthNames + static_cast<uint64_t>(month) * 4ULL))
      {
        break;
      }

      ++month;
      if (month == 12)
      {
        throw std::invalid_argument("Error parsing DateTime: Month number.");
      }
    }

    if (str[3] != ' ')
    {
      throw std::invalid_argument("Error parsing DateTime.");
    }

    str += 4; // parsed month

    if (!IsDigit(str[0]) || !IsDigit(str[1]) || !IsDigit(str[2]) || !IsDigit(str[3])
        || str[4] != ' ')
    {
      throw std::invalid_argument("Error parsing DateTime: Year.");
    }

    int year = (str[0] - '0') * 1000 + (str[1] - '0') * 100 + (str[2] - '0') * 10 + (str[3] - '0');
    if (year < MinYear)
    {
      throw std::invalid_argument("Error parsing DateTime: Year is less than 1601.");
    }

    // days in month validity check
    if (!ValidateDay(monthDay, month, year))
    {
      throw std::invalid_argument("Error parsing DateTime: Invalid day in month.");
    }

    str += 5; // parsed year
    int const yearDay = GetYearDay(month, monthDay, year);

    if (!IsDigit<2>(str[0]) || !IsDigit(str[1]) || str[2] != ':' || !IsDigit<5>(str[3])
        || !IsDigit(str[4]))
    {
      throw std::invalid_argument("Error parsing DateTime: Hour and minutes.");
    }

    int const hour = StringToDoubleDigitInt(str);
    if (hour > 23)
    {
      throw std::invalid_argument("Error parsing DateTime: hour > 23.");
    }
    str += 3; // parsed hour

    int const minute = StringToDoubleDigitInt(str);
    str += 2; // parsed mins

    int sec = 0;
    if (str[0] == ':')
    {
      if (!IsDigit<6>(str[1]) || !IsDigit(str[2]) || str[3] != ' ')
      {
        throw std::invalid_argument("Error parsing DateTime.");
      }

      sec = StringToDoubleDigitInt(str + 1);
      str += 4; // parsed seconds
    }
    else if (str[0] == ' ')
    {
      str += 1; // parsed seconds
    }
    else
    {
      throw std::invalid_argument("Error parsing DateTime.");
    }

    if (sec > 60)
    { // 60 to allow leap seconds
      throw std::invalid_argument("Error parsing DateTime: Seconds > 60.");
    }

    year -= MinYear;
    int const daysSince1601 = year * DaysInYear + CountLeapYears(year) + yearDay;

    if (parsedWeekday != 7)
    {
      int const actualWeekday = (daysSince1601 + 1) % 7;
      if (parsedWeekday != actualWeekday)
      {
        throw std::invalid_argument("Error parsing DateTime: Weekday.");
      }
    }

    secondsSince1601 = static_cast<int64_t>(daysSince1601) * SecondsInDay
        + static_cast<int64_t>(hour) * SecondsInHour
        + static_cast<int64_t>(minute) * SecondsInMinute + sec;

    fracSec = 0;
    if (!StringStartsWith(str, "GMT") && !StringStartsWith(str, "UT"))
    {
      // some timezone adjustment necessary
      auto tzCh = '-';
      int tzHours;
      int tzMinutes = 0;
      if (StringStartsWith(str, "EDT"))
      {
        tzHours = 4;
      }
      else if (StringStartsWith(str, "EST") || StringStartsWith(str, "CDT"))
      {
        tzHours = 5;
      }
      else if (StringStartsWith(str, "CST") || StringStartsWith(str, "MDT"))
      {
        tzHours = 6;
      }
      else if (StringStartsWith(str, "MST") || StringStartsWith(str, "PDT"))
      {
        tzHours = 7;
      }
      else if (StringStartsWith(str, "PST"))
      {
        tzHours = 8;
      }
      else if (
          (str[0] == '+' || str[0] == '-') && IsDigit<2>(str[1]) && IsDigit(str[2])
          && IsDigit<5>(str[3]) && IsDigit(str[4]))
      {
        tzCh = str[0];
        tzHours = StringToDoubleDigitInt(str + 1);
        tzMinutes = StringToDoubleDigitInt(str + 3);
      }
      else
      {
        throw std::invalid_argument("Error parsing DateTime: Time zone.");
      }

      secondsSince1601
          = AdjustTimezone(secondsSince1601, static_cast<unsigned char>(tzCh), tzHours, tzMinutes);

      if (secondsSince1601 < 0)
      {
        throw std::invalid_argument(
            "Error parsing DateTime: year is < 1601 after time zone adjustments.");
      }
    }
  }
  else if (format == DateFormat::Rfc3339)
  {
    // parse year
    if (!IsDigit(str[0]) || !IsDigit(str[1]) || !IsDigit(str[2]) || !IsDigit(str[3]))
    {
      throw std::invalid_argument("Error parsing DateTime: Year.");
    }

    int year = (str[0] - '0') * 1000 + (str[1] - '0') * 100 + (str[2] - '0') * 10 + (str[3] - '0');
    if (year < MinYear)
    {
      throw std::invalid_argument("Error parsing DateTime: Year < 1601.");
    }

    str += 4;
    if (*str == '-')
    {
      ++str;
    }

    // parse month
    if (!IsDigit<1>(str[0]) || !IsDigit(str[1]))
    {
      throw std::invalid_argument("Error parsing DateTime: Month number.");
    }

    int month = StringToDoubleDigitInt(str);
    if (month < 1 || month > 12)
    {
      throw std::invalid_argument("Error parsing DateTime: Invalid month number.");
    }

    month -= 1;
    str += 2;

    if (*str == '-')
    {
      ++str;
    }

    // parse day
    if (!IsDigit<3>(str[0]) || !IsDigit(str[1]))
    {
      throw std::invalid_argument("Error parsing DateTime: Day.");
    }

    int monthDay = StringToDoubleDigitInt(str);
    if (!ValidateDay(monthDay, month, year))
    {
      throw std::invalid_argument("Error parsing DateTime: Day of the month.");
    }

    int const yearDay = GetYearDay(month, monthDay, year);

    str += 2;
    year -= MinYear;
    int daysSince1601 = year * DaysInYear + CountLeapYears(year) + yearDay;

    if (str[0] != 'T' && str[0] != 't')
    {
      // No time
      secondsSince1601 = static_cast<int64_t>(daysSince1601) * SecondsInDay;
      return DateTime(std::chrono::seconds(secondsSince1601) + DateTime::Duration(fracSec));
    }

    ++str; // skip 'T'

    // parse hour
    if (!IsDigit<2>(str[0]) || !IsDigit(str[1]))
    {
      throw std::invalid_argument("Error parsing DateTime: Hour.");
    }

    int const hour = StringToDoubleDigitInt(str);
    str += 2;
    if (hour > 23)
    {
      throw std::invalid_argument("Error parsing DateTime: Invalid hour number.");
    }

    if (*str == ':')
    {
      ++str;
    }

    // parse minute
    if (!IsDigit<5>(str[0]) || !IsDigit(str[1]))
    {
      throw std::invalid_argument("Error parsing DateTime: Minute.");
    }

    int const minute = StringToDoubleDigitInt(str);
    // minute > 59 is impossible because we checked that the first digit is <= 5 in the basic format
    // check above

    str += 2;

    if (*str == ':')
    {
      ++str;
    }

    // parse seconds
    if (!IsDigit<6>(str[0]) || !IsDigit(str[1]))
    {
      throw std::invalid_argument("Error parsing DateTime: Seconds.");
    }

    int const sec = StringToDoubleDigitInt(str);
    // We allow 60 to account for leap seconds
    if (sec > 60)
    {
      throw std::invalid_argument("Error parsing DateTime: Invalid seconds number.");
    }

    str += 2;
    if (str[0] == '.' && IsDigit(str[1]))
    {
      ++str;
      int digits = 7;
      for (;;)
      {
        fracSec *= 10;
        fracSec += static_cast<uint64_t>(*str) - static_cast<uint64_t>('0');

        --digits;
        ++str;

        if (digits == 0)
        {
          while (IsDigit(*str))
          {
            // consume remaining fractional second digits we can't use
            ++str;
          }

          break;
        }

        if (!IsDigit(*str))
        {
          // no more digits in the input, do the remaining multiplies we need
          for (; digits != 0; --digits)
          {
            fracSec *= 10;
          }

          break;
        }
      }
    }

    secondsSince1601 = static_cast<int64_t>(daysSince1601) * SecondsInDay
        + static_cast<int64_t>(hour) * SecondsInHour
        + static_cast<int64_t>(minute) * SecondsInMinute + sec;

    if (str[0] == 'Z' || str[0] == 'z')
    {
      // no adjustment needed for zulu time
    }
    else if (str[0] == '+' || str[0] == '-')
    {
      unsigned char const offsetDirection = static_cast<unsigned char>(str[0]);
      if (!IsDigit<2>(str[1]) || !IsDigit(str[2]) || str[3] != ':' || !IsDigit<5>(str[4])
          || !IsDigit(str[5]))
      {
        throw std::invalid_argument("Error parsing DateTime.");
      }

      secondsSince1601 = AdjustTimezone(
          secondsSince1601,
          offsetDirection,
          StringToDoubleDigitInt(str + 1),
          StringToDoubleDigitInt(str + 4));
      if (secondsSince1601 < 0)
      {
        throw std::invalid_argument("Error parsing DateTime.");
      }
    }
    else
    {
      // the timezone is malformed, but cpprestsdk currently accepts this as no timezone
    }
  }
  else
  {
    throw std::invalid_argument("Unrecognized date format.");
  }

  return DateTime(std::chrono::seconds(secondsSince1601) + DateTime::Duration(fracSec));
}

DateTime::DateTime(
    int16_t year,
    int8_t month,
    int8_t day,
    int8_t hour,
    int8_t minute,
    int8_t second)
{
  // We should combine creation/validation logic, so it is reusable from both parsing functions and
  // from this constructor
  if (year > MaxYear || year < MinYear)
  {
    throw std::invalid_argument(
        year > MaxYear ? "The requested year exceeds the year 9999."
                       : "The requested year is less than the year 1601.");
  }

  if (month <= 0 || month > 12)
  {
    throw std::invalid_argument("Invalid month value.");
  }

  if (day <= 0 || day > 31)
  {
    throw std::invalid_argument("Invalid day value.");
  }

  if (hour < 0 || hour > 23)
  {
    throw std::invalid_argument("Invalid hour value.");
  }

  if (minute < 0 || minute > 59)
  {
    throw std::invalid_argument("Invalid minute value.");
  }

  if (second < 0 || second > 60)
  {
    throw std::invalid_argument("Invalid seconds value.");
  }

  if (!ValidateDay(day, month - 1, year))
  {
    throw std::invalid_argument("Invalid day of the month.");
  }

  char outBuffer[38]{}; // Thu, 01 Jan 1970 00:00:00 GMT\0
                        // 1970-01-01T00:00:00.1234567Z\0

  char* outCursor = outBuffer;
#ifdef _MSC_VER
  sprintf_s(
#else
  std::sprintf(
#endif
      outCursor,
#ifdef _MSC_VER
      20,
#endif
      "%04d-%02d-%02dT%02d:%02d:%02d",
      year,
      month,
      day,
      hour,
      minute,
      second);

  outCursor += 19;

  *outCursor = 'Z';
  ++outCursor;
  auto const dt = DateTime::Parse(std::string(outBuffer, outCursor), DateFormat::Rfc3339);
  m_since1601 = dt.m_since1601;
}
