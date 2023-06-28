// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "credential_test_helper.hpp"
#include "private/token_credential_impl.hpp"

#include <memory>
#include <utility>

#include <gtest/gtest.h>

using Azure::DateTime;
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
      : TokenCredential("TokenCredentialImplTester"), m_httpMethod(std::move(httpMethod)),
        m_url(std::move(url)), m_tokenCredentialImpl(new TokenCredentialImpl(options))
  {
  }

  explicit TokenCredentialImplTester(
      std::function<void()> throwingFunction,
      TokenCredentialOptions const& options)
      : TokenCredential("TokenCredentialImplTester"),
        m_throwingFunction(std::move(throwingFunction)),
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

// Disable deprecation warning
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
// This credential is needed to test the default behavior when the customer has custom credential
// they implemented while using earlier versions of the SDK which didn't have a constructor with
// credentialName.
class CustomTokenCredential : public TokenCredential {
public:
  AccessToken GetToken(TokenRequestContext const&, Context const&) const override { return {}; }
};
#if defined(_MSC_VER)
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif // _MSC_VER

} // namespace

TEST(CustomTokenCredential, GetCredentialName)
{
  CustomTokenCredential cred;
  EXPECT_EQ(cred.GetCredentialName(), "Custom Credential");
}

TEST(TokenCredentialImpl, Normal)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        return std::make_unique<TokenCredentialImplTester>(
            HttpMethod::Delete, Url("https://outlook.com/"), options);
      },
      {{"https://azure.com/.default", "https://microsoft.com/.default"},
       {"https://azure.com/.default", "https://microsoft.com/.default"},
       {"https://azure.com/.default", "https://microsoft.com/.default"}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"access_token\":\"ACCESSTOKEN2\", \"expires_in\":7200}",
          "{\"ab\":1,\"expires_in\":9999,\"cd\":2,\"access_token\":\"ACCESSTOKEN3\",\"ef\":3}"});

  EXPECT_EQ(actual.Requests.size(), 3U);
  EXPECT_EQ(actual.Responses.size(), 3U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);
  auto const& request2 = actual.Requests.at(2);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);
  auto const& response2 = actual.Responses.at(2);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Delete);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Delete);
  EXPECT_EQ(request2.HttpMethod, HttpMethod::Delete);

  EXPECT_EQ(request0.AbsoluteUrl, "https://outlook.com");
  EXPECT_EQ(request1.AbsoluteUrl, "https://outlook.com");
  EXPECT_EQ(request2.AbsoluteUrl, "https://outlook.com");

  {
    constexpr char expectedBody[] = "https://azure.com/.default https://microsoft.com/.default ";
    EXPECT_EQ(request0.Body, expectedBody);
    EXPECT_EQ(request1.Body, expectedBody);
    EXPECT_EQ(request2.Body, expectedBody);

    EXPECT_NE(request0.Headers.find("Content-Length"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));

    EXPECT_NE(request2.Headers.find("Content-Length"), request2.Headers.end());
    EXPECT_EQ(request2.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));
  }

  EXPECT_NE(request0.Headers.find("Content-Type"), request0.Headers.end());
  EXPECT_EQ(request0.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request1.Headers.find("Content-Type"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request2.Headers.find("Content-Type"), request2.Headers.end());
  EXPECT_EQ(request2.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");
  EXPECT_EQ(response2.AccessToken.Token, "ACCESSTOKEN3");

  using namespace std::chrono_literals;
  EXPECT_GE(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LE(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GE(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LE(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);

  EXPECT_GE(response2.AccessToken.ExpiresOn, response2.EarliestExpiration + 9999s);
  EXPECT_LE(response2.AccessToken.ExpiresOn, response2.LatestExpiration + 9999s);
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
      {{"https://azure.com/.default", "https://microsoft.com/.default"}},
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
      {{"https://azure.com/.default", "https://microsoft.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN\"}"},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(token = credential.GetToken(tokenRequestContext, context), int);
        return token;
      }));
}

TEST(TokenCredentialImpl, FormatScopes)
{
  // Not testing with 0 scopes:
  // It is a caller's responsibility to never give an empty vector of scopes to the FormatScopes()
  // member function. The class is in _detail, so this kind of contract is ok. It allows for less
  // unnecessary checks, because, realistically, calling code would test the scopes for being empty
  // first, in order to decide whether to append "&scopes=" at all, or not.

  // 1 scope
  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"https://azure.com"}, false),
      "https%3A%2F%2Fazure.com"); // cspell:disable-line

  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"https://azure.com"}, true),
      "https%3A%2F%2Fazure.com"); // cspell:disable-line

  // 1 scope, ends with '/'
  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"https://azure.com/"}, false),
      "https%3A%2F%2Fazure.com%2F"); // cspell:disable-line

  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"https://azure.com/"}, true),
      "https%3A%2F%2Fazure.com%2F"); // cspell:disable-line

  // 1 scope, ends with '/.default'
  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"https://azure.com/.default"}, false),
      "https%3A%2F%2Fazure.com%2F.default"); // cspell:disable-line

  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"https://azure.com/.default"}, true),
      "https%3A%2F%2Fazure.com"); // cspell:disable-line

  // 2 scopes
  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"https://azure.com", "https://microsoft.com"}, false),
      "https%3A%2F%2Fazure.com https%3A%2F%2Fmicrosoft.com"); // cspell:disable-line

  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"https://azure.com", "https://microsoft.com"}, true),
      "https%3A%2F%2Fazure.com https%3A%2F%2Fmicrosoft.com"); // cspell:disable-line

  // 2 scopes, reverse order
  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"https://microsoft.com", "https://azure.com"}, false),
      "https%3A%2F%2Fmicrosoft.com https%3A%2F%2Fazure.com"); // cspell:disable-line

  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"https://microsoft.com", "https://azure.com"}, true),
      "https%3A%2F%2Fmicrosoft.com https%3A%2F%2Fazure.com"); // cspell:disable-line

  // 2 scopes, one ends with '/'
  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"https://azure.com", "https://microsoft.com/"}, false),
      "https%3A%2F%2Fazure.com https%3A%2F%2Fmicrosoft.com%2F"); // cspell:disable-line

  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"https://azure.com", "https://microsoft.com/"}, true),
      "https%3A%2F%2Fazure.com https%3A%2F%2Fmicrosoft.com%2F"); // cspell:disable-line

  // 2 scopes, one ends with '/.default'
  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes(
          {"https://azure.com", "https://microsoft.com/.default"}, false),
      "https%3A%2F%2Fazure.com https%3A%2F%2Fmicrosoft.com%2F.default"); // cspell:disable-line

  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes(
          {"https://azure.com", "https://microsoft.com/.default"}, true),
      "https%3A%2F%2Fazure.com https%3A%2F%2Fmicrosoft.com%2F.default"); // cspell:disable-line

  // 2 scopes, both end with '/.default', reverse order
  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes(
          {"https://microsoft.com/.default", "https://azure.com/.default"}, false),
      "https%3A%2F%2Fmicrosoft.com%2F.default https%3A%2F%2Fazure.com%2F.default"); // cspell:disable-line

  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes(
          {"https://microsoft.com/.default", "https://azure.com/.default"}, true),
      "https%3A%2F%2Fmicrosoft.com%2F.default https%3A%2F%2Fazure.com%2F.default"); // cspell:disable-line

  // Spaces inside scopes get encoded, but the spaces separating scopes are not
  EXPECT_EQ(TokenCredentialImpl::FormatScopes({"a b", "c d", "e f"}, false), "a%20b c%20d e%20f");

  // 1 scope, '/.default' only, gets removed when treated as single resource
  EXPECT_EQ(TokenCredentialImpl::FormatScopes({"/.default"}, false), "%2F.default");
  EXPECT_EQ(TokenCredentialImpl::FormatScopes({"/.default"}, true), "");

  // 2 scopes, '/.default' only
  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"/.default", "/.default"}, false),
      "%2F.default %2F.default");
  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"/.default", "/.default"}, true),
      "%2F.default %2F.default");

  // Very short single scope, maybe can be '/.default'
  EXPECT_EQ(TokenCredentialImpl::FormatScopes({"/.outlook"}, true), "%2F.outlook");

  // Very short single scope, clearly can't end with '/.default'
  EXPECT_EQ(TokenCredentialImpl::FormatScopes({"/.xbox"}, true), "%2F.xbox");

  // Duplicates kept
  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"https://azure.com", "https://azure.com"}, false),
      "https%3A%2F%2Fazure.com https%3A%2F%2Fazure.com"); // cspell:disable-line

  EXPECT_EQ(
      TokenCredentialImpl::FormatScopes({"https://azure.com", "https://azure.com"}, true),
      "https%3A%2F%2Fazure.com https%3A%2F%2Fazure.com"); // cspell:disable-line
}

