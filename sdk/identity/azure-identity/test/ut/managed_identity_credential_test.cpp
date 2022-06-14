// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/managed_identity_credential.hpp"

#include "credential_test_helper.hpp"

#include <fstream>

#include <gtest/gtest.h>

using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Http::HttpMethod;
using Azure::Core::Http::HttpStatusCode;
using Azure::Identity::ManagedIdentityCredential;
using Azure::Identity::Test::_detail::CredentialTestHelper;

TEST(ManagedIdentityCredential, AppServiceV2017)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com/"},
            {"MSI_SECRET", "CLIENTSECRET1"},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET2"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}, {{}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}",
          "{\"expires_in\":9999, \"access_token\":\"ACCESSTOKEN3\"}"});

  EXPECT_EQ(actual.Requests.size(), 3U);
  EXPECT_EQ(actual.Responses.size(), 3U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);
  auto const& request2 = actual.Requests.at(2);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);
  auto const& response2 = actual.Responses.at(2);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request2.HttpMethod, HttpMethod::Get);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://microsoft.com"
      "?api-version=2017-09-01"
      "&resource=https%3A%2F%2Fazure.com"); // cspell:disable-line

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://microsoft.com"
      "?api-version=2017-09-01"
      "&resource=https%3A%2F%2Foutlook.com"); // cspell:disable-line

  EXPECT_EQ(
      request2.AbsoluteUrl,
      "https://microsoft.com"
      "?api-version=2017-09-01");

  EXPECT_TRUE(request0.Body.empty());
  EXPECT_TRUE(request1.Body.empty());
  EXPECT_TRUE(request2.Body.empty());

  {
    EXPECT_NE(request0.Headers.find("secret"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("secret"), "CLIENTSECRET1");

    EXPECT_NE(request1.Headers.find("secret"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("secret"), "CLIENTSECRET1");

    EXPECT_NE(request2.Headers.find("secret"), request2.Headers.end());
    EXPECT_EQ(request2.Headers.at("secret"), "CLIENTSECRET1");
  }

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

TEST(ManagedIdentityCredential, AppServiceV2017ClientId)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com/"},
            {"MSI_SECRET", "CLIENTSECRET1"},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET2"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(
            "fedcba98-7654-3210-0123-456789abcdef", options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}, {{}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}",
          "{\"expires_in\":9999, \"access_token\":\"ACCESSTOKEN3\"}"});

  EXPECT_EQ(actual.Requests.size(), 3U);
  EXPECT_EQ(actual.Responses.size(), 3U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);
  auto const& request2 = actual.Requests.at(2);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);
  auto const& response2 = actual.Responses.at(2);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request2.HttpMethod, HttpMethod::Get);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://microsoft.com"
      "?api-version=2017-09-01"
      "&clientid=fedcba98-7654-3210-0123-456789abcdef"
      "&resource=https%3A%2F%2Fazure.com"); // cspell:disable-line

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://microsoft.com"
      "?api-version=2017-09-01"
      "&clientid=fedcba98-7654-3210-0123-456789abcdef"
      "&resource=https%3A%2F%2Foutlook.com"); // cspell:disable-line

  EXPECT_EQ(
      request2.AbsoluteUrl,
      "https://microsoft.com"
      "?api-version=2017-09-01"
      "&clientid=fedcba98-7654-3210-0123-456789abcdef");

  EXPECT_TRUE(request0.Body.empty());
  EXPECT_TRUE(request1.Body.empty());
  EXPECT_TRUE(request2.Body.empty());

  {
    EXPECT_NE(request0.Headers.find("secret"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("secret"), "CLIENTSECRET1");

    EXPECT_NE(request1.Headers.find("secret"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("secret"), "CLIENTSECRET1");

    EXPECT_NE(request2.Headers.find("secret"), request2.Headers.end());
    EXPECT_EQ(request2.Headers.at("secret"), "CLIENTSECRET1");
  }

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

TEST(ManagedIdentityCredential, AppServiceV2017InvalidUrl)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  using Azure::Core::Credentials::AuthenticationException;
  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com:INVALID/"},
            {"MSI_SECRET", "CLIENTSECRET1"},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET2"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        std::unique_ptr<ManagedIdentityCredential const> appServiceV2017ManagedIdentityCredential;
        EXPECT_THROW(
            appServiceV2017ManagedIdentityCredential
            = std::make_unique<ManagedIdentityCredential>(options),
            AuthenticationException);

        return appServiceV2017ManagedIdentityCredential;
      },
      {},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"}));
}

