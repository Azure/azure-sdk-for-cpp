// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "test_base.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <limits>
#include <random>
#include <string>

namespace Azure { namespace Storage { namespace Test {

  const std::string& StandardStorageConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(k_StandardStorageConnectionString) != 0)
      {
        return k_StandardStorageConnectionString;
      }
      return std::getenv("STANDARD_STORAGE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& PremiumStorageConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(k_PremiumStorageConnectionString) != 0)
      {
        return k_PremiumStorageConnectionString;
      }
      return std::getenv("PREMIUM_STORAGE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& BlobStorageConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(k_BlobStorageConnectionString) != 0)
      {
        return k_BlobStorageConnectionString;
      }
      return std::getenv("BLOB_STORAGE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& PremiumFileConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(k_PremiumFileConnectionString) != 0)
      {
        return k_PremiumFileConnectionString;
      }
      return std::getenv("PREMIUM_FILE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& ADLSGen2ConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(k_ADLSGen2ConnectionString) != 0)
      {
        return k_ADLSGen2ConnectionString;
      }
      return std::getenv("ADLS_GEN2_CONNECTION_STRING");
    }();
    return connectionString;
  }

  static thread_local std::mt19937_64 random_generator(std::random_device{}());

  static char random_char()
  {
    const char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::uniform_int_distribution<std::size_t> distribution(0, sizeof(charset) - 2);
    return charset[distribution(random_generator)];
  }

  std::string RandomString()
  {
    std::string str;
    str.resize(10);
    std::generate(str.begin(), str.end(), random_char);
    return str;
  }

  std::string LowercaseRandomString()
  {
    auto str = RandomString();
    std::transform(
        str.begin(), str.end(), str.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    return str;
  }

  void RandomBuffer(char* buffer, std::size_t length)
  {
    char* start_addr = buffer;
    char* end_addr = buffer + length;

    const std::size_t rand_int_size = sizeof(uint64_t);

    while (uintptr_t(start_addr) % rand_int_size != 0 && start_addr < end_addr)
    {
      *(start_addr++) = random_char();
    }

    std::uniform_int_distribution<uint64_t> distribution(
        0ULL, std::numeric_limits<uint64_t>::max());
    while (start_addr + rand_int_size <= end_addr)
    {
      *reinterpret_cast<uint64_t*>(start_addr) = distribution(random_generator);
      start_addr += rand_int_size;
    }
    while (start_addr < end_addr)
    {
      *(start_addr++) = random_char();
    }
  }

}}} // namespace Azure::Storage::Test
