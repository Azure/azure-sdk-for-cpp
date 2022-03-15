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
      Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
      Azure::Core::Context const&) const override
  {
    EXPECT_EQ(
        tokenRequestContext.AuthorizationUri.Value().GetAbsoluteUrl(),
        "https://login.windows.net/72f988bf-86f1-41af-91ab-2d7cd011db47/oauth2/v2.0/token");
    EXPECT_EQ(tokenRequestContext.Scopes.size(), size_t(1));
    EXPECT_EQ(tokenRequestContext.Scopes[0], "https://vault.azure.net/.default");
    EXPECT_EQ(tokenRequestContext.TenantId.Value(), "72f988bf-86f1-41af-91ab-2d7cd011db47");
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
    Azure::Core::Http::RawResponse response(
        2, 2, Azure::Core::Http::HttpStatusCode::Unauthorized, "test");
    auto returnValue = std::make_unique<Azure::Core::Http::RawResponse>(response);
    returnValue->SetHeader(
        "www-authenticate",
        "Bearer authorization=\"https://login.windows.net/72f988bf-86f1-41af-91ab-2d7cd011db47\", "
        "resource=\"https://vault.azure.net\"");
    return returnValue;
  }

  std::unique_ptr<HttpPolicy> Clone() const override
  {
    return std::make_unique<TestTransportPolicy>(*this);
  }
};

} // namespace

TEST(ChallengeBasedAuthenticationPolicy, InitialTest)
{
  using namespace std::chrono_literals;
  auto accessToken = std::make_shared<Azure::Core::Credentials::AccessToken>();

  std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;

  policies.emplace_back(
      std::make_unique<Azure::Core::Http::Policies::_internal::ChallengeBasedAuthenticationPolicy>(
          std::make_shared<TestTokenCredential>(accessToken),
          Azure::Core::Credentials::TokenRequestContext{
              {"https://microsoft.com/.default"},
          }));

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
