// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_certificate_credential.hpp"

#include "credential_test_helper.hpp"

#include <gtest/gtest.h>

using Azure::Core::Http::HttpMethod;
using Azure::Identity::ClientCertificateCredential;
using Azure::Identity::ClientCertificateCredentialOptions;
using Azure::Identity::Test::_detail::CredentialTestHelper;

TEST(ClientCertificateCredential, Regular)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        ClientCertificateCredentialOptions options;
        options.Transport.Transport = transport;

        return std::make_unique<ClientCertificateCredential>(
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
    constexpr char expectedBodyStart0[]
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&scope=https%3A%2F%2Fazure.com%2F.default" // cspell:disable-line
          "&client_assertion=";

    constexpr char expectedBodyStart1[]
        = "grant_type=client_credentials"
          "&client_assertion_type=urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&client_assertion=";

    EXPECT_GT(request0.Body.size(), (sizeof(expectedBodyStart0) - 1));
    EXPECT_GT(request1.Body.size(), (sizeof(expectedBodyStart1) - 1));
   
    EXPECT_EQ(request0.Body.substr(0, (sizeof(expectedBodyStart0) - 1)), expectedBodyStart0);
    EXPECT_EQ(request1.Body.substr(0, (sizeof(expectedBodyStart1) - 1)), expectedBodyStart1);

    EXPECT_NE(request0.Headers.find("Content-Length"), request0.Headers.end());
    EXPECT_GT(std::stoi(request0.Headers.at("Content-Length")), (sizeof(expectedBodyStart0) - 1));

    EXPECT_NE(request1.Headers.find("Content-Length"), request1.Headers.end());
    EXPECT_GT(std::stoi(request1.Headers.at("Content-Length")), (sizeof(expectedBodyStart1) - 1));
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
