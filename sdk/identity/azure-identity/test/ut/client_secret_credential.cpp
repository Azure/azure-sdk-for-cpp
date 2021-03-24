// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @brief makes sure azure/identity.hpp can be included.
 *
 * @remark This file will catch any issue while trying to use/include the identity.hpp header
 *
 */

#include "azure/identity/client_secret_credential.hpp"

#include <azure/core/internal/http/test_transport.hpp>
#include <azure/core/internal/system_clock.hpp>
#include <azure/core/io/body_stream.hpp>

#include <gtest/gtest.h>

using namespace Azure::Identity;

namespace {

struct CredentialResult
{
  struct RequestInfo
  {
    std::string AbsoluteUrl;
    Azure::Core::CaseInsensitiveMap Headers;
    std::string Body;
  } Request;

  Azure::Core::Credentials::AccessToken Response;
};

CredentialResult TestClientSecretCredential(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& clientSecret,
    ClientSecretCredentialOptions credentialOptions,
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::DateTime const& clockOverride,
    std::string const& responseBody)
{
  CredentialResult result;

  auto responseVec = std::vector<uint8_t>(responseBody.begin(), responseBody.end());
  credentialOptions.Transport.Transport
      = std::make_shared<Azure::Core::Http::_internal::TestTransport>([&](auto request, auto) {
          auto const bodyVec = request.GetBodyStream()->ReadToEnd(Azure::Core::Context());

          result.Request
              = {request.GetUrl().GetAbsoluteUrl(),
                 request.GetHeaders(),
                 std::string(bodyVec.begin(), bodyVec.end())};

          auto response = std::make_unique<Azure::Core::Http::RawResponse>(
              1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");

          response->SetBodyStream(std::make_unique<Azure::Core::IO::MemoryBodyStream>(responseVec));

          return response;
        });

  ClientSecretCredential credential(tenantId, clientId, clientSecret, credentialOptions);

  Azure::Core::_internal::SystemClock overriddenSystemClock(
      [=]() { return static_cast<std::chrono::system_clock::time_point>(clockOverride); });

  result.Response = credential.GetToken(tokenRequestContext, Azure::Core::Context());

  return result;
}
} // namespace

TEST(ClientSecretCredential, Regular)
{
  ClientSecretCredentialOptions options;
  options.AuthorityHost = "https://authority.url/";
  auto const actual = TestClientSecretCredential(
      "01234567-89ab-cdef-fedc-ba8976543210",
      "fedcba98-7654-3210-0123-456789abcdef",
      "CLIENTSECRET",
      options,
      {{"https://resource.url/.default"}},
      Azure::DateTime(2021, 1, 1, 0),
      "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}");

  EXPECT_EQ(
      actual.Request.AbsoluteUrl,
      "https://authority.url/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    constexpr char expectedBody[] = "grant_type=client_credentials"
                                    "&client_id=fedcba98-7654-3210-0123-456789abcdef"
                                    "&client_secret=CLIENTSECRET"
                                    "&scope=https%3A%2F%2Fresource.url%2F.default";

    EXPECT_EQ(actual.Request.Body, expectedBody);

    EXPECT_NE(actual.Request.Headers.find("Content-Length"), actual.Request.Headers.end());
    EXPECT_EQ(
        actual.Request.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));
  }

  EXPECT_NE(actual.Request.Headers.find("Content-Type"), actual.Request.Headers.end());
  EXPECT_EQ(actual.Request.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(actual.Response.Token, "ACCESSTOKEN1");
  EXPECT_EQ(actual.Response.ExpiresOn, Azure::DateTime(2021, 1, 1, 1));
  EXPECT_EQ(actual.Response.ExpiresOn.ToString(), Azure::DateTime(2021, 1, 1, 1).ToString());
}

TEST(ClientSecretCredential, AzureStack)
{
  ClientSecretCredentialOptions options;
  options.AuthorityHost = "https://authority.url/";
  auto const actual = TestClientSecretCredential(
      "adfs",
      "fedcba98-7654-3210-0123-456789abcdef",
      "CLIENTSECRET",
      options,
      {{"https://resource.url/.default"}},
      Azure::DateTime(2021, 1, 1, 0),
      "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}");

  EXPECT_EQ(actual.Request.AbsoluteUrl, "https://authority.url/adfs/oauth2/token");

  {
    constexpr char expectedBody[] = "grant_type=client_credentials"
                                    "&client_id=fedcba98-7654-3210-0123-456789abcdef"
                                    "&client_secret=CLIENTSECRET"
                                    "&scope=https%3A%2F%2Fresource.url";

    EXPECT_EQ(actual.Request.Body, expectedBody);

    EXPECT_NE(actual.Request.Headers.find("Content-Length"), actual.Request.Headers.end());
    EXPECT_EQ(
        actual.Request.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));
  }

  EXPECT_NE(actual.Request.Headers.find("Content-Type"), actual.Request.Headers.end());
  EXPECT_EQ(actual.Request.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(actual.Request.Headers.find("Host"), actual.Request.Headers.end());
  EXPECT_EQ(actual.Request.Headers.at("Host"), "authority.url");

  EXPECT_EQ(actual.Response.Token, "ACCESSTOKEN1");
  EXPECT_EQ(actual.Response.ExpiresOn, Azure::DateTime(2021, 1, 1, 1));
}
