// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/token_cache_internals.hpp"

#include <mutex>

#include <gtest/gtest.h>

using Azure::DateTime;
using Azure::Core::Credentials::AccessToken;
using Azure::Identity::_detail::TokenCache;

using namespace std::chrono_literals;

TEST(TokenCache, KeyComparison)
{
  using Key = TokenCache::Internals::CacheKey;
  Key const key1{"a", "b", "c", "d"};
  EXPECT_FALSE(key1 < key1);

  {
    Key const key1dup{"a", "b", "c", "d"};

    EXPECT_FALSE(key1 < key1dup);
    EXPECT_FALSE(key1dup < key1);
  }

  Key const key2{"a", "b", "c", "~"};
  Key const key3{"a", "b", "~", "d"};
  Key const key4{"a", "~", "c", "d"};
  Key const key5{"~", "b", "c", "d"};

  EXPECT_TRUE(key1 < key2);
  EXPECT_TRUE(key1 < key3);
  EXPECT_TRUE(key1 < key4);
  EXPECT_TRUE(key1 < key5);
  EXPECT_FALSE(key2 < key1);
  EXPECT_FALSE(key3 < key1);
  EXPECT_FALSE(key4 < key1);
  EXPECT_FALSE(key5 < key1);

  EXPECT_TRUE(key2 < key3);
  EXPECT_TRUE(key2 < key4);
  EXPECT_TRUE(key2 < key5);
  EXPECT_FALSE(key3 < key2);
  EXPECT_FALSE(key4 < key2);
  EXPECT_FALSE(key5 < key2);

  EXPECT_TRUE(key3 < key4);
  EXPECT_TRUE(key3 < key5);
  EXPECT_FALSE(key4 < key3);
  EXPECT_FALSE(key5 < key3);

  EXPECT_TRUE(key4 < key5);
  EXPECT_FALSE(key5 < key4);
}

TEST(TokenCache, GetReuseRefresh)
{
  TokenCache::Clear();

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 0UL);

  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;
  auto const Yesterday = Tomorrow - 48h;

  {
    auto const token1 = TokenCache::GetToken("A", "B", "C", "D", 3min, [=]() {
      AccessToken result;
      result.Token = "T1";
      result.ExpiresOn = Tomorrow;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);

    EXPECT_EQ(token1.ExpiresOn, Tomorrow);
    EXPECT_EQ(token1.Token, "T1");

    auto const token2 = TokenCache::GetToken("A", "B", "C", "D", 3min, [=]() {
      EXPECT_FALSE("getNewToken does not get invoked when the existing cache value is good");
      AccessToken result;
      result.Token = "T2";
      result.ExpiresOn = Tomorrow + 24h;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);

    EXPECT_EQ(token1.ExpiresOn, token2.ExpiresOn);
    EXPECT_EQ(token1.Token, token2.Token);
  }

  {
    TokenCache::Internals::Cache[{"A", "B", "C", "D"}]->AccessToken.ExpiresOn = Yesterday;

    auto const token = TokenCache::GetToken("A", "B", "C", "D", 3min, [=]() {
      AccessToken result;
      result.Token = "T3";
      result.ExpiresOn = Tomorrow + 1min;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);

    EXPECT_EQ(token.ExpiresOn, Tomorrow + 1min);
    EXPECT_EQ(token.Token, "T3");
  }
}

TEST(TokenCache, TwoThreadsAttemptToInsertTheSameKey)
{
  TokenCache::Clear();

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 0UL);

  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;

  TokenCache::Internals::OnBeforeCacheWriteLock = [=]() {
    TokenCache::Internals::OnBeforeCacheWriteLock = nullptr;
    static_cast<void>(TokenCache::GetToken("A", "B", "C", "D", 3min, [=]() {
      AccessToken result;
      result.Token = "T1";
      result.ExpiresOn = Tomorrow;
      return result;
    }));
  };

  auto const token = TokenCache::GetToken("A", "B", "C", "D", 3min, [=]() {
    EXPECT_FALSE("getNewToken does not get invoked when the fresh value was inserted just before "
                 "acquiring cache write lock");
    AccessToken result;
    result.Token = "T2";
    result.ExpiresOn = Tomorrow + 1min;
    return result;
  });

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);

  EXPECT_EQ(token.ExpiresOn, Tomorrow);
  EXPECT_EQ(token.Token, "T1");
}

