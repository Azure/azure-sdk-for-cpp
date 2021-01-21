// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <cstdint>
#include <limits>
#include <vector>

#include <azure/core/base64.hpp>
#include <azure/core/datetime.hpp>
#include <azure/core/http/body_stream.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <gtest/gtest.h>

namespace Azure { namespace Storage { namespace Test {

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

  constexpr static const char* DummyETag = "0x8D83B58BDF51D75";
  constexpr static const char* DummyETag2 = "0x8D812645BFB0CDE";
  constexpr static const char* DummyMd5 = "tQbD1aMPeB+LiPffUwFQJQ==";
  constexpr static const char* DummyCrc64 = "+DNR5PON4EM=";

  uint64_t RandomInt(
      uint64_t minNumber = std::numeric_limits<uint64_t>::min(),
      uint64_t maxNumber = std::numeric_limits<uint64_t>::max());

  std::string RandomString(size_t size = 10);

  std::string LowercaseRandomString(size_t size = 10);

  Storage::Metadata RandomMetadata(size_t size = 5);

  void RandomBuffer(char* buffer, std::size_t length);
  inline void RandomBuffer(uint8_t* buffer, std::size_t length)
  {
    RandomBuffer(reinterpret_cast<char*>(buffer), length);
  }
  std::vector<uint8_t> RandomBuffer(std::size_t length);

  inline std::vector<uint8_t> ReadBodyStream(std::unique_ptr<Azure::Core::Http::BodyStream>& stream)
  {
    Azure::Core::Context context;
    return Azure::Core::Http::BodyStream::ReadToEnd(context, *stream);
  }

  inline std::vector<uint8_t> ReadBodyStream(
      std::unique_ptr<Azure::Core::Http::BodyStream>&& stream)
  {
    return ReadBodyStream(stream);
  }

  std::vector<uint8_t> ReadFile(const std::string& filename);

  void DeleteFile(const std::string& filename);

  std::string InferSecondaryUrl(const std::string primaryUri);

  bool IsValidTime(const Azure::Core::DateTime& datetime);

  inline std::string Base64EncodeText(const std::string& text)
  {
    return Azure::Core::Base64Encode(std::vector<uint8_t>(text.begin(), text.end()));
  }

}}} // namespace Azure::Storage::Test
