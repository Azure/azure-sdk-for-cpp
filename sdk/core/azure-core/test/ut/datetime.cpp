// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include <azure/core/datetime.hpp>

#include <limits>

using namespace Azure::Core;

TEST(DateTime, ParseDateAndTimeBasic)
{
  auto dt1 = DateTime::FromString("20130517T00:00:00Z", DateTime::DateFormat::Iso8601);
  EXPECT_NE(0u, dt1.ToInterval());

  auto dt2 = DateTime::FromString("Fri, 17 May 2013 00:00:00 GMT", DateTime::DateFormat::Rfc1123);
  EXPECT_NE(0u, dt2.ToInterval());

  EXPECT_EQ(dt1.ToInterval(), dt2.ToInterval());
}

TEST(DateTime, ParseDateAndTimeExtended)
{
  auto dt1 = DateTime::FromString("2013-05-17T00:00:00Z", DateTime::DateFormat::Iso8601);
  EXPECT_NE(0u, dt1.ToInterval());

  auto dt2 = DateTime::FromString("Fri, 17 May 2013 00:00:00 GMT", DateTime::DateFormat::Rfc1123);
  EXPECT_NE(0u, dt2.ToInterval());

  EXPECT_EQ(dt1.ToInterval(), dt2.ToInterval());
}

TEST(DateTime, ParseDateBasic)
{
  {
    auto dt = DateTime::FromString("20130517", DateTime::DateFormat::Iso8601);
    EXPECT_NE(0u, dt.ToInterval());
  }
}

TEST(DateTime, ParseDateExtended)
{
  {
    auto dt = DateTime::FromString("2013-05-17", DateTime::DateFormat::Iso8601);
    EXPECT_NE(0u, dt.ToInterval());
  }
}

namespace {
void TestDateTimeRoundtrip(std::string const& str, std::string const& strExpected)
{
  auto dt = DateTime::FromString(str, DateTime::DateFormat::Iso8601);
  auto const str2 = dt.ToString(DateTime::DateFormat::Iso8601);
  EXPECT_EQ(str2, strExpected);
}

void TestDateTimeRoundtrip(std::string const& str) { TestDateTimeRoundtrip(str, str); }
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

  // lose the last '999' without rounding up
  TestDateTimeRoundtrip("2013-11-19T14:30:59.1234567999Z", "2013-11-19T14:30:59.1234567Z");
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

TEST(DateTime, ParseTimeRoundripYear1601) { TestDateTimeRoundtrip("1601-01-01T00:00:00Z"); }

TEST(DateTime, ParseTimeRoundripYear9999) { TestDateTimeRoundtrip("9999-12-31T23:59:59.9999999Z"); }

TEST(DateTime, EmittingTimeCorrectDay)
{
  auto const test = DateTime() + 132004507640000000LL; // 2019-04-22T23:52:44 is a Monday
  auto const actual = test.ToString(DateTime::DateFormat::Rfc1123);
  std::string const expected("Mon");
  EXPECT_EQ(actual.substr(0, 3), expected);
}

namespace {
void TestRfc1123IsTimeT(char const* str, DateTime::IntervalType t)
{
  auto const dt = DateTime::FromString(str, DateTime::DateFormat::Rfc1123);
  auto interval = dt.ToInterval();
  EXPECT_EQ(0, interval % 10000000);
  interval /= 10000000;
  interval -= 11644473600; // NT epoch adjustment
  EXPECT_EQ(t, interval);
}
} // namespace

TEST(DateTime, ParseTimeRfc1123AcceptsEachDay)
{
  TestRfc1123IsTimeT("01 Jan 1970 00:00:00 GMT", 0);
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
  TestRfc1123IsTimeT("19 Jan 2038 03:14:06 GMT", std::numeric_limits<int>::max() - 1);
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
      "01 Jan 1600 00:00:00 GMT", // year too small
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
      "14 Jan 2019 23:16:21 G0100", // bad tzoffsets
      "01 Jan 1970 00:00:00 +2400",
      "01 Jan 1970 00:00:00 -3000",
      "01 Jan 1970 00:00:00 +2160",
      "01 Jan 1970 00:00:00 -2400",
      "01 Jan 1970 00:00:00 -2160",
      "00 Jan 1971 00:00:00 GMT", // zero month day
  };

  for (auto const& str : badStrings)
  {
    auto const dt = DateTime::FromString(str, DateTime::DateFormat::Rfc1123);
    EXPECT_EQ(0, dt.ToInterval());
  }
}

