// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include <azure/core/datetime.hpp>

#include <chrono>
#include <ctime>
#include <limits>

using namespace Azure;

// cspell:words AGMT, CEST

TEST(DateTime, ParseDateAndTimeBasic)
{
  auto dt1 = DateTime::Parse("20130517T00:00:00Z", DateTime::DateFormat::Rfc3339);
  auto dt2 = DateTime::Parse("Fri, 17 May 2013 00:00:00 GMT", DateTime::DateFormat::Rfc1123);

  EXPECT_NE(0, dt2.time_since_epoch().count());
  EXPECT_EQ(dt1, dt2);
}

TEST(DateTime, ParseDateAndTimeExtended)
{
  auto dt1 = DateTime::Parse("2013-05-17T00:00:00Z", DateTime::DateFormat::Rfc3339);
  EXPECT_NE(0, dt1.time_since_epoch().count());

  auto dt2 = DateTime::Parse("Fri, 17 May 2013 00:00:00 GMT", DateTime::DateFormat::Rfc1123);
  EXPECT_NE(0, dt2.time_since_epoch().count());
  EXPECT_EQ(dt1, dt2);
}

TEST(DateTime, ParseDateBasic)
{
  {
    auto dt = DateTime::Parse("20130517", DateTime::DateFormat::Rfc3339);
    EXPECT_NE(0, dt.time_since_epoch().count());
    EXPECT_EQ(dt.ToString(), "2013-05-17T00:00:00Z");
  }
}

TEST(DateTime, ParseDateExtended)
{
  auto dt = DateTime::Parse("2013-05-17", DateTime::DateFormat::Rfc3339);
  EXPECT_NE(0, dt.time_since_epoch().count());
}

namespace {
template <DateTime::TimeFractionFormat TF = DateTime::TimeFractionFormat::DropTrailingZeros>
void TestDateTimeRoundtrip(std::string const& str, std::string const& strExpected)
{
  auto dt = DateTime::Parse(str, DateTime::DateFormat::Rfc3339);
  auto const str2 = dt.ToString(DateTime::DateFormat::Rfc3339, TF);
  EXPECT_EQ(str2, strExpected);
}

template <DateTime::TimeFractionFormat TF = DateTime::TimeFractionFormat::DropTrailingZeros>
void TestDateTimeRoundtrip(std::string const& str)
{
  TestDateTimeRoundtrip<TF>(str, str);
}
} // namespace

TEST(DateTime, ParseTimeRoundrip1)
{
  // Preserve all 7 digits after the comma:
  TestDateTimeRoundtrip("2013-11-19T14:30:59.1234567Z");
}

TEST(DateTime, ParseTimeRoundrip2)
{
  // lose the last '000'
  TestDateTimeRoundtrip("2013-11-19T14:30:59.1234567000Z", "2013-11-19T14:30:59.1234567Z");

  // Round up
  TestDateTimeRoundtrip("2013-11-19T14:30:59.123456650Z", "2013-11-19T14:30:59.1234567Z");

  // Round up
  TestDateTimeRoundtrip("2013-11-19T14:30:59.999999950Z", "2013-11-19T14:31:00Z");

  // Round down
  TestDateTimeRoundtrip("2013-11-19T14:30:59.123456749Z", "2013-11-19T14:30:59.1234567Z");
}