TEST(TokenCredentialImpl, NoExpiration)
{
  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        return std::make_unique<TokenCredentialImplTester>(
            HttpMethod::Delete, Url("https://outlook.com/"), options);
      },
      {{"https://azure.com/.default", "https://microsoft.com/.default"}},
      {"{\"access_token\":\"ACCESSTOKEN\"}"},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}

TEST(TokenCredentialImpl, NoToken)
{
  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        return std::make_unique<TokenCredentialImplTester>(
            HttpMethod::Delete, Url("https://outlook.com/"), options);
      },
      {{"https://azure.com/.default", "https://microsoft.com/.default"}},
      {"{\"expires_in\":3600}"},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}

TEST(TokenCredentialImpl, NullResponse)
{
  using Azure::Core::Http::RawResponse;
  using Azure::Core::Http::Request;
  using Azure::Core::Http::Policies::HttpPolicy;
  using Azure::Core::Http::Policies::NextHttpPolicy;

  class NullResponsePolicy : public HttpPolicy {
  public:
    std::unique_ptr<HttpPolicy> Clone() const override
    {
      return std::make_unique<NullResponsePolicy>(*this);
    }

    std::unique_ptr<RawResponse> Send(Request&, NextHttpPolicy, Context const&) const override
    {
      return nullptr;
    }
  };

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;
        options.PerOperationPolicies.push_back(std::make_unique<NullResponsePolicy>());

        return std::make_unique<TokenCredentialImplTester>(
            HttpMethod::Delete, Url("https://microsoft.com/"), options);
      },
      {{"https://azure.com/.default"}},
      {{"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN\"}"}},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}

