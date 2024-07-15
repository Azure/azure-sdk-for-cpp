// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/client_assertion_credential.hpp"
#include "credential_test_helper.hpp"

#include <cstdio>
#include <fstream>

#include <gtest/gtest.h>

using Azure::Core::_internal::Environment;
using Azure::Core::Credentials::AccessToken;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Http::HttpMethod;
using Azure::Identity::ClientAssertionCredential;
using Azure::Identity::ClientAssertionCredentialOptions;
using Azure::Identity::Test::_detail::CredentialTestHelper;

std::string GetAssertion_Throw(Azure::Core::Context const&)
{
  throw std::runtime_error(
      "The test is not expected to call this function used for assertion callback.");
}

std::string GetAssertion_Test(Azure::Core::Context const&) { return "sample-assertion"; }

TEST(ClientAssertionCredential, GetCredentialName)
{
  std::string tenantId = "01234567-89ab-cdef-fedc-ba8976543210";
  std::string clientId = "fedcba98-7654-3210-0123-456789abcdef";
  std::string serviceConnectionId = "abc";
  std::string systemAccessToken = "123";

  ClientAssertionCredential const cred(tenantId, clientId, GetAssertion_Throw);

  EXPECT_EQ(cred.GetCredentialName(), "ClientAssertionCredential");
}

TEST(ClientAssertionCredential, GetOptionsFromEnvironment)
{
  std::string tenantId = "01234567-89ab-cdef-fedc-ba8976543210";
  std::string clientId = "fedcba98-7654-3210-0123-456789abcdef";

  {
    std::map<std::string, std::string> envVars = {{"AZURE_AUTHORITY_HOST", ""}};
    CredentialTestHelper::EnvironmentOverride const env(envVars);

    ClientAssertionCredentialOptions options;
    ClientAssertionCredential const cred(tenantId, clientId, GetAssertion_Throw, options);
    EXPECT_EQ(cred.GetCredentialName(), "ClientAssertionCredential");

    EXPECT_EQ(options.AuthorityHost, "https://login.microsoftonline.com/");
  }

  {
    std::map<std::string, std::string> envVars = {{"AZURE_AUTHORITY_HOST", "foo"}};
    CredentialTestHelper::EnvironmentOverride const env(envVars);

    ClientAssertionCredentialOptions options;
    options.AuthorityHost = "bar";
    EXPECT_EQ(options.AuthorityHost, "bar");
  }

  {
    std::map<std::string, std::string> envVars
        = {{"AZURE_AUTHORITY_HOST", "https://microsoft.com/"}};
    CredentialTestHelper::EnvironmentOverride const env(envVars);

    ClientAssertionCredentialOptions options;
    EXPECT_EQ(options.AuthorityHost, "https://microsoft.com/");
  }
}

TEST(ClientAssertionCredential, InvalidArgs)
{
  std::string tenantId = "01234567-89ab-cdef-fedc-ba8976543210";
  std::string clientId = "fedcba98-7654-3210-0123-456789abcdef";

  // Empty Tenant ID
  {
    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");

    ClientAssertionCredential const cred("", clientId, GetAssertion_Throw);
    EXPECT_THROW(cred.GetToken(trc, {}), AuthenticationException);
  }

  // Invalid Tenant ID
  {
    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");

    ClientAssertionCredential const cred("!=invalidTenantId=!", clientId, GetAssertion_Throw);
    EXPECT_THROW(cred.GetToken(trc, {}), AuthenticationException);
  }

  // Empty client ID
  {
    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");

    ClientAssertionCredential const cred(tenantId, "", GetAssertion_Throw);
    EXPECT_THROW(cred.GetToken(trc, {}), AuthenticationException);
  }

  // Empty assertion callback
  {
    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");

    ClientAssertionCredential const cred(tenantId, clientId, nullptr);
    EXPECT_THROW(cred.GetToken(trc, {}), AuthenticationException);
  }
  {
    std::function<std::string(Azure::Core::Context const&)> emptyCallBack;
    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");

    ClientAssertionCredential const cred(tenantId, clientId, emptyCallBack);
    EXPECT_THROW(cred.GetToken(trc, {}), AuthenticationException);
  }
}

