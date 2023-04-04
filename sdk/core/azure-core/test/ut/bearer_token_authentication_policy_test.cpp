// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/http/policies/policy.hpp>

#include <azure/core/credentials/credentials.hpp>
#include <azure/core/internal/http/pipeline.hpp>

#include <gtest/gtest.h>

using Azure::Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy;

using Azure::Core::Context;
using Azure::Core::Url;
using Azure::Core::Credentials::AccessToken;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenCredential;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Http::HttpMethod;
using Azure::Core::Http::HttpStatusCode;
using Azure::Core::Http::RawResponse;
using Azure::Core::Http::Request;
using Azure::Core::Http::_internal::HttpPipeline;
using Azure::Core::Http::Policies::HttpPolicy;
using Azure::Core::Http::Policies::NextHttpPolicy;

namespace {
class TestTokenCredential final : public TokenCredential {
private:
  std::shared_ptr<AccessToken const> m_accessToken;

public:
  explicit TestTokenCredential(std::shared_ptr<AccessToken const> accessToken)
      : TokenCredential("TestTokenCredential"), m_accessToken(accessToken)
  {
  }

  AccessToken GetToken(TokenRequestContext const&, Context const&) const override
  {
    return *m_accessToken;
  }
};

class TestTransportPolicy final : public HttpPolicy {
public:
  std::unique_ptr<RawResponse> Send(Request&, NextHttpPolicy, Context const&) const override
  {
    return std::make_unique<RawResponse>(1, 1, HttpStatusCode::Ok, "TestStatus");
  }

  std::unique_ptr<HttpPolicy> Clone() const override
  {
    return std::make_unique<TestTransportPolicy>(*this);
  }
};
} // namespace

TEST(BearerTokenAuthenticationPolicy, InitialGet)
{
  using namespace std::chrono_literals;
  auto accessToken = std::make_shared<AccessToken>();

  std::vector<std::unique_ptr<HttpPolicy>> policies;

  TokenRequestContext tokenRequestContext;
  tokenRequestContext.Scopes = {"https://microsoft.com/.default"};

  policies.emplace_back(std::make_unique<BearerTokenAuthenticationPolicy>(
      std::make_shared<TestTokenCredential>(accessToken), tokenRequestContext));

  policies.emplace_back(std::make_unique<TestTransportPolicy>());

  HttpPipeline pipeline(policies);

  {
    Request request(HttpMethod::Get, Url("https://www.azure.com"));

    *accessToken = {"ACCESSTOKEN1", std::chrono::system_clock::now() + 1h};

    pipeline.Send(request, Context());

    {
      auto const headers = request.GetHeaders();
      auto const authHeader = headers.find("authorization");
      EXPECT_NE(authHeader, headers.end());
      EXPECT_EQ(authHeader->second, "Bearer ACCESSTOKEN1");
    }
  }
}

TEST(BearerTokenAuthenticationPolicy, ReuseWhileValid)
{
  using namespace std::chrono_literals;
  auto accessToken = std::make_shared<AccessToken>();

  std::vector<std::unique_ptr<HttpPolicy>> policies;

  TokenRequestContext tokenRequestContext;
  tokenRequestContext.Scopes = {"https://microsoft.com/.default"};

  policies.emplace_back(std::make_unique<BearerTokenAuthenticationPolicy>(
      std::make_shared<TestTokenCredential>(accessToken), tokenRequestContext));

  policies.emplace_back(std::make_unique<TestTransportPolicy>());

  HttpPipeline pipeline(policies);

  {
    Request request(HttpMethod::Get, Url("https://www.azure.com"));

    *accessToken = {"ACCESSTOKEN1", std::chrono::system_clock::now() + 5min};

    pipeline.Send(request, Context());
  }

  {
    Request request(HttpMethod::Get, Url("https://www.azure.com"));

    *accessToken = {"ACCESSTOKEN2", std::chrono::system_clock::now() + 1h};

    pipeline.Send(request, Context());

    {
      auto const headers = request.GetHeaders();
      auto const authHeader = headers.find("authorization");
      EXPECT_NE(authHeader, headers.end());
      EXPECT_EQ(authHeader->second, "Bearer ACCESSTOKEN1");
    }
  }
}

