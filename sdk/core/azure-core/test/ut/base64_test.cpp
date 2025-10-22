// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/base64.hpp>

#include <random>
#include <string>
#include <vector>

#include <gtest/gtest.h>

using namespace Azure::Core;

TEST(Base64, Basic)
{
  int maxLength = 7;

  std::vector<uint8_t> data;
  for (uint8_t i = 0; i < maxLength; i++)
  {
    data.push_back(i + 1);
  }

  std::string result = Convert::Base64Encode(data);
  EXPECT_PRED2(
      [](std::string expect, std::string actual) { return expect == actual; },
      result,
      "AQIDBAUGBw=="); // cspell:disable-line
  EXPECT_TRUE(std::equal(data.begin(), data.end(), Convert::Base64Decode(result).begin()));

  std::vector<uint8_t> subsection = std::vector<uint8_t>(data.begin(), data.begin() + 1);
  result = Convert::Base64Encode(subsection);
  EXPECT_PRED2(
      [](std::string expect, std::string actual) { return expect == actual; }, result, "AQ==");
  EXPECT_TRUE(
      std::equal(subsection.begin(), subsection.end(), Convert::Base64Decode(result).begin()));

  subsection = std::vector<uint8_t>(data.begin(), data.begin() + 2);
  result = Convert::Base64Encode(subsection);
  EXPECT_PRED2(
      [](std::string expect, std::string actual) { return expect == actual; }, result, "AQI=");
  EXPECT_TRUE(
      std::equal(subsection.begin(), subsection.end(), Convert::Base64Decode(result).begin()));

  subsection = std::vector<uint8_t>(data.begin(), data.begin() + 3);
  result = Convert::Base64Encode(subsection);
  EXPECT_PRED2(
      [](std::string expect, std::string actual) { return expect == actual; },
      result,
      "AQID"); // cspell:disable-line
  EXPECT_TRUE(
      std::equal(subsection.begin(), subsection.end(), Convert::Base64Decode(result).begin()));

  subsection = std::vector<uint8_t>(data.begin(), data.begin() + 4);
  result = Convert::Base64Encode(subsection);
  EXPECT_PRED2(
      [](std::string expect, std::string actual) { return expect == actual; },
      result,
      "AQIDBA=="); // cspell:disable-line
  EXPECT_TRUE(
      std::equal(subsection.begin(), subsection.end(), Convert::Base64Decode(result).begin()));

  subsection = std::vector<uint8_t>(data.begin(), data.begin() + 5);
  result = Convert::Base64Encode(subsection);
  EXPECT_PRED2(
      [](std::string expect, std::string actual) { return expect == actual; },
      result,
      "AQIDBAU="); // cspell:disable-line
  EXPECT_TRUE(
      std::equal(subsection.begin(), subsection.end(), Convert::Base64Decode(result).begin()));

  subsection = std::vector<uint8_t>(data.begin(), data.begin() + 6);
  result = Convert::Base64Encode(subsection);
  EXPECT_PRED2(
      [](std::string expect, std::string actual) { return expect == actual; },
      result,
      "AQIDBAUG"); // cspell:disable-line
  EXPECT_TRUE(
      std::equal(subsection.begin(), subsection.end(), Convert::Base64Decode(result).begin()));
}

static thread_local std::mt19937_64 random_generator(std::random_device{}());

static char RandomChar()
{
  const char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::uniform_int_distribution<size_t> distribution(0, sizeof(charset) - 2);
  return charset[distribution(random_generator)];
}

void RandomBuffer(char* buffer, size_t length)
{
  char* start_addr = buffer;
  char* end_addr = buffer + length;

  const size_t rand_int_size = sizeof(uint64_t);

  while (uintptr_t(start_addr) % rand_int_size != 0 && start_addr < end_addr)
  {
    *(start_addr++) = RandomChar();
  }

  std::uniform_int_distribution<uint64_t> distribution(0ULL, std::numeric_limits<uint64_t>::max());
  while (start_addr + rand_int_size <= end_addr)
  {
    *reinterpret_cast<uint64_t*>(start_addr) = distribution(random_generator);
    start_addr += rand_int_size;
  }
  while (start_addr < end_addr)
  {
    *(start_addr++) = RandomChar();
  }
}

