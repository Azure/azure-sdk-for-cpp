// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_secret_credential.hpp"
#include "azure/identity/detail/token_cache.hpp"

#include <mutex>

#include <gtest/gtest.h>

using Azure::DateTime;
using Azure::Core::Credentials::AccessToken;
using Azure::Identity::_detail::TokenCache;

namespace {
class TestableTokenCache final : public TokenCache {
public:
  using TokenCache::CacheValue;
  using TokenCache::m_cache;
  using TokenCache::m_cacheMutex;

  mutable std::function<void()> m_onBeforeCacheWriteLock;
  mutable std::function<void()> m_onBeforeItemWriteLock;

  void OnBeforeCacheWriteLock() const override
  {
    if (m_onBeforeCacheWriteLock != nullptr)
    {
      m_onBeforeCacheWriteLock();
    }
  }

  void OnBeforeItemWriteLock() const override
  {
    if (m_onBeforeItemWriteLock != nullptr)
    {
      m_onBeforeItemWriteLock();
    }
  }
};
} // namespace

using namespace std::chrono_literals;

TEST(TokenCache, GetReuseRefresh)
{
  TestableTokenCache tokenCache;

  EXPECT_EQ(tokenCache.m_cache.size(), 0UL);

  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;
  auto const Yesterday = Tomorrow - 48h;

  {
    auto const token1 = tokenCache.GetToken("A", {}, 2min, [=]() {
      AccessToken result;
      result.Token = "T1";
      result.ExpiresOn = Tomorrow;
      return result;
    });

    EXPECT_EQ(tokenCache.m_cache.size(), 1UL);

    EXPECT_EQ(token1.ExpiresOn, Tomorrow);
    EXPECT_EQ(token1.Token, "T1");

    auto const token2 = tokenCache.GetToken("A", {}, 2min, [=]() {
      EXPECT_FALSE("getNewToken does not get invoked when the existing cache value is good");
      AccessToken result;
      result.Token = "T2";
      result.ExpiresOn = Tomorrow + 24h;
      return result;
    });

    EXPECT_EQ(tokenCache.m_cache.size(), 1UL);

    EXPECT_EQ(token1.ExpiresOn, token2.ExpiresOn);
    EXPECT_EQ(token1.Token, token2.Token);
  }

  {
    tokenCache.m_cache[{"A", {}}]->AccessToken.ExpiresOn = Yesterday;

    auto const token = tokenCache.GetToken("A", {}, 2min, [=]() {
      AccessToken result;
      result.Token = "T3";
      result.ExpiresOn = Tomorrow + 1min;
      return result;
    });

    EXPECT_EQ(tokenCache.m_cache.size(), 1UL);

    EXPECT_EQ(token.ExpiresOn, Tomorrow + 1min);
    EXPECT_EQ(token.Token, "T3");
  }
}

TEST(TokenCache, TwoThreadsAttemptToInsertTheSameKey)
{
  TestableTokenCache tokenCache;

  EXPECT_EQ(tokenCache.m_cache.size(), 0UL);

  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;

  tokenCache.m_onBeforeCacheWriteLock = [&]() {
    tokenCache.m_onBeforeCacheWriteLock = nullptr;
    static_cast<void>(tokenCache.GetToken("A", {}, 2min, [=]() {
      AccessToken result;
      result.Token = "T1";
      result.ExpiresOn = Tomorrow;
      return result;
    }));
  };

  auto const token = tokenCache.GetToken("A", {}, 2min, [=]() {
    EXPECT_FALSE("getNewToken does not get invoked when the fresh value was inserted just before "
                 "acquiring cache write lock");
    AccessToken result;
    result.Token = "T2";
    result.ExpiresOn = Tomorrow + 1min;
    return result;
  });

  EXPECT_EQ(tokenCache.m_cache.size(), 1UL);

  EXPECT_EQ(token.ExpiresOn, Tomorrow);
  EXPECT_EQ(token.Token, "T1");
}

