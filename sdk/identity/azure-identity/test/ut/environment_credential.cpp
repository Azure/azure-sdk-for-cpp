// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/environment_credential.hpp"

#include <azure/core/io/body_stream.hpp>
#include <azure/core/platform.hpp>

#include "test_transport.hpp"

#include <gtest/gtest.h>
#include <stdlib.h>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <windows.h>
#endif

#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.

using namespace Azure::Identity;

namespace {
class EnvironmentOverride final {
  class Environment final {
    static void SetVariable(std::string const& name, std::string const& value)
    {
#if defined(_MSC_VER)
      static_cast<void>(_putenv((name + "=" + value).c_str()));
#else
      if (value.empty())
      {
        static_cast<void>(unsetenv(name.c_str()));
      }
      else
      {
        static_cast<void>(setenv(name.c_str(), value.c_str(), 1));
      }
#endif
    }

  public:
    static std::string GetVariable(std::string const& name)
    {
#if defined(_MSC_VER)
#pragma warning(push)
// warning C4996: 'getenv': This function or variable may be unsafe. Consider using _dupenv_s
// instead.
#pragma warning(disable : 4996)
#endif
      auto const result = std::getenv(name.c_str());
      return result != nullptr ? result : "";
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    }

    static void SetVariables(std::map<std::string, std::string> const& vars)
    {
      for (auto var : vars)
      {
        SetVariable(var.first, var.second);
      }
    }
  };

  std::map<std::string, std::string> m_originalEnv;

public:
  ~EnvironmentOverride() { Environment::SetVariables(m_originalEnv); }

  EnvironmentOverride(
      std::string const& tenantId,
      std::string const& clientId,
      std::string const& clientSecret,
      std::string const& authorityHost,
      std::string const& username,
      std::string const& password,
      std::string const& clientCertificatePath)
  {
    std::map<std::string, std::string> const NewEnv = {
        {"AZURE_TENANT_ID", tenantId},
        {"AZURE_CLIENT_ID", clientId},
        {"AZURE_CLIENT_SECRET", clientSecret},
        {"AZURE_AUTHORITY_HOST", authorityHost},
        {"AZURE_USERNAME", username},
        {"AZURE_PASSWORD", password},
        {"AZURE_CLIENT_CERTIFICATE_PATH", clientCertificatePath},
    };

    for (auto var : NewEnv)
    {
      m_originalEnv[var.first] = Environment::GetVariable(var.first);
    }

    try
    {
      Environment::SetVariables(NewEnv);
    }
    catch (...)
    {
      Environment::SetVariables(m_originalEnv);
      throw;
    }
  }
};

struct CredentialResult final
{
  struct
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

CredentialResult TestEnvironmentCredential(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& clientSecret,
    std::string const& authorityHost,
    std::string const& username,
    std::string const& password,
    std::string const& clientCertificatePath,
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    std::string const& responseBody)
{
  CredentialResult result;

  auto responseVec = std::vector<uint8_t>(responseBody.begin(), responseBody.end());

  Azure::Core::Credentials::TokenCredentialOptions credentialOptions;
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

  EnvironmentOverride env(
      tenantId, clientId, clientSecret, authorityHost, username, password, clientCertificatePath);

  EnvironmentCredential credential(credentialOptions);
  result.Response.AccessToken = credential.GetToken(tokenRequestContext, Azure::Core::Context());
  result.Response.Latest = std::chrono::system_clock::now();

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

#endif
