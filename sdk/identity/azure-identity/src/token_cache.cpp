// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/detail/token_cache.hpp"

#include <mutex>

using Azure::Identity::_detail::TokenCache;

using Azure::DateTime;
using Azure::Core::Credentials::AccessToken;

bool TokenCache::IsFresh(
    std::shared_ptr<TokenCache::CacheValue> const& item,
    DateTime::duration minimumExpiration,
    std::chrono::system_clock::time_point now)
{
  return item->AccessToken.ExpiresOn > (DateTime(now) + minimumExpiration);
}

std::shared_ptr<TokenCache::CacheValue> TokenCache::GetOrCreateValue(
    std::string const& key,
    DateTime::duration minimumExpiration) const
{
  {
    std::shared_lock<std::shared_timed_mutex> cacheReadLock(m_cacheMutex);

    auto const found = m_cache.find(key);
    if (found != TokenCache::m_cache.end())
    {
      return found->second;
    }
  }

#if defined(TESTING_BUILD)
  if (m_onBeforeCacheWriteLock != nullptr)
  {
    m_onBeforeCacheWriteLock();
  }
#endif

  std::unique_lock<std::shared_timed_mutex> cacheWriteLock(m_cacheMutex);

  // Search cache for the second time, in case the item was inserted between releasing the read lock
  // and acquiring the write lock.
  auto const found = m_cache.find(key);
  if (found != m_cache.end())
  {
    return found->second;
  }

  // Clean up cache from expired items (once every N insertions).
  {
    auto const cacheSize = m_cache.size();

    // N: cacheSize (before insertion) is >= 32 and is a power of two.
    // 32 as a starting point does not have any special meaning.
    //
    // Power of 2 trick:
    // https://www.exploringbinary.com/ten-ways-to-check-if-an-integer-is-a-power-of-two-in-c/

    if (cacheSize >= 32 && (cacheSize & (cacheSize - 1)) == 0)
    {
      auto now = std::chrono::system_clock::now();

      auto iter = m_cache.begin();
      while (iter != m_cache.end())
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
          if (lock.try_lock() && !IsFresh(item, minimumExpiration, now))
          {
            m_cache.erase(curr);
          }
        }
      }
    }
  }

  // Insert the blank value value and return it.
  return m_cache[key] = std::make_shared<CacheValue>();
}

AccessToken TokenCache::GetToken(
    std::string const& scopeString,
    DateTime::duration minimumExpiration,
    std::function<AccessToken()> const& getNewToken) const
{
  auto const item = GetOrCreateValue(scopeString, minimumExpiration);

  {
    std::shared_lock<std::shared_timed_mutex> itemReadLock(item->ElementMutex);

    if (IsFresh(item, minimumExpiration, std::chrono::system_clock::now()))
    {
      return item->AccessToken;
    }
  }

#if defined(TESTING_BUILD)
  if (m_onBeforeItemWriteLock != nullptr)
  {
    m_onBeforeItemWriteLock();
  }
#endif

  std::unique_lock<std::shared_timed_mutex> itemWriteLock(item->ElementMutex);

  // Check the expiration for the second time, in case it just got updated, after releasing the
  // itemReadLock, and before acquiring itemWriteLock.
  if (IsFresh(item, minimumExpiration, std::chrono::system_clock::now()))
  {
    return item->AccessToken;
  }

  auto const newToken = getNewToken();
  item->AccessToken = newToken;
  return newToken;
}
