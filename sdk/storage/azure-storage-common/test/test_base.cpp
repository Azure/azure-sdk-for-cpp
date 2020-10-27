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
#include <ctime>
#include <iomanip>
#include <limits>
#include <random>
#include <sstream>
#include <string>

#include "azure/core/http/http.hpp"
#include "azure/core/strings.hpp"

namespace Azure { namespace Storage { namespace Test {

  constexpr static const char* c_StandardStorageConnectionString = "";
  constexpr static const char* c_PremiumStorageConnectionString = "";
  constexpr static const char* c_BlobStorageConnectionString = "";
  constexpr static const char* c_PremiumFileConnectionString = "";
  constexpr static const char* c_AdlsGen2ConnectionString = "";
  constexpr static const char* c_AadTenantId = "";
  constexpr static const char* c_AadClientId = "";
  constexpr static const char* c_AadClientSecret = "";

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
      if (strlen(c_StandardStorageConnectionString) != 0)
      {
        return c_StandardStorageConnectionString;
      }
      return GetEnv("STANDARD_STORAGE_CONNECTION_STRING");
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
      return GetEnv("PREMIUM_STORAGE_CONNECTION_STRING");
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
      return GetEnv("BLOB_STORAGE_CONNECTION_STRING");
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
      return GetEnv("PREMIUM_FILE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& AdlsGen2ConnectionString()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(c_AdlsGen2ConnectionString) != 0)
      {
        return c_AdlsGen2ConnectionString;
      }
      return GetEnv("ADLS_GEN2_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& AadTenantId()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(c_AadTenantId) != 0)
      {
        return c_AadTenantId;
      }
      return GetEnv("AAD_TENANT_ID");
    }();
    return connectionString;
  }

  const std::string& AadClientId()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(c_AadClientId) != 0)
      {
        return c_AadClientId;
      }
      return GetEnv("AAD_CLIENT_ID");
    }();
    return connectionString;
  }

  const std::string& AadClientSecret()
  {
    const static std::string connectionString = []() -> std::string {
      if (strlen(c_AadClientSecret) != 0)
      {
        return c_AadClientSecret;
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

  std::map<std::string, std::string> RandomMetadata(size_t size)
  {
    std::map<std::string, std::string> result;
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

  std::string ToIso8601(
      const std::chrono::system_clock::time_point& timePoint,
      int numDecimalDigits)
  {
    std::time_t epoch_seconds = std::chrono::system_clock::to_time_t(timePoint);
    struct tm ct;
#ifdef _WIN32
    gmtime_s(&ct, &epoch_seconds);
#else
    gmtime_r(&epoch_seconds, &ct);
#endif
    std::string time_str;
    time_str.resize(64);
    std::strftime(&time_str[0], time_str.length(), "%Y-%m-%dT%H:%M:%S", &ct);
    time_str = time_str.data();
    if (numDecimalDigits != 0)
    {
      time_str += ".";
      auto time_point_second = std::chrono::time_point_cast<std::chrono::seconds>(timePoint);
      auto decimal_part = timePoint - time_point_second;
      uint64_t num_nanoseconds
          = std::chrono::duration_cast<std::chrono::nanoseconds>(decimal_part).count();
      std::string decimal_part_str = std::to_string(num_nanoseconds);
      decimal_part_str = std::string(9 - decimal_part_str.length(), '0') + decimal_part_str;
      decimal_part_str.resize(numDecimalDigits);
      time_str += decimal_part_str;
    }
    time_str += "Z";
    return time_str;
  }

  std::string ToRfc1123(const std::chrono::system_clock::time_point& timePoint)
  {
    std::time_t epoch_seconds = std::chrono::system_clock::to_time_t(timePoint);
    struct tm ct;
#ifdef _WIN32
    gmtime_s(&ct, &epoch_seconds);
#else
    gmtime_r(&epoch_seconds, &ct);
#endif
    std::stringstream ss;
    ss.imbue(std::locale("C"));
    ss << std::put_time(&ct, "%a, %d %b %Y %H:%M:%S GMT");
    return ss.str();
  }

  std::chrono::system_clock::time_point FromRfc1123(const std::string& timeStr)
  {
    std::tm t;
    std::stringstream ss(timeStr);
    ss.imbue(std::locale("C"));
    ss >> std::get_time(&t, "%a, %d %b %Y %H:%M:%S GMT");
#ifdef _WIN32
    time_t tt = _mkgmtime(&t);
#else
    time_t tt = timegm(&t);
#endif
    return std::chrono::system_clock::from_time_t(tt);
  }

  std::string InferSecondaryUri(const std::string primaryUri)
  {
    Azure::Core::Http::Url secondaryUri(primaryUri);
    std::string primaryHost = secondaryUri.GetHost();
    auto dotPos = primaryHost.find(".");
    std::string accountName = primaryHost.substr(0, dotPos);
    std::string secondaryHost = accountName + "-secondary" + primaryHost.substr(dotPos);
    secondaryUri.SetHost(secondaryHost);
    return secondaryUri.GetAbsoluteUrl();
  }

}}} // namespace Azure::Storage::Test