TEST(ManagedIdentityCredential, AppServiceV2017UnsupportedUrl)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  using Azure::Core::Credentials::AuthenticationException;
  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com:65536/"},
            {"MSI_SECRET", "CLIENTSECRET1"},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET2"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        std::unique_ptr<ManagedIdentityCredential const> appServiceV2017ManagedIdentityCredential;
        EXPECT_THROW(
            appServiceV2017ManagedIdentityCredential
            = std::make_unique<ManagedIdentityCredential>(options),
            AuthenticationException);

        return appServiceV2017ManagedIdentityCredential;
      },
      {},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"}));
}

TEST(ManagedIdentityCredential, AppServiceV2019)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com/"},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET2"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}, {{}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}",
          "{\"expires_in\":9999, \"access_token\":\"ACCESSTOKEN3\"}"});

  EXPECT_EQ(actual.Requests.size(), 3U);
  EXPECT_EQ(actual.Responses.size(), 3U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);
  auto const& request2 = actual.Requests.at(2);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);
  auto const& response2 = actual.Responses.at(2);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request2.HttpMethod, HttpMethod::Get);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://visualstudio.com"
      "?api-version=2019-08-01"
      "&resource=https%3A%2F%2Fazure.com"); // cspell:disable-line

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://visualstudio.com"
      "?api-version=2019-08-01"
      "&resource=https%3A%2F%2Foutlook.com"); // cspell:disable-line

  EXPECT_EQ(
      request2.AbsoluteUrl,
      "https://visualstudio.com"
      "?api-version=2019-08-01");

  EXPECT_TRUE(request0.Body.empty());
  EXPECT_TRUE(request1.Body.empty());
  EXPECT_TRUE(request2.Body.empty());

  {
    EXPECT_NE(request0.Headers.find("X-IDENTITY-HEADER"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("X-IDENTITY-HEADER"), "CLIENTSECRET2");

    EXPECT_NE(request1.Headers.find("X-IDENTITY-HEADER"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("X-IDENTITY-HEADER"), "CLIENTSECRET2");

    EXPECT_NE(request2.Headers.find("X-IDENTITY-HEADER"), request2.Headers.end());
    EXPECT_EQ(request2.Headers.at("X-IDENTITY-HEADER"), "CLIENTSECRET2");
  }

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

TEST(ManagedIdentityCredential, AppServiceV2019ClientId)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", "CLIENTSECRET1"},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET2"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(
            "fedcba98-7654-3210-0123-456789abcdef", options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}, {{}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}",
          "{\"expires_in\":9999, \"access_token\":\"ACCESSTOKEN3\"}"});

  EXPECT_EQ(actual.Requests.size(), 3U);
  EXPECT_EQ(actual.Responses.size(), 3U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);
  auto const& request2 = actual.Requests.at(2);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);
  auto const& response2 = actual.Responses.at(2);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request2.HttpMethod, HttpMethod::Get);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://visualstudio.com"
      "?api-version=2019-08-01"
      "&client_id=fedcba98-7654-3210-0123-456789abcdef"
      "&resource=https%3A%2F%2Fazure.com"); // cspell:disable-line

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://visualstudio.com"
      "?api-version=2019-08-01"
      "&client_id=fedcba98-7654-3210-0123-456789abcdef"
      "&resource=https%3A%2F%2Foutlook.com"); // cspell:disable-line

  EXPECT_EQ(
      request2.AbsoluteUrl,
      "https://visualstudio.com"
      "?api-version=2019-08-01"
      "&client_id=fedcba98-7654-3210-0123-456789abcdef");

  EXPECT_TRUE(request0.Body.empty());
  EXPECT_TRUE(request1.Body.empty());
  EXPECT_TRUE(request2.Body.empty());

  {
    EXPECT_NE(request0.Headers.find("X-IDENTITY-HEADER"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("X-IDENTITY-HEADER"), "CLIENTSECRET2");

    EXPECT_NE(request1.Headers.find("X-IDENTITY-HEADER"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("X-IDENTITY-HEADER"), "CLIENTSECRET2");

    EXPECT_NE(request2.Headers.find("X-IDENTITY-HEADER"), request2.Headers.end());
    EXPECT_EQ(request2.Headers.at("X-IDENTITY-HEADER"), "CLIENTSECRET2");
  }

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

TEST(ManagedIdentityCredential, AppServiceV2019InvalidUrl)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  using Azure::Core::Credentials::AuthenticationException;
  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", "CLIENTSECRET1"},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com:INVALID/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET2"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        std::unique_ptr<ManagedIdentityCredential const> appServiceV2019ManagedIdentityCredential;
        EXPECT_THROW(
            appServiceV2019ManagedIdentityCredential
            = std::make_unique<ManagedIdentityCredential>(options),
            AuthenticationException);

        return appServiceV2019ManagedIdentityCredential;
      },
      {},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"}));
}

