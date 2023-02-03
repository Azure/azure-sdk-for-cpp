// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "test_base.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <limits>
#include <random>
#include <sstream>
#include <string>

#include <azure/core/http/http.hpp>
#include <azure/core/internal/strings.hpp>
#include <azure/core/platform.hpp>

namespace Azure { namespace Storage { namespace Blobs { namespace Models {

  bool operator==(const SignedIdentifier& lhs, const SignedIdentifier& rhs)
  {
    return lhs.Id == rhs.Id && lhs.StartsOn.HasValue() == rhs.StartsOn.HasValue()
        && (!lhs.StartsOn.HasValue() || lhs.StartsOn.Value() == rhs.StartsOn.Value())
        && lhs.ExpiresOn.HasValue() == rhs.ExpiresOn.HasValue()
        && (!lhs.ExpiresOn.HasValue() || lhs.ExpiresOn.Value() == rhs.ExpiresOn.Value())
        && lhs.Permissions == rhs.Permissions;
  }

}}}} // namespace Azure::Storage::Blobs::Models

namespace Azure { namespace Storage { namespace Test {

  constexpr static const char* StandardStorageConnectionStringValue = "";
  constexpr static const char* PremiumStorageConnectionStringValue = "";
  constexpr static const char* BlobStorageConnectionStringValue = "";
  constexpr static const char* PremiumFileConnectionStringValue = "";
  constexpr static const char* AdlsGen2ConnectionStringValue = "";
  constexpr static const char* AadTenantIdValue = "";
  constexpr static const char* AadClientIdValue = "";
  constexpr static const char* AadClientSecretValue = "";

  void StorageTest::SetUp()
  {
    Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);

    // Need to call this function to skip live-only cases
    GetTestName();
    if (m_testContext.IsLiveMode())
    {
      m_randomGenerator.seed(std::random_device{}());
    }
    else
    {
      auto seedStr = GetIdentifier();
      std::seed_seq seedSeq(seedStr.begin(), seedStr.end());
      m_randomGenerator.seed(seedSeq);
    }
  }

  void StorageTest::TearDown()
  {
    for (auto& f : m_resourceCleanupFunctions)
    {
      try
      {
        f();
      }
      catch (...)
      {
      }
    }
    TestBase::TearDown();
  }