inline void RandomBuffer(uint8_t* buffer, size_t length)
{
  RandomBuffer(reinterpret_cast<char*>(buffer), length);
}

TEST(Base64, Roundtrip)
{
  for (size_t len : {0, 10, 100, 1000, 10000})
  {
    std::vector<uint8_t> data;
    data.resize(len);
    RandomBuffer(data.data(), data.size());
    EXPECT_EQ(Convert::Base64Decode(Convert::Base64Encode(data)), data);
  }
}

TEST(Base64, RoundtripString)
{
  for (size_t len : {0, 10, 100, 1000, 10000})
  {
    std::string data;
    data.resize(len);
    RandomBuffer(reinterpret_cast<uint8_t*>(const_cast<char*>(data.data())), data.size());

    std::vector<uint8_t> expected{data.begin(), data.end()};
    EXPECT_EQ(Convert::Base64Decode(_internal::Convert::Base64Encode(data)), expected);
  }
}

TEST(Base64, ValidDecode)
{
  // cspell::disable
  EXPECT_NO_THROW(Convert::Base64Decode(Convert::Base64Encode(std::vector<uint8_t>{})));
  EXPECT_NO_THROW(Convert::Base64Decode(""));
  EXPECT_NO_THROW(Convert::Base64Decode("aa=="));
  EXPECT_NO_THROW(Convert::Base64Decode("aaa="));
  // cspell::enable
}

TEST(Base64, InvalidDecode)
{
  // cspell::disable
  EXPECT_THROW(Convert::Base64Decode("a"), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("aa"), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("aaa"), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("a==="), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("===="), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("@#!%"), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("ABCD%GA="), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("ABCDE^A="), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("ABCDEF&="), std::runtime_error);

  EXPECT_THROW(Convert::Base64Decode("ABD%GA=="), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("ABDE^A=="), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("ABDEF&=="), std::runtime_error);

  EXPECT_THROW(Convert::Base64Decode("AD%GA==="), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("ADE^A==="), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("ADEF&==="), std::runtime_error);

  EXPECT_THROW(Convert::Base64Decode("ABCD===="), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("ADEF====="), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("ADEF======"), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("QQ======"), std::runtime_error);

  EXPECT_THROW(Convert::Base64Decode("AB===CD="), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("AB==CD=="), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("AB=CD==="), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("AB====CD"), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("AD=====EF"), std::runtime_error);
  EXPECT_THROW(Convert::Base64Decode("AD======EF"), std::runtime_error);

  // cspell::enable
}

// Base64Url Tests
TEST(Base64Url, BasicEncode)
{
  // Test empty input
  std::vector<uint8_t> emptyData;
  std::string result = _internal::Base64Url::Base64UrlEncode(emptyData);
  EXPECT_EQ(result, "");

  // Test single byte (padding should be removed)
  std::vector<uint8_t> oneByte = {0x01};
  result = _internal::Base64Url::Base64UrlEncode(oneByte);
  EXPECT_EQ(result, "AQ"); // Base64 would be "AQ==", Base64Url removes padding

  // Test two bytes (padding should be removed)
  std::vector<uint8_t> twoBytes = {0x01, 0x02};
  result = _internal::Base64Url::Base64UrlEncode(twoBytes);
  EXPECT_EQ(result, "AQI"); // Base64 would be "AQI=", Base64Url removes padding

  // Test three bytes (no padding needed)
  std::vector<uint8_t> threeBytes = {0x01, 0x02, 0x03};
  result = _internal::Base64Url::Base64UrlEncode(threeBytes);
  EXPECT_EQ(result, "AQID"); // cspell:disable-line

  // Test data with + and / in Base64 encoding (should be replaced with - and _)
  // Base64 for these bytes would contain '+' and '/', which should be replaced in Base64Url
  std::vector<uint8_t> specialChars = {0xFB, 0xEF};
  result = _internal::Base64Url::Base64UrlEncode(specialChars);
  // Verify it doesn't contain + or /
  EXPECT_EQ(result.find('+'), std::string::npos);
  EXPECT_EQ(result.find('/'), std::string::npos);

  // Test data that generates + in Base64
  std::vector<uint8_t> dataWithPlus = {0xFB}; // This should generate '+' in standard Base64
  result = _internal::Base64Url::Base64UrlEncode(dataWithPlus);
  EXPECT_EQ(result.find('+'), std::string::npos);
  EXPECT_NE(result.find('-'), std::string::npos); // Should have '-' instead

  // Test data that generates / in Base64
  std::vector<uint8_t> dataWithSlash = {0xFF}; // This should generate '/' in standard Base64
  result = _internal::Base64Url::Base64UrlEncode(dataWithSlash);
  EXPECT_EQ(result.find('/'), std::string::npos);
  EXPECT_NE(result.find('_'), std::string::npos); // Should have '_' instead
}

