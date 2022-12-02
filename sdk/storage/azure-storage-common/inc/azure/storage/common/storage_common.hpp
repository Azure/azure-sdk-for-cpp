// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <azure/core/case_insensitive_containers.hpp>
#include <azure/core/http/policies/policy.hpp>

#include "azure/storage/common/internal/constants.hpp"
#include "azure/storage/common/internal/storage_per_retry_policy.hpp"

namespace Azure { namespace Storage {

  constexpr static const char* AccountEncryptionKey = "$account-encryption-key";

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
  struct ContentHash final
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

  namespace _internal {
    ContentHash FromBase64String(const std::string& base64String, HashAlgorithm algorithm);
    std::string ToBase64String(const ContentHash& hash);
  } // namespace _internal
  using Metadata = Azure::Core::CaseInsensitiveMap;

}} // namespace Azure::Storage