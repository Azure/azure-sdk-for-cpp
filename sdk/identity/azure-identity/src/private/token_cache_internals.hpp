// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Token cache internals and test hooks.
 */

#pragma once

#include "token_cache.hpp"

#include <azure/core/credentials/credentials.hpp>

#include <chrono>
#include <functional>
#include <map>
#include <shared_mutex>
#include <string>
#include <vector>

namespace Azure { namespace Identity { namespace _detail {
  /**
   * @brief Implements internal aspects of token cache and provides test hooks.
   *
   */
  class TokenCache::Internals final {
  private:
    Internals() = delete;
    ~Internals() = delete;

  public:
    /**
     * @brief Represents a unique set of characteristics that are used to distinguish between cache
     * entries.
     *
     */
    struct CacheKey
    {
      std::string TenantId; ///< Tenant ID.
      std::string ClientId; ///< Client ID.
      std::string AuthorityHost; ///< Authority Host.
      std::string Scopes; ///< Authentication Scopes as a single string.

      bool operator<(TokenCache::Internals::CacheKey const& other) const;
    };

    /**
     * @brief The cache itself.
     *
     */
    static std::map<CacheKey, Core::Credentials::AccessToken> Cache;

    /**
     * @brief Vector of expiration for the cache entries, from earliest to the latest.
     *
     */
    static std::vector<std::pair<decltype(Core::Credentials::AccessToken::ExpiresOn), CacheKey>>
        Expirations;

    /**
     * @brief Cache and Expirations are kept in sync. The mutex is for accessing both of them.
     *
     */
    static std::shared_timed_mutex Mutex;

#if defined(TESTING_BUILD)
    /**
     * @brief A function that gets called before write lock is acquired.
     *
     */
    static std::function<void()> OnBeforeWriteLock;
#endif
  };

  inline bool TokenCache::Internals::CacheKey::operator<(
      TokenCache::Internals::CacheKey const& other) const
  {
    return std::tie(TenantId, ClientId, AuthorityHost, Scopes) < std::tie(other.TenantId, other.ClientId, other.AuthorityHost, other.Scopes);
  }
}}} // namespace Azure::Identity::_detail
