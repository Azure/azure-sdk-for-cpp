// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "test_base.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <string>

namespace Azure { namespace Storage { namespace Test {

  constexpr static const char* c_StandardStorageConnectionString = "";
  constexpr static const char* c_PremiumStorageConnectionString = "";
  constexpr static const char* c_BlobStorageConnectionString = "";
  constexpr static const char* c_PremiumFileConnectionString = "";
  constexpr static const char* c_ADLSGen2ConnectionString = "";

  const std::string& StandardStorageConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(c_StandardStorageConnectionString) != 0)
      {
        return c_StandardStorageConnectionString;
      }
      return std::getenv("STANDARD_STORAGE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& PremiumStorageConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(c_PremiumStorageConnectionString) != 0)
      {
        return c_PremiumStorageConnectionString;
      }
      return std::getenv("PREMIUM_STORAGE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& BlobStorageConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(c_BlobStorageConnectionString) != 0)
      {
        return c_BlobStorageConnectionString;
      }
      return std::getenv("BLOB_STORAGE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& PremiumFileConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(c_PremiumFileConnectionString) != 0)
      {
        return c_PremiumFileConnectionString;
      }
      return std::getenv("PREMIUM_FILE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& ADLSGen2ConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(c_ADLSGen2ConnectionString) != 0)
      {
        return c_ADLSGen2ConnectionString;
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

  std::string RandomString(size_t size)
  {
    std::string str;
    str.resize(size);
    std::generate(str.begin(), str.end(), random_char);
    return str;
  }

  std::string LowercaseRandomString(size_t size)
  {
    auto str = RandomString(size);
    std::transform(
        str.begin(), str.end(), str.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    return str;
  }

  std::map<std::string, std::string> RandomMetadata(size_t size)
  {
    std::map<std::string, std::string> result;
    for (unsigned i = 0; i < size; ++i)
    {
      // TODO: Use mixed casing after Azure::Core lower cases the headers.
      // Metadata keys cannot start with a number.
      result.insert_or_assign("m" + LowercaseRandomString(5), LowercaseRandomString(5));
    }
    return result;
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

  std::vector<uint8_t> ReadFile(const std::string& filename)
  {
    FILE* fin = fopen(filename.data(), "rb");
    if (!fin)
    {
      throw std::runtime_error("failed to open file");
    }
    fseek(fin, 0, SEEK_END);
    int64_t fileSize = ftell(fin);
    std::vector<uint8_t> fileContent(static_cast<std::size_t>(fileSize));
    fseek(fin, 0, SEEK_SET);
    fread(fileContent.data(), static_cast<size_t>(fileSize), 1, fin);
    fclose(fin);
    return fileContent;
  }

  void DeleteFile(const std::string& filename) { std::remove(filename.data()); }
  
  std::vector<uint8_t> RandomBuffer(std::size_t length)
  {
    std::vector<uint8_t> result(length);
    uint8_t* dataPtr = const_cast<uint8_t*>(result.data());
    RandomBuffer(reinterpret_cast<char*>(dataPtr), length);
    return result;
  }

}}} // namespace Azure::Storage::Test