TEST(TokenCache, TwoThreadsAttemptToUpdateTheSameToken)
{
  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;
  auto const Yesterday = Tomorrow - 48h;

  {
    TestableTokenCache tokenCache;

    EXPECT_EQ(tokenCache.m_cache.size(), 0UL);

    tokenCache.m_onBeforeItemWriteLock = [&]() {
      tokenCache.m_onBeforeItemWriteLock = nullptr;
      auto const item = tokenCache.m_cache[{"A", {}}];
      item->AccessToken.Token = "T1";
      item->AccessToken.ExpiresOn = Tomorrow;
    };

    auto const token = tokenCache.GetToken("A", {}, 2min, [=]() {
      EXPECT_FALSE("getNewToken does not get invoked when the fresh value was inserted just before "
                   "acquiring item write lock");
      AccessToken result;
      result.Token = "T2";
      result.ExpiresOn = Tomorrow + 1min;
      return result;
    });

    EXPECT_EQ(tokenCache.m_cache.size(), 1UL);

    EXPECT_EQ(token.ExpiresOn, Tomorrow);
    EXPECT_EQ(token.Token, "T1");
  }

  // Same as above, but the token that was inserted is already expired.
  {
    TestableTokenCache tokenCache;

    tokenCache.m_onBeforeItemWriteLock = [&]() {
      tokenCache.m_onBeforeItemWriteLock = nullptr;
      auto const item = tokenCache.m_cache[{"A", {}}];
      item->AccessToken.Token = "T3";
      item->AccessToken.ExpiresOn = Yesterday;
    };

    auto const token = tokenCache.GetToken("A", {}, 2min, [=]() {
      AccessToken result;
      result.Token = "T4";
      result.ExpiresOn = Tomorrow + 3min;
      return result;
    });

    EXPECT_EQ(tokenCache.m_cache.size(), 1UL);

    EXPECT_EQ(token.ExpiresOn, Tomorrow + 3min);
    EXPECT_EQ(token.Token, "T4");
  }
}

