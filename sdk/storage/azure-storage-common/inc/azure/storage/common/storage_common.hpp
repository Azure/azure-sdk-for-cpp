// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <azure/core/http/policy.hpp>
#include <azure/core/internal/strings.hpp>

#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"
#include "azure/storage/common/storage_retry_policy.hpp"

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

  namespace Details {

    /*
     * Policies order:
     * Shared built-in per-operation policies
     * Service-specific built-in per-operation policies
     * Customer-defined per-operation policies
     * Retry policy
     * Shared built-in per-retry policies
     * Service-specific built-in per-retry policies
     * Customer-defined per-retry policies
     * Authentication policy
     * Transport policy
     */

    template <class T>
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> ConstructPolicies(
        std::unique_ptr<Azure::Core::Http::HttpPolicy> serviceBuiltinPerOperationPolicy,
        std::unique_ptr<Azure::Core::Http::HttpPolicy> authenticationPolicy,
        T&& clientOptions)
    {
      std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
      {
        Azure::Core::Http::Internal::ValuePolicyOptions options;
        options.HeaderValues[Details::HttpHeaderXMsVersion] = clientOptions.ApiVersion;
        policies.emplace_back(std::make_unique<Azure::Core::Http::Internal::ValuePolicy>(options));
      }
      policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
      if (serviceBuiltinPerOperationPolicy)
      {
        policies.emplace_back(std::move(serviceBuiltinPerOperationPolicy));
      }
      for (const auto& p : clientOptions.PerOperationPolicies)
      {
        policies.emplace_back(p->Clone());
      }
      policies.emplace_back(std::make_unique<Storage::Details::StorageRetryPolicy>(
          std::forward<T>(clientOptions).RetryOptions));
      policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
      for (const auto& p : clientOptions.PerRetryPolicies)
      {
        policies.emplace_back(p->Clone());
      }
      if (authenticationPolicy)
      {
        policies.emplace_back(std::move(authenticationPolicy));
      }
      policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
          std::forward<T>(clientOptions).TransportPolicyOptions));
      return policies;
    }

  } // namespace Details

}} // namespace Azure::Storage
