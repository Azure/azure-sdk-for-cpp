// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/environment_credential.hpp"

#include "credential_test_helper.hpp"

#include <gtest/gtest.h>

using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Http::HttpMethod;
using Azure::Identity::EnvironmentCredential;
using Azure::Identity::Test::_detail::CredentialTestHelper;

TEST(EnvironmentCredential, RegularClientSecretCredential)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", "01234567-89ab-cdef-fedc-ba8976543210"},
             {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
             {"AZURE_CLIENT_SECRET", "CLIENTSECRET"},
             {"AZURE_AUTHORITY_HOST", "https://microsoft.com/"},
             {"AZURE_USERNAME", ""},
             {"AZURE_PASSWORD", ""},
             {"AZURE_CLIENT_CERTIFICATE_PATH", ""}});

        return std::make_unique<EnvironmentCredential>(options);
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});

  EXPECT_EQ(actual.Requests.size(), 1U);
  EXPECT_EQ(actual.Responses.size(), 1U);
  auto const& request = actual.Requests.at(0);
  auto const& response = actual.Responses.at(0);

  EXPECT_EQ(request.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(
      request.AbsoluteUrl,
      "https://microsoft.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    constexpr char expectedBody[]
        = "grant_type=client_credentials"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&client_secret=CLIENTSECRET"
          "&scope=https%3A%2F%2Fazure.com%2F.default"; // cspell:disable-line

    EXPECT_EQ(request.Body, expectedBody);

    EXPECT_NE(request.Headers.find("Content-Length"), request.Headers.end());
    EXPECT_EQ(request.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));
  }

  EXPECT_NE(request.Headers.find("Content-Type"), request.Headers.end());
  EXPECT_EQ(request.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response.AccessToken.Token, "ACCESSTOKEN1");

  using namespace std::chrono_literals;
  EXPECT_GE(response.AccessToken.ExpiresOn, response.EarliestExpiration + 3600s);
  EXPECT_LE(response.AccessToken.ExpiresOn, response.LatestExpiration + 3600s);
}

TEST(EnvironmentCredential, AzureStackClientSecretCredential)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", "adfs"},
             {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
             {"AZURE_CLIENT_SECRET", "CLIENTSECRET"},
             {"AZURE_AUTHORITY_HOST", "https://microsoft.com/"},
             {"AZURE_USERNAME", ""},
             {"AZURE_PASSWORD", ""},
             {"AZURE_CLIENT_CERTIFICATE_PATH", ""}});

        return std::make_unique<EnvironmentCredential>(options);
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});

  EXPECT_EQ(actual.Requests.size(), 1U);
  EXPECT_EQ(actual.Responses.size(), 1U);
  auto const& request = actual.Requests.at(0);
  auto const& response = actual.Responses.at(0);

  EXPECT_EQ(request.AbsoluteUrl, "https://microsoft.com/adfs/oauth2/token");

  {
    constexpr char expectedBody[] = "grant_type=client_credentials"
                                    "&client_id=fedcba98-7654-3210-0123-456789abcdef"
                                    "&client_secret=CLIENTSECRET"
                                    "&scope=https%3A%2F%2Fazure.com"; // cspell:disable-line

    EXPECT_EQ(request.Body, expectedBody);

    EXPECT_NE(request.Headers.find("Content-Length"), request.Headers.end());
    EXPECT_EQ(request.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));
  }

  EXPECT_NE(request.Headers.find("Content-Type"), request.Headers.end());
  EXPECT_EQ(request.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request.Headers.find("Host"), request.Headers.end());
  EXPECT_EQ(request.Headers.at("Host"), "microsoft.com");

  EXPECT_EQ(response.AccessToken.Token, "ACCESSTOKEN1");

  using namespace std::chrono_literals;
  EXPECT_GE(response.AccessToken.ExpiresOn, response.EarliestExpiration + 3600s);
  EXPECT_LE(response.AccessToken.ExpiresOn, response.LatestExpiration + 3600s);
}

