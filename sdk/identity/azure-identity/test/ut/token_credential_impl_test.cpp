// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/token_credential_impl.hpp"

#include "credential_test_helper.hpp"

#include <memory>
#include <utility>

#include <gtest/gtest.h>

using Azure::Core::Context;
using Azure::Core::Url;
using Azure::Core::Credentials::AccessToken;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenCredential;
using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Http::HttpMethod;
using Azure::Identity::_detail::TokenCredentialImpl;
using Azure::Identity::Test::_detail::CredentialTestHelper;

namespace {
class TokenCredentialImplTester : public TokenCredential {
private:
  std::function<void()> m_throwingFunction = []() {};
  HttpMethod m_httpMethod = HttpMethod(std::string());
  Url m_url;
  std::unique_ptr<TokenCredentialImpl> m_tokenCredentialImpl;

public:
  explicit TokenCredentialImplTester(
      HttpMethod httpMethod,
      Url url,
      TokenCredentialOptions const& options)
      : m_httpMethod(std::move(httpMethod)), m_url(std::move(url)),
        m_tokenCredentialImpl(new TokenCredentialImpl(options))
  {
  }

  explicit TokenCredentialImplTester(
      std::function<void()> throwingFunction,
      TokenCredentialOptions const& options)
      : m_throwingFunction(std::move(throwingFunction)),
        m_tokenCredentialImpl(new TokenCredentialImpl(options))
  {
  }

  AccessToken GetToken(TokenRequestContext const& tokenRequestContext, Context const& context)
      const override
  {
    return m_tokenCredentialImpl->GetToken(context, [&]() {
      m_throwingFunction();

      std::string scopesStr;
      for (auto const& scope : tokenRequestContext.Scopes)
      {
        scopesStr += scope + " ";
      }

      return std::make_unique<TokenCredentialImpl::TokenRequest>(m_httpMethod, m_url, scopesStr);
    });
  }
};
} // namespace

TEST(TokenCredentialImpl, Normal)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        return std::make_unique<TokenCredentialImplTester>(
            HttpMethod::Delete, Url("https://outlook.com/"), options);
      },
      {{{"https://azure.com/.default", "https://microsoft.com/.default"}}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN\"}"});

  EXPECT_EQ(actual.Requests.size(), 1U);
  EXPECT_EQ(actual.Responses.size(), 1U);

  auto const& request = actual.Requests.at(0);

  auto const& response = actual.Responses.at(0);

  EXPECT_EQ(request.HttpMethod, HttpMethod::Delete);

  EXPECT_EQ(request.AbsoluteUrl, "https://outlook.com");

  {
    constexpr char expectedBody[] = "https://azure.com/.default https://microsoft.com/.default ";
    EXPECT_EQ(request.Body, expectedBody);

    EXPECT_NE(request.Headers.find("Content-Length"), request.Headers.end());
    EXPECT_EQ(request.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));
  }

  EXPECT_NE(request.Headers.find("Content-Type"), request.Headers.end());
  EXPECT_EQ(request.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response.AccessToken.Token, "ACCESSTOKEN");

  using namespace std::chrono_literals;
  EXPECT_GT(response.AccessToken.ExpiresOn, response.EarliestExpiration + 3600s);
  EXPECT_LT(response.AccessToken.ExpiresOn, response.LatestExpiration + 3600s);
}

TEST(TokenCredentialImpl, StdException)
{
  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        return std::make_unique<TokenCredentialImplTester>(
            []() { throw std::exception(); }, options);
      },
      {{{"https://azure.com/.default", "https://microsoft.com/.default"}}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN\"}"},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}

TEST(TokenCredentialImpl, ThrowInt)
{
  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        return std::make_unique<TokenCredentialImplTester>([]() { throw 0; }, options);
      },
      {{{"https://azure.com/.default", "https://microsoft.com/.default"}}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN\"}"},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}
