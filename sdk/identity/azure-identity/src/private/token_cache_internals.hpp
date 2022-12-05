// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Token cache internals and test hooks.
 */

#pragma once

#include "token_cache.hpp"

#if defined(TESTING_BUILD)
#include "azure/identity/dll_import_export.hpp"
#endif

#include <azure/core/credentials/credentials.hpp>

#include <functional>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>
#include <tuple>

namespace Azure { namespace Identity { namespace _detail {
  /**
   * @brief Implements internal aspects of token cache and provides test hooks.
   *
   */
  class TokenCache::Internals final {
    Internals() = delete;
    ~Internals() = delete;

  public:
    /**
     * @brief Represents a unique set of characteristics that are used to distinguish between cache
     * entries.
     *
     */
    struct CacheKey final
    {
      std::string TenantId; ///< Tenant ID.
      std::string ClientId; ///< Client ID.
      std::string AuthorityHost; ///< Authority Host.
      std::string Scopes; ///< Authentication Scopes as a single string.

      bool operator<(TokenCache::Internals::CacheKey const& other) const
      {
        return std::tie(TenantId, ClientId, AuthorityHost, Scopes)
            < std::tie(other.TenantId, other.ClientId, other.AuthorityHost, other.Scopes);
      }
    };

    /**
     * @brief Represents immediate cache value (token) and a synchronization primitive to handle its
     * updates.
     *
     */
    struct CacheValue final
    {
      std::shared_timed_mutex ElementMutex;
      Core::Credentials::AccessToken AccessToken;
    };

    /**
     * @brief The cache itself.
     *
     */
    static std::map<CacheKey, std::shared_ptr<CacheValue>> Cache;

    /**
     * @brief Mutex to access the cache container.
     *
     */
    static std::shared_timed_mutex CacheMutex;

#if defined(TESTING_BUILD)
    /**
     * A test hook that gets invoked before cache write lock gets acquired.
     *
     */
    AZ_IDENTITY_DLLEXPORT static std::function<void()> OnBeforeCacheWriteLock;

    /**
     * A test hook that gets invoked before item write lock gets acquired.
     *
     */
    AZ_IDENTITY_DLLEXPORT static std::function<void()> OnBeforeItemWriteLock;
#endif
  };
}}} // namespace Azure::Identity::_detail
