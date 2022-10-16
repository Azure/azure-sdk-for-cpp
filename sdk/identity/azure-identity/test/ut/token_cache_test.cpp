// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/token_cache_internals.hpp"

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
  EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;
  auto const Yesterday = Tomorrow - 48h;

  {
    auto const token1 = TokenCache::GetToken("A", "B", "C", "D", [=]() {
      AccessToken result;
      result.Token = "T1";
      result.ExpiresOn = Tomorrow;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);
    EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

    EXPECT_EQ(token1.ExpiresOn, Tomorrow);
    EXPECT_EQ(token1.Token, "T1");

    auto const token2 = TokenCache::GetToken("A", "B", "C", "D", [=]() {
      EXPECT_FALSE("getNewToken does not get invoked when the existing cache value is good");
      AccessToken result;
      result.Token = "T2";
      result.ExpiresOn = Tomorrow + 24h;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);
    EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

    EXPECT_EQ(token1.ExpiresOn, token2.ExpiresOn);
    EXPECT_EQ(token1.Token, token2.Token);
  }

  {
    TokenCache::Internals::Cache[{"A", "B", "C", "D"}].ExpiresOn = Yesterday;
    TokenCache::Internals::Expirations[0].first = Yesterday;

    auto const token = TokenCache::GetToken("A", "B", "C", "D", [=]() {
      AccessToken result;
      result.Token = "T3";
      result.ExpiresOn = Tomorrow + 1min;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);
    EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

    EXPECT_EQ(token.ExpiresOn, Tomorrow + 1min);
    EXPECT_EQ(token.Token, "T3");
  }
}

TEST(TokenCache, CleanupExpired)
{
  TokenCache::Clear();

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 0UL);
  EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;
  auto const Yesterday = Tomorrow - 48h;

  {
    auto const token = TokenCache::GetToken("AAA", "AAA", "AAA", "AAA", [=]() {
      AccessToken result;
      result.Token = "T1";
      result.ExpiresOn = Tomorrow;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);
    EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

    EXPECT_EQ(token.ExpiresOn, Tomorrow);
    EXPECT_EQ(token.Token, "T1");
  }

  {
    auto const token = TokenCache::GetToken("BBB", "BBB", "BBB", "BBB", [=]() {
      AccessToken result;
      result.Token = "T2";
      result.ExpiresOn = Tomorrow + 1min;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 2UL);
    EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

    EXPECT_EQ(token.ExpiresOn, Tomorrow + 1min);
    EXPECT_EQ(token.Token, "T2");
  }

  {
    auto const token = TokenCache::GetToken("ccc", "ccc", "ccc", "ccc", [=]() {
      AccessToken result;
      result.Token = "T3";
      result.ExpiresOn = Tomorrow - 1min;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 3UL);
    EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

    EXPECT_EQ(token.ExpiresOn, Tomorrow - 1min);
    EXPECT_EQ(token.Token, "T3");
  }

  {
    auto const token = TokenCache::GetToken("ddd", "ddd", "ddd", "ddd", [=]() {
      AccessToken result;
      result.Token = "T4";
      result.ExpiresOn = Tomorrow - 2min;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 4UL);
    EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

    EXPECT_EQ(token.ExpiresOn, Tomorrow - 2min);
    EXPECT_EQ(token.Token, "T4");
  }

  {
    EXPECT_EQ(TokenCache::Internals::Expirations[0].second.TenantId, "ddd");
    EXPECT_EQ(TokenCache::Internals::Expirations[1].second.TenantId, "ccc");
    EXPECT_EQ(TokenCache::Internals::Expirations[2].second.TenantId, "AAA");
    EXPECT_EQ(TokenCache::Internals::Expirations[3].second.TenantId, "BBB");

    EXPECT_EQ(TokenCache::Internals::Expirations[0].first, Tomorrow - 2min);
    EXPECT_EQ(TokenCache::Internals::Expirations[1].first, Tomorrow - 1min);
    EXPECT_EQ(TokenCache::Internals::Expirations[2].first, Tomorrow);
    EXPECT_EQ(TokenCache::Internals::Expirations[3].first, Tomorrow + 1min);

    TokenCache::Internals::Cache[{"ddd", "ddd", "ddd", "ddd"}].ExpiresOn = Yesterday - 1min;
    TokenCache::Internals::Expirations[0].first = Yesterday - 1min;

    TokenCache::Internals::Cache[{"ccc", "ccc", "ccc", "ccc"}].ExpiresOn = Yesterday;
    TokenCache::Internals::Expirations[1].first = Yesterday;
  }

  // Getting cached nonexpiring token does not trigger cache cleanup.
  {
    auto const token = TokenCache::GetToken("AAA", "AAA", "AAA", "AAA", [=]() {
      AccessToken result;
      result.Token = "T5";
      result.ExpiresOn = Tomorrow + 2min;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 4UL);
    EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

    EXPECT_EQ(token.ExpiresOn, Tomorrow);
    EXPECT_EQ(token.Token, "T1");
  }

  // Getting an expired token does clean up the cache.
  {
    auto const token = TokenCache::GetToken("ccc", "ccc", "ccc", "ccc", [=]() {
      AccessToken result;
      result.Token = "T6";
      result.ExpiresOn = Tomorrow + 3min;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 3UL);
    EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

    EXPECT_EQ(token.ExpiresOn, Tomorrow + 3min);
    EXPECT_EQ(token.Token, "T6");

    EXPECT_EQ(TokenCache::Internals::Expirations[0].second.TenantId, "AAA");
    EXPECT_EQ(TokenCache::Internals::Expirations[1].second.TenantId, "BBB");
    EXPECT_EQ(TokenCache::Internals::Expirations[2].second.TenantId, "ccc");

    EXPECT_EQ(TokenCache::Internals::Expirations[0].first, Tomorrow);
    EXPECT_EQ(TokenCache::Internals::Expirations[1].first, Tomorrow + 1min);
    EXPECT_EQ(TokenCache::Internals::Expirations[2].first, Tomorrow + 3min);
  }

  // Appending new token to the cache also does trigger cache cleanup.
  {
    // Prepare: expire cache items
    {
      TokenCache::Internals::Cache[{"AAA", "AAA", "AAA", "AAA"}].ExpiresOn = Yesterday - 3min;
      TokenCache::Internals::Expirations[0].first = Yesterday - 3min;

      TokenCache::Internals::Cache[{"BBB", "BBB", "BBB", "BBB"}].ExpiresOn = Yesterday - 2min;
      TokenCache::Internals::Expirations[1].first = Yesterday - 2min;
    }

    auto const token = TokenCache::GetToken("EeE", "EeE", "EeE", "EeE", [=]() {
      AccessToken result;
      result.Token = "T7";
      result.ExpiresOn = Tomorrow + 4min;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 2UL);
    EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

    EXPECT_EQ(token.ExpiresOn, Tomorrow + 4min);
    EXPECT_EQ(token.Token, "T7");

    EXPECT_EQ(TokenCache::Internals::Expirations[0].second.TenantId, "ccc");
    EXPECT_EQ(TokenCache::Internals::Expirations[1].second.TenantId, "EeE");

    EXPECT_EQ(TokenCache::Internals::Expirations[0].first, Tomorrow + 3min);
    EXPECT_EQ(TokenCache::Internals::Expirations[1].first, Tomorrow + 4min);
  }
}

TEST(TokenCache, WasUpdatedBeforeWriteLock)
{
  TokenCache::Clear();

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 0UL);
  EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;
  auto const Yesterday = Tomorrow - 48h;

  {
    TokenCache::Internals::OnBeforeWriteLock = [=]() {
      TokenCache::Internals::OnBeforeWriteLock = nullptr;
      static_cast<void>(TokenCache::GetToken("A", "B", "C", "D", [=]() {
        AccessToken result;
        result.Token = "T1";
        result.ExpiresOn = Tomorrow;
        return result;
      }));
    };

    auto const token = TokenCache::GetToken("A", "B", "C", "D", [=]() {
      EXPECT_FALSE("getNewToken does not get invoked when the fresh value was inserted just before "
                   "acquiring write lock");
      AccessToken result;
      result.Token = "T2";
      result.ExpiresOn = Tomorrow + 1min;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);
    EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

    EXPECT_EQ(token.ExpiresOn, Tomorrow);
    EXPECT_EQ(token.Token, "T1");
  }

  // Same as above, but the token that was inserted is already expired.
  {
    TokenCache::Clear();

    TokenCache::Internals::OnBeforeWriteLock = [=]() {
      TokenCache::Internals::OnBeforeWriteLock = nullptr;
      static_cast<void>(TokenCache::GetToken("A", "B", "C", "D", [=]() {
        AccessToken result;
        result.Token = "T3";
        result.ExpiresOn = Tomorrow + 2min;
        return result;
      }));

      TokenCache::Internals::Cache[{"A", "B", "C", "D"}].ExpiresOn = Yesterday;
      TokenCache::Internals::Expirations[0].first = Yesterday;
    };

    auto const token = TokenCache::GetToken("A", "B", "C", "D", [=]() {
      AccessToken result;
      result.Token = "T4";
      result.ExpiresOn = Tomorrow + 3min;
      return result;
    });

    EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);
    EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

    EXPECT_EQ(token.ExpiresOn, Tomorrow + 3min);
    EXPECT_EQ(token.Token, "T4");
  }
}
