// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/environment_credential.hpp"

#include <azure/core/internal/environment.hpp>
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

CredentialResult TestEnvironmentCredential(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& clientSecret,
    std::string const& authorityHost,
    std::string const& username,
    std::string const& password,
    std::string const& clientCertificatePath,
    EnvironmentCredentialOptions credentialOptions,
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

  std::map<std::string, std::string> env;
  if (!tenantId.empty())
  {
    env["AZURE_TENANT_ID"] = tenantId;
  }

  if (!clientId.empty())
  {
    env["AZURE_CLIENT_ID"] = clientId;
  }

  if (!clientSecret.empty())
  {
    env["AZURE_CLIENT_SECRET"] = clientSecret;
  }

  if (!authorityHost.empty())
  {
    env["AZURE_AUTHORITY_HOST"] = authorityHost;
  }

  if (!username.empty())
  {
    env["AZURE_USERNAME"] = username;
  }

  if (!password.empty())
  {
    env["AZURE_PASSWORD"] = password;
  }

  if (!clientCertificatePath.empty())
  {
    env["AZURE_CLIENT_CERTIFICATE_PATH"] = clientCertificatePath;
  }

  Azure::Core::_internal::Environment overriddenEnvironment(
      [&](auto varName) { return env.at(varName).c_str(); });

  EnvironmentCredential credential(credentialOptions);

  Azure::Core::_internal::SystemClock overriddenSystemClock(
      [&]() { return static_cast<std::chrono::system_clock::time_point>(clockOverride); });

  result.Response = credential.GetToken(tokenRequestContext, Azure::Core::Context());

  return result;
}
} // namespace

TEST(EnvironmentCredential, RegularClientSecretCredential)
{
  auto const actual = TestEnvironmentCredential(
      "01234567-89ab-cdef-fedc-ba8976543210",
      "fedcba98-7654-3210-0123-456789abcdef",
      "CLIENTSECRET",
      "https://microsoft.com/",
      "",
      "",
      "",
      EnvironmentCredentialOptions(),
      {{"https://azure.com/.default"}},
      Azure::DateTime(2021, 1, 1, 0),
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

  EXPECT_EQ(actual.Response.Token, "ACCESSTOKEN1");
  EXPECT_EQ(actual.Response.ExpiresOn, Azure::DateTime(2021, 1, 1, 1));
  EXPECT_EQ(actual.Response.ExpiresOn.ToString(), Azure::DateTime(2021, 1, 1, 1).ToString());
}

TEST(EnvironmentCredential, AzureStackClientSecretCredential)
{
  auto const actual = TestEnvironmentCredential(
      "adfs",
      "fedcba98-7654-3210-0123-456789abcdef",
      "CLIENTSECRET",
      "https://microsoft.com/",
      "",
      "",
      "",
      EnvironmentCredentialOptions(),
      {{"https://azure.com/.default"}},
      Azure::DateTime(2021, 1, 1, 0),
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

  EXPECT_EQ(actual.Response.Token, "ACCESSTOKEN1");
  EXPECT_EQ(actual.Response.ExpiresOn, Azure::DateTime(2021, 1, 1, 1));
}
