// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "test_base.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <limits>
#include <random>
#include <sstream>
#include <string>

#include <azure/core/http/http.hpp>
#include <azure/core/platform.hpp>
#include <azure/core/strings.hpp>

namespace Azure { namespace Storage { namespace Test {

  constexpr static const char* StandardStorageConnectionStringValue = "";
  constexpr static const char* PremiumStorageConnectionStringValue = "";
  constexpr static const char* BlobStorageConnectionStringValue = "";
  constexpr static const char* PremiumFileConnectionStringValue = "";
  constexpr static const char* AdlsGen2ConnectionStringValue = "";
  constexpr static const char* AadTenantIdValue = "";
  constexpr static const char* AadClientIdValue = "";
  constexpr static const char* AadClientSecretValue = "";

  std::string GetEnv(const std::string& name)
  {
    const char* ret = std::getenv(name.data());
    if (!ret)
    {
      throw std::runtime_error(
          name + " is required to run the tests but not set as an environment variable.");
    }
    return std::string(ret);
  }

  const std::string& StandardStorageConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(StandardStorageConnectionStringValue) != 0)
      {
        return StandardStorageConnectionStringValue;
      }
      return GetEnv("STANDARD_STORAGE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& PremiumStorageConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(PremiumStorageConnectionStringValue) != 0)
      {
        return PremiumStorageConnectionStringValue;
      }
      return GetEnv("PREMIUM_STORAGE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& BlobStorageConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(BlobStorageConnectionStringValue) != 0)
      {
        return BlobStorageConnectionStringValue;
      }
      return GetEnv("BLOB_STORAGE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& PremiumFileConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(PremiumFileConnectionStringValue) != 0)
      {
        return PremiumFileConnectionStringValue;
      }
      return GetEnv("PREMIUM_FILE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& AdlsGen2ConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(AdlsGen2ConnectionStringValue) != 0)
      {
        return AdlsGen2ConnectionStringValue;
      }
      return GetEnv("ADLS_GEN2_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& AadTenantId()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(AadTenantIdValue) != 0)
      {
        return AadTenantIdValue;
      }
      return GetEnv("AAD_TENANT_ID");
    }();
    return connectionString;
  }

  const std::string& AadClientId()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(AadClientIdValue) != 0)
      {
        return AadClientIdValue;
      }
      return GetEnv("AAD_CLIENT_ID");
    }();
    return connectionString;
  }

  const std::string& AadClientSecret()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(AadClientSecretValue) != 0)
      {
        return AadClientSecretValue;
      }
      return GetEnv("AAD_CLIENT_SECRET");
    }();
    return connectionString;
  }

  static thread_local std::mt19937_64 random_generator(std::random_device{}());

  uint64_t RandomInt(uint64_t minNumber, uint64_t maxNumber)
  {
    std::uniform_int_distribution<uint64_t> distribution(minNumber, maxNumber);
    return distribution(random_generator);
  }

  static char RandomChar()
  {
    const char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::uniform_int_distribution<std::size_t> distribution(0, sizeof(charset) - 2);
    return charset[distribution(random_generator)];
  }

  std::string RandomString(size_t size)
  {
    std::string str;
    str.resize(size);
    std::generate(str.begin(), str.end(), RandomChar);
    return str;
  }

  std::string LowercaseRandomString(size_t size)
  {
    return Azure::Core::Strings::ToLower(RandomString(size));
  }

  Storage::Metadata RandomMetadata(size_t size)
  {
    Storage::Metadata result;
    for (size_t i = 0; i < size; ++i)
    {
      // TODO: Use mixed casing after Azure::Core lower cases the headers.
      // Metadata keys cannot start with a number.
      result["m" + LowercaseRandomString(5)] = RandomString(5);
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
      *(start_addr++) = RandomChar();
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
      *(start_addr++) = RandomChar();
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
    std::size_t elementsRead = fread(fileContent.data(), static_cast<size_t>(fileSize), 1, fin);
    if (elementsRead != 1 && fileSize != 0)
    {
      throw std::runtime_error("failed to read file");
    }
    fclose(fin);
    return fileContent;
  }

  void DeleteFile(const std::string& filename) { std::remove(filename.data()); }

  std::vector<uint8_t> RandomBuffer(std::size_t length)
  {
    std::vector<uint8_t> result(length);
    char* dataPtr = reinterpret_cast<char*>(&result[0]);
    RandomBuffer(dataPtr, length);
    return result;
  }

  std::string InferSecondaryUrl(const std::string primaryUrl)
  {
    Azure::Core::Http::Url secondaryUri(primaryUrl);
    std::string primaryHost = secondaryUri.GetHost();
    auto dotPos = primaryHost.find(".");
    std::string accountName = primaryHost.substr(0, dotPos);
    std::string secondaryHost = accountName + "-secondary" + primaryHost.substr(dotPos);
    secondaryUri.SetHost(secondaryHost);
    return secondaryUri.GetAbsoluteUrl();
  }

  bool IsValidTime(const Azure::Core::DateTime& datetime)
  {
    // We assume datetime within a week is valid.
    const auto minTime = std::chrono::system_clock::now() - std::chrono::hours(24 * 7);
    const auto maxTime = std::chrono::system_clock::now() + std::chrono::hours(24 * 7);
    return datetime > minTime && datetime < maxTime;
  }

}}} // namespace Azure::Storage::Test
