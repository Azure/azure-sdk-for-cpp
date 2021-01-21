// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <azure/core/internal/strings.hpp>

namespace Azure { namespace Storage {

  template <class... T> void unused(T&&...) {}

  // Remove these two after DataLake lease client and file lease client are done.
  constexpr int32_t InfiniteLeaseDuration = -1;
  std::string CreateUniqueLeaseId();

  /**
   * @brief The algorithm used for hash.
   */
  enum class HashAlgorithm
  {
    /**
     * @brief MD5 message digest algorithm.
     */
    Md5,

    /**
     * @brief Cyclic redundancy check.
     */
    Crc64,
  };

  /**
   * @brief Hash used to check content integrity.
   */
  struct ContentHash
  {
    /**
     * @brief Binary hash value.
     */
    std::vector<uint8_t> Value;

    /**
     * @brief The algorithm used for hash.
     */
    HashAlgorithm Algorithm = HashAlgorithm::Md5;
  };

  namespace Details {
    struct CaseInsensitiveComparator
    {
      bool operator()(const std::string& lhs, const std::string& rhs) const
      {
        return std::lexicographical_compare(
            lhs.begin(), lhs.end(), rhs.begin(), rhs.end(), [](char c1, char c2) {
              return Core::Internal::Strings::ToLower(c1) < Core::Internal::Strings::ToLower(c2);
            });
      }
    };

    ContentHash FromBase64String(const std::string& base64String, HashAlgorithm algorithm);
    std::string ToBase64String(const ContentHash& hash);
  } // namespace Details
  using Metadata = std::map<std::string, std::string, Details::CaseInsensitiveComparator>;

}} // namespace Azure::Storage
