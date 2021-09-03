// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/base64.hpp>
#include <gtest/gtest.h>
#include <random>
#include <string>
#include <vector>

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