namespace {
std::string MakeTokenResponse(
    std::string const& number,
    std::string const& expiresInValue,
    std::string const& expiresOnValue)
{
  return ("{\"access_token\":\"ACCESSTOKEN" + number + "\"")
      + (expiresInValue.empty() ? "" : (",\"expires_in\":" + expiresInValue))
      + (expiresOnValue.empty() ? "" : (",\"expires_on\":" + expiresOnValue)) + "}";
}
} // namespace

TEST(TokenCredentialImpl, ExpirationFormats)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        return std::make_unique<TokenCredentialImplTester>(
            HttpMethod::Get, Url("https://microsoft.com/"), options);
      },
      std::vector<decltype(TokenRequestContext::Scopes)>(47, {"https://azure.com/.default"}),
      std::vector<std::string>{
          MakeTokenResponse("00", "3600", ""),
          MakeTokenResponse("01", "\"3600\"", ""),
          MakeTokenResponse("02", "\"unknown format\"", ""),
          MakeTokenResponse("03", "\"\"", ""),
          MakeTokenResponse("04", "null", ""),
          MakeTokenResponse("05", "", "43040261106"),
          MakeTokenResponse("06", "", "\"43040261106\""),
          MakeTokenResponse("07", "", "\"3333-11-22T04:05:06.000Z\""),
          MakeTokenResponse("08", "", "\"Sun, 22 Nov 3333 04:05:06 GMT\""),
          MakeTokenResponse("09", "", "\"unknown format\""),
          MakeTokenResponse("10", "", "\"\""),
          MakeTokenResponse("11", "", "null"),
          MakeTokenResponse("12", "3600", "43040261106"),
          MakeTokenResponse("13", "3600", "\"43040261106\""),
          MakeTokenResponse("14", "3600", "\"3333-11-22T04:05:06.000Z\""),
          MakeTokenResponse("15", "3600", "\"Sun, 22 Nov 3333 04:05:06 GMT\""),
          MakeTokenResponse("16", "3600", "\"unknown format\""),
          MakeTokenResponse("17", "3600", "\"\""),
          MakeTokenResponse("18", "3600", "null"),
          MakeTokenResponse("19", "\"3600\"", "43040261106"),
          MakeTokenResponse("20", "\"3600\"", "\"43040261106\""),
          MakeTokenResponse("21", "\"3600\"", "\"3333-11-22T04:05:06.000Z\""),
          MakeTokenResponse("22", "\"3600\"", "\"Sun, 22 Nov 3333 04:05:06 GMT\""),
          MakeTokenResponse("23", "\"3600\"", "\"unknown format\""),
          MakeTokenResponse("24", "\"3600\"", "\"\""),
          MakeTokenResponse("25", "\"3600\"", "null"),
          MakeTokenResponse("26", "\"unknown format\"", "43040261106"),
          MakeTokenResponse("27", "\"unknown format\"", "\"43040261106\""),
          MakeTokenResponse("28", "\"unknown format\"", "\"3333-11-22T04:05:06.000Z\""),
          MakeTokenResponse("29", "\"unknown format\"", "\"Sun, 22 Nov 3333 04:05:06 GMT\""),
          MakeTokenResponse("30", "\"unknown format\"", "\"unknown format\""),
          MakeTokenResponse("31", "\"unknown format\"", "\"\""),
          MakeTokenResponse("32", "\"unknown format\"", "null"),
          MakeTokenResponse("33", "\"\"", "43040261106"),
          MakeTokenResponse("34", "\"\"", "\"43040261106\""),
          MakeTokenResponse("35", "\"\"", "\"3333-11-22T04:05:06.000Z\""),
          MakeTokenResponse("36", "\"\"", "\"Sun, 22 Nov 3333 04:05:06 GMT\""),
          MakeTokenResponse("37", "\"\"", "\"unknown format\""),
          MakeTokenResponse("38", "\"\"", "\"\""),
          MakeTokenResponse("39", "\"\"", "null"),
          MakeTokenResponse("40", "null", "43040261106"),
          MakeTokenResponse("41", "null", "\"43040261106\""),
          MakeTokenResponse("42", "null", "\"3333-11-22T04:05:06.000Z\""),
          MakeTokenResponse("43", "null", "\"Sun, 22 Nov 3333 04:05:06 GMT\""),
          MakeTokenResponse("44", "null", "\"unknown format\""),
          MakeTokenResponse("45", "null", "\"\""),
          MakeTokenResponse("46", "null", "null"),
      },
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        token.Token = "FAILED";

        try
        {
          token = credential.GetToken(tokenRequestContext, context);
        }
        catch (AuthenticationException const&)
        {
        }

        return token;
      });

  EXPECT_EQ(actual.Requests.size(), 47U);
  EXPECT_EQ(actual.Responses.size(), 47U);

  auto const& response00 = actual.Responses.at(0);
  auto const& response01 = actual.Responses.at(1);
  auto const& response02 = actual.Responses.at(2);
  auto const& response03 = actual.Responses.at(3);
  auto const& response04 = actual.Responses.at(4);
  auto const& response05 = actual.Responses.at(5);
  auto const& response06 = actual.Responses.at(6);
  auto const& response07 = actual.Responses.at(7);
  auto const& response08 = actual.Responses.at(8);
  auto const& response09 = actual.Responses.at(9);
  auto const& response10 = actual.Responses.at(10);
  auto const& response11 = actual.Responses.at(11);
  auto const& response12 = actual.Responses.at(12);
  auto const& response13 = actual.Responses.at(13);
  auto const& response14 = actual.Responses.at(14);
  auto const& response15 = actual.Responses.at(15);
  auto const& response16 = actual.Responses.at(16);
  auto const& response17 = actual.Responses.at(17);
  auto const& response18 = actual.Responses.at(18);
  auto const& response19 = actual.Responses.at(19);
  auto const& response20 = actual.Responses.at(20);
  auto const& response21 = actual.Responses.at(21);
  auto const& response22 = actual.Responses.at(22);
  auto const& response23 = actual.Responses.at(23);
  auto const& response24 = actual.Responses.at(24);
  auto const& response25 = actual.Responses.at(25);
  auto const& response26 = actual.Responses.at(26);
  auto const& response27 = actual.Responses.at(27);
  auto const& response28 = actual.Responses.at(28);
  auto const& response29 = actual.Responses.at(29);
  auto const& response30 = actual.Responses.at(30);
  auto const& response31 = actual.Responses.at(31);
  auto const& response32 = actual.Responses.at(32);
  auto const& response33 = actual.Responses.at(33);
  auto const& response34 = actual.Responses.at(34);
  auto const& response35 = actual.Responses.at(35);
  auto const& response36 = actual.Responses.at(36);
  auto const& response37 = actual.Responses.at(37);
  auto const& response38 = actual.Responses.at(38);
  auto const& response39 = actual.Responses.at(39);
  auto const& response40 = actual.Responses.at(40);
  auto const& response41 = actual.Responses.at(41);
  auto const& response42 = actual.Responses.at(42);
  auto const& response43 = actual.Responses.at(43);
  auto const& response44 = actual.Responses.at(44);
  auto const& response45 = actual.Responses.at(45);
  auto const& response46 = actual.Responses.at(46);

  EXPECT_EQ(response00.AccessToken.Token, "ACCESSTOKEN00");
  EXPECT_EQ(response01.AccessToken.Token, "ACCESSTOKEN01");
  EXPECT_EQ(response02.AccessToken.Token, "FAILED");
  EXPECT_EQ(response03.AccessToken.Token, "FAILED");
  EXPECT_EQ(response04.AccessToken.Token, "FAILED");
  EXPECT_EQ(response05.AccessToken.Token, "ACCESSTOKEN05");
  EXPECT_EQ(response06.AccessToken.Token, "ACCESSTOKEN06");
  EXPECT_EQ(response07.AccessToken.Token, "ACCESSTOKEN07");
  EXPECT_EQ(response08.AccessToken.Token, "ACCESSTOKEN08");
  EXPECT_EQ(response09.AccessToken.Token, "FAILED");
  EXPECT_EQ(response10.AccessToken.Token, "FAILED");
  EXPECT_EQ(response11.AccessToken.Token, "FAILED");
  EXPECT_EQ(response12.AccessToken.Token, "ACCESSTOKEN12");
  EXPECT_EQ(response13.AccessToken.Token, "ACCESSTOKEN13");
  EXPECT_EQ(response14.AccessToken.Token, "ACCESSTOKEN14");
  EXPECT_EQ(response15.AccessToken.Token, "ACCESSTOKEN15");
  EXPECT_EQ(response16.AccessToken.Token, "ACCESSTOKEN16");
  EXPECT_EQ(response17.AccessToken.Token, "ACCESSTOKEN17");
  EXPECT_EQ(response18.AccessToken.Token, "ACCESSTOKEN18");
  EXPECT_EQ(response19.AccessToken.Token, "ACCESSTOKEN19");
  EXPECT_EQ(response20.AccessToken.Token, "ACCESSTOKEN20");
  EXPECT_EQ(response21.AccessToken.Token, "ACCESSTOKEN21");
  EXPECT_EQ(response22.AccessToken.Token, "ACCESSTOKEN22");
  EXPECT_EQ(response23.AccessToken.Token, "ACCESSTOKEN23");
  EXPECT_EQ(response24.AccessToken.Token, "ACCESSTOKEN24");
  EXPECT_EQ(response25.AccessToken.Token, "ACCESSTOKEN25");
  EXPECT_EQ(response26.AccessToken.Token, "ACCESSTOKEN26");
  EXPECT_EQ(response27.AccessToken.Token, "ACCESSTOKEN27");
  EXPECT_EQ(response28.AccessToken.Token, "ACCESSTOKEN28");
  EXPECT_EQ(response29.AccessToken.Token, "ACCESSTOKEN29");
  EXPECT_EQ(response30.AccessToken.Token, "FAILED");
  EXPECT_EQ(response31.AccessToken.Token, "FAILED");
  EXPECT_EQ(response32.AccessToken.Token, "FAILED");
  EXPECT_EQ(response33.AccessToken.Token, "ACCESSTOKEN33");
  EXPECT_EQ(response34.AccessToken.Token, "ACCESSTOKEN34");
  EXPECT_EQ(response35.AccessToken.Token, "ACCESSTOKEN35");
  EXPECT_EQ(response36.AccessToken.Token, "ACCESSTOKEN36");
  EXPECT_EQ(response37.AccessToken.Token, "FAILED");
  EXPECT_EQ(response38.AccessToken.Token, "FAILED");
  EXPECT_EQ(response39.AccessToken.Token, "FAILED");
  EXPECT_EQ(response40.AccessToken.Token, "ACCESSTOKEN40");
  EXPECT_EQ(response41.AccessToken.Token, "ACCESSTOKEN41");
  EXPECT_EQ(response42.AccessToken.Token, "ACCESSTOKEN42");
  EXPECT_EQ(response43.AccessToken.Token, "ACCESSTOKEN43");
  EXPECT_EQ(response44.AccessToken.Token, "FAILED");
  EXPECT_EQ(response45.AccessToken.Token, "FAILED");
  EXPECT_EQ(response46.AccessToken.Token, "FAILED");

  using namespace std::chrono_literals;
  EXPECT_GE(response00.AccessToken.ExpiresOn, response00.EarliestExpiration + 3600s);
  EXPECT_LE(response00.AccessToken.ExpiresOn, response00.LatestExpiration + 3600s);

  EXPECT_GE(response01.AccessToken.ExpiresOn, response01.EarliestExpiration + 3600s);
  EXPECT_LE(response01.AccessToken.ExpiresOn, response01.LatestExpiration + 3600s);

  EXPECT_EQ(response05.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));
  EXPECT_EQ(response06.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));
  EXPECT_EQ(response07.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));
  EXPECT_EQ(response08.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));

  EXPECT_GE(response12.AccessToken.ExpiresOn, response12.EarliestExpiration + 3600s);
  EXPECT_LE(response12.AccessToken.ExpiresOn, response12.LatestExpiration + 3600s);

  EXPECT_GE(response13.AccessToken.ExpiresOn, response13.EarliestExpiration + 3600s);
  EXPECT_LE(response13.AccessToken.ExpiresOn, response13.LatestExpiration + 3600s);

  EXPECT_GE(response14.AccessToken.ExpiresOn, response14.EarliestExpiration + 3600s);
  EXPECT_LE(response14.AccessToken.ExpiresOn, response14.LatestExpiration + 3600s);

  EXPECT_GE(response15.AccessToken.ExpiresOn, response15.EarliestExpiration + 3600s);
  EXPECT_LE(response15.AccessToken.ExpiresOn, response15.LatestExpiration + 3600s);

  EXPECT_GE(response16.AccessToken.ExpiresOn, response16.EarliestExpiration + 3600s);
  EXPECT_LE(response16.AccessToken.ExpiresOn, response16.LatestExpiration + 3600s);

  EXPECT_GE(response17.AccessToken.ExpiresOn, response17.EarliestExpiration + 3600s);
  EXPECT_LE(response17.AccessToken.ExpiresOn, response17.LatestExpiration + 3600s);

  EXPECT_GE(response18.AccessToken.ExpiresOn, response18.EarliestExpiration + 3600s);
  EXPECT_LE(response18.AccessToken.ExpiresOn, response18.LatestExpiration + 3600s);

  EXPECT_GE(response19.AccessToken.ExpiresOn, response19.EarliestExpiration + 3600s);
  EXPECT_LE(response19.AccessToken.ExpiresOn, response19.LatestExpiration + 3600s);

  EXPECT_GE(response20.AccessToken.ExpiresOn, response20.EarliestExpiration + 3600s);
  EXPECT_LE(response20.AccessToken.ExpiresOn, response20.LatestExpiration + 3600s);

  EXPECT_GE(response21.AccessToken.ExpiresOn, response21.EarliestExpiration + 3600s);
  EXPECT_LE(response21.AccessToken.ExpiresOn, response21.LatestExpiration + 3600s);

  EXPECT_GE(response22.AccessToken.ExpiresOn, response22.EarliestExpiration + 3600s);
  EXPECT_LE(response22.AccessToken.ExpiresOn, response22.LatestExpiration + 3600s);

  EXPECT_GE(response23.AccessToken.ExpiresOn, response23.EarliestExpiration + 3600s);
  EXPECT_LE(response23.AccessToken.ExpiresOn, response23.LatestExpiration + 3600s);

  EXPECT_GE(response24.AccessToken.ExpiresOn, response24.EarliestExpiration + 3600s);
  EXPECT_LE(response24.AccessToken.ExpiresOn, response24.LatestExpiration + 3600s);

  EXPECT_GE(response25.AccessToken.ExpiresOn, response25.EarliestExpiration + 3600s);
  EXPECT_LE(response25.AccessToken.ExpiresOn, response25.LatestExpiration + 3600s);

  EXPECT_EQ(response26.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));
  EXPECT_EQ(response27.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));
  EXPECT_EQ(response28.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));
  EXPECT_EQ(response29.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));

  EXPECT_EQ(response33.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));
  EXPECT_EQ(response34.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));
  EXPECT_EQ(response35.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));
  EXPECT_EQ(response36.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));

  EXPECT_EQ(response40.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));
  EXPECT_EQ(response41.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));
  EXPECT_EQ(response42.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));
  EXPECT_EQ(response43.AccessToken.ExpiresOn, DateTime(3333, 11, 22, 4, 5, 6));
}