TEST(DateTime, ParseTimeIso8601BoundaryCases)
{
  // boundary cases:
  TestDateTimeRoundtrip("1970-01-01T00:00:00Z"); // epoch
  TestDateTimeRoundtrip("2038-01-19T03:14:06+00:00", "2038-01-19T03:14:06Z"); // INT_MAX - 1
  TestDateTimeRoundtrip(
      "2038-01-19T03:13:07-00:01",
      "2038-01-19T03:14:07Z"); // INT_MAX after subtacting 1
  TestDateTimeRoundtrip("2038-01-19T03:14:07-00:00", "2038-01-19T03:14:07Z");
}

TEST(DateTime, ParseTimeIso8601UsesEachTimezoneDigit)
{
  TestDateTimeRoundtrip("2019-01-14T23:16:21+00:00", "2019-01-14T23:16:21Z");
  TestDateTimeRoundtrip("2019-01-14T23:16:21-00:01", "2019-01-14T23:17:21Z");
  TestDateTimeRoundtrip("2019-01-14T23:16:21+00:01", "2019-01-14T23:15:21Z");
  TestDateTimeRoundtrip("2019-01-14T23:16:21-01:00", "2019-01-15T00:16:21Z");
  TestDateTimeRoundtrip("2019-01-14T23:16:21+01:00", "2019-01-14T22:16:21Z");
}

TEST(DateTime, ParseTimeIso8601UsesEachDigit)
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

TEST(DateTime, ParseTimeIso8601AcceptsMonthMaxDays)
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

TEST(DateTime, ParseTimeIso8601AcceptsLowercaseTZ)
{
  TestDateTimeRoundtrip("1970-01-01t00:00:00Z", "1970-01-01T00:00:00Z");
  TestDateTimeRoundtrip("1970-01-01T00:00:00z", "1970-01-01T00:00:00Z");
}

TEST(DateTime, ParseTimeRoundtripAcceptsInvalidNoTrailingTimezone)
{
  // No digits after the dot, or non-digits. This is not a valid input, but we should not choke on
  // it, Simply ignore the bad fraction
  std::string const badStrings[] = {"2013-11-19T14:30:59.Z", "2013-11-19T14:30:59.a12Z"};
  std::string const strCorrected = "2013-11-19T14:30:59Z";

  for (auto const& str : badStrings)
  {
    auto const dt = DateTime::FromString(str, DateTime::DateFormat::Iso8601);
    auto const str2 = dt.ToString(DateTime::DateFormat::Iso8601);
    EXPECT_EQ(str2, strCorrected);
  }
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
      "1600-01-01T00:00:00Z", // year too small
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
      "1601-01-01T00:00:00+00:01", // time zone underflow
      // "1970-01-01T00:00:00.Z", // accepted as invalid timezone above
      "1970-01-01T00:00:00+24:00", // bad tzoffsets
      "1970-01-01T00:00:00-30:00",
      "1970-01-01T00:00:00+21:60",
      "1970-01-01T00:00:00-24:00",
      "1970-01-01T00:00:00-21:60",
      "1971-01-00", // zero month day
  };

  for (auto const& str : badStrings)
  {
    auto const dt = DateTime::FromString(str, DateTime::DateFormat::Iso8601);
    EXPECT_EQ(dt.ToInterval(), 0);
  }
}

TEST(DateTime, ParseDatesBefore1900)
{
  TestDateTimeRoundtrip("1899-01-01T00:00:00Z");
  auto dt1 = DateTime::FromString("1899-01-01T00:00:00Z", DateTime::DateFormat::Iso8601);
  auto dt2 = DateTime::FromString("Sun, 1 Jan 1899 00:00:00 GMT", DateTime::DateFormat::Rfc1123);
  EXPECT_EQ(dt1.ToInterval(), dt2.ToInterval());

  TestDateTimeRoundtrip("1601-01-01T00:00:00Z");
  auto dt3 = DateTime::FromString("1601-01-01T00:00:00Z", DateTime::DateFormat::Iso8601);
  auto dt4 = DateTime::FromString("Mon, 1 Jan 1601 00:00:00 GMT", DateTime::DateFormat::Rfc1123);
  EXPECT_EQ(dt3.ToInterval(), dt4.ToInterval());
  EXPECT_EQ(0u, dt3.ToInterval());
}