  const std::string& StorageTest::StandardStorageConnectionString()
  {
    const static std::string connectionString = [&]() -> std::string {
      if (strlen(StandardStorageConnectionStringValue) != 0)
      {
        return StandardStorageConnectionStringValue;
      }
      return GetEnv("STANDARD_STORAGE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& StorageTest::PremiumStorageConnectionString()
  {
    const static std::string connectionString = [&]() -> std::string {
      if (strlen(PremiumStorageConnectionStringValue) != 0)
      {
        return PremiumStorageConnectionStringValue;
      }
      return GetEnv("PREMIUM_STORAGE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& StorageTest::BlobStorageConnectionString()
  {
    const static std::string connectionString = [&]() -> std::string {
      if (strlen(BlobStorageConnectionStringValue) != 0)
      {
        return BlobStorageConnectionStringValue;
      }
      return GetEnv("BLOB_STORAGE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& StorageTest::PremiumFileConnectionString()
  {
    const static std::string connectionString = [&]() -> std::string {
      if (strlen(PremiumFileConnectionStringValue) != 0)
      {
        return PremiumFileConnectionStringValue;
      }
      return GetEnv("PREMIUM_FILE_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& StorageTest::AdlsGen2ConnectionString()
  {
    const static std::string connectionString = [&]() -> std::string {
      if (strlen(AdlsGen2ConnectionStringValue) != 0)
      {
        return AdlsGen2ConnectionStringValue;
      }
      return GetEnv("ADLS_GEN2_CONNECTION_STRING");
    }();
    return connectionString;
  }

  const std::string& StorageTest::AadTenantId()
  {
    const static std::string connectionString = [&]() -> std::string {
      if (strlen(AadTenantIdValue) != 0)
      {
        return AadTenantIdValue;
      }
      return GetEnv("AAD_TENANT_ID");
    }();
    return connectionString;
  }

  const std::string& StorageTest::AadClientId()
  {
    const static std::string connectionString = [&]() -> std::string {
      if (strlen(AadClientIdValue) != 0)
      {
        return AadClientIdValue;
      }
      return GetEnv("AAD_CLIENT_ID");
    }();
    return connectionString;
  }

  const std::string& StorageTest::AadClientSecret()
  {
    const static std::string connectionString = [&]() -> std::string {
      if (strlen(AadClientSecretValue) != 0)
      {
        return AadClientSecretValue;
      }
      return GetEnv("AAD_CLIENT_SECRET");
    }();
    return connectionString;
  }

  std::string StorageTest::AppendQueryParameters(
      const Azure::Core::Url& url,
      const std::string& queryParameters)
  {
    std::string absoluteUrl = url.GetAbsoluteUrl();
    if (queryParameters.empty())
    {
      return absoluteUrl;
    }
    const auto& existingQP = url.GetQueryParameters();
    bool startWithQuestion = queryParameters[0] == '?';
    if (existingQP.empty())
    {
      if (startWithQuestion)
      {
        absoluteUrl = absoluteUrl + queryParameters;
      }
      else
      {
        absoluteUrl = absoluteUrl + '?' + queryParameters;
      }
    }
    else
    {
      absoluteUrl += '&';
      if (startWithQuestion)
      {
        absoluteUrl = absoluteUrl + queryParameters.substr(1);
      }
      else
      {
        absoluteUrl = absoluteUrl + queryParameters;
      }
    }
    return absoluteUrl;
  }

  uint64_t StorageTest::RandomInt(uint64_t minNumber, uint64_t maxNumber)
  {
    std::uniform_int_distribution<uint64_t> distribution(minNumber, maxNumber);
    return distribution(m_randomGenerator);
  }

  char StorageTest::RandomChar()
  {
    const char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::uniform_int_distribution<size_t> distribution(0, sizeof(charset) - 2);
    return charset[distribution(m_randomGenerator)];
  }

  std::string StorageTest::RandomString(size_t size)
  {
    std::string str;
    str.resize(size);
    std::generate(str.begin(), str.end(), [this]() { return RandomChar(); });
    return str;
  }

  std::string StorageTest::LowercaseRandomString(size_t size)
  {
    return Azure::Core::_internal::StringExtensions::ToLower(RandomString(size));
  }

  Storage::Metadata StorageTest::RandomMetadata(size_t size)
  {
    Storage::Metadata result;
    for (size_t i = 0; i < size; ++i)
    {
      result["meta" + LowercaseRandomString(5)] = RandomString(10);
    }
    return result;
  }

  void StorageTest::RandomBuffer(char* buffer, size_t length)
  {
    char* start_addr = buffer;
    char* end_addr = buffer + length;

    const size_t rand_int_size = sizeof(uint64_t);

    while (uintptr_t(start_addr) % rand_int_size != 0 && start_addr < end_addr)
    {
      *(start_addr++) = RandomChar();
    }

    std::uniform_int_distribution<uint64_t> distribution(
        0ULL, std::numeric_limits<uint64_t>::max());
    while (start_addr + rand_int_size <= end_addr)
    {
      *reinterpret_cast<uint64_t*>(start_addr) = distribution(m_randomGenerator);
      start_addr += rand_int_size;
    }
    while (start_addr < end_addr)
    {
      *(start_addr++) = RandomChar();
    }
  }

  std::vector<uint8_t> StorageTest::RandomBuffer(size_t length)
  {
    std::vector<uint8_t> result(length);
    if (length != 0)
    {
      char* dataPtr = reinterpret_cast<char*>(&result[0]);
      RandomBuffer(dataPtr, length);
    }
    return result;
  }

  std::string StorageTest::RandomUUID()
  {
    std::vector<uint8_t> randomNum = RandomBuffer(16);
    char buffer[37];

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%2.2x%2.2x%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x-%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x",
        randomNum[0],
        randomNum[1],
        randomNum[2],
        randomNum[3],
        randomNum[4],
        randomNum[5],
        randomNum[6],
        randomNum[7],
        randomNum[8],
        randomNum[9],
        randomNum[10],
        randomNum[11],
        randomNum[12],
        randomNum[13],
        randomNum[14],
        randomNum[15]);

    return std::string(buffer);
  }

  std::vector<uint8_t> StorageTest::ReadFile(const std::string& filename)
  {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
    FILE* fin = fopen(filename.data(), "rb");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

    if (!fin)
    {
      throw std::runtime_error("Failed to open file.");
    }
    fseek(fin, 0, SEEK_END);
    int64_t fileSize = ftell(fin);
    std::vector<uint8_t> fileContent(static_cast<size_t>(fileSize));
    fseek(fin, 0, SEEK_SET);
    size_t elementsRead = fread(fileContent.data(), static_cast<size_t>(fileSize), 1, fin);
    if (elementsRead != 1 && fileSize != 0)
    {
      throw std::runtime_error("Failed to read file.");
    }
    fclose(fin);
    return fileContent;
  }

  void StorageTest::WriteFile(const std::string& filename, const std::vector<uint8_t>& content)
  {
    std::ofstream f(filename, std::ofstream::binary);
    f.write(reinterpret_cast<const char*>(content.data()), content.size());
  }

  void StorageTest::DeleteFile(const std::string& filename) { std::remove(filename.data()); }

  std::string StorageTest::InferSecondaryUrl(const std::string primaryUrl)
  {
    Azure::Core::Url secondaryUri(primaryUrl);
    std::string primaryHost = secondaryUri.GetHost();
    auto dotPos = primaryHost.find(".");
    std::string accountName = primaryHost.substr(0, dotPos);
    std::string secondaryHost = accountName + "-secondary" + primaryHost.substr(dotPos);
    secondaryUri.SetHost(secondaryHost);
    return secondaryUri.GetAbsoluteUrl();
  }

  const Azure::ETag StorageTest::DummyETag("0x8D83B58BDF51D75");
  const Azure::ETag StorageTest::DummyETag2("0x8D812645BFB0CDE");

}}} // namespace Azure::Storage::Test
