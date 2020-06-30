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

  std::vector<uint8_t> ReadBodyStream(std::unique_ptr<Azure::Core::Http::BodyStream>& stream)
  {
    std::vector<uint8_t> body;
    if (stream->Length() == static_cast<decltype(stream->Length())>(-1))
    {
      std::size_t bufferSize = static_cast<std::size_t>(16_KB);
      auto readBuffer = std::make_unique<uint8_t[]>(bufferSize);
      while (true)
      {
        auto bytesRead = stream->Read(readBuffer.get(), bufferSize);
        if (bytesRead == 0)
        {
          break;
        }
        body.insert(body.end(), readBuffer.get(), readBuffer.get() + bytesRead);
      }
    }
    else
    {
      body.resize(static_cast<std::size_t>(stream->Length()));
      std::size_t offset = 0;
      while (true)
      {
        auto bytesRead = stream->Read(&body[offset], body.size() - offset);
        offset += static_cast<std::size_t>(bytesRead);
        if (bytesRead == 0 || offset == body.size())
        {
          break;
        }
      }
      if (offset != body.size())
      {
        throw std::runtime_error("failed to read all content from body stream");
      }
    }
    return body;
  }

}}} // namespace Azure::Storage::Test
