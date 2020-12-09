// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "azure/core/strings.hpp"

namespace Azure { namespace Storage {

  template <class... T> void unused(T&&...) {}

  constexpr int32_t InfiniteLeaseDuration = -1;
  constexpr static const char* AccountEncryptionKey = "$account-encryption-key";
  constexpr static const char* ETagWildcard = "*";

  std::string CreateUniqueLeaseId();

  namespace Details {
    struct CaseInsensitiveComparator
    {
      bool operator()(const std::string& lhs, const std::string& rhs) const
      {
        return std::lexicographical_compare(
            lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](char c1, char c2) {
              return Core::Strings::ToLower(c1) < Core::Strings::ToLower(c2);
            });
      }
    };
  } // namespace Details
  using Metadata = std::map<std::string, std::string, Details::CaseInsensitiveComparator>;

  enum class HashAlgorithm
  {
    Md5,
    Crc64,
  };

  struct ContentHash
  {
    std::vector<uint8_t> Value;
    HashAlgorithm Algorithm;
  };

}} // namespace Azure::Storage
