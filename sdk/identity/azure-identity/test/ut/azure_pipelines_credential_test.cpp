// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/azure_pipelines_credential.hpp"
#include "credential_test_helper.hpp"

#include <cstdio>
#include <fstream>

#include <gtest/gtest.h>

using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Http::HttpMethod;
using Azure::Identity::AzurePipelinesCredential;
using Azure::Identity::AzurePipelinesCredentialOptions;
using Azure::Identity::Test::_detail::CredentialTestHelper;

TEST(AzurePipelinesCredential, GetCredentialName)
{
  std::string tenantId = "01234567-89ab-cdef-fedc-ba8976543210";
  std::string clientId = "fedcba98-7654-3210-0123-456789abcdef";
  std::string serviceConnectionId = "abc";
  std::string systemAccessToken = "123";

  AzurePipelinesCredential const cred(tenantId, clientId, serviceConnectionId, systemAccessToken);

  EXPECT_EQ(cred.GetCredentialName(), "AzurePipelinesCredential");
}

TEST(AzurePipelinesCredential, GetOptionsFromEnvironment)
{
  std::string tenantId = "01234567-89ab-cdef-fedc-ba8976543210";
  std::string clientId = "fedcba98-7654-3210-0123-456789abcdef";
  std::string serviceConnectionId = "abc";
  std::string systemAccessToken = "123";

  {
    std::map<std::string, std::string> envVars = {{"AZURE_AUTHORITY_HOST", ""}};
    CredentialTestHelper::EnvironmentOverride const env(envVars);

    AzurePipelinesCredentialOptions options;
    AzurePipelinesCredential const cred(
        tenantId, clientId, serviceConnectionId, systemAccessToken, options);
    EXPECT_EQ(cred.GetCredentialName(), "AzurePipelinesCredential");

    EXPECT_EQ(options.AuthorityHost, "https://login.microsoftonline.com/");
  }

  {
    std::map<std::string, std::string> envVars = {{"AZURE_AUTHORITY_HOST", "foo"}};
    CredentialTestHelper::EnvironmentOverride const env(envVars);

    AzurePipelinesCredentialOptions options;
    options.AuthorityHost = "bar";
    EXPECT_EQ(options.AuthorityHost, "bar");
  }

  {
    std::map<std::string, std::string> envVars
        = {{"AZURE_AUTHORITY_HOST", "https://microsoft.com/"}};
    CredentialTestHelper::EnvironmentOverride const env(envVars);

    AzurePipelinesCredentialOptions options;
    EXPECT_EQ(options.AuthorityHost, "https://microsoft.com/");
  }
}

TEST(AzurePipelinesCredential, InvalidArgs)
{
  std::string tenantId = "01234567-89ab-cdef-fedc-ba8976543210";
  std::string clientId = "fedcba98-7654-3210-0123-456789abcdef";
  std::string serviceConnectionId = "abc";
  std::string systemAccessToken = "123";

  std::map<std::string, std::string> validEnvVars
      = {{"SYSTEM_OIDCREQUESTURI", "https://localhost/instance"}};

  // Empty Oidc Request Uri
  {
    std::map<std::string, std::string> invalidEnvVars = {{"SYSTEM_OIDCREQUESTURI", ""}};
    CredentialTestHelper::EnvironmentOverride const env(invalidEnvVars);

    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");

    AzurePipelinesCredential const cred(tenantId, clientId, serviceConnectionId, systemAccessToken);
    EXPECT_THROW(cred.GetToken(trc, {}), AuthenticationException);
    AzurePipelinesCredentialOptions options;
    AzurePipelinesCredential const credWithOptions(
        tenantId, clientId, serviceConnectionId, systemAccessToken, options);
    EXPECT_THROW(credWithOptions.GetToken(trc, {}), AuthenticationException);
  }

  // Empty Tenant ID
  {
    CredentialTestHelper::EnvironmentOverride const env(validEnvVars);

    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");

    AzurePipelinesCredential const cred("", clientId, serviceConnectionId, systemAccessToken);
    EXPECT_THROW(cred.GetToken(trc, {}), AuthenticationException);
  }

  // Invalid Tenant ID
  {
    CredentialTestHelper::EnvironmentOverride const env(validEnvVars);

    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");

    AzurePipelinesCredential const cred(
        "!=invalidTenantId=!", clientId, serviceConnectionId, systemAccessToken);
    EXPECT_THROW(cred.GetToken(trc, {}), AuthenticationException);
  }

  // Empty client ID
  {
    CredentialTestHelper::EnvironmentOverride const env(validEnvVars);

    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");

    AzurePipelinesCredential const cred(tenantId, "", serviceConnectionId, systemAccessToken);
    EXPECT_THROW(cred.GetToken(trc, {}), AuthenticationException);
  }

  // Empty service connection ID
  {
    CredentialTestHelper::EnvironmentOverride const env(validEnvVars);

    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");

    AzurePipelinesCredential const cred(tenantId, clientId, "", systemAccessToken);
    EXPECT_THROW(cred.GetToken(trc, {}), AuthenticationException);
  }

  // Empty system access token
  {
    CredentialTestHelper::EnvironmentOverride const env(validEnvVars);

    TokenRequestContext trc;
    trc.Scopes.push_back("https://storage.azure.com/.default");

    AzurePipelinesCredential const cred(tenantId, clientId, serviceConnectionId, "");
    EXPECT_THROW(cred.GetToken(trc, {}), AuthenticationException);
  }
}

