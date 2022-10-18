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

    EXPECT_EQ(token1.ExpiresOn, token2.ExpiresOn);
    EXPECT_EQ(token1.Token, token2.Token);
  }

  {
    TokenCache::Internals::Cache[{"A", "B", "C", "D"}]->AccessToken.ExpiresOn = Yesterday;

    auto const token = TokenCache::GetToken("A", "B", "C", "D", [=]() {
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
    static_cast<void>(TokenCache::GetToken("A", "B", "C", "D", [=]() {
      AccessToken result;
      result.Token = "T1";
      result.ExpiresOn = Tomorrow;
      return result;
    }));
  };

  auto const token = TokenCache::GetToken("A", "B", "C", "D", [=]() {
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

    auto const token = TokenCache::GetToken("A", "B", "C", "D", [=]() {
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

    auto const token = TokenCache::GetToken("A", "B", "C", "D", [=]() {
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
    static_cast<void>(TokenCache::GetToken(n, n, n, n, [=]() {
      AccessToken result;
      result.Token = "T1";
      result.ExpiresOn = Tomorrow;
      return result;
    }));
  }

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 65UL);

  for (auto i = 1; i <= 3; ++i)
  {
    auto const n = std::to_string(i);
    TokenCache::Internals::Cache[{n, n, n, n}]->AccessToken.ExpiresOn = Yesterday;
  }

  for (auto i = 66; i <= 128; ++i)
  {
    auto const n = std::to_string(i);
    static_cast<void>(TokenCache::GetToken(n, n, n, n, [=]() {
      AccessToken result;
      result.Token = "T1";
      result.ExpiresOn = Tomorrow;
      return result;
    }));
  }

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 128UL);

  for (auto i = 1; i <= 3; ++i)
  {
    auto const n = std::to_string(i);
    EXPECT_NE(TokenCache::Internals::Cache.find({n, n, n, n}), TokenCache::Internals::Cache.end());
  }

  static_cast<void>(TokenCache::GetToken("129", "129", "129", "129", [=]() {
    AccessToken result;
    result.Token = "T1";
    result.ExpiresOn = Tomorrow;
    return result;
  }));

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 126UL);

  for (auto i = 1; i <= 3; ++i)
  {
    auto const n = std::to_string(i);
    EXPECT_EQ(TokenCache::Internals::Cache.find({n, n, n, n}), TokenCache::Internals::Cache.end());
  }

  for (auto i = 2; i <= 3; ++i)
  {
    auto const n = std::to_string(i);
    static_cast<void>(TokenCache::GetToken(n, n, n, n, [=]() {
      AccessToken result;
      result.Token = "T2";
      result.ExpiresOn = Tomorrow;
      return result;
    }));
  }

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 128UL);

  for (auto i = 21; i <= 129; ++i)
  {
    auto const n = std::to_string(i);
    TokenCache::Internals::Cache[{n, n, n, n}]->AccessToken.ExpiresOn = Yesterday;
  }

  static_cast<void>(TokenCache::GetToken("1", "1", "1", "1", [=]() {
    AccessToken result;
    result.Token = "T2";
    result.ExpiresOn = Tomorrow;
    return result;
  }));

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 20UL);

  for (auto i = 1; i <= 20; ++i)
  {
    auto const n = std::to_string(i);
    EXPECT_NE(TokenCache::Internals::Cache.find({n, n, n, n}), TokenCache::Internals::Cache.end());
  }

  for (auto i = 21; i <= 128; ++i)
  {
    auto const n = std::to_string(i);
    EXPECT_EQ(TokenCache::Internals::Cache.find({n, n, n, n}), TokenCache::Internals::Cache.end());
  }
}