TEST(TokenCache, TwoThreadsAttemptToUpdateTheSameToken)
{
  TokenCache::Clear();

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 0UL);

  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;
  auto const Yesterday = Tomorrow - 48h;

  {
    TokenCache::Internals::OnBeforeItemWriteLock = [=]() {
      TokenCache::Internals::OnBeforeItemWriteLock = nullptr;
      auto const item = TokenCache::Internals::Cache[{"A", "B", "C", "D"}];
      item->AccessToken.Token = "T1";
      item->AccessToken.ExpiresOn = Tomorrow;
    };

    auto const token = TokenCache::GetToken("A", "B", "C", "D", 3min, [=]() {
      EXPECT_FALSE("getNewToken does not get invoked when the fresh value was inserted just before "
                   "acquiring item write lock");
      AccessToken result;
      result.Token = "T2";
      result.ExpiresOn = Tomorrow + 1min;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);

    EXPECT_EQ(token.ExpiresOn, Tomorrow);
    EXPECT_EQ(token.Token, "T1");
  }

  // Same as above, but the token that was inserted is already expired.
  {
    TokenCache::Clear();

    TokenCache::Internals::OnBeforeItemWriteLock = [=]() {
      TokenCache::Internals::OnBeforeItemWriteLock = nullptr;
      auto const item = TokenCache::Internals::Cache[{"A", "B", "C", "D"}];
      item->AccessToken.Token = "T3";
      item->AccessToken.ExpiresOn = Yesterday;
    };

    auto const token = TokenCache::GetToken("A", "B", "C", "D", 3min, [=]() {
      AccessToken result;
      result.Token = "T4";
      result.ExpiresOn = Tomorrow + 3min;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);

    EXPECT_EQ(token.ExpiresOn, Tomorrow + 3min);
    EXPECT_EQ(token.Token, "T4");
  }
}

TEST(TokenCache, ExpiredCleanup)
{
  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;
  auto const Yesterday = Tomorrow - 48h;

  TokenCache::Clear();
  EXPECT_EQ(TokenCache::Internals::Cache.size(), 0UL);

  for (auto i = 1; i <= 65; ++i)
  {
    auto const n = std::to_string(i);
    static_cast<void>(TokenCache::GetToken(n, n, n, n, 3min, [=]() {
      AccessToken result;
      result.Token = "T1";
      result.ExpiresOn = Tomorrow;
      return result;
    }));
  }

  // Simply: we added 64+1 token, none of them has expired. None are expected to be cleaned up.
  EXPECT_EQ(TokenCache::Internals::Cache.size(), 65UL);

  // Let's expire 3 of them, with numbers from 1 to 3.
  for (auto i = 1; i <= 3; ++i)
  {
    auto const n = std::to_string(i);
    TokenCache::Internals::Cache[{n, n, n, n}]->AccessToken.ExpiresOn = Yesterday;
  }

  // Add tokens up to 128 total. When 129th gets added, clean up should get triggered.
  for (auto i = 66; i <= 128; ++i)
  {
    auto const n = std::to_string(i);
    static_cast<void>(TokenCache::GetToken(n, n, n, n, 3min, [=]() {
      AccessToken result;
      result.Token = "T1";
      result.ExpiresOn = Tomorrow;
      return result;
    }));
  }

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 128UL);

  // Count is at 128. Tokens from 1 to 3 are still in cache even though they are expired.
  for (auto i = 1; i <= 3; ++i)
  {
    auto const n = std::to_string(i);
    EXPECT_NE(TokenCache::Internals::Cache.find({n, n, n, n}), TokenCache::Internals::Cache.end());
  }

  // One more addition to the cache and cleanup for the expired ones will get triggered.
  static_cast<void>(TokenCache::GetToken("129", "129", "129", "129", 3min, [=]() {
    AccessToken result;
    result.Token = "T1";
    result.ExpiresOn = Tomorrow;
    return result;
  }));

  // We were at 128 before we added 1 more, and now we're at 126. 3 were deleted, 1 was added.
  EXPECT_EQ(TokenCache::Internals::Cache.size(), 126UL);

  // Items from 1 to 3 should no longer be in the cache.
  for (auto i = 1; i <= 3; ++i)
  {
    auto const n = std::to_string(i);
    EXPECT_EQ(TokenCache::Internals::Cache.find({n, n, n, n}), TokenCache::Internals::Cache.end());
  }

  // Let's expire items from 21 all the way up to 129.
  for (auto i = 21; i <= 129; ++i)
  {
    auto const n = std::to_string(i);
    TokenCache::Internals::Cache[{n, n, n, n}]->AccessToken.ExpiresOn = Yesterday;
  }

  // Re-add items 2 and 3. Adding them should not trigger cleanup. After adding, cache should get to
  // 128 items (with numbers from 2 to 129, and number 1 missing).
  for (auto i = 2; i <= 3; ++i)
  {
    auto const n = std::to_string(i);
    static_cast<void>(TokenCache::GetToken(n, n, n, n, 3min, [=]() {
      AccessToken result;
      result.Token = "T2";
      result.ExpiresOn = Tomorrow;
      return result;
    }));
  }

  // Cache is now at 128 again (items from 2 to 129). Adding 1 more will trigger cleanup.
  EXPECT_EQ(TokenCache::Internals::Cache.size(), 128UL);

  // Now let's lock some of the items for reading, and some for writing. Cleanup should not block on
  // token release, but will simply move on, without doing anything to the ones that were locked.
  // Out of 4 locked, two are expired, so they should get cleared under normla circumstances, but
  // this time they will remain in the cache.
  std::shared_lock<std::shared_timed_mutex> readLockForUnexpired(
      TokenCache::Internals::Cache[{"2", "2", "2", "2"}]->ElementMutex);

  std::shared_lock<std::shared_timed_mutex> readLockForExpired(
      TokenCache::Internals::Cache[{"127", "127", "127", "127"}]->ElementMutex);

  std::unique_lock<std::shared_timed_mutex> writeLockForUnexpired(
      TokenCache::Internals::Cache[{"3", "3", "3", "3"}]->ElementMutex);

  std::unique_lock<std::shared_timed_mutex> writeLockForExpired(
      TokenCache::Internals::Cache[{"128", "128", "128", "128"}]->ElementMutex);

  // Count is at 128. Inserting the 129th element, and it will trigger cleanup.
  static_cast<void>(TokenCache::GetToken("1", "1", "1", "1", 3min, [=]() {
    AccessToken result;
    result.Token = "T2";
    result.ExpiresOn = Tomorrow;
    return result;
  }));

  // These should be 20 unexpired items + two that are expired but were locked, so 22 total.
  EXPECT_EQ(TokenCache::Internals::Cache.size(), 22UL);

  for (auto i = 1; i <= 20; ++i)
  {
    auto const n = std::to_string(i);
    EXPECT_NE(TokenCache::Internals::Cache.find({n, n, n, n}), TokenCache::Internals::Cache.end());
  }

  EXPECT_NE(
      TokenCache::Internals::Cache.find({"127", "127", "127", "127"}),
      TokenCache::Internals::Cache.end());

  EXPECT_NE(
      TokenCache::Internals::Cache.find({"128", "128", "128", "128"}),
      TokenCache::Internals::Cache.end());

  for (auto i = 21; i <= 126; ++i)
  {
    auto const n = std::to_string(i);
    EXPECT_EQ(TokenCache::Internals::Cache.find({n, n, n, n}), TokenCache::Internals::Cache.end());
  }
}

