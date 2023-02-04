// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/platform.hpp>

#include <chrono>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

#include <azure/core/base64.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/etag.hpp>
#include <azure/core/io/body_stream.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/storage/blobs.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <gtest/gtest.h>

namespace Azure { namespace Storage {

  namespace Test {

    class StorageTest : public Azure::Core::Test::TestBase {
    public:
<<<<<<< HEAD
      StorageTest() { TestBase::SetUpTestSuiteLocal(AZURE_TEST_ASSETS_DIR); }
=======
      const static Azure::ETag DummyETag;
      const static Azure::ETag DummyETag2;
      /* cspell:disable-next-line */
      constexpr static const char* DummyMd5 = "tQbD1aMPeB+LiPffUwFQJQ==";
      /* cspell:disable-next-line */
      constexpr static const char* DummyCrc64 = "+DNR5PON4EM=";
>>>>>>> 476e518c (test-proxy)

    protected:
      const std::string& StandardStorageConnectionString();
      const std::string& PremiumStorageConnectionString();
      const std::string& BlobStorageConnectionString();
      const std::string& PremiumFileConnectionString();
      const std::string& AdlsGen2ConnectionString();
      const std::string& AadTenantId();
      const std::string& AadClientId();
      const std::string& AadClientSecret();

      void SetUp() override;
      void TearDown() override;

      /**
       * @brief Retruns a string related to test suite name and test case name.
       */
      std::string GetIdentifier() const
      {
        size_t MaxLength = 63;
        std::string name = m_testContext.GetTestSuiteName() + m_testContext.GetTestName();
        if (name[0] == '-')
        {
          name = name.substr(1);
        }
        if (name.length() > MaxLength)
        {
          name.resize(MaxLength);
        }
        return name;
      }

      /**
       * @brief Retruns a lowercase string related to test suite name and test case name.
       */
      std::string GetLowercaseIdentifier() const
      {
        return Azure::Core::_internal::StringExtensions::ToLower(GetIdentifier());
      }

      bool IsValidTime(const Azure::DateTime& datetime) const
      {
        // Playback won't check dates
        if (m_testContext.IsPlaybackMode())
        {
          return true;
        }

        // We assume datetime within a week is valid.
        const auto minTime = std::chrono::system_clock::now() - std::chrono::hours(24 * 7);
        const auto maxTime = std::chrono::system_clock::now() + std::chrono::hours(24 * 7);
        return datetime > minTime && datetime < maxTime;
      }

      static std::string GetTestEncryptionScope()
      {
        static const std::string TestEncryptionScope("EncryptionScopeForTest");
        return TestEncryptionScope;
      }

      static std::string AppendQueryParameters(
          const Azure::Core::Url& url,
          const std::string& queryParameters);

      /**
       * Random functions below are not thread-safe. You must NOT call them from multiple threads.
       *
       * To make record-playback testing work, you have to call these functions in a determined way,
       * e.g. always in the same order, the same times and with the same parameters.
       *
       * Note that in C++, evaluation order of function parameters is undefined. So you CANNOT do:
       * `auto ret = RandomInt() + RandomInt();`
       * or
       * `auto ret = function1(RandomInt()) + function2(RandomInt());`
       */
      uint64_t RandomInt(
          uint64_t minNumber = std::numeric_limits<uint64_t>::min(),
          uint64_t maxNumber = std::numeric_limits<uint64_t>::max());
      char RandomChar();
      std::string RandomString(size_t size = 10);
      std::string LowercaseRandomString(size_t size = 10);
      Storage::Metadata RandomMetadata(size_t size = 5);
      void RandomBuffer(char* buffer, size_t length);
      void RandomBuffer(uint8_t* buffer, size_t length)
      {
        RandomBuffer(reinterpret_cast<char*>(buffer), length);
      }
      std::vector<uint8_t> RandomBuffer(size_t length);
      std::string RandomUUID();

      static std::vector<uint8_t> ReadBodyStream(
          std::unique_ptr<Azure::Core::IO::BodyStream>& stream)
      {
        Azure::Core::Context context;
        return stream->ReadToEnd(context);
      }

      static std::vector<uint8_t> ReadBodyStream(
          std::unique_ptr<Azure::Core::IO::BodyStream>&& stream)
      {
        return ReadBodyStream(stream);
      }

      static std::vector<uint8_t> ReadFile(const std::string& filename);

      static void WriteFile(const std::string& filename, const std::vector<uint8_t>& content);

      static void DeleteFile(const std::string& filename);

      static std::string InferSecondaryUrl(const std::string primaryUri);

      static std::string Base64EncodeText(const std::string& text)
      {
        return Azure::Core::Convert::Base64Encode(std::vector<uint8_t>(text.begin(), text.end()));
      }

    protected:
      std::vector<std::function<void()>> m_resourceCleanupFunctions;

    private:
      std::mt19937_64 m_randomGenerator;
    };

    constexpr inline unsigned long long operator""_KB(unsigned long long x) { return x * 1024; }
    constexpr inline unsigned long long operator""_MB(unsigned long long x)
    {
      return x * 1024 * 1024;
    }
    constexpr inline unsigned long long operator""_GB(unsigned long long x)
    {
      return x * 1024 * 1024 * 1024;
    }
    constexpr inline unsigned long long operator""_TB(unsigned long long x)
    {
      return x * 1024 * 1024 * 1024 * 1024;
    }

  } // namespace Test

  namespace Files { namespace DataLake { namespace _detail {

    Azure::Core::Url GetBlobUrlFromUrl(const Azure::Core::Url& url);
    Azure::Core::Url GetDfsUrlFromUrl(const Azure::Core::Url& url);
    std::string GetBlobUrlFromUrl(const std::string& url);
    std::string GetDfsUrlFromUrl(const std::string& url);

  }}} // namespace Files::DataLake::_detail

}} // namespace Azure::Storage