TEST(TokenCache, ExpiredCleanup)
{
  // Expected cleanup points are when cache size is in the Fibonacci sequence:
  // 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, ...
  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;
  auto const Yesterday = Tomorrow - 48h;

  TestableTokenCache tokenCache;
  EXPECT_EQ(tokenCache.m_cache.size(), 0UL);

  for (auto i = 1; i <= 35; ++i)
  {
    auto const n = std::to_string(i);
    static_cast<void>(tokenCache.GetToken(n, {}, 2min, [=]() {
      AccessToken result;
      result.Token = "T1";
      result.ExpiresOn = Tomorrow;
      return result;
    }));
  }

  // Simply: we added 34+1 token, none of them has expired. None are expected to be cleaned up.
  EXPECT_EQ(tokenCache.m_cache.size(), 35UL);

  // Let's expire 3 of them, with numbers from 1 to 3.
  for (auto i = 1; i <= 3; ++i)
  {
    auto const n = std::to_string(i);
    tokenCache.m_cache[{n, {}}]->AccessToken.ExpiresOn = Yesterday;
  }

  // Add tokens up to 55 total. When 56th gets added, clean up should get triggered.
  for (auto i = 36; i <= 55; ++i)
  {
    auto const n = std::to_string(i);
    static_cast<void>(tokenCache.GetToken(n, {}, 2min, [=]() {
      AccessToken result;
      result.Token = "T1";
      result.ExpiresOn = Tomorrow;
      return result;
    }));
  }

  EXPECT_EQ(tokenCache.m_cache.size(), 55UL);

  // Count is at 55. Tokens from 1 to 3 are still in cache even though they are expired.
  for (auto i = 1; i <= 3; ++i)
  {
    auto const n = std::to_string(i);
    EXPECT_NE(tokenCache.m_cache.find({n, {}}), tokenCache.m_cache.end());
  }

  // One more addition to the cache and cleanup for the expired ones will get triggered.
  static_cast<void>(tokenCache.GetToken("56", {}, 2min, [=]() {
    AccessToken result;
    result.Token = "T1";
    result.ExpiresOn = Tomorrow;
    return result;
  }));

  // We were at 55 before we added 1 more, and now we're at 53. 3 were deleted, 1 was added.
  EXPECT_EQ(tokenCache.m_cache.size(), 53UL);

  // Items from 1 to 3 should no longer be in the cache.
  for (auto i = 1; i <= 3; ++i)
  {
    auto const n = std::to_string(i);
    EXPECT_EQ(tokenCache.m_cache.find({n, {}}), tokenCache.m_cache.end());
  }

  // Let's expire items from 21 all the way up to 56.
  for (auto i = 21; i <= 56; ++i)
  {
    auto const n = std::to_string(i);
    tokenCache.m_cache[{n, {}}]->AccessToken.ExpiresOn = Yesterday;
  }

  // Re-add items 2 and 3. Adding them should not trigger cleanup. After adding, cache should get to
  // 55 items (with numbers from 2 to 56, and number 1 missing).
  for (auto i = 2; i <= 3; ++i)
  {
    auto const n = std::to_string(i);
    static_cast<void>(tokenCache.GetToken(n, {}, 2min, [=]() {
      AccessToken result;
      result.Token = "T2";
      result.ExpiresOn = Tomorrow;
      return result;
    }));
  }

  // Cache is now at 55 again (items from 2 to 56). Adding 1 more will trigger cleanup.
  EXPECT_EQ(tokenCache.m_cache.size(), 55UL);

  // Now let's lock some of the items for reading, and some for writing. Cleanup should not block on
  // token release, but will simply move on, without doing anything to the ones that were locked.
  // Out of 4 locked, two are expired, so they should get cleared under normal circumstances, but
  // this time they will remain in the cache.
  std::shared_lock<std::shared_timed_mutex> readLockForUnexpired(
      tokenCache.m_cache[{"2", {}}]->ElementMutex);

  std::shared_lock<std::shared_timed_mutex> readLockForExpired(
      tokenCache.m_cache[{"54", {}}]->ElementMutex);

  std::unique_lock<std::shared_timed_mutex> writeLockForUnexpired(
      tokenCache.m_cache[{"3", {}}]->ElementMutex);

  std::unique_lock<std::shared_timed_mutex> writeLockForExpired(
      tokenCache.m_cache[{"55", {}}]->ElementMutex);

  // Count is at 55. Inserting the 56th element, and it will trigger cleanup.
  static_cast<void>(tokenCache.GetToken("1", {}, 2min, [=]() {
    AccessToken result;
    result.Token = "T2";
    result.ExpiresOn = Tomorrow;
    return result;
  }));

  // These should be 20 unexpired items + two that are expired but were locked, so 22 total.
  EXPECT_EQ(tokenCache.m_cache.size(), 22UL);

  for (auto i = 1; i <= 20; ++i)
  {
    auto const n = std::to_string(i);
    EXPECT_NE(tokenCache.m_cache.find({n, {}}), tokenCache.m_cache.end());
  }

  EXPECT_NE(tokenCache.m_cache.find({"54", {}}), tokenCache.m_cache.end());

  EXPECT_NE(tokenCache.m_cache.find({"55", {}}), tokenCache.m_cache.end());

  for (auto i = 21; i <= 53; ++i)
  {
    auto const n = std::to_string(i);
    EXPECT_EQ(tokenCache.m_cache.find({n, {}}), tokenCache.m_cache.end());
  }
}

