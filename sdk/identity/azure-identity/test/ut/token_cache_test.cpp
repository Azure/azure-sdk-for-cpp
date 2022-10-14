// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/token_cache_internals.hpp"

#include <gtest/gtest.h>

using Azure::DateTime;
using Azure::Core::Credentials::AccessToken;
using Azure::Identity::_detail::TokenCache;

using namespace std::chrono_literals;

namespace {
void ClearTokenCache()
{
  TokenCache::Internals::Cache.clear();
  TokenCache::Internals::Expirations.clear();
}
} // namespace

TEST(TokenCache, GetReuseRefresh)
{
  ClearTokenCache();

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 0UL);
  EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;
  auto const Yesterday = Tomorrow - 48h;

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

  AccessToken expiredToken;
  expiredToken.ExpiresOn = Yesterday;
  TokenCache::Internals::Cache[{"A", "B", "C", "D"}].ExpiresOn = Yesterday;
  TokenCache::Internals::Expirations[0].first = expiredToken.ExpiresOn;

  auto const token3 = TokenCache::GetToken("A", "B", "C", "D", [=]() {
    AccessToken result;
    result.Token = "T3";
    result.ExpiresOn = Tomorrow + 1min;
    return result;
  });

  EXPECT_EQ(TokenCache::Internals::Cache.size(), 1UL);
  EXPECT_EQ(TokenCache::Internals::Cache.size(), TokenCache::Internals::Expirations.size());

  EXPECT_EQ(token3.ExpiresOn, Tomorrow + 1min);
  EXPECT_EQ(token3.Token, "T3");
}
