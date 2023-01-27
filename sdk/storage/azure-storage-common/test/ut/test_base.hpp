// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/platform.hpp>

#include <chrono>
#include <cstdint>
#include <limits>
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
      StorageTest() { TestBase::SetUpTestSuiteLocal(AZURE_TEST_ASSETS_DIR); }

    protected:
      const std::string& StandardStorageConnectionString();
      const std::string& PremiumStorageConnectionString();
      const std::string& BlobStorageConnectionString();
      const std::string& PremiumFileConnectionString();
      const std::string& AdlsGen2ConnectionString();
      const std::string& AadTenantId();
      const std::string& AadClientId();
      const std::string& AadClientSecret();

      std::string GetContainerValidName() const
      {
        std::string name(m_testContext.GetTestSuiteName() + m_testContext.GetTestName());
        // Make sure the name is less than 63 characters long
        auto const nameSize = name.size();
        size_t const maxContainerNameSize = 63;
        if (nameSize > maxContainerNameSize)
        {
          name = std::string(name.begin() + nameSize - maxContainerNameSize, name.end());
        }
        // Check name won't start with `-`
        if (name[0] == '-')
        {
          name = std::string(name.begin() + 1, name.end());
        }
        return Azure::Core::_internal::StringExtensions::ToLower(name);
      }

      std::string GetFileSystemValidName() const
      {
        std::string name(m_testContext.GetTestSuiteName() + m_testContext.GetTestName());
        // Make sure the name is less than 63 characters long
        auto const nameSize = name.size();
        size_t const maxContainerNameSize = 63;
        if (nameSize > maxContainerNameSize)
        {
          name = std::string(name.begin() + nameSize - maxContainerNameSize, name.end());
        }
        // Check name won't start with `-`
        if (name[0] == '-')
        {
          name = std::string(name.begin() + 1, name.end());
        }
        return Azure::Core::_internal::StringExtensions::ToLower(name);
      }

      static std::string GetTestEncryptionScope()
      {
        static const std::string TestEncryptionScope("EncryptionScopeForTest");
        return TestEncryptionScope;
      }

      static std::string AppendQueryParameters(
          const Azure::Core::Url& url,
          const std::string& queryParameters);

      /* cspell:disable-next-line */
      constexpr static const char* DummyMd5 = "tQbD1aMPeB+LiPffUwFQJQ==";
      /* cspell:disable-next-line */
      constexpr static const char* DummyCrc64 = "+DNR5PON4EM=";

      static uint64_t RandomInt(
          uint64_t minNumber = std::numeric_limits<uint64_t>::min(),
          uint64_t maxNumber = std::numeric_limits<uint64_t>::max());

      static std::string RandomString(size_t size = 10);

      std::string GetStringOfSize(size_t size = 10, bool lowercase = false);

      static std::string LowercaseRandomString(size_t size = 10);

      static Storage::Metadata GetMetadata(size_t size = 5);

      static void RandomBuffer(char* buffer, size_t length);
      static void RandomBuffer(uint8_t* buffer, size_t length)
      {
        RandomBuffer(reinterpret_cast<char*>(buffer), length);
      }
      static std::vector<uint8_t> RandomBuffer(size_t length);

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

      void SetUp() override
      {
        Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
      }

    public:
      const static Azure::ETag DummyETag;
      const static Azure::ETag DummyETag2;
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

    class CryptFunctionsTest : public StorageTest {
    };

    class ClientSecretCredentialTest : public StorageTest {

    private:
      std::unique_ptr<Azure::Storage::Blobs::BlobContainerClient> m_client;

    protected:
      std::shared_ptr<Core::Credentials::TokenCredential> m_credential;
      std::string m_containerName;

      // Required to rename the test propertly once the test is started.
      // We can only know the test instance name until the test instance is run.
      Azure::Storage::Blobs::BlobContainerClient const& GetClientForTest(
          std::string const& testName)
      {
        // set the interceptor for the current test
        m_testContext.RenameTest(testName);
        return *m_client;
      }

      void SetUp() override
      {
        StorageTest::SetUp();
        m_containerName = Azure::Core::_internal::StringExtensions::ToLower(GetTestName());

        m_credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
            AadTenantId(), AadClientId(), AadClientSecret());

        Azure::Storage::Blobs::BlobClientOptions options;

        m_client = InitTestClient<
            Azure::Storage::Blobs::BlobContainerClient,
            Azure::Storage::Blobs::BlobClientOptions>(
            Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
                StandardStorageConnectionString(), m_containerName)
                .GetUrl(),
            m_credential,
            options);
      }
    };

  } // namespace Test

  namespace Files { namespace DataLake { namespace _detail {

    Azure::Core::Url GetBlobUrlFromUrl(const Azure::Core::Url& url);
    Azure::Core::Url GetDfsUrlFromUrl(const Azure::Core::Url& url);
    std::string GetBlobUrlFromUrl(const std::string& url);
    std::string GetDfsUrlFromUrl(const std::string& url);

  }}} // namespace Files::DataLake::_detail

}} // namespace Azure::Storage