TEST(ManagedIdentityCredential, AppServiceV2019UnsupportedUrl)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  using Azure::Core::Credentials::AuthenticationException;
  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com/"},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com:65536/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET2"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        std::unique_ptr<ManagedIdentityCredential const> appServiceV2019ManagedIdentityCredential;
        EXPECT_THROW(
            appServiceV2019ManagedIdentityCredential
            = std::make_unique<ManagedIdentityCredential>(options),
            AuthenticationException);

        return appServiceV2019ManagedIdentityCredential;
      },
      {},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"}));
}

TEST(ManagedIdentityCredential, CloudShell)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com/"},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", ""},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "SECRET2"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}, {{}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}",
          "{\"expires_in\":9999, \"access_token\":\"ACCESSTOKEN3\"}"});

  EXPECT_EQ(actual.Requests.size(), 3U);
  EXPECT_EQ(actual.Responses.size(), 3U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);
  auto const& request2 = actual.Requests.at(2);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);
  auto const& response2 = actual.Responses.at(2);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request2.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(request0.AbsoluteUrl, "https://microsoft.com");
  EXPECT_EQ(request1.AbsoluteUrl, "https://microsoft.com");
  EXPECT_EQ(request2.AbsoluteUrl, "https://microsoft.com");

  EXPECT_EQ(request0.Body, "resource=https%3A%2F%2Fazure.com"); // cspell:disable-line
  EXPECT_EQ(request1.Body, "resource=https%3A%2F%2Foutlook.com"); // cspell:disable-line
  EXPECT_EQ(request2.Body, std::string());

  {
    EXPECT_NE(request0.Headers.find("Metadata"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("Metadata"), "true");

    EXPECT_NE(request1.Headers.find("Metadata"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Metadata"), "true");

    EXPECT_NE(request2.Headers.find("Metadata"), request2.Headers.end());
    EXPECT_EQ(request2.Headers.at("Metadata"), "true");
  }

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

TEST(ManagedIdentityCredential, CloudShellClientId)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com/"},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(
            "fedcba98-7654-3210-0123-456789abcdef", options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}, {{}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}",
          "{\"expires_in\":9999, \"access_token\":\"ACCESSTOKEN3\"}"});

  EXPECT_EQ(actual.Requests.size(), 3U);
  EXPECT_EQ(actual.Responses.size(), 3U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);
  auto const& request2 = actual.Requests.at(2);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);
  auto const& response2 = actual.Responses.at(2);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request2.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(request0.AbsoluteUrl, "https://microsoft.com");
  EXPECT_EQ(request1.AbsoluteUrl, "https://microsoft.com");
  EXPECT_EQ(request2.AbsoluteUrl, "https://microsoft.com");

  EXPECT_EQ(
      request0.Body,
      "resource=https%3A%2F%2Fazure.com&client_id=fedcba98-7654-3210-0123-456789abcdef"); // cspell:disable-line

  EXPECT_EQ(
      request1.Body,
      "resource=https%3A%2F%2Foutlook.com&client_id=fedcba98-7654-3210-0123-456789abcdef"); // cspell:disable-line

  EXPECT_EQ(request2.Body, "client_id=fedcba98-7654-3210-0123-456789abcdef");

  {
    EXPECT_NE(request0.Headers.find("Metadata"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("Metadata"), "true");

    EXPECT_NE(request1.Headers.find("Metadata"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Metadata"), "true");

    EXPECT_NE(request2.Headers.find("Metadata"), request2.Headers.end());
    EXPECT_EQ(request2.Headers.at("Metadata"), "true");
  }

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

TEST(ManagedIdentityCredential, CloudShellInvalidUrl)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com:INVALID/"},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        std::unique_ptr<ManagedIdentityCredential const> cloudShellManagedIdentityCredential;
        EXPECT_THROW(
            cloudShellManagedIdentityCredential
            = std::make_unique<ManagedIdentityCredential>(options),
            AuthenticationException);

        return cloudShellManagedIdentityCredential;
      },
      {},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"}));
}

TEST(ManagedIdentityCredential, AzureArc)
{
  {
    std::ofstream secretFile(
        "managed_identity_credential_test1.txt", std::ios_base::out | std::ios_base::trunc);

    secretFile << "SECRET1";
  }

  {
    std::ofstream secretFile(
        "managed_identity_credential_test2.txt", std::ios_base::out | std::ios_base::trunc);

    secretFile << "SECRET2";
  }

  {
    std::ofstream secretFile(
        "managed_identity_credential_test3.txt", std::ios_base::out | std::ios_base::trunc);

    secretFile << "SECRET3";
  }

  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}, {{}}},
      {{HttpStatusCode::Unauthorized,
        "",
        {{"WWW-Authenticate", "ABC ABC=managed_identity_credential_test1.txt"}}},
       {HttpStatusCode::Ok, "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}", {}},
       {HttpStatusCode::Unauthorized,
        "",
        {{"WWW-Authenticate", "XYZ XYZ=managed_identity_credential_test2.txt"}}},
       {HttpStatusCode::Ok, "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}", {}},
       {HttpStatusCode::Unauthorized,
        "",
        {{"WWW-Authenticate", "ABC ABC=managed_identity_credential_test3.txt"}}},
       {HttpStatusCode::Ok, "{\"expires_in\":9999, \"access_token\":\"ACCESSTOKEN3\"}", {}}});

  EXPECT_EQ(actual.Requests.size(), 6U);
  EXPECT_EQ(actual.Responses.size(), 3U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);
  auto const& request2 = actual.Requests.at(2);
  auto const& request3 = actual.Requests.at(3);
  auto const& request4 = actual.Requests.at(4);
  auto const& request5 = actual.Requests.at(5);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);
  auto const& response2 = actual.Responses.at(2);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request2.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request3.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request4.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request5.HttpMethod, HttpMethod::Get);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://visualstudio.com?api-version=2019-11-01&resource=https%3A%2F%2Fazure.com"); // cspell:disable-line

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://visualstudio.com?api-version=2019-11-01&resource=https%3A%2F%2Fazure.com"); // cspell:disable-line

  EXPECT_EQ(
      request2.AbsoluteUrl,
      "https://visualstudio.com?api-version=2019-11-01&resource=https%3A%2F%2Foutlook.com"); // cspell:disable-line

  EXPECT_EQ(
      request3.AbsoluteUrl,
      "https://visualstudio.com?api-version=2019-11-01&resource=https%3A%2F%2Foutlook.com"); // cspell:disable-line

  EXPECT_EQ(request4.AbsoluteUrl, "https://visualstudio.com?api-version=2019-11-01");
  EXPECT_EQ(request5.AbsoluteUrl, "https://visualstudio.com?api-version=2019-11-01");

  EXPECT_TRUE(request0.Body.empty());
  EXPECT_TRUE(request1.Body.empty());
  EXPECT_TRUE(request2.Body.empty());
  EXPECT_TRUE(request3.Body.empty());
  EXPECT_TRUE(request4.Body.empty());
  EXPECT_TRUE(request5.Body.empty());

  {
    EXPECT_NE(request0.Headers.find("Metadata"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("Metadata"), "true");

    EXPECT_NE(request1.Headers.find("Metadata"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Metadata"), "true");

    EXPECT_NE(request2.Headers.find("Metadata"), request2.Headers.end());
    EXPECT_EQ(request2.Headers.at("Metadata"), "true");

    EXPECT_NE(request3.Headers.find("Metadata"), request3.Headers.end());
    EXPECT_EQ(request3.Headers.at("Metadata"), "true");

    EXPECT_NE(request4.Headers.find("Metadata"), request4.Headers.end());
    EXPECT_EQ(request4.Headers.at("Metadata"), "true");

    EXPECT_NE(request5.Headers.find("Metadata"), request5.Headers.end());
    EXPECT_EQ(request5.Headers.at("Metadata"), "true");
  }

  {
    EXPECT_EQ(request0.Headers.find("Authorization"), request0.Headers.end());

    EXPECT_NE(request1.Headers.find("Authorization"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Authorization"), "Basic SECRET1");

    EXPECT_EQ(request2.Headers.find("Authorization"), request2.Headers.end());

    EXPECT_NE(request3.Headers.find("Authorization"), request3.Headers.end());
    EXPECT_EQ(request3.Headers.at("Authorization"), "Basic SECRET2");

    EXPECT_EQ(request4.Headers.find("Authorization"), request4.Headers.end());

    EXPECT_NE(request5.Headers.find("Authorization"), request5.Headers.end());
    EXPECT_EQ(request5.Headers.at("Authorization"), "Basic SECRET3");
  }

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

TEST(ManagedIdentityCredential, AzureArcClientId)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        std::unique_ptr<ManagedIdentityCredential const> azureArcManagedIdentityCredential;
        EXPECT_THROW(
            azureArcManagedIdentityCredential = std::make_unique<ManagedIdentityCredential>(
                "fedcba98-7654-3210-0123-456789abcdef", options),
            AuthenticationException);

        return azureArcManagedIdentityCredential;
      },
      {},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"}));
}

TEST(ManagedIdentityCredential, AzureArcAuthHeaderMissing)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}},
      {{HttpStatusCode::Unauthorized, "", {}},
       {HttpStatusCode::Ok, "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}", {}}},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}