TEST(BearerTokenAuthenticationPolicy, RefreshNearExpiry)
{
  using namespace std::chrono_literals;
  auto accessToken = std::make_shared<AccessToken>();

  std::vector<std::unique_ptr<HttpPolicy>> policies;

  TokenRequestContext tokenRequestContext;
  tokenRequestContext.Scopes = {"https://microsoft.com/.default"};

  policies.emplace_back(std::make_unique<BearerTokenAuthenticationPolicy>(
      std::make_shared<TestTokenCredential>(accessToken), tokenRequestContext));

  policies.emplace_back(std::make_unique<TestTransportPolicy>());

  HttpPipeline pipeline(policies);

  {
    Request request(HttpMethod::Get, Url("https://www.azure.com"));

    *accessToken = {"ACCESSTOKEN1", std::chrono::system_clock::now() + 2min};

    pipeline.Send(request, Context());
  }

  {
    Request request(HttpMethod::Get, Url("https://www.azure.com"));

    *accessToken = {"ACCESSTOKEN2", std::chrono::system_clock::now() + 1h};

    pipeline.Send(request, Context());

    {
      auto const headers = request.GetHeaders();
      auto const authHeader = headers.find("authorization");
      EXPECT_NE(authHeader, headers.end());
      EXPECT_EQ(authHeader->second, "Bearer ACCESSTOKEN2");
    }
  }
}

TEST(BearerTokenAuthenticationPolicy, RefreshAfterExpiry)
{
  using namespace std::chrono_literals;
  auto accessToken = std::make_shared<AccessToken>();

  std::vector<std::unique_ptr<HttpPolicy>> policies;

  TokenRequestContext tokenRequestContext;
  tokenRequestContext.Scopes = {"https://microsoft.com/.default"};

  policies.emplace_back(std::make_unique<BearerTokenAuthenticationPolicy>(
      std::make_shared<TestTokenCredential>(accessToken), tokenRequestContext));

  policies.emplace_back(std::make_unique<TestTransportPolicy>());

  HttpPipeline pipeline(policies);

  {
    Request request(HttpMethod::Get, Url("https://www.azure.com"));

    *accessToken = {"ACCESSTOKEN1", std::chrono::system_clock::now()};

    pipeline.Send(request, Context());
  }

  {
    Request request(HttpMethod::Get, Url("https://www.azure.com"));

    *accessToken = {"ACCESSTOKEN2", std::chrono::system_clock::now() + 1h};

    pipeline.Send(request, Context());

    {
      auto const headers = request.GetHeaders();
      auto const authHeader = headers.find("authorization");
      EXPECT_NE(authHeader, headers.end());
      EXPECT_EQ(authHeader->second, "Bearer ACCESSTOKEN2");
    }
  }
}

TEST(BearerTokenAuthenticationPolicy, NonHttps)
{
  using namespace std::chrono_literals;
  auto accessToken = std::make_shared<AccessToken>();

  std::vector<std::unique_ptr<HttpPolicy>> policies;

  TokenRequestContext tokenRequestContext;
  tokenRequestContext.Scopes = {"https://microsoft.com/.default"};

  policies.emplace_back(std::make_unique<BearerTokenAuthenticationPolicy>(
      std::make_shared<TestTokenCredential>(accessToken), tokenRequestContext));

  policies.emplace_back(std::make_unique<TestTransportPolicy>());

  HttpPipeline pipeline(policies);

  Request request(HttpMethod::Get, Url("http://www.azure.com"));

  *accessToken = {"ACCESSTOKEN1", std::chrono::system_clock::now()};

  EXPECT_THROW(static_cast<void>(pipeline.Send(request, Context())), AuthenticationException);
}

namespace {
class TestBearerTokenAuthenticationPolicy final : public BearerTokenAuthenticationPolicy {
public:
  TestBearerTokenAuthenticationPolicy(
      std::shared_ptr<TokenCredential const> credential,
      TokenRequestContext tokenRequestContext)
      : BearerTokenAuthenticationPolicy(credential, tokenRequestContext)
  {
  }

