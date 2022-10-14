// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/token_cache.hpp"
#include "private/token_cache_internals.hpp"

#include <algorithm>
#include <chrono>
#include <mutex>
#include <utility>

using Azure::Identity::_detail::TokenCache;

using Azure::Core::Credentials::AccessToken;

decltype(TokenCache::Internals::Cache) TokenCache::Internals::Cache;
decltype(TokenCache::Internals::Expirations) TokenCache::Internals::Expirations;
decltype(TokenCache::Internals::Mutex) TokenCache::Internals::Mutex;

#if defined(TESTING_BUILD)
decltype(TokenCache::Internals::OnBeforeWriteLock) TokenCache::Internals::OnBeforeWriteLock;
#endif

AccessToken TokenCache::GetToken(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& authorityHost,
    std::string const& scopes,
    std::function<AccessToken()> const& getNewToken)
{
  // If cached token expires in less than MinExpiry from now, it's cached value won't be returned,
  // newer value will be requested.
  constexpr auto MinExpiry = std::chrono::minutes(3);

  Internals::CacheKey const key{tenantId, clientId, authorityHost, scopes};

  {
    std::shared_lock<std::shared_timed_mutex> readLock(Internals::Mutex);

    auto found = Internals::Cache.find(key);
    if (found != Internals::Cache.end()
        && found->second.ExpiresOn > std::chrono::system_clock::now() + MinExpiry)
    {
      return found->second;
    }
  }

#if defined(TESTING_BUILD)
  if (TokenCache::Internals::OnBeforeWriteLock != nullptr)
  {
    TokenCache::Internals::OnBeforeWriteLock();
  }
#endif

  std::unique_lock<std::shared_timed_mutex> writeLock(Internals::Mutex);

  // Search cache for the second time in case a new entry was appended between releasing readLock
  // and acquiring writeLock.
  {
    auto found = Internals::Cache.find(key);
    if (found != Internals::Cache.end()
        && found->second.ExpiresOn > std::chrono::system_clock::now() + MinExpiry)
    {
      return found->second;
    }
  }

  // Clean up expired and expiring cache entries
  auto const expirationPredicate
      = [](decltype(Internals::Expirations)::value_type const& x,
           decltype(Internals::Expirations)::value_type const& y) { return x.first < y.first; };
  {
    // Expirations vector is sorted from the the past to the future.
    // Find the last expired entry.
    auto const lastExpired = std::upper_bound(
        Internals::Expirations.begin(),
        Internals::Expirations.end(),
        decltype(Internals::Expirations)::value_type{
            std::chrono::system_clock::now() + MinExpiry, {}},
        expirationPredicate);

    if (lastExpired != Internals::Expirations.begin())
    {
      // All the enries in expirations vector starting from the beginning and including the one
      // found above, are expired.
      for (auto e = Internals::Expirations.begin(); e != lastExpired; ++e)
      {
        Internals::Cache.erase(e->second);
      }

      if (lastExpired != Internals::Expirations.end())
      {
        Internals::Cache.erase(lastExpired->second);
        Internals::Expirations.erase(Internals::Expirations.begin(), lastExpired + 1);
      }
      else
      {
        Internals::Expirations.clear();
      }
    }
  }

  auto const newToken = getNewToken();
  Internals::Cache[key] = newToken;

  {
    auto const expiry = std::make_pair(newToken.ExpiresOn, key);

    // Expirations vector is sorted from the past to the future.
    // Insert the new entry there to the right place.
    // Tokens that expire before it, will come before, and these that expire after, will come after.
    Internals::Expirations.insert(
        std::upper_bound(
            Internals::Expirations.begin(),
            Internals::Expirations.end(),
            expiry,
            expirationPredicate),
        expiry);
  }

  return newToken;
}