TEST(ManagedIdentityCredential, AzureArcUnexpectedHttpStatusCode)
{
  {
    std::ofstream secretFile(
        "managed_identity_credential_test0.txt", std::ios_base::out | std::ios_base::trunc);

    secretFile << "SECRET0";
  }

  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}},
      {{HttpStatusCode::Forbidden,
        "",
        {{"WWW-Authenticate", "ABC ABC=managed_identity_credential_test0.txt"}}},
       {HttpStatusCode::Ok, "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}", {}}},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}

TEST(ManagedIdentityCredential, AzureArcAuthHeaderNoEquals)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}},
      {{HttpStatusCode::Unauthorized, "", {{"WWW-Authenticate", "ABCSECRET1"}}},
       {HttpStatusCode::Ok, "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}", {}}},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}

TEST(ManagedIdentityCredential, AzureArcAuthHeaderTwoEquals)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}},
      {{HttpStatusCode::Unauthorized, "", {{"WWW-Authenticate", "ABC=SECRET1=SECRET2"}}},
       {HttpStatusCode::Ok, "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}", {}}},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}

TEST(ManagedIdentityCredential, AzureArcInvalidUrl)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com:INVALID/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        std::unique_ptr<ManagedIdentityCredential const> azureArcManagedIdentityCredential;
        EXPECT_THROW(
            azureArcManagedIdentityCredential
            = std::make_unique<ManagedIdentityCredential>(options),
            AuthenticationException);

        return azureArcManagedIdentityCredential;
      },
      {},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"}));
}