TEST(Base64Url, BasicDecode)
{
  // Test empty input
  std::vector<uint8_t> result = _internal::Base64Url::Base64UrlDecode("");
  EXPECT_TRUE(result.empty());

  // Test single byte (no padding in input)
  result = _internal::Base64Url::Base64UrlDecode("AQ");
  EXPECT_EQ(result.size(), 1u);
  EXPECT_EQ(result[0], 0x01);

  // Test two bytes (no padding in input)
  result = _internal::Base64Url::Base64UrlDecode("AQI");
  EXPECT_EQ(result.size(), 2u);
  EXPECT_EQ(result[0], 0x01);
  EXPECT_EQ(result[1], 0x02);

  // Test three bytes
  result = _internal::Base64Url::Base64UrlDecode("AQID"); // cspell:disable-line
  EXPECT_EQ(result.size(), 3u);
  EXPECT_EQ(result[0], 0x01);
  EXPECT_EQ(result[1], 0x02);
  EXPECT_EQ(result[2], 0x03);

  // Test with URL-safe characters (- and _)
  std::vector<uint8_t> encoded = {0xFB, 0xEF};
  std::string base64UrlEncoded = _internal::Base64Url::Base64UrlEncode(encoded);
  result = _internal::Base64Url::Base64UrlDecode(base64UrlEncoded);
  EXPECT_EQ(result, encoded);
}

TEST(Base64Url, RoundtripEncodeDecode)
{
  // Test various lengths to verify padding handling
  for (size_t len : {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 100, 1000})
  {
    std::vector<uint8_t> data;
    data.resize(len);
    RandomBuffer(data.data(), data.size());

    std::string encoded = _internal::Base64Url::Base64UrlEncode(data);
    std::vector<uint8_t> decoded = _internal::Base64Url::Base64UrlDecode(encoded);

    EXPECT_EQ(decoded, data) << "Roundtrip failed for length " << len;

    // Verify no padding characters in encoded output
    EXPECT_EQ(encoded.find('='), std::string::npos)
        << "Base64Url encoded string should not contain padding";

    // Verify no standard Base64 special chars (+ and /)
    EXPECT_EQ(encoded.find('+'), std::string::npos)
        << "Base64Url encoded string should not contain '+'";
    EXPECT_EQ(encoded.find('/'), std::string::npos)
        << "Base64Url encoded string should not contain '/'";
  }
}

TEST(Base64Url, SpecialCharacterReplacement)
{
  // Create data that will produce '+' and '/' in standard Base64
  // 0x3E in 6-bit encoding = 62 = '+' in Base64, should be '-' in Base64Url
  // 0x3F in 6-bit encoding = 63 = '/' in Base64, should be '_' in Base64Url

  // Pattern that generates both + and /
  // We need bytes that when encoded produce these characters
  std::vector<uint8_t> data;
  for (int i = 0; i < 256; i++)
  {
    data.push_back(static_cast<uint8_t>(i));
  }

  std::string encoded = _internal::Base64Url::Base64UrlEncode(data);

  // Verify the encoding uses URL-safe characters
  EXPECT_EQ(encoded.find('+'), std::string::npos) << "Should not contain '+'";
  EXPECT_EQ(encoded.find('/'), std::string::npos) << "Should not contain '/'";

  // Verify roundtrip works
  std::vector<uint8_t> decoded = _internal::Base64Url::Base64UrlDecode(encoded);
  EXPECT_EQ(decoded, data);
}