TEST(TokenCache, MinimumExpiration)
{
  TestableTokenCache tokenCache;

  EXPECT_EQ(tokenCache.m_cache.size(), 0UL);

  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;

  auto const token1 = tokenCache.GetToken("A", {}, 2min, [=]() {
    AccessToken result;
    result.Token = "T1";
    result.ExpiresOn = Tomorrow;
    return result;
  });

  EXPECT_EQ(tokenCache.m_cache.size(), 1UL);

  EXPECT_EQ(token1.ExpiresOn, Tomorrow);
  EXPECT_EQ(token1.Token, "T1");

  auto const token2 = tokenCache.GetToken("A", {}, 24h, [=]() {
    AccessToken result;
    result.Token = "T2";
    result.ExpiresOn = Tomorrow + 1h;
    return result;
  });

  EXPECT_EQ(tokenCache.m_cache.size(), 1UL);

  EXPECT_EQ(token2.ExpiresOn, Tomorrow + 1h);
  EXPECT_EQ(token2.Token, "T2");
}

TEST(TokenCache, MultithreadedAccess)
{
  TestableTokenCache tokenCache;

  EXPECT_EQ(tokenCache.m_cache.size(), 0UL);

  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;

  auto const token1 = tokenCache.GetToken("A", {}, 2min, [=]() {
    AccessToken result;
    result.Token = "T1";
    result.ExpiresOn = Tomorrow;
    return result;
  });

  EXPECT_EQ(tokenCache.m_cache.size(), 1UL);

  EXPECT_EQ(token1.ExpiresOn, Tomorrow);
  EXPECT_EQ(token1.Token, "T1");

  {
    std::shared_lock<std::shared_timed_mutex> itemReadLock(
        tokenCache.m_cache[{"A", {}}]->ElementMutex);

    {
      std::shared_lock<std::shared_timed_mutex> cacheReadLock(tokenCache.m_cacheMutex);

      // Parallel threads read both the container and the item we're accessing, and we can
      // access it in parallel as well.
      auto const token2 = tokenCache.GetToken("A", {}, 2min, [=]() {
        EXPECT_FALSE("getNewToken does not get invoked when the existing cache value is good");
        AccessToken result;
        result.Token = "T2";
        result.ExpiresOn = Tomorrow + 1h;
        return result;
      });

      EXPECT_EQ(tokenCache.m_cache.size(), 1UL);

      EXPECT_EQ(token2.ExpiresOn, token1.ExpiresOn);
      EXPECT_EQ(token2.Token, token1.Token);
    }

    // The cache is unlocked, but one item is being read in a parallel thread, which does not
    // prevent new items (with different key) from being appended to cache.
    auto const token3 = tokenCache.GetToken("B", {}, 2min, [=]() {
      AccessToken result;
      result.Token = "T3";
      result.ExpiresOn = Tomorrow + 2h;
      return result;
    });

    EXPECT_EQ(tokenCache.m_cache.size(), 2UL);

    EXPECT_EQ(token3.ExpiresOn, Tomorrow + 2h);
    EXPECT_EQ(token3.Token, "T3");
  }

  {
    std::unique_lock<std::shared_timed_mutex> itemWriteLock(
        tokenCache.m_cache[{"A", {}}]->ElementMutex);

    // The cache is unlocked, but one item is being written in a parallel thread, which does not
    // prevent new items (with different key) from being appended to cache.
    auto const token3 = tokenCache.GetToken("C", {}, 2min, [=]() {
      AccessToken result;
      result.Token = "T4";
      result.ExpiresOn = Tomorrow + 3h;
      return result;
    });

    EXPECT_EQ(tokenCache.m_cache.size(), 3UL);

    EXPECT_EQ(token3.ExpiresOn, Tomorrow + 3h);
    EXPECT_EQ(token3.Token, "T4");
  }
}

using Azure::Core::Context;
using Azure::Core::Http::HttpTransport;
using Azure::Core::Http::RawResponse;
using Azure::Core::Http::Request;

