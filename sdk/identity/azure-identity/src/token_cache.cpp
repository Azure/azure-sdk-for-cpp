// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/token_cache.hpp"
#include "private/token_cache_internals.hpp"

#include <chrono>
#include <mutex>

using Azure::Identity::_detail::TokenCache;

using Azure::Core::Credentials::AccessToken;

decltype(TokenCache::Internals::Cache) TokenCache::Internals::Cache;
decltype(TokenCache::Internals::CacheMutex) TokenCache::Internals::CacheMutex;

#if defined(TESTING_BUILD)
std::function<void()> TokenCache::Internals::OnBeforeCacheWriteLock;
std::function<void()> TokenCache::Internals::OnBeforeItemWriteLock;
#endif

namespace {
bool IsFresh(
    std::shared_ptr<TokenCache::Internals::CacheValue> const& item,
    std::chrono::system_clock::time_point now)
{
  // If cached token expires in less than MinExpiry from now, it's cached value won't be returned,
  // newer value will be requested.
  constexpr auto MinExpiry = std::chrono::minutes(3);

  return item->AccessToken.ExpiresOn > now + MinExpiry;
}

std::shared_ptr<TokenCache::Internals::CacheValue> GetOrCreateValue(
    TokenCache::Internals::CacheKey const key)
{
  {
    std::shared_lock<std::shared_timed_mutex> cacheReadLock(TokenCache::Internals::CacheMutex);

    auto const found = TokenCache::Internals::Cache.find(key);
    if (found != TokenCache::Internals::Cache.end())
    {
      return found->second;
    }
  }

#if defined(TESTING_BUILD)
  if (TokenCache::Internals::OnBeforeCacheWriteLock != nullptr)
  {
    TokenCache::Internals::OnBeforeCacheWriteLock();
  }
#endif

  std::unique_lock<std::shared_timed_mutex> cacheWriteLock(TokenCache::Internals::CacheMutex);

  // Search cache for the second time, in case the item was inserted between releasing the read lock
  // and acquiring the write lock.
  auto const found = TokenCache::Internals::Cache.find(key);
  if (found != TokenCache::Internals::Cache.end())
  {
    return found->second;
  }

  // Clean up cache from expired items (once every N insertions).
  {
    auto const cacheSize = TokenCache::Internals::Cache.size();

    // N: cacheSize (before insertion) is >= 32 and is a power of two.
    if (cacheSize >= 32 && (cacheSize & (cacheSize - 1)) == 0)
    {
      auto now = std::chrono::system_clock::now();

      auto iter = TokenCache::Internals::Cache.begin();
      while (iter != TokenCache::Internals::Cache.end())
      {
        // Should we end up erasing the element, iterator to current will become invalid, after
        // which we can't increment it. So we copy current, and safely advance the loop iterator.
        auto const curr = iter;
        ++iter;

        // We will try to obtain a write lock, but in a non-blocking way. We only lock it if no one
        // was holding it for read and write at a time. If it's busy in any way, we don't wait, but
        // move on.
        auto const item = curr->second;
        {
          std::unique_lock<std::shared_timed_mutex> lock(item->ElementMutex, std::defer_lock);
          if (lock.try_lock() && !IsFresh(item, now))
          {
            TokenCache::Internals::Cache.erase(curr);
          }
        }
      }
    }
  }

  // Insert the blank value value and return it.
  return TokenCache::Internals::Cache[key] = std::make_shared<TokenCache::Internals::CacheValue>();
}
} // namespace

AccessToken TokenCache::GetToken(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& authorityHost,
    std::string const& scopes,
    std::function<AccessToken()> const& getNewToken)
{
  auto const item = GetOrCreateValue({tenantId, clientId, authorityHost, scopes});

  {
    std::shared_lock<std::shared_timed_mutex> itemReadLock(item->ElementMutex);

    if (IsFresh(item, std::chrono::system_clock::now()))
    {
      return item->AccessToken;
    }
  }

  {
#if defined(TESTING_BUILD)
    if (TokenCache::Internals::OnBeforeItemWriteLock != nullptr)
    {
      TokenCache::Internals::OnBeforeItemWriteLock();
    }
#endif

    std::unique_lock<std::shared_timed_mutex> itemWriteLock(item->ElementMutex);

    // Check the expiration for the second time, in case it just got updated, after releasing the
    // itemReadLock, and before acquiring itemWriteLock.
    if (IsFresh(item, std::chrono::system_clock::now()))
    {
      return item->AccessToken;
    }

    auto const newToken = getNewToken();
    item->AccessToken = newToken;
    return newToken;
  }
}

#if defined(TESTING_BUILD)
void TokenCache::Clear()
{
  std::unique_lock<std::shared_timed_mutex> cacheWriteLock(TokenCache::Internals::CacheMutex);
  Internals::Cache.clear();
}
#endif
