// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>

#include <gtest/gtest.h>

namespace {
class TestTokenCredential final : public Azure::Core::Credentials::TokenCredential {
private:
  std::shared_ptr<Azure::Core::Credentials::AccessToken const> m_accessToken;

public:
  explicit TestTokenCredential(
      std::shared_ptr<Azure::Core::Credentials::AccessToken const> accessToken)
      : m_accessToken(accessToken)
  {
  }

  Azure::Core::Credentials::AccessToken GetToken(
      Azure::Core::Credentials::TokenRequestContext const&,
      Azure::Core::Context const&) const override
  {
    return *m_accessToken;
  }
};

class TestTransportPolicy final : public Azure::Core::Http::Policies::HttpPolicy {
public:
  std::unique_ptr<Azure::Core::Http::RawResponse> Send(
      Azure::Core::Http::Request&,
      Azure::Core::Http::Policies::NextHttpPolicy,
      Azure::Core::Context const&) const override
  {
    return nullptr;
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
  auto accessToken = std::make_shared<Azure::Core::Credentials::AccessToken>();

  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;

  {
    Azure::Core::Credentials::TokenRequestContext tokenRequestContext;
    tokenRequestContext.Scopes = {"https://microsoft.com/.default"};

    policies.emplace_back(
        std::make_unique<Azure::Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy>(
            std::make_shared<TestTokenCredential>(accessToken), tokenRequestContext));
  }

  policies.emplace_back(std::make_unique<TestTransportPolicy>());

  Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

  {
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Url("https://www.azure.com"));

    *accessToken = {"ACCESSTOKEN1", std::chrono::system_clock::now() + 1h};

    pipeline.Send(request, Azure::Core::Context());

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
  auto accessToken = std::make_shared<Azure::Core::Credentials::AccessToken>();

  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;

  {
    Azure::Core::Credentials::TokenRequestContext tokenRequestContext;
    tokenRequestContext.Scopes = {"https://microsoft.com/.default"};

    policies.emplace_back(
        std::make_unique<Azure::Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy>(
            std::make_shared<TestTokenCredential>(accessToken), tokenRequestContext));
  }

  policies.emplace_back(std::make_unique<TestTransportPolicy>());

  Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

  {
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Url("https://www.azure.com"));

    *accessToken = {"ACCESSTOKEN1", std::chrono::system_clock::now() + 5min};

    pipeline.Send(request, Azure::Core::Context());
  }

  {
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Url("https://www.azure.com"));

    *accessToken = {"ACCESSTOKEN2", std::chrono::system_clock::now() + 1h};

    pipeline.Send(request, Azure::Core::Context());

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
  auto accessToken = std::make_shared<Azure::Core::Credentials::AccessToken>();

  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;

  {
    Azure::Core::Credentials::TokenRequestContext tokenRequestContext;
    tokenRequestContext.Scopes = {"https://microsoft.com/.default"};

    policies.emplace_back(
        std::make_unique<Azure::Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy>(
            std::make_shared<TestTokenCredential>(accessToken), tokenRequestContext));
  }

  policies.emplace_back(std::make_unique<TestTransportPolicy>());

  Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

  {
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Url("https://www.azure.com"));

    *accessToken = {"ACCESSTOKEN1", std::chrono::system_clock::now() + 2min};

    pipeline.Send(request, Azure::Core::Context());
  }

  {
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Url("https://www.azure.com"));

    *accessToken = {"ACCESSTOKEN2", std::chrono::system_clock::now() + 1h};

    pipeline.Send(request, Azure::Core::Context());

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
  auto accessToken = std::make_shared<Azure::Core::Credentials::AccessToken>();

  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;

  {
    Azure::Core::Credentials::TokenRequestContext tokenRequestContext;
    tokenRequestContext.Scopes = {"https://microsoft.com/.default"};

    policies.emplace_back(
        std::make_unique<Azure::Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy>(
            std::make_shared<TestTokenCredential>(accessToken), tokenRequestContext));
  }

  policies.emplace_back(std::make_unique<TestTransportPolicy>());

  Azure::Core::Http::_internal::HttpPipeline pipeline(policies);

  {
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Url("https://www.azure.com"));

    *accessToken = {"ACCESSTOKEN1", std::chrono::system_clock::now()};

    pipeline.Send(request, Azure::Core::Context());
  }

  {
    Azure::Core::Http::Request request(
        Azure::Core::Http::HttpMethod::Get, Azure::Core::Url("https://www.azure.com"));

    *accessToken = {"ACCESSTOKEN2", std::chrono::system_clock::now() + 1h};

    pipeline.Send(request, Azure::Core::Context());

    {
      auto const headers = request.GetHeaders();
      auto const authHeader = headers.find("authorization");
      EXPECT_NE(authHeader, headers.end());
      EXPECT_EQ(authHeader->second, "Bearer ACCESSTOKEN2");
    }
  }
}