namespace {
class TestTransport final : public HttpTransport {
  int m_attemptNumber = 0;
  std::vector<uint8_t> m_responseBuf;

public:
  // Returns token response with 3600 seconds expiration (1 hour), and the value of the
  // client_secret parameter from the body + attempt number as token value.
  std::unique_ptr<RawResponse> Send(Request& request, Context const&) override
  {
    using Azure::Core::Http::HttpStatusCode;
    using Azure::Core::IO::BodyStream;
    using Azure::Core::IO::MemoryBodyStream;

    ++m_attemptNumber;

    std::string clientSecret;
    {
      std::string const ClientSecretStart = "client_secret=";

      auto const reqBodyVec = request.GetBodyStream()->ReadToEnd();
      auto const reqBodyStr = std::string(reqBodyVec.cbegin(), reqBodyVec.cend());

      auto clientSecretStartPos = reqBodyStr.find(ClientSecretStart);
      if (clientSecretStartPos != std::string::npos)
      {
        clientSecretStartPos += ClientSecretStart.size();
        auto const clientSecretEndPos = reqBodyStr.find('&', clientSecretStartPos);

        clientSecret = (clientSecretEndPos == std::string::npos)
            ? reqBodyStr.substr(clientSecretStartPos)
            : reqBodyStr.substr(clientSecretStartPos, clientSecretEndPos - clientSecretStartPos);
      }
    }

    auto const respBodyStr = std::string("{ \"access_token\" : \"") + clientSecret
        + std::to_string(m_attemptNumber) + "\", \"expires_in\" : 3600 }";

    m_responseBuf.assign(respBodyStr.cbegin(), respBodyStr.cend());

    auto resp = std::make_unique<RawResponse>(1, 1, HttpStatusCode::Ok, "OK");
    resp->SetBodyStream(std::make_unique<MemoryBodyStream>(m_responseBuf));
    return resp;
  }
};
} // namespace

TEST(TokenCache, PerCredInstance)
{
  using Azure::Core::Credentials::TokenCredentialOptions;
  using Azure::Core::Credentials::TokenRequestContext;
  using Azure::Identity::ClientSecretCredential;

  TokenRequestContext getCached;
  getCached.Scopes = {"https://vault.azure.net/.default"};
  getCached.MinimumExpiration = 1s;

  TokenCredentialOptions credOptions;
  credOptions.Transport.Transport = std::make_shared<TestTransport>();

  ClientSecretCredential credA("TenantId", "ClientId", "SecretA", credOptions);
  ClientSecretCredential credB("TenantId", "ClientId", "SecretB", credOptions);

  {
    auto const tokenA1 = credA.GetToken(getCached, {}); // Should populate
    EXPECT_EQ(tokenA1.Token, "SecretA1");
  }

  {
    auto const tokenA2 = credA.GetToken(getCached, {}); // Should get previously populated value
    EXPECT_EQ(tokenA2.Token, "SecretA1");
  }

  {
    auto const tokenB = credB.GetToken(getCached, {});
    EXPECT_EQ(
        tokenB.Token,
        "SecretB2"); // if token cache was shared between instances, the value would be
                     // "SecretA1"
  }

  {
    auto const tokenA3 = credA.GetToken(getCached, {}); // Should still get the cached value
    EXPECT_EQ(tokenA3.Token, "SecretA1");
  }

  auto getNew = getCached;
  getNew.MinimumExpiration += 3600s;

  {
    auto const tokenA4 = credA.GetToken(getNew, {}); // Should get the new value
    EXPECT_EQ(tokenA4.Token, "SecretA3");
  }

  {
    auto const tokenA5 = credA.GetToken(getNew, {}); // Should get the new value
    EXPECT_EQ(tokenA5.Token, "SecretA4");
  }

  {
    auto const tokenA6
        = credA.GetToken(getCached, {}); // Should get the cached, recently refreshed value
    EXPECT_EQ(tokenA6.Token, "SecretA4");
  }
}