TEST(EnvironmentCredential, Unavailable)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", ""},
             {"AZURE_CLIENT_ID", ""},
             {"AZURE_CLIENT_SECRET", ""},
             {"AZURE_AUTHORITY_HOST", ""},
             {"AZURE_USERNAME", ""},
             {"AZURE_PASSWORD", ""},
             {"AZURE_CLIENT_CERTIFICATE_PATH", ""}});

        return std::make_unique<EnvironmentCredential>(options);
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}

TEST(EnvironmentCredential, ClientSecretDefaultAuthority)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", "01234567-89ab-cdef-fedc-ba8976543210"},
             {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
             {"AZURE_CLIENT_SECRET", "CLIENTSECRET"},
             {"AZURE_AUTHORITY_HOST", ""},
             {"AZURE_USERNAME", ""},
             {"AZURE_PASSWORD", ""},
             {"AZURE_CLIENT_CERTIFICATE_PATH", ""}});

        return std::make_unique<EnvironmentCredential>(options);
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});

  EXPECT_EQ(actual.Requests.size(), 1U);
  EXPECT_EQ(actual.Responses.size(), 1U);
  auto const& request = actual.Requests.at(0);
  auto const& response = actual.Responses.at(0);

  EXPECT_EQ(request.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(
      request.AbsoluteUrl,
      "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    constexpr char expectedBody[]
        = "grant_type=client_credentials"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&client_secret=CLIENTSECRET"
          "&scope=https%3A%2F%2Fazure.com%2F.default"; // cspell:disable-line

    EXPECT_EQ(request.Body, expectedBody);

    EXPECT_NE(request.Headers.find("Content-Length"), request.Headers.end());
    EXPECT_EQ(request.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));
  }

  EXPECT_NE(request.Headers.find("Content-Type"), request.Headers.end());
  EXPECT_EQ(request.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response.AccessToken.Token, "ACCESSTOKEN1");

  using namespace std::chrono_literals;
  EXPECT_GE(response.AccessToken.ExpiresOn, response.EarliestExpiration + 3600s);
  EXPECT_LE(response.AccessToken.ExpiresOn, response.LatestExpiration + 3600s);
}

TEST(EnvironmentCredential, ClientSecretNoTenantId)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", ""},
             {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
             {"AZURE_CLIENT_SECRET", "CLIENTSECRET"},
             {"AZURE_AUTHORITY_HOST", "https://microsoft.com/"},
             {"AZURE_USERNAME", ""},
             {"AZURE_PASSWORD", ""},
             {"AZURE_CLIENT_CERTIFICATE_PATH", ""}});

        return std::make_unique<EnvironmentCredential>(options);
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}

TEST(EnvironmentCredential, ClientSecretNoClientId)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", "01234567-89ab-cdef-fedc-ba8976543210"},
             {"AZURE_CLIENT_ID", ""},
             {"AZURE_CLIENT_SECRET", "CLIENTSECRET"},
             {"AZURE_AUTHORITY_HOST", "https://microsoft.com/"},
             {"AZURE_USERNAME", ""},
             {"AZURE_PASSWORD", ""},
             {"AZURE_CLIENT_CERTIFICATE_PATH", ""}});

        return std::make_unique<EnvironmentCredential>(options);
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}

TEST(EnvironmentCredential, ClientSecretNoClientSecret)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", "01234567-89ab-cdef-fedc-ba8976543210"},
             {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
             {"AZURE_CLIENT_SECRET", ""},
             {"AZURE_AUTHORITY_HOST", "https://microsoft.com/"},
             {"AZURE_USERNAME", ""},
             {"AZURE_PASSWORD", ""},
             {"AZURE_CLIENT_CERTIFICATE_PATH", ""}});

        return std::make_unique<EnvironmentCredential>(options);
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}
