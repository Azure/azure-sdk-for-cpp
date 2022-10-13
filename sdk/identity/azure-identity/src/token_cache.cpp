// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/token_cache.hpp"

#include <algorithm>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <vector>

using Azure::Identity::_detail::TokenCache;

using Azure::Core::Credentials::AccessToken;

namespace {
struct CacheKey
{
  std::string TenantId;
  std::string ClientId;
  std::string AuthorityHost;
  std::string Scopes;
};

bool operator<(CacheKey const& x, CacheKey const& y)
{
  {
    auto const compare = x.TenantId.compare(y.TenantId);
    if (compare != 0)
    {
      return compare < 0;
    }
  }

  {
    auto const compare = x.ClientId.compare(y.ClientId);
    if (compare != 0)
    {
      return compare < 0;
    }
  }

  {
    auto const compare = x.AuthorityHost.compare(y.AuthorityHost);
    if (compare != 0)
    {
      return compare < 0;
    }
  }

  return x.Scopes < y.Scopes;
}

// The cache itself.
static std::map<CacheKey, AccessToken> g_cache;

// Vector of expiration for the cache entries, from earliest to the latest.
static std::vector<std::pair<decltype(AccessToken::ExpiresOn), CacheKey>> g_expirations;

// g_cache and g_expirations are kept in sync. The mutex is for accessing both of them.
static std::shared_timed_mutex g_mutex;
} // namespace

AccessToken TokenCache::GetToken(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& authorityHost,
    std::string const& scopes,
    std::function<AccessToken()> const& getNewToken)
{
  CacheKey const key{tenantId, clientId, authorityHost, scopes};

  {
    std::shared_lock<std::shared_timed_mutex> readLock(g_mutex);

    auto found = g_cache.find(key);
    if (found != g_cache.end()
        && found->second.ExpiresOn > std::chrono::system_clock::now() + std::chrono::minutes(2))
    {
      return found->second;
    }
  }

  std::unique_lock<std::shared_timed_mutex> writeLock(g_mutex);

  // Search cache for the second time in case a new entry was appended between releasing readLock
  // and acquiring writeLock.
  {
    auto found = g_cache.find(key);
    if (found != g_cache.end()
        && found->second.ExpiresOn > std::chrono::system_clock::now() + std::chrono::minutes(2))
    {
      return found->second;
    }
  }

  // Clean up expired and expiring cache entries
  auto const expirationPredicate
      = [](decltype(g_expirations)::value_type const& x,
           decltype(g_expirations)::value_type const& y) { return x.first < y.first; };
  {
    // Expirations vector is sorted from the the past to the future.
    // Find the last expired entry.
    auto const lastExpired = std::upper_bound(
        g_expirations.begin(),
        g_expirations.end(),
        decltype(g_expirations)::value_type{
            std::chrono::system_clock::now() + std::chrono::minutes(2), {}},
        expirationPredicate);

    if (lastExpired != g_expirations.begin())
    {
      // All the enries in expirations vector starting from the beginning and including the one
      // found above, are expired.
      for (auto e = g_expirations.begin(); e != lastExpired; ++e)
      {
        g_cache.erase(e->second);
      }

      if (lastExpired != g_expirations.end())
      {
        g_cache.erase(lastExpired->second);
        g_expirations.erase(g_expirations.begin(), lastExpired + 1);
      }
      else
      {
        g_expirations.clear();
      }
    }
  }

  auto const newToken = getNewToken();
  g_cache[key] = newToken;

  {
    auto const expiry = std::make_pair(newToken.ExpiresOn, key);

    // Expirations vector is sorted from the past to the future.
    // Insert the new entry there to the right place.
    // Tokens that expire before it, will come before, and these that expire after, will come after.
    g_expirations.insert(
        std::upper_bound(g_expirations.begin(), g_expirations.end(), expiry, expirationPredicate),
        expiry);
    ;
  }

  return newToken;
}