TEST(ClientAssertionCredential, Regular)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        ClientAssertionCredentialOptions options;
        options.Transport.Transport = transport;

        std::string tenantId = "01234567-89ab-cdef-fedc-ba8976543210";
        std::string clientId = "fedcba98-7654-3210-0123-456789abcdef";

        return std::make_unique<ClientAssertionCredential>(
            tenantId, clientId, GetAssertion_Test, options);
      },
      {{{"https://azure.com/.default"}}, {{}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  EXPECT_EQ(actual.Requests.size(), 2U);
  EXPECT_EQ(actual.Responses.size(), 2U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    constexpr char expectedBodyStart0[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&scope=https%3A%2F%2Fazure.com%2F.default"
          "&client_assertion=sample-assertion"; // cspell:enable

    constexpr char expectedBodyStart1[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&client_assertion=sample-assertion"; // cspell:enable

    EXPECT_EQ(request0.Body.size(), (sizeof(expectedBodyStart0) - 1));
    EXPECT_EQ(request1.Body.size(), (sizeof(expectedBodyStart1) - 1));

    EXPECT_EQ(request0.Body.substr(0, (sizeof(expectedBodyStart0) - 1)), expectedBodyStart0);
    EXPECT_EQ(request1.Body.substr(0, (sizeof(expectedBodyStart1) - 1)), expectedBodyStart1);

    EXPECT_NE(request0.Headers.find("Content-Length"), request0.Headers.end());
    EXPECT_EQ(
        std::stoi(request0.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart0) - 1));

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_EQ(
        std::stoi(request1.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart1) - 1));
  }

  EXPECT_NE(request0.Headers.find("Content-Type"), request0.Headers.end());
  EXPECT_EQ(request0.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request1.Headers.find("Content-Type"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GE(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LE(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GE(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LE(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

TEST(ClientAssertionCredential, AzureStack)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        ClientAssertionCredentialOptions options;
        options.Transport.Transport = transport;

        std::string tenantId = "adfs";
        std::string clientId = "fedcba98-7654-3210-0123-456789abcdef";

        return std::make_unique<ClientAssertionCredential>(
            tenantId, clientId, GetAssertion_Test, options);
      },
      {{{"https://azure.com/.default"}}, {{}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  EXPECT_EQ(actual.Requests.size(), 2U);
  EXPECT_EQ(actual.Responses.size(), 2U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(request0.AbsoluteUrl, "https://login.microsoftonline.com/adfs/oauth2/token");

  EXPECT_EQ(request1.AbsoluteUrl, "https://login.microsoftonline.com/adfs/oauth2/token");

  {
    constexpr char expectedBodyStart0[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&scope=https%3A%2F%2Fazure.com"
          "&client_assertion=sample-assertion"; // cspell:enable

    constexpr char expectedBodyStart1[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&client_assertion=sample-assertion"; // cspell:enable

    EXPECT_EQ(request0.Body.size(), (sizeof(expectedBodyStart0) - 1));
    EXPECT_EQ(request1.Body.size(), (sizeof(expectedBodyStart1) - 1));

    EXPECT_EQ(request0.Body.substr(0, (sizeof(expectedBodyStart0) - 1)), expectedBodyStart0);
    EXPECT_EQ(request1.Body.substr(0, (sizeof(expectedBodyStart1) - 1)), expectedBodyStart1);

    EXPECT_NE(request0.Headers.find("Content-Length"), request0.Headers.end());
    EXPECT_EQ(
        std::stoi(request0.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart0) - 1));

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_EQ(
        std::stoi(request1.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart1) - 1));
  }

  EXPECT_NE(request0.Headers.find("Content-Type"), request0.Headers.end());
  EXPECT_EQ(request0.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request1.Headers.find("Content-Type"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GE(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LE(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GE(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LE(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

TEST(ClientAssertionCredential, Authority)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        ClientAssertionCredentialOptions options;
        options.Transport.Transport = transport;

        std::string tenantId = "01234567-89ab-cdef-fedc-ba8976543210";
        std::string clientId = "fedcba98-7654-3210-0123-456789abcdef";
        options.AuthorityHost = "https://microsoft.com/";

        return std::make_unique<ClientAssertionCredential>(
            tenantId, clientId, GetAssertion_Test, options);
      },
      {{{"https://azure.com/.default"}}, {{}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  EXPECT_EQ(actual.Requests.size(), 2U);
  EXPECT_EQ(actual.Responses.size(), 2U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://microsoft.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://microsoft.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    constexpr char expectedBodyStart0[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&scope=https%3A%2F%2Fazure.com%2F.default"
          "&client_assertion=sample-assertion"; // cspell:enable

    constexpr char expectedBodyStart1[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&client_assertion=sample-assertion"; // cspell:enable

    EXPECT_EQ(request0.Body.size(), (sizeof(expectedBodyStart0) - 1));
    EXPECT_EQ(request1.Body.size(), (sizeof(expectedBodyStart1) - 1));

    EXPECT_EQ(request0.Body.substr(0, (sizeof(expectedBodyStart0) - 1)), expectedBodyStart0);
    EXPECT_EQ(request1.Body.substr(0, (sizeof(expectedBodyStart1) - 1)), expectedBodyStart1);

    EXPECT_NE(request0.Headers.find("Content-Length"), request0.Headers.end());
    EXPECT_EQ(
        std::stoi(request0.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart0) - 1));

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_EQ(
        std::stoi(request1.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart1) - 1));
  }

  EXPECT_NE(request0.Headers.find("Content-Type"), request0.Headers.end());
  EXPECT_EQ(request0.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request1.Headers.find("Content-Type"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GE(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LE(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GE(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LE(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

TEST(ClientAssertionCredential, HttpSchemeNotSupported)
{
  std::map<std::string, std::string> envVars = {{"AZURE_AUTHORITY_HOST", "http://microsoft.com/"}};
  CredentialTestHelper::EnvironmentOverride const env(envVars);

  try
  {
    auto const actual = CredentialTestHelper::SimulateTokenRequest(
        [](auto transport) {
          ClientAssertionCredentialOptions options;
          options.Transport.Transport = transport;

          std::string tenantId = "01234567-89ab-cdef-fedc-ba8976543210";
          std::string clientId = "fedcba98-7654-3210-0123-456789abcdef";

          return std::make_unique<ClientAssertionCredential>(
              tenantId, clientId, GetAssertion_Throw, options);
        },
        {{{"https://azure.com/.default"}}},
        std::vector<std::string>{"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});
  }
  catch (AuthenticationException const& e)
  {
    EXPECT_TRUE(std::string(e.what()).find("https") != std::string::npos) << e.what();
  }
}
