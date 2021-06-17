// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/environment_credential.hpp"

#include "credential_test_helper.hpp"

#include <gtest/gtest.h>

using Azure::Core::Context;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Identity::EnvironmentCredential;
using Azure::Identity::Test::_detail::CredentialTestHelper;

TEST(EnvironmentCredential, RegularClientSecretCredential)
{
  if (!CredentialTestHelper::EnvironmentOverride::IsEnvironmentAvailable)
  {
    return;
  }

  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", "01234567-89ab-cdef-fedc-ba8976543210"},
             {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
             {"AZURE_CLIENT_SECRET", "CLIENTSECRET"},
             {"AZURE_AUTHORITY_HOST", "https://microsoft.com/"}});

        return std::make_unique<EnvironmentCredential>(options);
      },
      {{"https://azure.com/.default"}},
      "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}");

  EXPECT_EQ(actual.Requests.size(), 1);
  auto const& request = actual.Requests[0];

  EXPECT_EQ(
      request.AbsoluteUrl,
      "https://microsoft.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    constexpr char expectedBody[] = "grant_type=client_credentials"
                                    "&client_id=fedcba98-7654-3210-0123-456789abcdef"
                                    "&client_secret=CLIENTSECRET"
                                    "&scope=https%3A%2F%2Fazure.com%2F.default";

    EXPECT_EQ(request.Body, expectedBody);

    EXPECT_NE(request.Headers.find("Content-Length"), request.Headers.end());
    EXPECT_EQ(request.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));
  }

  EXPECT_NE(request.Headers.find("Content-Type"), request.Headers.end());
  EXPECT_EQ(request.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(actual.Response.AccessToken.Token, "ACCESSTOKEN1");

  using namespace std::chrono_literals;
  EXPECT_GT(actual.Response.AccessToken.ExpiresOn, actual.Response.EarliestExpiration + 3600s);
  EXPECT_LT(actual.Response.AccessToken.ExpiresOn, actual.Response.LatestExpiration + 3600s);
}

TEST(EnvironmentCredential, AzureStackClientSecretCredential)
{
  if (!CredentialTestHelper::EnvironmentOverride::IsEnvironmentAvailable)
  {
    return;
  }

  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", "adfs"},
             {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
             {"AZURE_CLIENT_SECRET", "CLIENTSECRET"},
             {"AZURE_AUTHORITY_HOST", "https://microsoft.com/"}});

        return std::make_unique<EnvironmentCredential>(options);
      },
      {{"https://azure.com/.default"}},
      "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}");

  EXPECT_EQ(actual.Requests.size(), 1);
  auto const& request = actual.Requests[0];

  EXPECT_EQ(request.AbsoluteUrl, "https://microsoft.com/adfs/oauth2/token");

  {
    constexpr char expectedBody[] = "grant_type=client_credentials"
                                    "&client_id=fedcba98-7654-3210-0123-456789abcdef"
                                    "&client_secret=CLIENTSECRET"
                                    "&scope=https%3A%2F%2Fazure.com";

    EXPECT_EQ(request.Body, expectedBody);

    EXPECT_NE(request.Headers.find("Content-Length"), request.Headers.end());
    EXPECT_EQ(request.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));
  }

  EXPECT_NE(request.Headers.find("Content-Type"), request.Headers.end());
  EXPECT_EQ(request.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request.Headers.find("Host"), request.Headers.end());
  EXPECT_EQ(request.Headers.at("Host"), "microsoft.com");

  EXPECT_EQ(actual.Response.AccessToken.Token, "ACCESSTOKEN1");

  using namespace std::chrono_literals;
  EXPECT_GT(actual.Response.AccessToken.ExpiresOn, actual.Response.EarliestExpiration + 3600s);
  EXPECT_LT(actual.Response.AccessToken.ExpiresOn, actual.Response.LatestExpiration + 3600s);
}

TEST(EnvironmentCredential, Unavailable)
{
  CredentialTestHelper::EnvironmentOverride const env(
      {{"AZURE_TENANT_ID", ""},
       {"AZURE_CLIENT_ID", ""},
       {"AZURE_CLIENT_SECRET", ""},
       {"AZURE_AUTHORITY_HOST", ""},
       {"AZURE_USERNAME", ""},
       {"AZURE_PASSWORD", ""},
       {"AZURE_CLIENT_CERTIFICATE_PATH", ""}});

  EnvironmentCredential credential;

  EXPECT_THROW(
      credential.GetToken({{"https://azure.com/.default"}}, Context()), AuthenticationException);
}