  std::unique_ptr<HttpPolicy> Clone() const override
  {
    return std::make_unique<TestBearerTokenAuthenticationPolicy>(*this);
  }

protected:
  bool AuthorizeRequestOnChallenge(std::string const&, Request&, Context const&) const override
  {
    EXPECT_FALSE("AuthorizeRequestOnChallenge() should not get called if AuthorizeAndSendRequest() "
                 "was successful.");
    return false;
  }
};

class TestTokenCredentialForBearerTokenAuthenticationPolicy final : public TokenCredential {
public:
  explicit TestTokenCredentialForBearerTokenAuthenticationPolicy()
      : TokenCredential("TestTokenCredentialForBearerTokenAuthenticationPolicy")
  {
  }

  AccessToken GetToken(TokenRequestContext const& tokenRequestContext, Context const&)
      const override
  {
    EXPECT_EQ(tokenRequestContext.Scopes.size(), 1);
    EXPECT_EQ(tokenRequestContext.Scopes[0], "https://microsoft.com/.default");

    EXPECT_TRUE(tokenRequestContext.TenantId.empty());

    AccessToken result;
    result.Token = "ACCESSTOKEN";
    return result;
  }
};

class TestChallengeBasedAuthenticationPolicy final : public BearerTokenAuthenticationPolicy {
private:
  bool m_successfulAuthOnChallenge;

public:
  TestChallengeBasedAuthenticationPolicy(
      std::shared_ptr<TokenCredential const> credential,
      TokenRequestContext tokenRequestContext,
      bool successfulAuthOnChallenge)
      : BearerTokenAuthenticationPolicy(credential, tokenRequestContext),
        m_successfulAuthOnChallenge(successfulAuthOnChallenge)
  {
  }

  std::unique_ptr<HttpPolicy> Clone() const override
  {
    return std::make_unique<TestChallengeBasedAuthenticationPolicy>(*this);
  }

protected:
  std::unique_ptr<RawResponse> AuthorizeAndSendRequest(
      Request& request,
      NextHttpPolicy& nextHttpPolicy,
      Context const& context) const override
  {
    EXPECT_EQ(request.GetUrl().GetAbsoluteUrl(), "https://www.azure.com");

    TokenRequestContext trc;
    trc.Scopes = {"https://visualstudio.com/.default"};
    trc.TenantId = "TestTenantId1";

    AuthenticateAndAuthorizeRequest(request, trc, context);

    // Simulate as if we got challenge-based response and not a HTTP 200.
    static_cast<void>(nextHttpPolicy.Send(request, context));
    auto response = std::make_unique<RawResponse>(1, 1, HttpStatusCode::Unauthorized, "TestStatus");
    response->SetHeader("WWW-Authenticate", "TestChallenge");
    return response;
  }

  bool AuthorizeRequestOnChallenge(
      std::string const& challenge,
      Request& request,
      Context const& context) const override
  {
    EXPECT_EQ(challenge, "TestChallenge");

    TokenRequestContext trc;
    trc.Scopes = {"https://xbox.com/.default"};
    trc.TenantId = "TestTenantId2";

    if (m_successfulAuthOnChallenge)
    {
      AuthenticateAndAuthorizeRequest(request, trc, context);
      return true;
    }

    return false;
  }
};

class TestTokenCredentialForChallengeBasedTokenAuthenticationPolicy final : public TokenCredential {
private:
  mutable int m_invokedTimes;

public:
  explicit TestTokenCredentialForChallengeBasedTokenAuthenticationPolicy()
      : TokenCredential("TestTokenCredentialForChallengeBasedTokenAuthenticationPolicy"),
        m_invokedTimes(0)
  {
  }