TEST(DateTime, decimals)
{
  {
    std::string strExpected("2020-10-13T21:06:15.3300000Z");
    auto dt = DateTime::Parse("2020-10-13T21:06:15.33Z", DateTime::DateFormat::Rfc3339);
    auto const str2
        = dt.ToString(DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    EXPECT_EQ(str2, strExpected);
  }

  {
    std::string strExpected("2020-10-13T21:06:15.0000000Z");
    auto dt = DateTime::Parse("2020-10-13T21:06:15Z", DateTime::DateFormat::Rfc3339);
    auto const str2
        = dt.ToString(DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    EXPECT_EQ(str2, strExpected);
  }

  {
    std::string strExpected("2020-10-13T21:06:15.1234500Z");
    auto dt = DateTime::Parse("2020-10-13T21:06:15.12345Z", DateTime::DateFormat::Rfc3339);
    auto const str2
        = dt.ToString(DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits);
    EXPECT_EQ(str2, strExpected);
  }
}

TEST(DateTime, noDecimals)
{
  {
    std::string strExpected("2020-10-13T21:06:15Z");
    auto dt = DateTime::Parse("2020-10-13T21:06:15Z", DateTime::DateFormat::Rfc3339);
    auto const str2
        = dt.ToString(DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::Truncate);
    EXPECT_EQ(str2, strExpected);
  }

  {
    std::string strExpected("2020-10-13T21:06:15Z");
    auto dt = DateTime::Parse("2020-10-13T21:06:15.99999Z", DateTime::DateFormat::Rfc3339);
    auto const str2
        = dt.ToString(DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::Truncate);
    EXPECT_EQ(str2, strExpected);
  }
}

TEST(DateTime, sameResultFromDefaultRfc3339)
{
  {
    auto dt = DateTime::Parse("2020-10-13T21:06:15.33000000Z", DateTime::DateFormat::Rfc3339);
    auto dt2 = DateTime::Parse("2020-10-13T21:06:15.330000000Z", DateTime::DateFormat::Rfc3339);
    auto const str1 = dt.ToString(
        DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::DropTrailingZeros);
    auto const str2 = dt2.ToString(DateTime::DateFormat::Rfc3339);
    EXPECT_EQ(str1, str2);
    EXPECT_EQ(str1, dt2.ToString());
  }
}

TEST(DateTime, ParseTimeRoundrip3)
{
  // leading 0-s after the comma, tricky to parse correctly
  TestDateTimeRoundtrip("2013-11-19T14:30:59.00123Z");
}

TEST(DateTime, ParseTimeRoundrip4)
{
  // another leading 0 test
  TestDateTimeRoundtrip("2013-11-19T14:30:59.0000001Z");
}

TEST(DateTime, ParseTimeRoundrip5)
{
  // this is going to be truncated
  TestDateTimeRoundtrip("2013-11-19T14:30:59.00000001Z", "2013-11-19T14:30:59Z");
}

TEST(DateTime, ParseTimeRoundrip6)
{
  // Only one digit after the dot
  TestDateTimeRoundtrip("2013-11-19T14:30:59.5Z");
}

TEST(DateTime, ParseTimeRoundripYear0001) { TestDateTimeRoundtrip("0001-01-01T00:00:00Z"); }

TEST(DateTime, ParseTimeRoundripYear9999) { TestDateTimeRoundtrip("9999-12-31T23:59:59.9999999Z"); }

TEST(DateTime, EmittingTimeCorrectDay)
{
  DateTime const test
      = DateTime() + std::chrono::seconds(63691573964LL); // 2019-04-22T23:52:44 is a Monday
  auto const actual = test.ToString(DateTime::DateFormat::Rfc1123);
  std::string const expected("Mon");
  EXPECT_EQ(actual.substr(0, 3), expected);
}

namespace {
void TestRfc1123IsTimeT(char const* str, int64_t t)
{
  auto const dt = DateTime::Parse(str, DateTime::DateFormat::Rfc1123);
  int64_t interval = dt.time_since_epoch().count();

  EXPECT_EQ(0, interval % 10000000);
  interval /= 10000000;
  interval -= 11644473600; // NT epoch adjustment
  interval -= 50491123200; // Diff between NT epoch and year 0001
  EXPECT_EQ(t, interval);
}
} // namespace

TEST(DateTime, ParseTimeRfc1123AcceptsEachDay)
{
  TestRfc1123IsTimeT("1 Jan 1970 00:00:00 GMT", 0);
  TestRfc1123IsTimeT("01 Jan 1970 00:00:00 GMT", 0);
  TestRfc1123IsTimeT("Fri, 2 Jan 1970 00:00:00 GMT", 86400 * 1);
  TestRfc1123IsTimeT("Fri, 02 Jan 1970 00:00:00 GMT", 86400 * 1);
  TestRfc1123IsTimeT("Sat, 03 Jan 1970 00:00:00 GMT", 86400 * 2);
  TestRfc1123IsTimeT("Sun, 04 Jan 1970 00:00:00 GMT", 86400 * 3);
  TestRfc1123IsTimeT("Mon, 05 Jan 1970 00:00:00 GMT", 86400 * 4);
  TestRfc1123IsTimeT("Tue, 06 Jan 1970 00:00:00 GMT", 86400 * 5);
  TestRfc1123IsTimeT("Wed, 07 Jan 1970 00:00:00 GMT", 86400 * 6);
}

TEST(DateTime, ParseTimeRfc1123BoundaryCases)
{
  TestRfc1123IsTimeT("01 Jan 1970 00:00:00 GMT", 0);
  TestRfc1123IsTimeT(
      "19 Jan 2038 03:14:06 GMT", static_cast<int64_t>(std::numeric_limits<int>::max()) - 1);
  TestRfc1123IsTimeT("19 Jan 2038 03:13:07 -0001", std::numeric_limits<int>::max());
  TestRfc1123IsTimeT("19 Jan 2038 03:14:07 -0000", std::numeric_limits<int>::max());
  TestRfc1123IsTimeT("14 Jan 2019 23:16:21 +0000", 1547507781);
  TestRfc1123IsTimeT("14 Jan 2019 23:16:21 -0001", 1547507841);
  TestRfc1123IsTimeT("14 Jan 2019 23:16:21 +0001", 1547507721);
  TestRfc1123IsTimeT("14 Jan 2019 23:16:21 -0100", 1547511381);
  TestRfc1123IsTimeT("14 Jan 2019 23:16:21 +0100", 1547504181);
}

TEST(DateTime, ParseTimeRfc1123UseEachField)
{
  TestRfc1123IsTimeT("02 Jan 1970 00:00:00 GMT", 86400);
  TestRfc1123IsTimeT("12 Jan 1970 00:00:00 GMT", 950400);
  TestRfc1123IsTimeT("01 Feb 1970 00:00:00 GMT", 2678400);
  TestRfc1123IsTimeT("01 Jan 2000 00:00:00 GMT", 946684800);
  TestRfc1123IsTimeT("01 Jan 2100 00:00:00 GMT", 4102444800);
  TestRfc1123IsTimeT("01 Jan 1990 00:00:00 GMT", 631152000);
  TestRfc1123IsTimeT("01 Jan 1971 00:00:00 GMT", 31536000);
  TestRfc1123IsTimeT("01 Jan 1970 10:00:00 GMT", 36000);
  TestRfc1123IsTimeT("01 Jan 1970 01:00:00 GMT", 3600);
  TestRfc1123IsTimeT("01 Jan 1970 00:10:00 GMT", 600);
  TestRfc1123IsTimeT("01 Jan 1970 00:01:00 GMT", 60);
  TestRfc1123IsTimeT("01 Jan 1970 00:00:10 GMT", 10);
  TestRfc1123IsTimeT("01 Jan 1970 00:00:01 GMT", 1);
  TestRfc1123IsTimeT("01 Jan 1970 10:00:00 GMT", 36000);
  TestRfc1123IsTimeT("01 Jan 1970 02:00:00 PST", 36000);
  TestRfc1123IsTimeT("01 Jan 1970 03:00:00 PDT", 36000);
  TestRfc1123IsTimeT("01 Jan 1970 03:00:00 MST", 36000);
  TestRfc1123IsTimeT("01 Jan 1970 04:00:00 MDT", 36000);
  TestRfc1123IsTimeT("01 Jan 1970 04:00:00 CST", 36000);
  TestRfc1123IsTimeT("01 Jan 1970 05:00:00 CDT", 36000);
  TestRfc1123IsTimeT("01 Jan 1970 05:00:00 EST", 36000);
  TestRfc1123IsTimeT("01 Jan 1970 06:00:00 EDT", 36000);
  TestRfc1123IsTimeT("01 Jan 1970 06:00:00 -0400", 36000);
  TestRfc1123IsTimeT("01 Jan 1970 05:59:00 -0401", 36000);
}

TEST(DateTime, ParseTimeRfc1123MaxDays)
{
  TestRfc1123IsTimeT("31 Jan 1970 00:00:00 GMT", 2592000);
  TestRfc1123IsTimeT("28 Feb 2019 00:00:00 GMT", 1551312000); // non leap year allows feb 28
  TestRfc1123IsTimeT("29 Feb 2020 00:00:00 GMT", 1582934400); // leap year allows feb 29
  TestRfc1123IsTimeT("31 Mar 1970 00:00:00 GMT", 7689600);
  TestRfc1123IsTimeT("30 Apr 1970 00:00:00 GMT", 10281600);
  TestRfc1123IsTimeT("31 May 1970 00:00:00 GMT", 12960000);
  TestRfc1123IsTimeT("30 Jun 1970 00:00:00 GMT", 15552000);
  TestRfc1123IsTimeT("31 Jul 1970 00:00:00 GMT", 18230400);
  TestRfc1123IsTimeT("31 Aug 1970 00:00:00 GMT", 20908800);
  TestRfc1123IsTimeT("30 Sep 1970 00:00:00 GMT", 23500800);
  TestRfc1123IsTimeT("31 Oct 1970 00:00:00 GMT", 26179200);
  TestRfc1123IsTimeT("30 Nov 1970 00:00:00 GMT", 28771200);
  TestRfc1123IsTimeT("31 Dec 1970 00:00:00 GMT", 31449600);
}

TEST(DateTime, ParseTimeRfc1123InvalidCases)
{
  std::string const badStrings[] = {
      "Ahu, 01 Jan 1970 00:00:00 GMT", // bad letters in each place
      "TAu, 01 Jan 1970 00:00:00 GMT",
      "ThA, 01 Jan 1970 00:00:00 GMT",
      "ThuA 01 Jan 1970 00:00:00 GMT",
      "Thu,A01 Jan 1970 00:00:00 GMT",
      "Thu, A1 Jan 1970 00:00:00 GMT",
      "Thu, 0A Jan 1970 00:00:00 GMT",
      "Thu, 01AJan 1970 00:00:00 GMT",
      "Thu, 01 Aan 1970 00:00:00 GMT",
      "Thu, 01 JAn 1970 00:00:00 GMT",
      "Thu, 01 JaA 1970 00:00:00 GMT",
      "Thu, 01 JanA1970 00:00:00 GMT",
      "Thu, 01 Jan A970 00:00:00 GMT",
      "Thu, 01 Jan 1A70 00:00:00 GMT",
      "Thu, 01 Jan 19A0 00:00:00 GMT",
      "Thu, 01 Jan 197A 00:00:00 GMT",
      "Thu, 01 Jan 1970A00:00:00 GMT",
      "Thu, 01 Jan 1970 A0:00:00 GMT",
      "Thu, 01 Jan 1970 0A:00:00 GMT",
      "Thu, 01 Jan 1970 00A00:00 GMT",
      "Thu, 01 Jan 1970 00:A0:00 GMT",
      "Thu, 01 Jan 1970 00:0A:00 GMT",
      "Thu, 01 Jan 1970 00:00A00 GMT",
      "Thu, 01 Jan 1970 00:00:A0 GMT",
      "Thu, 01 Jan 1970 00:00:0A GMT",
      "Thu, 01 Jan 1970 00:00:00AGMT",
      "Thu, 01 Jan 1970 00:00:00 AMT",
      "Thu, 01 Jan 1970 00:00:00 GAT",
      "Thu, 01 Jan 1970 00:00:00 GMA",
      "", // truncation
      "T",
      "Th",
      "Thu",
      "Thu,",
      "Thu, ",
      "Thu, 0",
      "Thu, 01",
      "Thu, 01 ",
      "Thu, 01 J",
      "Thu, 01 Ja",
      "Thu, 01 Jan",
      "Thu, 01 Jan ",
      "Thu, 01 Jan 1",
      "Thu, 01 Jan 19",
      "Thu, 01 Jan 197",
      "Thu, 01 Jan 1970",
      "Thu, 01 Jan 1970 ",
      "Thu, 01 Jan 1970 0",
      "Thu, 01 Jan 1970 00",
      "Thu, 01 Jan 1970 00:",
      "Thu, 01 Jan 1970 00:0",
      "Thu, 01 Jan 1970 00:00",
      "Thu, 01 Jan 1970 00:00:",
      "Thu, 01 Jan 1970 00:00:0",
      "Thu, 01 Jan 1970 00:00:00",
      "Thu, 01 Jan 1970 00:00:00 ",
      "Thu, 01 Jan 1970 00:00:00 G",
      "Thu, 01 Jan 1970 00:00:00 GM",
      "Fri, 01 Jan 1970 00:00:00 GMT", // wrong day
      "01 Jan 0000 00:00:00 GMT", // year too small
      "01 Xxx 1971 00:00:00 GMT", // month bad
      "00 Jan 1971 00:00:00 GMT", // day too small
      "32 Jan 1971 00:00:00 GMT", // day too big
      "30 Feb 1971 00:00:00 GMT", // day too big for feb
      "30 Feb 1971 00:00:00 GMT", // day too big for feb (non-leap year)
      "32 Mar 1971 00:00:00 GMT", // other months
      "31 Apr 1971 00:00:00 GMT",
      "32 May 1971 00:00:00 GMT",
      "31 Jun 1971 00:00:00 GMT",
      "32 Jul 1971 00:00:00 GMT",
      "32 Aug 1971 00:00:00 GMT",
      "31 Sep 1971 00:00:00 GMT",
      "32 Oct 1971 00:00:00 GMT",
      "31 Nov 1971 00:00:00 GMT",
      "32 Dec 1971 00:00:00 GMT",
      "01 Jan 1971 70:00:00 GMT", // hour too big
      "01 Jan 1971 24:00:00 GMT",
      "01 Jan 1971 00:60:00 GMT", // minute too big
      "01 Jan 1971 00:00:70 GMT", // second too big
      "01 Jan 1971 00:00:61 GMT",
      "01 Jan 1969 00:00:00 CEST", // bad tz
      "14 Jan 2019 23:16:21 G0100", // bad tz offsets
      //"01 Jan 1970 00:00:00 +2400",
      //"01 Jan 1970 00:00:00 -3000",
      "01 Jan 1970 00:00:00 +2160",
      //"01 Jan 1970 00:00:00 -2400",
      "01 Jan 1970 00:00:00 -2160",
      "00 Jan 1971 00:00:00 GMT", // zero month day
  };

  for (auto const& str : badStrings)
  {
    EXPECT_THROW(DateTime::Parse(str, DateTime::DateFormat::Rfc1123), std::invalid_argument);
  }
}

TEST(DateTime, ParseTimeRfc3339BoundaryCases)
{
  // boundary cases:
  TestDateTimeRoundtrip("1970-01-01T00:00:00Z"); // epoch
  TestDateTimeRoundtrip("2038-01-19T03:14:06+00:00", "2038-01-19T03:14:06Z"); // INT_MAX - 1
  TestDateTimeRoundtrip(
      "2038-01-19T03:13:07-00:01",
      "2038-01-19T03:14:07Z"); // INT_MAX after subtacting 1
  TestDateTimeRoundtrip("2038-01-19T03:14:07-00:00", "2038-01-19T03:14:07Z");

  // No ':' in time zone offset
  EXPECT_THROW(
      DateTime::Parse("2001-01-01T00:00:00+12345", DateTime::DateFormat::Rfc3339),
      std::invalid_argument);
}

TEST(DateTime, ParseUnrecognizedFormat)
{
  EXPECT_THROW(
      DateTime::Parse("2001-01-01T00:00:00", static_cast<DateTime::DateFormat>(42)),
      std::invalid_argument);
}

TEST(DateTime, ParseTimeRfc3339UsesEachTimezoneDigit)
{
  TestDateTimeRoundtrip("2019-01-14T23:16:21+00:00", "2019-01-14T23:16:21Z");
  TestDateTimeRoundtrip("2019-01-14T23:16:21-00:01", "2019-01-14T23:17:21Z");
  TestDateTimeRoundtrip("2019-01-14T23:16:21+00:01", "2019-01-14T23:15:21Z");
  TestDateTimeRoundtrip("2019-01-14T23:16:21-01:00", "2019-01-15T00:16:21Z");
  TestDateTimeRoundtrip("2019-01-14T23:16:21+01:00", "2019-01-14T22:16:21Z");
}

TEST(DateTime, ParseTimeRfc3339UsesEachDigit)
{
  TestDateTimeRoundtrip("1970-01-01T00:00:01Z");
  TestDateTimeRoundtrip("1970-01-01T00:01:00Z");
  TestDateTimeRoundtrip("1970-01-01T01:00:00Z");
  TestDateTimeRoundtrip("1970-01-02T00:00:00Z");
  TestDateTimeRoundtrip("1970-02-01T00:00:00Z");
  TestDateTimeRoundtrip("1971-01-01T00:00:00Z");

  TestDateTimeRoundtrip("1999-01-01T00:00:00Z");
  TestDateTimeRoundtrip("1970-12-01T00:00:00Z");
  TestDateTimeRoundtrip("1970-09-01T00:00:00Z");
  TestDateTimeRoundtrip("1970-01-30T00:00:00Z");
  TestDateTimeRoundtrip("1970-01-31T00:00:00Z");
  TestDateTimeRoundtrip("1970-01-01T23:00:00Z");
  TestDateTimeRoundtrip("1970-01-01T19:00:00Z");
  TestDateTimeRoundtrip("1970-01-01T00:59:00Z");
  TestDateTimeRoundtrip("1970-01-01T00:00:59Z");
  TestDateTimeRoundtrip("1970-01-01T00:00:60Z", "1970-01-01T00:01:00Z"); // leap seconds
}

TEST(DateTime, ParseTimeRfc3339AcceptsMonthMaxDays)
{
  TestDateTimeRoundtrip("1970-01-31T00:00:00Z"); // jan
  TestDateTimeRoundtrip("2019-02-28T00:00:00Z"); // non leap year allows feb 28
  TestDateTimeRoundtrip("2020-02-29T00:00:00Z"); // leap year allows feb 29
  TestDateTimeRoundtrip("1970-03-31T00:00:00Z"); // mar
  TestDateTimeRoundtrip("1970-04-30T00:00:00Z"); // apr
  TestDateTimeRoundtrip("1970-05-31T00:00:00Z"); // may
  TestDateTimeRoundtrip("1970-06-30T00:00:00Z"); // jun
  TestDateTimeRoundtrip("1970-07-31T00:00:00Z"); // jul
  TestDateTimeRoundtrip("1970-08-31T00:00:00Z"); // aug
  TestDateTimeRoundtrip("1970-09-30T00:00:00Z"); // sep
  TestDateTimeRoundtrip("1970-10-31T00:00:00Z"); // oct
  TestDateTimeRoundtrip("1970-11-30T00:00:00Z"); // nov
  TestDateTimeRoundtrip("1970-12-31T00:00:00Z"); // dec
}

TEST(DateTime, ParseTimeRfc3339AcceptsLowercaseTZ)
{
  TestDateTimeRoundtrip("1970-01-01t00:00:00Z", "1970-01-01T00:00:00Z");
  TestDateTimeRoundtrip("1970-01-01T00:00:00z", "1970-01-01T00:00:00Z");
}

TEST(DateTime, ParsingTimeRoundtripLeapYearLastDay)
{
  TestDateTimeRoundtrip("2016-12-31T20:59:59Z");
  TestDateTimeRoundtrip("2020-12-31T20:59:59Z");
  TestDateTimeRoundtrip("2021-01-01T20:59:59Z");
}

TEST(DateTime, ParseTimeRoundtripAcceptsInvalidNoTrailingTimezone)
{
  // No digits after the dot, or non-digits. This is not a valid input, but we should not choke on
  // it, Simply ignore the bad fraction
  std::string const badStrings[] = {"2013-11-19T14:30:59.Z", "2013-11-19T14:30:59.a12Z"};
  std::string const strCorrected = "2013-11-19T14:30:59Z";

  for (auto const& str : badStrings)
  {
    auto const dt = DateTime::Parse(str, DateTime::DateFormat::Rfc3339);
    auto const str2 = dt.ToString(DateTime::DateFormat::Rfc3339);
    EXPECT_EQ(str2, strCorrected);
  }
}

TEST(DateTime, ToStringNoArg)
{
  auto dt = DateTime::Parse("2013-05-17T01:02:03.1230000Z", DateTime::DateFormat::Rfc3339);
  EXPECT_EQ(dt.ToString(), "2013-05-17T01:02:03.123Z");
}

TEST(DateTime, ToStringOneArg)
{
  auto dt = DateTime::Parse("2013-05-17T01:02:03.1230000Z", DateTime::DateFormat::Rfc3339);
  EXPECT_EQ(dt.ToString(DateTime::DateFormat::Rfc3339), "2013-05-17T01:02:03.123Z");
  EXPECT_EQ(dt.ToString(DateTime::DateFormat::Rfc1123), "Fri, 17 May 2013 01:02:03 GMT");
}

TEST(DateTime, ToStringInvalid)
{
  auto dt = DateTime::Parse("2013-05-17T01:02:03.1230000Z", DateTime::DateFormat::Rfc3339);

  EXPECT_THROW(dt.ToString(static_cast<DateTime::DateFormat>(2)), std::invalid_argument);

  EXPECT_THROW(
      dt.ToString(DateTime::DateFormat::Rfc1123, DateTime::TimeFractionFormat::AllDigits),
      std::invalid_argument);
  EXPECT_THROW(
      dt.ToString(DateTime::DateFormat::Rfc1123, DateTime::TimeFractionFormat::DropTrailingZeros),
      std::invalid_argument);
  EXPECT_THROW(
      dt.ToString(DateTime::DateFormat::Rfc1123, DateTime::TimeFractionFormat::Truncate),
      std::invalid_argument);
  EXPECT_THROW(
      dt.ToString(DateTime::DateFormat::Rfc1123, static_cast<DateTime::TimeFractionFormat>(3)),
      std::invalid_argument);
}

TEST(DateTime, ParseTimeInvalid2)
{
  // Various unsupported cases. In all cases, we have produce an empty date time
  std::string const badStrings[] = {
      "", // empty
      ".Z", // too short
      ".Zx", // no trailing Z
      "3.14Z" // not a valid date
      "a971-01-01T00:00:00Z", // any non digits or valid separators
      "1a71-01-01T00:00:00Z",
      "19a1-01-01T00:00:00Z",
      "197a-01-01T00:00:00Z",
      "1971a01-01T00:00:00Z",
      "1971-a1-01T00:00:00Z",
      "1971-0a-01T00:00:00Z",
      "1971-01a01T00:00:00Z",
      "1971-01-a1T00:00:00Z",
      "1971-01-0aT00:00:00Z",
      // "1971-01-01a00:00:00Z", parsed as complete date
      "1971-01-01Ta0:00:00Z",
      "1971-01-01T0a:00:00Z",
      "1971-01-01T00a00:00Z",
      "1971-01-01T00:a0:00Z",
      "1971-01-01T00:0a:00Z",
      "1971-01-01T00:00a00Z",
      "1971-01-01T00:00:a0Z",
      "1971-01-01T00:00:0aZ",
      // "1971-01-01T00:00:00a", accepted as per invalid_no_trailing_timezone above
      "1", // truncation
      "19",
      "197",
      "1970",
      "1970-",
      "1970-0",
      "1970-01",
      "1970-01-",
      "1970-01-0",
      // "1970-01-01", complete date
      "1970-01-01T",
      "1970-01-01T0",
      "1970-01-01T00",
      "1970-01-01T00:",
      "1970-01-01T00:0",
      "1970-01-01T00:00",
      "1970-01-01T00:00:",
      "1970-01-01T00:00:0",
      // "1970-01-01T00:00:00", // accepted as invalid timezone above
      "0000-01-01T00:00:00Z", // year too small
      "1971-00-01T00:00:00Z", // month too small
      "1971-20-01T00:00:00Z", // month too big
      "1971-13-01T00:00:00Z",
      "1971-01-00T00:00:00Z", // day too small
      "1971-01-32T00:00:00Z", // day too big
      "1971-02-30T00:00:00Z", // day too big for feb
      "1971-02-30T00:00:00Z", // day too big for feb (non-leap year)
      "1971-03-32T00:00:00Z", // other months
      "1971-04-31T00:00:00Z",
      "1971-05-32T00:00:00Z",
      "1971-06-31T00:00:00Z",
      "1971-07-32T00:00:00Z",
      "1971-08-32T00:00:00Z",
      "1971-09-31T00:00:00Z",
      "1971-10-32T00:00:00Z",
      "1971-11-31T00:00:00Z",
      "1971-12-32T00:00:00Z",
      "1971-01-01T70:00:00Z", // hour too big
      "1971-01-01T24:00:00Z",
      "1971-01-01T00:60:00Z", // minute too big
      "1971-01-01T00:00:70Z", // second too big
      "1971-01-01T00:00:61Z",
      "0001-01-01T00:00:00+00:01", // time zone underflow
      // "1970-01-01T00:00:00.Z", // accepted as invalid timezone above
      //"1970-01-01T00:00:00+24:00", // bad tz offsets
      //"1970-01-01T00:00:00-30:00",
      "1970-01-01T00:00:00+21:60",
      //"1970-01-01T00:00:00-24:00",
      "1970-01-01T00:00:00-21:60",
      "1971-01-00", // zero month day
  };

  for (auto const& str : badStrings)
  {
    EXPECT_THROW(DateTime::Parse(str, DateTime::DateFormat::Rfc3339), std::invalid_argument);
  }
}

TEST(DateTime, ParseDatesBefore1900)
{
  TestDateTimeRoundtrip("1899-01-01T00:00:00Z");
  auto dt1 = DateTime::Parse("1899-01-01T00:00:00Z", DateTime::DateFormat::Rfc3339);
  auto dt2 = DateTime::Parse("Sun, 1 Jan 1899 00:00:00 GMT", DateTime::DateFormat::Rfc1123);
  EXPECT_EQ(dt1, dt2);

  TestDateTimeRoundtrip("0001-01-01T00:00:00Z");
  auto dt3 = DateTime::Parse("0001-01-01T00:00:00Z", DateTime::DateFormat::Rfc3339);
  auto dt4 = DateTime::Parse("Mon, 1 Jan 0001 00:00:00 GMT", DateTime::DateFormat::Rfc1123);
  EXPECT_EQ(dt3, dt4);
  EXPECT_EQ(0, dt3.time_since_epoch().count());
}

TEST(DateTime, ConstructorAndDuration)
{
  auto dt1 = DateTime::Parse("2020-11-03T15:30:45.1234567Z", DateTime::DateFormat::Rfc3339);
  auto dt2 = DateTime(2020, 11, 03, 15, 30, 45);
  dt2 += std::chrono::duration_cast<DateTime::duration>(std::chrono::nanoseconds(123456700));
  EXPECT_EQ(dt1, dt2);

  using namespace std::chrono_literals;
  auto duration = 8h + 29min + 14s + 876543300ns;

  auto dt3 = dt1 + duration;

  auto dt4 = DateTime::Parse("2020-11-04T00:00:00Z", DateTime::DateFormat::Rfc3339);
  EXPECT_EQ(dt3, dt4);
}

TEST(DateTime, ArithmeticOperators)
{
  auto const dt1 = DateTime(2020, 11, 03, 15, 30, 45);
  auto const dt2 = DateTime(2020, 11, 04, 15, 30, 45);
  auto dt3 = dt1;
  EXPECT_EQ(dt3, dt1);
  EXPECT_EQ(dt1, dt3);
  EXPECT_NE(dt3, dt2);
  EXPECT_NE(dt2, dt3);
  EXPECT_LT(dt1, dt2);
  EXPECT_LE(dt1, dt2);
  EXPECT_LE(dt1, dt3);
  EXPECT_LE(dt3, dt1);
  EXPECT_LE(dt3, dt2);
  EXPECT_GT(dt2, dt1);
  EXPECT_GE(dt2, dt1);

  using namespace std::chrono_literals;
  auto const diff = dt2 - dt1;
  EXPECT_EQ(24h, diff);
  EXPECT_LE(24h, diff);
  EXPECT_GE(24h, diff);

  dt3 += 24h;
  EXPECT_EQ(dt3, dt2);
  EXPECT_NE(dt3, dt1);

  dt3 -= 24h;
  EXPECT_EQ(dt3, dt1);
  EXPECT_NE(dt3, dt2);

  dt3 = dt1 + 12h;
  EXPECT_GT(dt3, dt1);
  EXPECT_LT(dt3, dt2);

  dt3 = dt2 - 24h;
  EXPECT_EQ(dt3, dt1);
}

TEST(DateTime, DefaultConstructible)
{
  DateTime dt;
  EXPECT_EQ(0, dt.time_since_epoch().count());
}

TEST(DateTime, ComparisonOperators)
{
  std::chrono::system_clock::time_point const chronoPast = std::chrono::system_clock::now();
  std::chrono::system_clock::time_point const chronoFuture = chronoPast + std::chrono::hours(1);

  DateTime const azcorePast = chronoPast;
  DateTime const azcoreFuture = chronoFuture;

  EXPECT_LT(azcorePast, chronoFuture);
  EXPECT_LT(chronoPast, azcoreFuture);

  EXPECT_GT(azcoreFuture, chronoPast);
  EXPECT_GT(chronoFuture, azcorePast);

  EXPECT_NE(azcorePast, chronoFuture);
  EXPECT_NE(azcoreFuture, chronoPast);
  EXPECT_NE(chronoPast, azcoreFuture);
  EXPECT_NE(chronoFuture, azcorePast);

  EXPECT_EQ(azcorePast, chronoPast);
  EXPECT_EQ(azcoreFuture, chronoFuture);
  EXPECT_EQ(chronoPast, azcorePast);
  EXPECT_EQ(chronoFuture, azcoreFuture);

  EXPECT_LE(azcorePast, chronoFuture);
  EXPECT_LE(azcorePast, chronoPast);
  EXPECT_LE(azcoreFuture, chronoFuture);
  EXPECT_LE(chronoPast, azcoreFuture);
  EXPECT_LE(chronoPast, azcorePast);
  EXPECT_LE(chronoFuture, azcoreFuture);

  EXPECT_GE(azcoreFuture, chronoPast);
  EXPECT_GE(azcorePast, chronoPast);
  EXPECT_GE(azcoreFuture, chronoFuture);
  EXPECT_GE(chronoFuture, azcorePast);
  EXPECT_GE(chronoPast, azcorePast);
  EXPECT_GE(chronoFuture, azcoreFuture);
}

TEST(DateTime, TimeRoundtrip)
{
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000000Z");

  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000001Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000002Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000003Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000004Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000005Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000006Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000007Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000008Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000009Z");

  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000010Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000020Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000030Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000040Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000050Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000060Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000070Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000080Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000090Z");

  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000100Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000200Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000300Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000400Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000500Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000600Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000700Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000800Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0000900Z");

  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0001000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0002000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0003000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0004000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0005000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0006000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0007000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0008000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0009000Z");

  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0010000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0020000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0030000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0040000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0050000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0060000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0070000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0080000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0090000Z");

  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0100000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0200000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0300000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0400000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0500000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0600000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0700000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0800000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.0900000Z");

  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.1000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.2000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.3000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.4000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.5000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.6000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.7000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.8000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:00.9000000Z");

  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:01.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:02.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:03.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:04.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:05.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:06.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:07.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:08.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:09.0000000Z");

  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:10.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:20.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:30.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:40.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:00:50.0000000Z");

  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:01:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:02:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:03:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:04:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:05:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:06:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:07:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:08:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:09:00.0000000Z");

  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:10:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:20:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:30:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:40:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T00:50:00.0000000Z");

  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T01:00:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T02:00:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T03:00:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T04:00:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T05:00:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T06:00:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T07:00:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T08:00:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T09:00:00.0000000Z");

  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T10:00:00.0000000Z");
  TestDateTimeRoundtrip<DateTime::TimeFractionFormat::AllDigits>("2021-02-05T20:00:00.0000000Z");
}

TEST(DateTime, ParseRoundUpInvalidDate)
{
  EXPECT_THROW(
      static_cast<void>(
          DateTime::Parse("9999-12-31T23:59:00-00:01", DateTime::DateFormat::Rfc3339)),
      std::invalid_argument);

  EXPECT_THROW(
      static_cast<void>(
          DateTime::Parse("9999-12-31T23:59:59.99999995", DateTime::DateFormat::Rfc3339)),
      std::invalid_argument);

  EXPECT_THROW(
      static_cast<void>(DateTime::Parse("9999-12-31T23:59:60", DateTime::DateFormat::Rfc3339)),
      std::invalid_argument);
}

TEST(DateTime, ToSystemClock)
{
  if (DateTime(std::chrono::system_clock::time_point::min())
      > DateTime(DateTime::time_point::min()))
  {
    EXPECT_THROW(
        static_cast<void>(static_cast<std::chrono::system_clock::time_point>(
            DateTime(DateTime::time_point::min()))),
        std::invalid_argument);
  }

  if (DateTime(std::chrono::system_clock::time_point::max())
      > DateTime(DateTime::time_point::max()))
  {
    EXPECT_THROW(
        static_cast<void>(static_cast<std::chrono::system_clock::time_point>(
            DateTime(DateTime::time_point::max()))),
        std::invalid_argument);
  }

  {
    auto const tt = std::chrono::system_clock::to_time_t(
        static_cast<std::chrono::system_clock::time_point>(DateTime(2021, 7, 8, 15, 34, 56)));

#ifdef _MSC_VER
#pragma warning(push)
// warning C4996: 'gmtime': This function or variable may be unsafe. Consider using gmtime_s
// instead.
#pragma warning(disable : 4996)
#endif
    auto const tm = std::gmtime(&tt);
#ifdef _MSC_VER
#pragma warning(pop)
#endif

    // https://en.cppreference.com/w/cpp/chrono/c/tm
    EXPECT_EQ(tm->tm_year, (2021 - 1900)); // std::tm::tm_year is 1900-based.
    EXPECT_EQ(tm->tm_mon, 6); // std::tm::tm_mon is 0-based.
    EXPECT_EQ(tm->tm_mday, 8);
    EXPECT_EQ(tm->tm_hour, 15);
    EXPECT_EQ(tm->tm_min, 34);
    EXPECT_EQ(tm->tm_sec, 56);
  }
}

TEST(DateTime, OutOfToStringRange)
{
  using namespace std::literals::chrono_literals;

  const DateTime underflow(DateTime(0001) - 1s);
  const DateTime overflow(DateTime(9999, 12, 31, 23, 59, 59) + 1s);

  EXPECT_THROW(static_cast<void>(underflow.ToString()), std::invalid_argument);
  EXPECT_THROW(static_cast<void>(overflow.ToString()), std::invalid_argument);
}

TEST(DateTime, LeapYear)
{
  EXPECT_NO_THROW(static_cast<void>(DateTime(2021, 1, 29)));
  EXPECT_NO_THROW(static_cast<void>(DateTime(2021, 2, 28)));
  EXPECT_THROW(static_cast<void>(DateTime(2021, 2, 29)), std::invalid_argument);
}
