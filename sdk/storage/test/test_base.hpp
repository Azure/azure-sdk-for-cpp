// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "gtest/gtest.h"

namespace Azure { namespace Storage { namespace Test {

  constexpr static const char* k_StandardStorageConnectionString = "";
  constexpr static const char* k_PremiumStorageConnectionString = "";
  constexpr static const char* k_BlobStorageConnectionString = "";
  constexpr static const char* k_PremiumFileConnectionString = "";
  constexpr static const char* k_ADLSGen2ConnectionString = "";

  const std::string& StandardStorageConnectionString();
  const std::string& PremiumStorageConnectionString();
  const std::string& BlobStorageConnectionString();
  const std::string& PremiumFileConnectionString();
  const std::string& ADLSGen2ConnectionString();

  inline unsigned long long operator""_KB(unsigned long long x) { return x * 1024; }
  inline unsigned long long operator""_MB(unsigned long long x) { return x * 1024 * 1024; }
  inline unsigned long long operator""_GB(unsigned long long x) { return x * 1024 * 1024 * 1024; }
  inline unsigned long long operator""_TB(unsigned long long x)
  {
    return x * 1024 * 1024 * 1024 * 1024;
  }

  std::string RandomString();

  std::string LowercaseRandomString();

  void RandomBuffer(char* buffer, std::size_t length);

}}} // namespace Azure::Storage::Test
