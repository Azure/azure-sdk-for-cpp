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
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <gtest/gtest.h>

namespace Azure { namespace Storage {

  namespace Test {

    const std::string& StandardStorageConnectionString();
    const std::string& PremiumStorageConnectionString();
    const std::string& BlobStorageConnectionString();
    const std::string& PremiumFileConnectionString();
    const std::string& AdlsGen2ConnectionString();
    const std::string& AadTenantId();
    const std::string& AadClientId();
    const std::string& AadClientSecret();

    constexpr static const char* TestEncryptionScope = "EncryptionScopeForTest";

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

    std::string AppendQueryParameters(
        const Azure::Core::Url& url,
        const std::string& queryParameters);

    const static Azure::ETag DummyETag("0x8D83B58BDF51D75");
    const static Azure::ETag DummyETag2("0x8D812645BFB0CDE");
    /* cspell:disable-next-line */
    constexpr static const char* DummyMd5 = "tQbD1aMPeB+LiPffUwFQJQ==";
    /* cspell:disable-next-line */
    constexpr static const char* DummyCrc64 = "+DNR5PON4EM=";

    uint64_t RandomInt(
        uint64_t minNumber = std::numeric_limits<uint64_t>::min(),
        uint64_t maxNumber = std::numeric_limits<uint64_t>::max());

    std::string RandomString(size_t size = 10);

    std::string LowercaseRandomString(size_t size = 10);

    Storage::Metadata RandomMetadata(size_t size = 5);

    void RandomBuffer(char* buffer, size_t length);
    inline void RandomBuffer(uint8_t* buffer, size_t length)
    {
      RandomBuffer(reinterpret_cast<char*>(buffer), length);
    }
    std::vector<uint8_t> RandomBuffer(size_t length);

    inline std::vector<uint8_t> ReadBodyStream(std::unique_ptr<Azure::Core::IO::BodyStream>& stream)
    {
      Azure::Core::Context context;
      return stream->ReadToEnd(context);
    }

    inline std::vector<uint8_t> ReadBodyStream(
        std::unique_ptr<Azure::Core::IO::BodyStream>&& stream)
    {
      return ReadBodyStream(stream);
    }

    std::vector<uint8_t> ReadFile(const std::string& filename);

    void DeleteFile(const std::string& filename);

    std::string InferSecondaryUrl(const std::string primaryUri);

    bool IsValidTime(const Azure::DateTime& datetime);

    inline std::string Base64EncodeText(const std::string& text)
    {
      return Azure::Core::Convert::Base64Encode(std::vector<uint8_t>(text.begin(), text.end()));
    }

  } // namespace Test

  namespace Files { namespace DataLake { namespace _detail {

    Azure::Core::Url GetBlobUrlFromUrl(const Azure::Core::Url& url);
    Azure::Core::Url GetDfsUrlFromUrl(const Azure::Core::Url& url);
    std::string GetBlobUrlFromUrl(const std::string& url);
    std::string GetDfsUrlFromUrl(const std::string& url);

  }}} // namespace Files::DataLake::_detail

}} // namespace Azure::Storage
