// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <azure/core/case_insensitive_map.hpp>

namespace Azure { namespace Storage {

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
    ContentHash FromBase64String(const std::string& base64String, HashAlgorithm algorithm);
    std::string ToBase64String(const ContentHash& hash);
  } // namespace Details
  using Metadata = Azure::Core::CaseInsensitiveMap;

}} // namespace Azure::Storage
