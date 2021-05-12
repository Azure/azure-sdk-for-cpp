// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_secret_credential.hpp"

#include <azure/core/io/body_stream.hpp>

#include "test_transport.hpp"

#include <gtest/gtest.h>

using namespace Azure::Identity;

namespace {

struct CredentialResult final
{
  struct RequestInfo final
  {
    std::string AbsoluteUrl;
    Azure::Core::CaseInsensitiveMap Headers;
    std::string Body;
  } Request;

  struct
  {
    std::chrono::system_clock::time_point Earliest;
    std::chrono::system_clock::time_point Latest;
    Azure::Core::Credentials::AccessToken AccessToken;
  } Response;
};

CredentialResult TestClientSecretCredential(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& clientSecret,
    ClientSecretCredentialOptions credentialOptions,
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    std::string const& responseBody)
{
  CredentialResult result;

  auto responseVec = std::vector<uint8_t>(responseBody.begin(), responseBody.end());
  credentialOptions.Transport.Transport = std::make_shared<TestTransport>([&](auto request, auto) {
    auto const bodyVec = request.GetBodyStream()->ReadToEnd(Azure::Core::Context());

    result.Request
        = {request.GetUrl().GetAbsoluteUrl(),
           request.GetHeaders(),
           std::string(bodyVec.begin(), bodyVec.end())};

    auto response = std::make_unique<Azure::Core::Http::RawResponse>(
        1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK");

    response->SetBodyStream(std::make_unique<Azure::Core::IO::MemoryBodyStream>(responseVec));

    result.Response.Earliest = std::chrono::system_clock::now();
    return response;
  });

  ClientSecretCredential credential(tenantId, clientId, clientSecret, credentialOptions);
  result.Response.AccessToken = credential.GetToken(tokenRequestContext, Azure::Core::Context());
  result.Response.Latest = std::chrono::system_clock::now();

  return result;
}
} // namespace

TEST(ClientSecretCredential, Regular)
{
  ClientSecretCredentialOptions options;
  options.AuthorityHost = "https://microsoft.com/";
  auto const actual = TestClientSecretCredential(
      "01234567-89ab-cdef-fedc-ba8976543210",
      "fedcba98-7654-3210-0123-456789abcdef",
      "CLIENTSECRET",
      options,
      {{"https://azure.com/.default"}},
      "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}");

  EXPECT_EQ(
      actual.Request.AbsoluteUrl,
      "https://microsoft.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    constexpr char expectedBody[] = "grant_type=client_credentials"
                                    "&client_id=fedcba98-7654-3210-0123-456789abcdef"
                                    "&client_secret=CLIENTSECRET"
                                    "&scope=https%3A%2F%2Fazure.com%2F.default";

    EXPECT_EQ(actual.Request.Body, expectedBody);

    EXPECT_NE(actual.Request.Headers.find("Content-Length"), actual.Request.Headers.end());
    EXPECT_EQ(
        actual.Request.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));
  }

  EXPECT_NE(actual.Request.Headers.find("Content-Type"), actual.Request.Headers.end());
  EXPECT_EQ(actual.Request.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(actual.Response.AccessToken.Token, "ACCESSTOKEN1");

  using namespace std::chrono_literals;
  EXPECT_GT(actual.Response.AccessToken.ExpiresOn, actual.Response.Earliest + 3600s);
  EXPECT_LT(actual.Response.AccessToken.ExpiresOn, actual.Response.Latest + 3600s);
}

TEST(ClientSecretCredential, AzureStack)
{
  ClientSecretCredentialOptions options;
  options.AuthorityHost = "https://microsoft.com/";
  auto const actual = TestClientSecretCredential(
      "adfs",
      "fedcba98-7654-3210-0123-456789abcdef",
      "CLIENTSECRET",
      options,
      {{"https://azure.com/.default"}},
      "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}");

  EXPECT_EQ(actual.Request.AbsoluteUrl, "https://microsoft.com/adfs/oauth2/token");

  {
    constexpr char expectedBody[] = "grant_type=client_credentials"
                                    "&client_id=fedcba98-7654-3210-0123-456789abcdef"
                                    "&client_secret=CLIENTSECRET"
                                    "&scope=https%3A%2F%2Fazure.com";

    EXPECT_EQ(actual.Request.Body, expectedBody);

    EXPECT_NE(actual.Request.Headers.find("Content-Length"), actual.Request.Headers.end());
    EXPECT_EQ(
        actual.Request.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));
  }

  EXPECT_NE(actual.Request.Headers.find("Content-Type"), actual.Request.Headers.end());
  EXPECT_EQ(actual.Request.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(actual.Request.Headers.find("Host"), actual.Request.Headers.end());
  EXPECT_EQ(actual.Request.Headers.at("Host"), "microsoft.com");

  EXPECT_EQ(actual.Response.AccessToken.Token, "ACCESSTOKEN1");

  using namespace std::chrono_literals;
  EXPECT_GT(actual.Response.AccessToken.ExpiresOn, actual.Response.Earliest + 3600s);
  EXPECT_LT(actual.Response.AccessToken.ExpiresOn, actual.Response.Latest + 3600s);
}
