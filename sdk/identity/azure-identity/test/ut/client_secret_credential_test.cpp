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
    constexpr char expectedBody0[]
        = "grant_type=client_credentials"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&client_secret=CLIENTSECRET"
          "&scope=https%3A%2F%2Fazure.com%2F.default"; // cspell:disable-line

    constexpr char expectedBody1[] = "grant_type=client_credentials"
                                     "&client_id=fedcba98-7654-3210-0123-456789abcdef"
                                     "&client_secret=CLIENTSECRET";

    EXPECT_EQ(request0.Body, expectedBody0);
    EXPECT_EQ(request1.Body, expectedBody1);

    EXPECT_NE(request0.Headers.find("Content-Length"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody0) - 1));

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody1) - 1));
  }

  EXPECT_NE(request0.Headers.find("Content-Type"), request0.Headers.end());
  EXPECT_EQ(request0.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request1.Headers.find("Content-Type"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GT(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LT(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GT(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LT(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
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

  EXPECT_EQ(request0.AbsoluteUrl, "https://microsoft.com/adfs/oauth2/token");
  EXPECT_EQ(request1.AbsoluteUrl, "https://microsoft.com/adfs/oauth2/token");

  {
    constexpr char expectedBody0[] = "grant_type=client_credentials"
                                     "&client_id=fedcba98-7654-3210-0123-456789abcdef"
                                     "&client_secret=CLIENTSECRET"
                                     "&scope=https%3A%2F%2Fazure.com"; // cspell:disable-line

    constexpr char expectedBody1[] = "grant_type=client_credentials"
                                     "&client_id=fedcba98-7654-3210-0123-456789abcdef"
                                     "&client_secret=CLIENTSECRET";

    EXPECT_EQ(request0.Body, expectedBody0);
    EXPECT_EQ(request1.Body, expectedBody1);

    EXPECT_NE(request0.Headers.find("Content-Length"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody0) - 1));

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody1) - 1));
  }

  EXPECT_NE(request0.Headers.find("Content-Type"), request0.Headers.end());
  EXPECT_EQ(request0.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request1.Headers.find("Content-Type"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request0.Headers.find("Host"), request0.Headers.end());
  EXPECT_EQ(request0.Headers.at("Host"), "microsoft.com");

  EXPECT_NE(request1.Headers.find("Host"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Host"), "microsoft.com");

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GT(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LT(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GT(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LT(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}