TEST(AzurePipelinesCredential, Regular)
{
  std::map<std::string, std::string> validEnvVars
      = {{"SYSTEM_OIDCREQUESTURI", "https://localhost/instance"}};
  CredentialTestHelper::EnvironmentOverride const env(validEnvVars);

  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        AzurePipelinesCredentialOptions options;
        options.Transport.Transport = transport;

        std::string tenantId = "01234567-89ab-cdef-fedc-ba8976543210";
        std::string clientId = "fedcba98-7654-3210-0123-456789abcdef";
        std::string serviceConnectionId = "a/bc";
        std::string systemAccessToken = "123";

        return std::make_unique<AzurePipelinesCredential>(
            tenantId, clientId, serviceConnectionId, systemAccessToken, options);
      },
      {{{"https://azure.com/.default"}}},
      std::vector<std::string>{
          "{\"oidcToken\":\"abc/d\"}", "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});

  EXPECT_EQ(actual.Requests.size(), 2U);
  EXPECT_EQ(actual.Responses.size(), 1U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://localhost/instance?api-version=7.1&serviceConnectionId=a%2Fbc");

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    constexpr char expectedBodyStart1[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-"
          "bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&scope=https%3A%2F%2Fazure.com%2F.default"
          "&client_assertion=abc%2Fd"; // cspell:enable

    EXPECT_EQ(request0.Body.size(), 0);
    EXPECT_EQ(request1.Body.size(), (sizeof(expectedBodyStart1) - 1));

    EXPECT_EQ(request1.Body, expectedBodyStart1);

    EXPECT_EQ(request0.Headers.find("Content-Length"), request0.Headers.end());

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_EQ(
        std::stoi(request1.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart1) - 1));
  }

  EXPECT_NE(request0.Headers.find("Content-Type"), request0.Headers.end());
  EXPECT_EQ(request0.Headers.at("Content-Type"), "application/json");

  EXPECT_NE(request1.Headers.find("Content-Type"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");

  using namespace std::chrono_literals;
  EXPECT_GE(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LE(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);
}

TEST(AzurePipelinesCredential, AzureStack)
{
  std::map<std::string, std::string> validEnvVars
      = {{"SYSTEM_OIDCREQUESTURI", "https://localhost/instance"}};
  CredentialTestHelper::EnvironmentOverride const env(validEnvVars);

  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        AzurePipelinesCredentialOptions options;
        options.Transport.Transport = transport;

        std::string tenantId = "adfs";
        std::string clientId = "fedcba98-7654-3210-0123-456789abcdef";
        std::string serviceConnectionId = "a/bc";
        std::string systemAccessToken = "123";

        return std::make_unique<AzurePipelinesCredential>(
            tenantId, clientId, serviceConnectionId, systemAccessToken, options);
      },
      {{{"https://azure.com/.default"}}},
      std::vector<std::string>{
          "{\"oidcToken\":\"abc/d\"}", "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});

  EXPECT_EQ(actual.Requests.size(), 2U);
  EXPECT_EQ(actual.Responses.size(), 1U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://localhost/instance?api-version=7.1&serviceConnectionId=a%2Fbc");

  EXPECT_EQ(request1.AbsoluteUrl, "https://login.microsoftonline.com/adfs/oauth2/token");

  {
    constexpr char expectedBodyStart1[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-"
          "bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&scope=https%3A%2F%2Fazure.com"
          "&client_assertion=abc%2Fd"; // cspell:enable

    EXPECT_EQ(request0.Body.size(), 0);
    EXPECT_EQ(request1.Body.size(), (sizeof(expectedBodyStart1) - 1));

    EXPECT_EQ(request1.Body, expectedBodyStart1);

    EXPECT_EQ(request0.Headers.find("Content-Length"), request0.Headers.end());

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_EQ(
        std::stoi(request1.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart1) - 1));
  }

  EXPECT_NE(request0.Headers.find("Content-Type"), request0.Headers.end());
  EXPECT_EQ(request0.Headers.at("Content-Type"), "application/json");

  EXPECT_NE(request1.Headers.find("Content-Type"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");

  using namespace std::chrono_literals;
  EXPECT_GE(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LE(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);
}

TEST(AzurePipelinesCredential, Authority)
{
  CredentialTestHelper::EnvironmentOverride const env(
      {{"SYSTEM_OIDCREQUESTURI", "https://localhost/instance"},
       {"AZURE_AUTHORITY_HOST", "https://microsoft.com/"}});

  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        AzurePipelinesCredentialOptions options;
        options.Transport.Transport = transport;

        std::string tenantId = "01234567-89ab-cdef-fedc-ba8976543210";
        std::string clientId = "fedcba98-7654-3210-0123-456789abcdef";
        std::string serviceConnectionId = "a/bc";
        std::string systemAccessToken = "123";

        return std::make_unique<AzurePipelinesCredential>(
            tenantId, clientId, serviceConnectionId, systemAccessToken, options);
      },
      {{{"https://azure.com/.default"}}},
      std::vector<std::string>{
          "{\"oidcToken\":\"abc/d\"}", "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});

  EXPECT_EQ(actual.Requests.size(), 2U);
  EXPECT_EQ(actual.Responses.size(), 1U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://localhost/instance?api-version=7.1&serviceConnectionId=a%2Fbc");

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://microsoft.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    constexpr char expectedBodyStart1[] // cspell:disable
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-"
          "bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&scope=https%3A%2F%2Fazure.com%2F.default"
          "&client_assertion=abc%2Fd"; // cspell:enable

    EXPECT_EQ(request0.Body.size(), 0);
    EXPECT_EQ(request1.Body.size(), (sizeof(expectedBodyStart1) - 1));

    EXPECT_EQ(request1.Body, expectedBodyStart1);

    EXPECT_EQ(request0.Headers.find("Content-Length"), request0.Headers.end());

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_EQ(
        std::stoi(request1.Headers.at("Content-Length")),
        static_cast<int>(sizeof(expectedBodyStart1) - 1));
  }

  EXPECT_NE(request0.Headers.find("Content-Type"), request0.Headers.end());
  EXPECT_EQ(request0.Headers.at("Content-Type"), "application/json");

  EXPECT_NE(request1.Headers.find("Content-Type"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");

  using namespace std::chrono_literals;
  EXPECT_GE(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LE(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);
}

TEST(AzurePipelinesCredential, HttpSchemeNotSupported)
{
  CredentialTestHelper::EnvironmentOverride const env(
      {{"SYSTEM_OIDCREQUESTURI", "https://localhost/instance"},
       {"AZURE_AUTHORITY_HOST", "http://microsoft.com/"}});

  try
  {
    auto const actual = CredentialTestHelper::SimulateTokenRequest(
        [](auto transport) {
          AzurePipelinesCredentialOptions options;
          options.Transport.Transport = transport;

          std::string tenantId = "01234567-89ab-cdef-fedc-ba8976543210";
          std::string clientId = "fedcba98-7654-3210-0123-456789abcdef";
          std::string serviceConnectionId = "a/bc";
          std::string systemAccessToken = "123";

          return std::make_unique<AzurePipelinesCredential>(
              tenantId, clientId, serviceConnectionId, systemAccessToken, options);
        },
        {{{"https://azure.com/.default"}}},
        std::vector<std::string>{
            "{\"oidcToken\":\"abc\"}", "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});
  }
  catch (AuthenticationException const& e)
  {
    EXPECT_TRUE(std::string(e.what()).find("https") != std::string::npos) << e.what();
  }
}

TEST(AzurePipelinesCredential, InvalidOidcResponse)
{
  std::map<std::string, std::string> validEnvVars
      = {{"SYSTEM_OIDCREQUESTURI", "https://localhost/instance"}};
  CredentialTestHelper::EnvironmentOverride const env(validEnvVars);

  std::string tenantId = "01234567-89ab-cdef-fedc-ba8976543210";
  std::string clientId = "fedcba98-7654-3210-0123-456789abcdef";
  std::string serviceConnectionId = "a/bc";
  std::string systemAccessToken = "123";

  // Non-OK response
  try
  {
    using Azure::Core::Http::HttpStatusCode;
    std::vector<std::string> const testScopes;
    CredentialTestHelper::TokenRequestSimulationServerResponse testResponse;
    testResponse.StatusCode = HttpStatusCode::BadRequest;
    testResponse.Body = "Invalid response body";

    static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
        [&](auto transport) {
          AzurePipelinesCredentialOptions options;
          options.Transport.Transport = transport;

          return std::make_unique<AzurePipelinesCredential>(
              tenantId, clientId, serviceConnectionId, systemAccessToken, options);
        },
        {testScopes},
        {testResponse}));

    EXPECT_TRUE(!"AzurePipelinesCredential should throw given the response above.");
  }
  catch (AuthenticationException const& ex)
  {
    std::string expectedMessage
        = "AzurePipelinesCredential : 400 (Test) response from the OIDC endpoint. Check service "
          "connection ID and Pipeline configuration.\n\nInvalid response body";
    EXPECT_EQ(ex.what(), expectedMessage) << ex.what();
  }

  // Invalid JSON
  EXPECT_THROW(
      CredentialTestHelper::SimulateTokenRequest(
          [&](auto transport) {
            AzurePipelinesCredentialOptions options;
            options.Transport.Transport = transport;

            return std::make_unique<AzurePipelinesCredential>(
                tenantId, clientId, serviceConnectionId, systemAccessToken, options);
          },
          {{{"https://azure.com/.default"}}},
          std::vector<std::string>{"{\"oidc\":\"abc\"]", ""}),
      AuthenticationException);

  // Missing token
  EXPECT_THROW(
      CredentialTestHelper::SimulateTokenRequest(
          [&](auto transport) {
            AzurePipelinesCredentialOptions options;
            options.Transport.Transport = transport;

            return std::make_unique<AzurePipelinesCredential>(
                tenantId, clientId, serviceConnectionId, systemAccessToken, options);
          },
          {{{"https://azure.com/.default"}}},
          std::vector<std::string>{"{\"oidc\":\"abc\"}", ""}),
      AuthenticationException);

  // Incorrect token type
  EXPECT_THROW(
      CredentialTestHelper::SimulateTokenRequest(
          [&](auto transport) {
            AzurePipelinesCredentialOptions options;
            options.Transport.Transport = transport;

            return std::make_unique<AzurePipelinesCredential>(
                tenantId, clientId, serviceConnectionId, systemAccessToken, options);
          },
          {{{"https://azure.com/.default"}}},
          std::vector<std::string>{"{\"oidcToken\":5}", ""}),
      AuthenticationException);
}