TEST(TokenCache, MinimumExpiration)
{
  TokenCache::Clear();

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 0UL);

  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;

  auto const token1 = TokenCache::GetToken("A", "B", "C", "D", 3min, [=]() {
    AccessToken result;
    result.Token = "T1";
    result.ExpiresOn = Tomorrow;
    return result;
  });

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);

  EXPECT_EQ(token1.ExpiresOn, Tomorrow);
  EXPECT_EQ(token1.Token, "T1");

  auto const token2 = TokenCache::GetToken("A", "B", "C", "D", 24h, [=]() {
    AccessToken result;
    result.Token = "T2";
    result.ExpiresOn = Tomorrow + 1h;
    return result;
  });

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);

  EXPECT_EQ(token2.ExpiresOn, Tomorrow + 1h);
  EXPECT_EQ(token2.Token, "T2");
}

TEST(TokenCache, MultithreadedAccess)
{
  TokenCache::Clear();

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 0UL);

  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;

  auto const token1 = TokenCache::GetToken("A", "B", "C", "D", 3min, [=]() {
    AccessToken result;
    result.Token = "T1";
    result.ExpiresOn = Tomorrow;
    return result;
  });

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);

  EXPECT_EQ(token1.ExpiresOn, Tomorrow);
  EXPECT_EQ(token1.Token, "T1");

  {
    std::shared_lock<std::shared_timed_mutex> itemReadLock(
        TokenCache::Internals::Cache[{"A", "B", "C", "D"}]->ElementMutex);

    {
      std::shared_lock<std::shared_timed_mutex> cacheReadLock(TokenCache::Internals::CacheMutex);

      // Parallel threads read both the container and the item we're accessing, and we can access it
      // in parallel as well.
      auto const token2 = TokenCache::GetToken("A", "B", "C", "D", 3min, [=]() {
        EXPECT_FALSE("getNewToken does not get invoked when the existing cache value is good");
        AccessToken result;
        result.Token = "T2";
        result.ExpiresOn = Tomorrow + 1h;
        return result;
      });

      EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);

      EXPECT_EQ(token2.ExpiresOn, token1.ExpiresOn);
      EXPECT_EQ(token2.Token, token1.Token);
    }

    // The cache is unlocked, but one item is being read in a parallel thread, which does not
    // prevent new items (with different key) from being appended to cache.
    auto const token3 = TokenCache::GetToken("E", "F", "G", "H", 3min, [=]() {
      AccessToken result;
      result.Token = "T3";
      result.ExpiresOn = Tomorrow + 2h;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 2UL);

    EXPECT_EQ(token3.ExpiresOn, Tomorrow + 2h);
    EXPECT_EQ(token3.Token, "T3");
  }

  {
    std::unique_lock<std::shared_timed_mutex> itemWriteLock(
        TokenCache::Internals::Cache[{"A", "B", "C", "D"}]->ElementMutex);

    // The cache is unlocked, but one item is being written in a parallel thread, which does not
    // prevent new items (with different key) from being appended to cache.
    auto const token3 = TokenCache::GetToken("I", "J", "K", "L", 3min, [=]() {
      AccessToken result;
      result.Token = "T4";
      result.ExpiresOn = Tomorrow + 3h;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 3UL);

    EXPECT_EQ(token3.ExpiresOn, Tomorrow + 3h);
    EXPECT_EQ(token3.Token, "T4");
  }
}