TEST(ManagedIdentityCredential, Imds)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", ""},
            {"IMDS_ENDPOINT", ""},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", ""},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}, {{}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}",
          "{\"expires_in\":9999, \"access_token\":\"ACCESSTOKEN3\"}"});

  EXPECT_EQ(actual.Requests.size(), 3U);
  EXPECT_EQ(actual.Responses.size(), 3U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);
  auto const& request2 = actual.Requests.at(2);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);
  auto const& response2 = actual.Responses.at(2);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request2.HttpMethod, HttpMethod::Get);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "http://169.254.169.254/metadata/identity/oauth2/token"
      "?api-version=2018-02-01"
      "&resource=https%3A%2F%2Fazure.com"); // cspell:disable-line

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "http://169.254.169.254/metadata/identity/oauth2/token"
      "?api-version=2018-02-01"
      "&resource=https%3A%2F%2Foutlook.com"); // cspell:disable-line

  EXPECT_EQ(
      request2.AbsoluteUrl,
      "http://169.254.169.254/metadata/identity/oauth2/token"
      "?api-version=2018-02-01");

  EXPECT_TRUE(request0.Body.empty());
  EXPECT_TRUE(request1.Body.empty());
  EXPECT_TRUE(request2.Body.empty());

  {
    EXPECT_NE(request0.Headers.find("Metadata"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("Metadata"), "true");

    EXPECT_NE(request1.Headers.find("Metadata"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Metadata"), "true");

    EXPECT_NE(request2.Headers.find("Metadata"), request2.Headers.end());
    EXPECT_EQ(request2.Headers.at("Metadata"), "true");
  }

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

TEST(ManagedIdentityCredential, ImdsClientId)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", ""},
            {"IMDS_ENDPOINT", ""},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", ""},
        });

        return std::make_unique<ManagedIdentityCredential>(
            "fedcba98-7654-3210-0123-456789abcdef", options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}, {{}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}",
          "{\"expires_in\":9999, \"access_token\":\"ACCESSTOKEN3\"}"});

  EXPECT_EQ(actual.Requests.size(), 3U);
  EXPECT_EQ(actual.Responses.size(), 3U);

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);
  auto const& request2 = actual.Requests.at(2);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);
  auto const& response2 = actual.Responses.at(2);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request2.HttpMethod, HttpMethod::Get);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "http://169.254.169.254/metadata/identity/oauth2/token"
      "?api-version=2018-02-01"
      "&client_id=fedcba98-7654-3210-0123-456789abcdef"
      "&resource=https%3A%2F%2Fazure.com"); // cspell:disable-line

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "http://169.254.169.254/metadata/identity/oauth2/token"
      "?api-version=2018-02-01"
      "&client_id=fedcba98-7654-3210-0123-456789abcdef"
      "&resource=https%3A%2F%2Foutlook.com"); // cspell:disable-line

  EXPECT_EQ(
      request2.AbsoluteUrl,
      "http://169.254.169.254/metadata/identity/oauth2/token"
      "?api-version=2018-02-01"
      "&client_id=fedcba98-7654-3210-0123-456789abcdef");

  EXPECT_TRUE(request0.Body.empty());
  EXPECT_TRUE(request1.Body.empty());
  EXPECT_TRUE(request2.Body.empty());

  {
    EXPECT_NE(request0.Headers.find("Metadata"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("Metadata"), "true");

    EXPECT_NE(request1.Headers.find("Metadata"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Metadata"), "true");

    EXPECT_NE(request2.Headers.find("Metadata"), request2.Headers.end());
    EXPECT_EQ(request2.Headers.at("Metadata"), "true");
  }

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

TEST(ManagedIdentityCredential, ImdsCreation)
{
  auto const actual1 = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", ""},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", ""},
        });

        return std::make_unique<ManagedIdentityCredential>(
            "fedcba98-7654-3210-0123-456789abcdef", options);
      },
      {{{"https://azure.com/.default"}}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});

  auto const actual2 = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", ""},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", ""},
        });

        return std::make_unique<ManagedIdentityCredential>(
            "01234567-89ab-cdef-fedc-ba9876543210", options);
      },
      {{{"https://outlook.com/.default"}}},
      {"{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  EXPECT_EQ(actual1.Requests.size(), 1U);
  EXPECT_EQ(actual1.Responses.size(), 1U);

  EXPECT_EQ(actual2.Requests.size(), 1U);
  EXPECT_EQ(actual2.Responses.size(), 1U);

  auto const& request1 = actual1.Requests.at(0);
  auto const& response1 = actual1.Responses.at(0);

  auto const& request2 = actual2.Requests.at(0);
  auto const& response2 = actual2.Responses.at(0);

  EXPECT_EQ(request1.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request2.HttpMethod, HttpMethod::Get);

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "http://169.254.169.254/metadata/identity/oauth2/token"
      "?api-version=2018-02-01"
      "&client_id=fedcba98-7654-3210-0123-456789abcdef"
      "&resource=https%3A%2F%2Fazure.com"); // cspell:disable-line

  EXPECT_EQ(
      request2.AbsoluteUrl,
      "http://169.254.169.254/metadata/identity/oauth2/token"
      "?api-version=2018-02-01"
      "&client_id=01234567-89ab-cdef-fedc-ba9876543210"
      "&resource=https%3A%2F%2Foutlook.com"); // cspell:disable-line

  EXPECT_TRUE(request1.Body.empty());
  EXPECT_TRUE(request2.Body.empty());

  {
    EXPECT_NE(request1.Headers.find("Metadata"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Metadata"), "true");

    EXPECT_NE(request2.Headers.find("Metadata"), request2.Headers.end());
    EXPECT_EQ(request2.Headers.at("Metadata"), "true");
  }

  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response2.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GE(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 3600s);
  EXPECT_LE(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 3600s);

  EXPECT_GE(response2.AccessToken.ExpiresOn, response2.EarliestExpiration + 7200s);
  EXPECT_LE(response2.AccessToken.ExpiresOn, response2.LatestExpiration + 7200s);
}