TEST(TokenCache, TenantId)
{
  TestableTokenCache tokenCache;

  EXPECT_EQ(tokenCache.m_cache.size(), 0UL);

  DateTime const Tomorrow = std::chrono::system_clock::now() + 24h;

  {
    auto const token = tokenCache.GetToken("A", "X", 2min, [=]() {
      AccessToken result;
      result.Token = "AX";
      result.ExpiresOn = Tomorrow;
      return result;
    });

    EXPECT_EQ(tokenCache.m_cache.size(), 1UL);

    EXPECT_EQ(token.ExpiresOn, Tomorrow);
    EXPECT_EQ(token.Token, "AX");
  }

  {
    auto const token = tokenCache.GetToken("B", "X", 2min, [=]() {
      AccessToken result;
      result.Token = "BX";
      result.ExpiresOn = Tomorrow;
      return result;
    });

    EXPECT_EQ(tokenCache.m_cache.size(), 2UL);

    EXPECT_EQ(token.ExpiresOn, Tomorrow);
    EXPECT_EQ(token.Token, "BX");
  }

  {
    auto const token = tokenCache.GetToken("A", "Y", 2min, [=]() {
      AccessToken result;
      result.Token = "AY";
      result.ExpiresOn = Tomorrow;
      return result;
    });

    EXPECT_EQ(tokenCache.m_cache.size(), 3UL);

    EXPECT_EQ(token.ExpiresOn, Tomorrow);
    EXPECT_EQ(token.Token, "AY");
  }

  {
    auto const token = tokenCache.GetToken("B", "Y", 2min, [=]() {
      AccessToken result;
      result.Token = "BY";
      result.ExpiresOn = Tomorrow;
      return result;
    });

    EXPECT_EQ(tokenCache.m_cache.size(), 4UL);

    EXPECT_EQ(token.ExpiresOn, Tomorrow);
    EXPECT_EQ(token.Token, "BY");
  }

  {
    auto const token = tokenCache.GetToken("A", "X", 2min, [=]() {
      EXPECT_FALSE("getNewToken does not get invoked when the existing cache value is good");
      AccessToken result;
      result.Token = "XA";
      result.ExpiresOn = Tomorrow + 24h;
      return result;
    });

    EXPECT_EQ(tokenCache.m_cache.size(), 4UL);

    EXPECT_EQ(token.ExpiresOn, Tomorrow);
    EXPECT_EQ(token.Token, "AX");
  }

  {
    auto const token = tokenCache.GetToken("B", "X", 2min, [=]() {
      EXPECT_FALSE("getNewToken does not get invoked when the existing cache value is good");
      AccessToken result;
      result.Token = "XB";
      result.ExpiresOn = Tomorrow + 24h;
      return result;
    });

    EXPECT_EQ(tokenCache.m_cache.size(), 4UL);

    EXPECT_EQ(token.ExpiresOn, Tomorrow);
    EXPECT_EQ(token.Token, "BX");
  }

  {
    auto const token = tokenCache.GetToken("A", "Y", 2min, [=]() {
      EXPECT_FALSE("getNewToken does not get invoked when the existing cache value is good");
      AccessToken result;
      result.Token = "YA";
      result.ExpiresOn = Tomorrow + 24h;
      return result;
    });

    EXPECT_EQ(tokenCache.m_cache.size(), 4UL);

    EXPECT_EQ(token.ExpiresOn, Tomorrow);
    EXPECT_EQ(token.Token, "AY");
  }

  {
    auto const token = tokenCache.GetToken("B", "Y", 2min, [=]() {
      EXPECT_FALSE("getNewToken does not get invoked when the existing cache value is good");
      AccessToken result;
      result.Token = "YB";
      result.ExpiresOn = Tomorrow + 24h;
      return result;
    });

    EXPECT_EQ(tokenCache.m_cache.size(), 4UL);

    EXPECT_EQ(token.ExpiresOn, Tomorrow);
    EXPECT_EQ(token.Token, "BY");
  }
}
