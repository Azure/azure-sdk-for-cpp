// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/managed_identity_credential.hpp"

#include "credential_test_helper.hpp"

#include <gtest/gtest.h>

using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Http::HttpMethod;
using Azure::Core::Http::HttpStatusCode;
using Azure::Identity::ManagedIdentityCredential;
using Azure::Identity::Test::_detail::CredentialTestHelper;

TEST(ManagedIdentityCredential, AppServiceRegular)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com/"},
            {"MSI_SECRET", "CLIENTSECRET"},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{"https://azure.com/.default"}},
      "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}");

  EXPECT_EQ(actual.Requests.size(), 1);
  auto const& request = actual.Requests[0];

  EXPECT_EQ(request.HttpMethod, HttpMethod::Get);

  EXPECT_EQ(
      request.AbsoluteUrl,
      "https://microsoft.com?api-version=2017-09-01&resource=https://azure.com");

  EXPECT_TRUE(request.Body.empty());

  EXPECT_NE(request.Headers.find("secret"), request.Headers.end());
  EXPECT_EQ(request.Headers.at("secret"), "CLIENTSECRET");

  EXPECT_EQ(actual.Response.AccessToken.Token, "ACCESSTOKEN1");

  using namespace std::chrono_literals;
  EXPECT_GT(actual.Response.AccessToken.ExpiresOn, actual.Response.EarliestExpiration + 3600s);
  EXPECT_LT(actual.Response.AccessToken.ExpiresOn, actual.Response.LatestExpiration + 3600s);
}