  AccessToken GetToken(TokenRequestContext const& tokenRequestContext, Context const&)
      const override
  {
    ++m_invokedTimes;

    EXPECT_GE(m_invokedTimes, 1);
    EXPECT_LE(m_invokedTimes, 2);

    if (m_invokedTimes == 1)
    {
      EXPECT_EQ(tokenRequestContext.Scopes.size(), 1);
      EXPECT_EQ(tokenRequestContext.Scopes[0], "https://visualstudio.com/.default");
      EXPECT_EQ(tokenRequestContext.TenantId, "TestTenantId1");

      AccessToken result;
      result.Token = "ACCESSTOKEN1";
      return result;
    }

    EXPECT_EQ(tokenRequestContext.Scopes.size(), 1);
    EXPECT_EQ(tokenRequestContext.Scopes[0], "https://xbox.com/.default");
    EXPECT_EQ(tokenRequestContext.TenantId, "TestTenantId2");

    AccessToken result;
    result.Token = "ACCESSTOKEN2";
    return result;
  }
};
} // namespace

TEST(BearerTokenAuthenticationPolicy, ChallengeBasedSupport)
{
  std::vector<std::unique_ptr<HttpPolicy>> policies;

  TokenRequestContext tokenRequestContext;
  tokenRequestContext.Scopes = {"https://microsoft.com/.default"};

  policies.emplace_back(std::make_unique<TestBearerTokenAuthenticationPolicy>(
      std::make_shared<TestTokenCredentialForBearerTokenAuthenticationPolicy>(),
      tokenRequestContext));

  policies.emplace_back(std::make_unique<TestTransportPolicy>());

  HttpPipeline pipeline(policies);

  Request request(HttpMethod::Get, Url("https://www.azure.com"));

  static_cast<void>(pipeline.Send(request, Context()));
  auto const headers = request.GetHeaders();
  auto const authHeader = headers.find("authorization");
  EXPECT_NE(authHeader, headers.end());
  EXPECT_EQ(authHeader->second, "Bearer ACCESSTOKEN");
}

TEST(BearerTokenAuthenticationPolicy, ChallengeBasedSuccess)
{
  std::vector<std::unique_ptr<HttpPolicy>> policies;

  TokenRequestContext tokenRequestContext;
  tokenRequestContext.Scopes = {"https://microsoft.com/.default"};

  policies.emplace_back(std::make_unique<TestChallengeBasedAuthenticationPolicy>(
      std::make_shared<TestTokenCredentialForChallengeBasedTokenAuthenticationPolicy>(),
      tokenRequestContext,
      true));

  policies.emplace_back(std::make_unique<TestTransportPolicy>());

  HttpPipeline pipeline(policies);

  Request request(HttpMethod::Get, Url("https://www.azure.com"));

  auto const response = pipeline.Send(request, Context());
  EXPECT_EQ(response->GetStatusCode(), HttpStatusCode::Ok);

  auto const headers = request.GetHeaders();
  auto const authHeader = headers.find("authorization");
  EXPECT_NE(authHeader, headers.end());
  EXPECT_EQ(authHeader->second, "Bearer ACCESSTOKEN2");
}

TEST(BearerTokenAuthenticationPolicy, ChallengeBasedFailure)
{
  std::vector<std::unique_ptr<HttpPolicy>> policies;

  TokenRequestContext tokenRequestContext;
  tokenRequestContext.Scopes = {"https://microsoft.com/.default"};

  policies.emplace_back(std::make_unique<TestChallengeBasedAuthenticationPolicy>(
      std::make_shared<TestTokenCredentialForChallengeBasedTokenAuthenticationPolicy>(),
      tokenRequestContext,
      false));

  policies.emplace_back(std::make_unique<TestTransportPolicy>());

  HttpPipeline pipeline(policies);

  Request request(HttpMethod::Get, Url("https://www.azure.com"));

  auto const response = pipeline.Send(request, Context());
  EXPECT_EQ(response->GetStatusCode(), HttpStatusCode::Unauthorized);

  auto const headers = request.GetHeaders();
  auto const authHeader = headers.find("authorization");
  EXPECT_NE(authHeader, headers.end());
  EXPECT_EQ(authHeader->second, "Bearer ACCESSTOKEN1");
}
