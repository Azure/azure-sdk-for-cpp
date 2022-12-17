// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Token cache.
 *
 */

#pragma once

#include <azure/core/credentials/credentials.hpp>

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>

namespace Azure { namespace Identity { namespace _detail {
  /**
   * @brief Access token cache.
   *
   */
  class TokenCache
#if !defined(TESTING_BUILD)
      final
#endif
  {
#if !defined(TESTING_BUILD)
  private:
#else
  protected:
#endif
    // A test hook that gets invoked before cache write lock gets acquired.
    virtual void OnBeforeCacheWriteLock() const {};

    // A test hook that gets invoked before item write lock gets acquired.
    virtual void OnBeforeItemWriteLock() const {};

    struct CacheValue
    {
      Core::Credentials::AccessToken AccessToken;
      std::shared_timed_mutex ElementMutex;
    };

    // The current cache Key, std::string Scopes, may later evolve to a struct that contains more
    // fields. All that depends on the fields in the TokenRequestContext that are used as
    // characteristics that go into the network request that gets the token.
    // If tomorrow we add Multi-Tenant Authentication, and the TenantID stops being an immutable
    // characteristic of a credential instance, but instead becomes variable depending on the fields
    // of the TokenRequestContext that are taken into consideration as network request for the token
    // is being sent, it should go into what will form the new CacheKey struct.
    // i.e. we want all the variable inputs for obtaining a token to be a part of the key, because
    // we want to have the same kind of result. There should be no "hidden variables".
    // Otherwise, the cache will stop functioning properly, because the value you'd get from cache
    // for a given key will fail to authenticate, but if the cache ends up calling the getNewToken
    // callback, you'll authenticate successfully (however the other caller who need to get the
    // token for slightly different context will not be as lucky).
    mutable std::map<std::string, std::shared_ptr<CacheValue>> m_cache;
    mutable std::shared_timed_mutex m_cacheMutex;

  private:
    TokenCache(TokenCache const&) = delete;
    TokenCache& operator=(TokenCache const&) = delete;

    // Checks cache element if cached value should be reused. Caller should be holding ElementMutex.
    static bool IsFresh(
        std::shared_ptr<CacheValue> const& item,
        DateTime::duration minimumExpiration,
        std::chrono::system_clock::time_point now);

    // Gets item from cache, or creates it, puts into cache, and returns.
    std::shared_ptr<CacheValue> GetOrCreateValue(
        std::string const& key,
        DateTime::duration minimumExpiration) const;

  public:
    TokenCache() = default;
    ~TokenCache() = default;

    /**
     * @brief Attempts to get token from cache, and if not found, gets the token using the function
     * provided, caches it, and returns its value.
     *
     * @param scopeString Authentication scopes (or resource) as string.
     * @param minimumExpiration Minimum token lifetime for the cached value to be returned.
     * @param getNewToken Function to get the new token for the given \p scopeString, in case when
     * cache does not have it, or if its remaining lifetime is less than \p minimumExpiration.
     *
     * @return Authentication token.
     *
     */
    Core::Credentials::AccessToken GetToken(
        std::string const& scopeString,
        DateTime::duration minimumExpiration,
        std::function<Core::Credentials::AccessToken()> const& getNewToken) const;
  };
}}} // namespace Azure::Identity::_detail
