// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_secret_credential.hpp"

#include "credential_test_helper.hpp"

#include <gtest/gtest.h>

using Azure::Core::Http::HttpMethod;
using Azure::Identity::ClientSecretCredential;
using Azure::Identity::ClientSecretCredentialOptions;
using Azure::Identity::Test::_detail::CredentialTestHelper;

TEST(ClientSecretCredential, Regular)
{

  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        ClientSecretCredentialOptions options;
        options.AuthorityHost = "https://microsoft.com/";
        options.Transport.Transport = transport;

        return std::make_unique<ClientSecretCredential>(
            "01234567-89ab-cdef-fedc-ba8976543210",
            "fedcba98-7654-3210-0123-456789abcdef",
            "CLIENTSECRET",
            options);
      },
      {{"https://azure.com/.default"}},
      "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}");

  EXPECT_EQ(actual.Requests.size(), 1);
  auto const& request = actual.Requests[0];

  EXPECT_EQ(request.HttpMethod, HttpMethod::Post);

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

TEST(ClientSecretCredential, AzureStack)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        ClientSecretCredentialOptions options;
        options.AuthorityHost = "https://microsoft.com/";
        options.Transport.Transport = transport;

        return std::make_unique<ClientSecretCredential>(
            "adfs", "fedcba98-7654-3210-0123-456789abcdef", "CLIENTSECRET", options);
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
