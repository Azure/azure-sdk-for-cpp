// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_secret_credential.hpp"

#include "credential_test_helper.hpp"

#include <gtest/gtest.h>

using Azure::Core::Http::HttpMethod;
using Azure::Identity::ChallengeClientSecretCredential;
using Azure::Identity::ClientSecretCredential;
using Azure::Identity::ClientSecretCredentialOptions;
using Azure::Identity::Test::_detail::CredentialTestHelper;

TEST(ClientSecretCredential, Regular)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        ClientSecretCredentialOptions options;
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
      "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

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
  EXPECT_GE(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LE(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GE(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LE(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

TEST(ClientSecretCredential, AzureStack)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        ClientSecretCredentialOptions options;
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

  EXPECT_EQ(request0.AbsoluteUrl, "https://login.microsoftonline.com/adfs/oauth2/token");
  EXPECT_EQ(request1.AbsoluteUrl, "https://login.microsoftonline.com/adfs/oauth2/token");

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
  EXPECT_EQ(request0.Headers.at("Host"), "login.microsoftonline.com");

  EXPECT_NE(request1.Headers.find("Host"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Host"), "login.microsoftonline.com");

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GE(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LE(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GE(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LE(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

TEST(ClientSecretCredential, Authority)
{
  auto const actual1 = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        ClientSecretCredentialOptions options;
        options.AuthorityHost = "https://microsoft.com/";
        options.Transport.Transport = transport;

        return std::make_unique<ClientSecretCredential>(
            "01234567-89ab-cdef-fedc-ba8976543210",
            "fedcba98-7654-3210-0123-456789abcdef",
            "CLIENTSECRET1",
            options);
      },
      {{{"https://azure.com/.default"}}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});

  auto const actual2 = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        ClientSecretCredentialOptions options;
        options.AuthorityHost = "https://xbox.com/";
        options.Transport.Transport = transport;

        return std::make_unique<ClientSecretCredential>(
            "adfs", "01234567-89ab-cdef-fedc-ba8976543210", "CLIENTSECRET2", options);
      },
      {{{"https://outlook.com/.default"}}},
      {"{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  EXPECT_EQ(actual1.Requests.size(), 1U);
  EXPECT_EQ(actual1.Responses.size(), 1U);

  EXPECT_EQ(actual2.Requests.size(), 1U);
  EXPECT_EQ(actual2.Responses.size(), 1U);

  auto const& request1 = actual1.Requests.at(0);
  auto const& request2 = actual2.Requests.at(0);

  auto const& response1 = actual1.Responses.at(0);
  auto const& response2 = actual2.Responses.at(0);

  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request2.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://microsoft.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  EXPECT_EQ(request2.AbsoluteUrl, "https://xbox.com/adfs/oauth2/token");

  {
    constexpr char expectedBody1[]
        = "grant_type=client_credentials"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&client_secret=CLIENTSECRET1"
          "&scope=https%3A%2F%2Fazure.com%2F.default"; // cspell:disable-line

    constexpr char expectedBody2[] = "grant_type=client_credentials"
                                     "&client_id=01234567-89ab-cdef-fedc-ba8976543210"
                                     "&client_secret=CLIENTSECRET2"
                                     "&scope=https%3A%2F%2Foutlook.com"; // cspell:disable-line

    EXPECT_EQ(request1.Body, expectedBody1);
    EXPECT_EQ(request2.Body, expectedBody2);

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody1) - 1));

    EXPECT_NE(request2.Headers.find("Content-Length"), request2.Headers.end());
    EXPECT_EQ(request2.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody2) - 1));
  }

  EXPECT_NE(request1.Headers.find("Content-Type"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request2.Headers.find("Content-Type"), request2.Headers.end());
  EXPECT_EQ(request2.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request2.Headers.find("Host"), request2.Headers.end());
  EXPECT_EQ(request2.Headers.at("Host"), "xbox.com");

  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response2.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GE(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 3600s);
  EXPECT_LE(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 3600s);

  EXPECT_GE(response2.AccessToken.ExpiresOn, response2.EarliestExpiration + 7200s);
  EXPECT_LE(response2.AccessToken.ExpiresOn, response2.LatestExpiration + 7200s);
}

TEST(ChallengeClientSecretCredential, Regular)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        ClientSecretCredentialOptions options;
        options.Transport.Transport = transport;

        return std::make_unique<ChallengeClientSecretCredential>(
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
      "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    constexpr char expectedBody0[]
        = "grant_type=client_credentials&client_id=fedcba98-7654-3210-"
          "0123-456789abcdef&client_secret=CLIENTSECRET&response_type="
          "token&scope=https%3A%2F%2Fazure.com%2F.default"; // cspell:disable-line

    constexpr char expectedBody1[]
        = "grant_type=client_credentials&client_id=fedcba98-7654-3210-0123-456789abcdef&client_"
          "secret=CLIENTSECRET&response_type=token"; // cspell:disable-line

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
  EXPECT_GE(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LE(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GE(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LE(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

TEST(ChallengeClientSecretCredential, AzureStack)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        ClientSecretCredentialOptions options;
        options.Transport.Transport = transport;

        return std::make_unique<ChallengeClientSecretCredential>(
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

  EXPECT_EQ(request0.AbsoluteUrl, "https://login.microsoftonline.com/adfs/oauth2/token");
  EXPECT_EQ(request1.AbsoluteUrl, "https://login.microsoftonline.com/adfs/oauth2/token");

  {
    constexpr char expectedBody0[] = "grant_type=client_credentials&client_id=fedcba98-7654-3210-"
                                     "0123-456789abcdef&client_secret=CLIENTSECRET&response_type="
                                     "token&scope=https%3A%2F%2Fazure.com"; // cspell:disable-line

    constexpr char expectedBody1[]
        = "grant_type=client_credentials&client_id=fedcba98-7654-3210-0123-456789abcdef&client_"
          "secret=CLIENTSECRET&response_type=token"; // cspell:disable-line

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
  EXPECT_EQ(request0.Headers.at("Host"), "login.microsoftonline.com");

  EXPECT_NE(request1.Headers.find("Host"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Host"), "login.microsoftonline.com");

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GE(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LE(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GE(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LE(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

TEST(ChallengeClientSecretCredential, Authority)
{
  auto const actual1 = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        ClientSecretCredentialOptions options;
        options.AuthorityHost = "https://microsoft.com/";
        options.Transport.Transport = transport;

        return std::make_unique<ChallengeClientSecretCredential>(
            "01234567-89ab-cdef-fedc-ba8976543210",
            "fedcba98-7654-3210-0123-456789abcdef",
            "CLIENTSECRET1",
            options);
      },
      {{{"https://azure.com/.default"}}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});

  auto const actual2 = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        ClientSecretCredentialOptions options;
        options.AuthorityHost = "https://xbox.com/";
        options.Transport.Transport = transport;

        return std::make_unique<ClientSecretCredential>(
            "adfs", "01234567-89ab-cdef-fedc-ba8976543210", "CLIENTSECRET2", options);
      },
      {{{"https://outlook.com/.default"}}},
      {"{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  EXPECT_EQ(actual1.Requests.size(), 1U);
  EXPECT_EQ(actual1.Responses.size(), 1U);

  EXPECT_EQ(actual2.Requests.size(), 1U);
  EXPECT_EQ(actual2.Responses.size(), 1U);

  auto const& request1 = actual1.Requests.at(0);
  auto const& request2 = actual2.Requests.at(0);

  auto const& response1 = actual1.Responses.at(0);
  auto const& response2 = actual2.Responses.at(0);

  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request2.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://microsoft.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  EXPECT_EQ(request2.AbsoluteUrl, "https://xbox.com/adfs/oauth2/token");

  {
    constexpr char expectedBody1[]
        = "grant_type=client_credentials&client_id=fedcba98-7654-3210-"
          "0123-456789abcdef&client_secret=CLIENTSECRET1&response_type="
          "token&scope=https%3A%2F%2Fazure.com%2F.default"; // cspell:disable-line

    constexpr char expectedBody2[] = "grant_type=client_credentials"
                                     "&client_id=01234567-89ab-cdef-fedc-ba8976543210"
                                     "&client_secret=CLIENTSECRET2"
                                     "&scope=https%3A%2F%2Foutlook.com"; // cspell:disable-line

    EXPECT_EQ(request1.Body, expectedBody1);
    EXPECT_EQ(request2.Body, expectedBody2);

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody1) - 1));

    EXPECT_NE(request2.Headers.find("Content-Length"), request2.Headers.end());
    EXPECT_EQ(request2.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody2) - 1));
  }

  EXPECT_NE(request1.Headers.find("Content-Type"), request1.Headers.end());
  EXPECT_EQ(request1.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request2.Headers.find("Content-Type"), request2.Headers.end());
  EXPECT_EQ(request2.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request2.Headers.find("Host"), request2.Headers.end());
  EXPECT_EQ(request2.Headers.at("Host"), "xbox.com");

  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response2.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GE(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 3600s);
  EXPECT_LE(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 3600s);

  EXPECT_GE(response2.AccessToken.ExpiresOn, response2.EarliestExpiration + 7200s);
  EXPECT_LE(response2.AccessToken.ExpiresOn, response2.LatestExpiration + 7200s);
}