TEST(Base64Url, PaddingHandling)
{
  // Test that Base64Url decode correctly adds padding

  // Length % 4 == 2 should add "=="
  std::vector<uint8_t> data1 = {0x01};
  std::string encoded1 = _internal::Base64Url::Base64UrlEncode(data1);
  EXPECT_EQ(encoded1.length() % 4, 2u);
  EXPECT_EQ(encoded1.find('='), std::string::npos);
  std::vector<uint8_t> decoded1 = _internal::Base64Url::Base64UrlDecode(encoded1);
  EXPECT_EQ(decoded1, data1);

  // Length % 4 == 3 should add "="
  std::vector<uint8_t> data2 = {0x01, 0x02};
  std::string encoded2 = _internal::Base64Url::Base64UrlEncode(data2);
  EXPECT_EQ(encoded2.length() % 4, 3u);
  EXPECT_EQ(encoded2.find('='), std::string::npos);
  std::vector<uint8_t> decoded2 = _internal::Base64Url::Base64UrlDecode(encoded2);
  EXPECT_EQ(decoded2, data2);

  // Length % 4 == 0 should not add padding
  std::vector<uint8_t> data3 = {0x01, 0x02, 0x03};
  std::string encoded3 = _internal::Base64Url::Base64UrlEncode(data3);
  EXPECT_EQ(encoded3.length() % 4, 0u);
  std::vector<uint8_t> decoded3 = _internal::Base64Url::Base64UrlDecode(encoded3);
  EXPECT_EQ(decoded3, data3);
}

TEST(Base64Url, InvalidDecodeInput)
{
  // Test length % 4 == 1 (invalid)
  EXPECT_THROW(
      _internal::Base64Url::Base64UrlDecode("A"), // cspell:disable-line
      std::invalid_argument);
  EXPECT_THROW(
      _internal::Base64Url::Base64UrlDecode("AAAAA"), // cspell:disable-line
      std::invalid_argument);
  EXPECT_THROW(
      _internal::Base64Url::Base64UrlDecode("AAAAAAAAA"), // cspell:disable-line
      std::invalid_argument);

  // Test invalid characters (that aren't valid in Base64)
  EXPECT_THROW(_internal::Base64Url::Base64UrlDecode("@@@@"), std::runtime_error);
  EXPECT_THROW(_internal::Base64Url::Base64UrlDecode("A@@@"), std::runtime_error);
  EXPECT_THROW(_internal::Base64Url::Base64UrlDecode("####"), std::runtime_error);
}

TEST(Base64Url, ComparisonWithStandardBase64)
{
  // Create test data
  std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};

  // Encode with standard Base64
  std::string base64 = Convert::Base64Encode(data);

  // Encode with Base64Url
  std::string base64Url = _internal::Base64Url::Base64UrlEncode(data);

  // Base64Url should not have padding
  EXPECT_NE(base64.find('='), std::string::npos); // Standard Base64 should have padding
  EXPECT_EQ(base64Url.find('='), std::string::npos); // Base64Url should not

  // Both should decode to the same data
  EXPECT_EQ(Convert::Base64Decode(base64), data);
  EXPECT_EQ(_internal::Base64Url::Base64UrlDecode(base64Url), data);
}

TEST(Base64Url, KnownVectors)
{
  // Test with known Base64 -> Base64Url conversions
  // Standard Base64: "hello" -> "aGVsbG8="
  // Base64Url should be: "aGVsbG8" (no padding)
  std::vector<uint8_t> hello = {'h', 'e', 'l', 'l', 'o'};
  std::string encoded = _internal::Base64Url::Base64UrlEncode(hello);
  EXPECT_EQ(encoded, "aGVsbG8"); // cspell:disable-line

  // Decode and verify
  std::vector<uint8_t> decoded
      = _internal::Base64Url::Base64UrlDecode("aGVsbG8"); // cspell:disable-line
  EXPECT_EQ(decoded, hello);

  // Test case with special character replacement needs
  // Using bytes that create both + and / in standard Base64
  // Standard Base64 for {0xFB, 0xFF, 0xFE} is "+//+", which includes both + and /
  std::vector<uint8_t> specialData = {0xFB, 0xFF, 0xFE};
  std::string specialEncoded = _internal::Base64Url::Base64UrlEncode(specialData);

  // Verify URL-safe
  EXPECT_EQ(specialEncoded.find('+'), std::string::npos);
  EXPECT_EQ(specialEncoded.find('/'), std::string::npos);

  // Verify roundtrip
  std::vector<uint8_t> specialDecoded = _internal::Base64Url::Base64UrlDecode(specialEncoded);
  EXPECT_EQ(specialDecoded, specialData);
}
