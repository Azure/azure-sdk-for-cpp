// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <azure/core/base64.hpp>
#include <azure/core/cryptography/hash.hpp>
#include <chrono>
#include <gtest/gtest.h>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace Azure::Core::Cryptography;

static std::vector<uint8_t> ComputeHash(const std::string& data)
{
  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.data());
  Md5Hash instance;
  return instance.Final(ptr, data.length());
}

static thread_local std::mt19937_64 random_generator(std::random_device{}());

static char RandomCharGenerator()
{
  const char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::uniform_int_distribution<size_t> distribution(0, sizeof(charset) - 2);
  return charset[distribution(random_generator)];
}

std::vector<uint8_t> RandomBuffer(size_t length)
{
  std::vector<uint8_t> result(length);
  char* dataPtr = reinterpret_cast<char*>(&result[0]);

  char* start_addr = dataPtr;
  char* end_addr = dataPtr + length;

  const size_t rand_int_size = sizeof(uint64_t);

  while (uintptr_t(start_addr) % rand_int_size != 0 && start_addr < end_addr)
  {
    *(start_addr++) = RandomCharGenerator();
  }

  std::uniform_int_distribution<uint64_t> distribution(0ULL, std::numeric_limits<uint64_t>::max());
  while (start_addr + rand_int_size <= end_addr)
  {
    *reinterpret_cast<uint64_t*>(start_addr) = distribution(random_generator);
    start_addr += rand_int_size;
  }
  while (start_addr < end_addr)
  {
    *(start_addr++) = RandomCharGenerator();
  }

  return result;
}

uint64_t RandomInt(uint64_t minNumber, uint64_t maxNumber)
{
  std::uniform_int_distribution<uint64_t> distribution(minNumber, maxNumber);
  return distribution(random_generator);
}

TEST(Md5Hash, Basic)
{
  Md5Hash md5empty;
  EXPECT_EQ(Azure::Core::Convert::Base64Encode(md5empty.Final()), "1B2M2Y8AsgTpgAmY7PhCfg==");
  EXPECT_EQ(Azure::Core::Convert::Base64Encode(ComputeHash("")), "1B2M2Y8AsgTpgAmY7PhCfg==");
  EXPECT_EQ(
      Azure::Core::Convert::Base64Encode(ComputeHash("Hello Azure!")), "Pz8543xut4RVSbb2g52Mww==");

  auto data = RandomBuffer(static_cast<size_t>(16777216));

  Md5Hash md5Single;
  Md5Hash md5Streaming;

  // There are two ways to get the hash value, a "single-shot" static API called `Hash()` and one
  // where you can stream partial data blocks with multiple calls to `Update()` and then once you
  // are done, call `Digest()` to calculate the hash of the whole set of data blocks.

  // What this test is saying is, split up a 16MB block into many 0-4MB chunks, and compare the
  // computed hash value when you have all the data with the streaming approach, and validate they
  // are equal.

  size_t length = 0;
  while (length < data.size())
  {
    size_t s = static_cast<size_t>(RandomInt(0, 4194304));
    s = std::min(s, data.size() - length);
    md5Streaming.Append(&data[length], s);
    md5Streaming.Append(&data[length], 0);
    length += s;
  }
  EXPECT_EQ(md5Streaming.Final(), md5Single.Final(data.data(), data.size()));
}

TEST(Md5Hash, ExpectThrow)
{
  std::string data = "";
  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.c_str());
  Md5Hash instance;

#if GTEST_HAS_DEATH_TEST
  ASSERT_DEATH(instance.Final(nullptr, 1), "");
  ASSERT_DEATH(instance.Append(nullptr, 1), "");
#endif

  EXPECT_EQ(
      Azure::Core::Convert::Base64Encode(instance.Final(ptr, data.length())),
      "1B2M2Y8AsgTpgAmY7PhCfg==");

#if GTEST_HAS_DEATH_TEST
#if defined(NDEBUG)
  // Release build won't provide assert msg
  ASSERT_DEATH(instance.Final(), "");
  ASSERT_DEATH(instance.Final(ptr, data.length()), "");
  ASSERT_DEATH(instance.Append(ptr, data.length()), "");
#else
  ASSERT_DEATH(instance.Final(), "Cannot call Final");
  ASSERT_DEATH(instance.Final(ptr, data.length()), "Cannot call Final");
  ASSERT_DEATH(instance.Append(ptr, data.length()), "Cannot call Append after calling Final");
#endif
#endif
}

TEST(Md5Hash, CtorDtor)
{
  {
    Md5Hash instance;
  }
}

TEST(Md5Hash, multiThread)
{
  auto hashThreadRoutine = [](int sleepFor) {
    Md5Hash instance;
    std::string data = "";
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data.c_str());

    std::this_thread::sleep_for(std::chrono::milliseconds(sleepFor));

    EXPECT_EQ(
        Azure::Core::Convert::Base64Encode(instance.Final(ptr, data.length())),
        "1B2M2Y8AsgTpgAmY7PhCfg==");
  };

  constexpr static int size = 100;
  std::vector<std::thread> pool;

  // Make 100 threads run after a little sleep
  // Each created thread will wait from 0 to 3 milliseconds to start to make threads overlap
  for (int counter = 0; counter < size; counter++)
  {
    pool.emplace_back(std::thread(hashThreadRoutine, counter % 4));
  }
  for (int counter = 0; counter < size; counter++)
  {
    pool[counter].join();
  }
}
