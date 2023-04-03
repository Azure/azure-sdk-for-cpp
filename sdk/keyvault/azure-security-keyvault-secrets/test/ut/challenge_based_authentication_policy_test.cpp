// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/shared/challenge_based_authentication_policy.hpp"

#include "azure/keyvault/shared/keyvault_shared.hpp"
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/identity/client_secret_credential.hpp>

#include <utility>
#include <vector>

#include <gtest/gtest.h>

// cspell:ignore Fvault Ftest

using Azure::Security::KeyVault::_internal::ChallengeBasedAuthenticationPolicy;

using Azure::Core::CaseInsensitiveMap;
using Azure::Core::Context;
using Azure::Core::Url;
using Azure::Core::_internal::ClientOptions;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenCredential;
using Azure::Core::Http::HttpStatusCode;
using Azure::Core::Http::HttpTransport;
using Azure::Core::Http::RawResponse;
using Azure::Core::Http::Request;
using Azure::Core::Http::_internal::HttpPipeline;
using Azure::Identity::ClientSecretCredentialOptions;

namespace {
class TestRequest final {
public:
  Azure::Core::Url Url;
  CaseInsensitiveMap Headers;
  std::string Body;

  explicit TestRequest(Request& request) : Url(request.GetUrl()), Headers(request.GetHeaders())
  {
    auto const bodyStreamPtr = request.GetBodyStream();
    if (bodyStreamPtr != nullptr)
    {
      auto const uint8Vector = bodyStreamPtr->ReadToEnd();
      auto const charPtr = reinterpret_cast<char const*>(uint8Vector.data());
      Body = std::string(charPtr, charPtr + uint8Vector.size());
    }
  }
};

class TestResponse final {
private:
  HttpStatusCode m_statusCode;
  std::shared_ptr<std::string const> m_body;
  CaseInsensitiveMap const m_headers;

public:
  TestResponse(HttpStatusCode statusCode, std::string body, CaseInsensitiveMap headers)
      : m_statusCode(std::move(statusCode)), m_body(std::make_shared<std::string>(std::move(body))),
        m_headers(std::move(headers))
  {
  }

  std::unique_ptr<RawResponse> CreateRawResponse() const
  {
    using Azure::Core::IO::MemoryBodyStream;

    auto response = std::make_unique<RawResponse>(1, 1, m_statusCode, "TestReasonPhrase");

    for (auto const& header : m_headers)
    {
      response->SetHeader(header.first, header.second);
    }

    auto const bodyPtr = reinterpret_cast<uint8_t const*>(m_body->data());
    response->SetBodyStream(std::make_unique<MemoryBodyStream>(bodyPtr, m_body->size()));

    return response;
  }
};

class TestHttpTransport final : public HttpTransport {
private:
  std::shared_ptr<std::vector<TestRequest>> m_requests;
  std::vector<TestResponse> m_responses;
  decltype(m_responses)::size_type m_currentResponse;

public:
  explicit TestHttpTransport(
      std::shared_ptr<std::vector<TestRequest>> requests,
      std::vector<TestResponse> responses)
      : m_requests(std::move(requests)), m_responses(std::move(responses)), m_currentResponse(0)
  {
  }

  std::unique_ptr<RawResponse> Send(Request& request, Context const&) override
  {
    EXPECT_LT(m_currentResponse, m_responses.size());

    m_requests->emplace_back(TestRequest(request));
    return m_responses.at(m_currentResponse++).CreateRawResponse();
  }
};

class TestKeyVaultClient final {
  std::shared_ptr<HttpPipeline> m_pipeline;
  Url m_vaultUrl;

public:
  explicit TestKeyVaultClient(
      std::string vaultUrl,
      std::shared_ptr<TokenCredential const> credential,
      std::shared_ptr<TestHttpTransport> testHttpTransport)
      : m_vaultUrl(vaultUrl)
  {
    using Azure::Core::Http::Policies::HttpPolicy;
    using Azure::Security::KeyVault::_internal::UrlScope;

    ClientOptions options;
    options.Transport.Transport = testHttpTransport;

    Azure::Core::Credentials::TokenRequestContext tokenContext;
    tokenContext.Scopes = {UrlScope::GetScopeFromUrl(m_vaultUrl)};

    std::vector<std::unique_ptr<HttpPolicy>> perRetryPolicies;
    perRetryPolicies.emplace_back(
        std::make_unique<ChallengeBasedAuthenticationPolicy>(credential, tokenContext));

    std::vector<std::unique_ptr<HttpPolicy>> perCallPolicies;

    m_pipeline = std::make_shared<HttpPipeline>(
        options,
        "TestKeyVaultClient",
        "1.0.0",
        std::move(perRetryPolicies),
        std::move(perCallPolicies));
  }

  std::unique_ptr<RawResponse> DoSomething(Context const& context = {}) const
  {
    using Azure::Core::Http::HttpMethod;
    auto request = Request(HttpMethod::Get, m_vaultUrl);
    return m_pipeline->Send(request, context);
  }
};

std::shared_ptr<TokenCredential> CreateTestCredential(
    std::shared_ptr<TestHttpTransport> testHttpTransport,
    decltype(ClientSecretCredentialOptions::AdditionallyAllowedTenants) additionallyAllowedTenants
    = {})
{
  using Azure::Identity::ClientSecretCredential;

  ClientSecretCredentialOptions options;
  options.Transport.Transport = testHttpTransport;
  options.AdditionallyAllowedTenants = additionallyAllowedTenants;

  return std::make_shared<ClientSecretCredential>(
      "OriginalTenantId", "ClientId", "ClientSecret", options);
}

std::string GetTenantIdFromClientSecretRequest(TestRequest const& request)
{
  auto const urlPath = request.Url.GetPath();
  auto const slashPos = urlPath.find('/');
  return (slashPos != std::string::npos) ? urlPath.substr(0, slashPos) : urlPath;
}

std::string GetScopeFromClientSecretRequest(TestRequest const& request)
{
  std::string const ScopeParam = "scope=";
  auto const scopeParamStart = request.Body.find(ScopeParam);
  if (scopeParamStart == std::string::npos)
  {
    return {};
  }

  auto const scopeValueStart = scopeParamStart + ScopeParam.length();
  auto const nextParamStart = request.Body.find('&', scopeValueStart);

  auto const scopeValueEnd
      = (nextParamStart != std::string::npos) ? nextParamStart : request.Body.length();

  return request.Body.substr(scopeValueStart, scopeValueEnd);
}

std::string GetAuthHeaderValueFromServiceRequest(TestRequest const& request)
{
  auto const authHeaderIter = request.Headers.find("authorization");
  return (authHeaderIter != request.Headers.end()) ? authHeaderIter->second : std::string{};
}
} // namespace

TEST(ChallengeBasedAuthenticationPolicy, BearerTokenAuthPolicyCompatible)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  static_cast<void>( //
      TestKeyVaultClient(
          "https://test.vault.azure.net",
          CreateTestCredential(std::make_shared<TestHttpTransport>(
              identityRequests,
              std::vector<TestResponse>{
                  TestResponse(
                      HttpStatusCode::Ok,
                      "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN\"}",
                      {}),
              })),
          std::make_shared<TestHttpTransport>(
              serviceRequests,
              std::vector<TestResponse>{
                  TestResponse(HttpStatusCode::Ok, {}, {}),
              }))
          .DoSomething());

  EXPECT_EQ(identityRequests->size(), 1);
  {
    auto const& identityRequest0 = identityRequests->at(0);
    EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
    EXPECT_EQ(
        GetScopeFromClientSecretRequest(identityRequest0),
        "https%3A%2F%2Fvault.azure.net%2F.default");
  }

  EXPECT_EQ(serviceRequests->size(), 1);
  {
    auto const& serviceRequest0 = serviceRequests->at(0);
    EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN");
  }
}

TEST(ChallengeBasedAuthenticationPolicy, AnotherScopeAsScope)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  static_cast<void>( //
      TestKeyVaultClient(
          "https://test.vault.azure.net",
          CreateTestCredential(std::make_shared<TestHttpTransport>(
              identityRequests,
              std::vector<TestResponse>{
                  TestResponse(
                      HttpStatusCode::Ok,
                      "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN1\"}",
                      {}),
                  TestResponse(
                      HttpStatusCode::Ok,
                      "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN2\"}",
                      {}),
              })),
          std::make_shared<TestHttpTransport>(
              serviceRequests,
              std::vector<TestResponse>{
                  TestResponse(
                      HttpStatusCode::Unauthorized,
                      {},
                      {
                          std::make_pair<std::string, std::string>(
                              "WWW-Authenticate",
                              "Bearer authorization=\"https://login.windows.net/OriginalTenantId\","
                              " scope=\"https://test.vault.azure.net/.default\""),
                      }),
                  TestResponse(HttpStatusCode::Ok, {}, {}),
              }))
          .DoSomething());

  EXPECT_EQ(identityRequests->size(), 2);
  {
    {
      auto const& identityRequest0 = identityRequests->at(0);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest0),
          "https%3A%2F%2Fvault.azure.net%2F.default");
    }

    {
      auto const& identityRequest1 = identityRequests->at(1);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest1), "OriginalTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest1),
          "https%3A%2F%2Ftest.vault.azure.net%2F.default");
    }
  }

  EXPECT_EQ(serviceRequests->size(), 2);
  {
    {
      auto const& serviceRequest0 = serviceRequests->at(0);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN1");
    }

    {
      auto const& serviceRequest1 = serviceRequests->at(1);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest1), "Bearer ACCESSTOKEN2");
    }
  }
}

TEST(ChallengeBasedAuthenticationPolicy, AnotherScopeAsResource)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  static_cast<void>( //
      TestKeyVaultClient(
          "https://test.vault.azure.net",
          CreateTestCredential(std::make_shared<TestHttpTransport>(
              identityRequests,
              std::vector<TestResponse>{
                  TestResponse(
                      HttpStatusCode::Ok,
                      "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN1\"}",
                      {}),
                  TestResponse(
                      HttpStatusCode::Ok,
                      "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN2\"}",
                      {}),
              })),
          std::make_shared<TestHttpTransport>(
              serviceRequests,
              std::vector<TestResponse>{
                  TestResponse(
                      HttpStatusCode::Unauthorized,
                      {},
                      {
                          std::make_pair<std::string, std::string>(
                              "WWW-Authenticate",
                              "Bearer authorization=\"https://login.windows.net/OriginalTenantId\","
                              " resource=\"https://test.vault.azure.net\""),
                      }),
                  TestResponse(HttpStatusCode::Ok, {}, {}),
              }))
          .DoSomething());

  EXPECT_EQ(identityRequests->size(), 2);
  {
    {
      auto const& identityRequest0 = identityRequests->at(0);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest0),
          "https%3A%2F%2Fvault.azure.net%2F.default");
    }

    {
      auto const& identityRequest1 = identityRequests->at(1);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest1), "OriginalTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest1),
          "https%3A%2F%2Ftest.vault.azure.net%2F.default");
    }
  }

  EXPECT_EQ(serviceRequests->size(), 2);
  {
    {
      auto const& serviceRequest0 = serviceRequests->at(0);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN1");
    }

    {
      auto const& serviceRequest1 = serviceRequests->at(1);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest1), "Bearer ACCESSTOKEN2");
    }
  }
}

TEST(ChallengeBasedAuthenticationPolicy, AnotherTenantAsterisk)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  static_cast<void>( //
      TestKeyVaultClient(
          "https://test.vault.azure.net",
          CreateTestCredential(
              std::make_shared<TestHttpTransport>(
                  identityRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Ok,
                          "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN1\"}",
                          {}),
                      TestResponse(
                          HttpStatusCode::Ok,
                          "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN2\"}",
                          {}),
                  }),
              {"*"}),
          std::make_shared<TestHttpTransport>(
              serviceRequests,
              std::vector<TestResponse>{
                  TestResponse(
                      HttpStatusCode::Unauthorized,
                      {},
                      {
                          std::make_pair<std::string, std::string>(
                              "WWW-Authenticate",
                              "Bearer authorization=\"https://login.windows.net/NewTenantId\","
                              " resource=\"https://vault.azure.net\""),
                      }),
                  TestResponse(HttpStatusCode::Ok, {}, {}),
              }))
          .DoSomething());

  EXPECT_EQ(identityRequests->size(), 2);
  {
    {
      auto const& identityRequest0 = identityRequests->at(0);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest0),
          "https%3A%2F%2Fvault.azure.net%2F.default");
    }

    {
      auto const& identityRequest1 = identityRequests->at(1);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest1), "NewTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest1),
          "https%3A%2F%2Fvault.azure.net%2F.default");
    }
  }

  EXPECT_EQ(serviceRequests->size(), 2);
  {
    {
      auto const& serviceRequest0 = serviceRequests->at(0);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN1");
    }

    {
      auto const& serviceRequest1 = serviceRequests->at(1);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest1), "Bearer ACCESSTOKEN2");
    }
  }
}

TEST(ChallengeBasedAuthenticationPolicy, AnotherTenantAndScopeWithAltNames)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  static_cast<void>( //
      TestKeyVaultClient(
          "https://test.vault.azure.net",
          CreateTestCredential(
              std::make_shared<TestHttpTransport>(
                  identityRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Ok,
                          "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN1\"}",
                          {}),
                      TestResponse(
                          HttpStatusCode::Ok,
                          "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN2\"}",
                          {}),
                  }),
              {"*"}),
          std::make_shared<TestHttpTransport>(
              serviceRequests,
              std::vector<TestResponse>{
                  TestResponse(
                      HttpStatusCode::Unauthorized,
                      {},
                      {
                          std::make_pair<std::string, std::string>(
                              "WWW-Authenticate",
                              "Bearer authorization_uri=\"https://login.windows.net/NewTenantId/\","
                              " scope=\"https://test.vault.azure.net/.default\""),
                      }),
                  TestResponse(HttpStatusCode::Ok, {}, {}),
              }))
          .DoSomething());

  EXPECT_EQ(identityRequests->size(), 2);
  {
    {
      auto const& identityRequest0 = identityRequests->at(0);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest0),
          "https%3A%2F%2Fvault.azure.net%2F.default");
    }

    {
      auto const& identityRequest1 = identityRequests->at(1);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest1), "NewTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest1),
          "https%3A%2F%2Ftest.vault.azure.net%2F.default");
    }
  }

  EXPECT_EQ(serviceRequests->size(), 2);
  {
    {
      auto const& serviceRequest0 = serviceRequests->at(0);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN1");
    }

    {
      auto const& serviceRequest1 = serviceRequests->at(1);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest1), "Bearer ACCESSTOKEN2");
    }
  }
}

TEST(ChallengeBasedAuthenticationPolicy, AnotherTenantExplicit)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  static_cast<void>( //
      TestKeyVaultClient(
          "https://test.vault.azure.net",
          CreateTestCredential(
              std::make_shared<TestHttpTransport>(
                  identityRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Ok,
                          "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN1\"}",
                          {}),
                      TestResponse(
                          HttpStatusCode::Ok,
                          "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN2\"}",
                          {}),
                  }),
              {"NewTenantId"}),
          std::make_shared<TestHttpTransport>(
              serviceRequests,
              std::vector<TestResponse>{
                  TestResponse(
                      HttpStatusCode::Unauthorized,
                      {},
                      {
                          std::make_pair<std::string, std::string>(
                              "WWW-Authenticate",
                              "Bearer authorization=\"https://login.windows.net/NewTenantId\","
                              " resource=\"https://vault.azure.net\""),
                      }),
                  TestResponse(HttpStatusCode::Ok, {}, {}),
              }))
          .DoSomething());

  EXPECT_EQ(identityRequests->size(), 2);
  {
    {
      auto const& identityRequest0 = identityRequests->at(0);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest0),
          "https%3A%2F%2Fvault.azure.net%2F.default");
    }

    {
      auto const& identityRequest1 = identityRequests->at(1);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest1), "NewTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest1),
          "https%3A%2F%2Fvault.azure.net%2F.default");
    }
  }

  EXPECT_EQ(serviceRequests->size(), 2);
  {
    {
      auto const& serviceRequest0 = serviceRequests->at(0);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN1");
    }

    {
      auto const& serviceRequest1 = serviceRequests->at(1);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest1), "Bearer ACCESSTOKEN2");
    }
  }
}

TEST(ChallengeBasedAuthenticationPolicy, AnotherTenantNotAllowed)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  EXPECT_THROW(
      static_cast<void>( //
          TestKeyVaultClient(
              "https://test.vault.azure.net",
              CreateTestCredential(
                  std::make_shared<TestHttpTransport>(
                      identityRequests,
                      std::vector<TestResponse>{
                          TestResponse(
                              HttpStatusCode::Ok,
                              "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN\"}",
                              {}),
                      }),
                  {"UnknownTenantId"}),
              std::make_shared<TestHttpTransport>(
                  serviceRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Unauthorized,
                          {},
                          {
                              std::make_pair<std::string, std::string>(
                                  "WWW-Authenticate",
                                  "Bearer authorization=\"https://login.windows.net/NewTenantId\","
                                  " resource=\"https://vault.azure.net\""),
                          }),
                  }))
              .DoSomething()),
      AuthenticationException);

  EXPECT_EQ(identityRequests->size(), 1);
  {
    auto const& identityRequest0 = identityRequests->at(0);
    EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
    EXPECT_EQ(
        GetScopeFromClientSecretRequest(identityRequest0),
        "https%3A%2F%2Fvault.azure.net%2F.default");
  }

  EXPECT_EQ(serviceRequests->size(), 1);
  {
    auto const& serviceRequest0 = serviceRequests->at(0);
    EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN");
  }
}

TEST(ChallengeBasedAuthenticationPolicy, MissingScope)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  auto const serviceResponse
      = TestKeyVaultClient(
            "https://test.vault.azure.net",
            CreateTestCredential(std::make_shared<TestHttpTransport>(
                identityRequests,
                std::vector<TestResponse>{
                    TestResponse(
                        HttpStatusCode::Ok,
                        "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN\"}",
                        {}),
                })),
            std::make_shared<TestHttpTransport>(
                serviceRequests,
                std::vector<TestResponse>{
                    TestResponse(
                        HttpStatusCode::Unauthorized,
                        {},
                        {
                            std::make_pair<std::string, std::string>(
                                "WWW-Authenticate",
                                "Bearer"
                                " authorization=\"https://login.windows.net/OriginalTenantId\""),
                        }),
                }))
            .DoSomething();

  EXPECT_TRUE(serviceResponse);
  {
    EXPECT_EQ(serviceResponse->GetStatusCode(), HttpStatusCode::Unauthorized);

    auto const& responseHeaders = serviceResponse->GetHeaders();
    auto const authHeader = responseHeaders.find("WWW-Authenticate");
    EXPECT_NE(authHeader, responseHeaders.end());
    EXPECT_EQ(
        authHeader->second, "Bearer authorization=\"https://login.windows.net/OriginalTenantId\"");
  }

  EXPECT_EQ(identityRequests->size(), 1);
  {
    auto const& identityRequest0 = identityRequests->at(0);
    EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
    EXPECT_EQ(
        GetScopeFromClientSecretRequest(identityRequest0),
        "https%3A%2F%2Fvault.azure.net%2F.default");
  }

  EXPECT_EQ(serviceRequests->size(), 1);
  {
    auto const& serviceRequest0 = serviceRequests->at(0);
    EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN");
  }
}

TEST(ChallengeBasedAuthenticationPolicy, EmptyScope)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  auto const serviceResponse
      = TestKeyVaultClient(
            "https://test.vault.azure.net",
            CreateTestCredential(std::make_shared<TestHttpTransport>(
                identityRequests,
                std::vector<TestResponse>{
                    TestResponse(
                        HttpStatusCode::Ok,
                        "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN\"}",
                        {}),
                })),
            std::make_shared<TestHttpTransport>(
                serviceRequests,
                std::vector<TestResponse>{
                    TestResponse(
                        HttpStatusCode::Unauthorized,
                        {},
                        {
                            std::make_pair<std::string, std::string>(
                                "WWW-Authenticate",
                                "Bearer"
                                " authorization=\"https://login.windows.net/OriginalTenantId\","
                                " scope=\"\""),
                        }),
                }))
            .DoSomething();

  EXPECT_TRUE(serviceResponse);
  {
    EXPECT_EQ(serviceResponse->GetStatusCode(), HttpStatusCode::Unauthorized);

    auto const& responseHeaders = serviceResponse->GetHeaders();
    auto const authHeader = responseHeaders.find("WWW-Authenticate");
    EXPECT_NE(authHeader, responseHeaders.end());
    EXPECT_EQ(
        authHeader->second,
        "Bearer authorization=\"https://login.windows.net/OriginalTenantId\", scope=\"\"");
  }

  EXPECT_EQ(identityRequests->size(), 1);
  {
    auto const& identityRequest0 = identityRequests->at(0);
    EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
    EXPECT_EQ(
        GetScopeFromClientSecretRequest(identityRequest0),
        "https%3A%2F%2Fvault.azure.net%2F.default");
  }

  EXPECT_EQ(serviceRequests->size(), 1);
  {
    auto const& serviceRequest0 = serviceRequests->at(0);
    EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN");
  }
}

TEST(ChallengeBasedAuthenticationPolicy, ScopeValidationInvalidUrl)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  EXPECT_THROW(
      static_cast<void>( //
          TestKeyVaultClient(
              "https://test.vault.azure.net",
              CreateTestCredential(std::make_shared<TestHttpTransport>(
                  identityRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Ok,
                          "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN\"}",
                          {}),
                  })),
              std::make_shared<TestHttpTransport>(
                  serviceRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Unauthorized,
                          {},
                          {
                              std::make_pair<std::string, std::string>(
                                  "WWW-Authenticate",
                                  "Bearer "
                                  "authorization=\"https://login.windows.net/OriginalTenantId\","
                                  " resource=\"nonparseable_url\""),
                          }),
                  }))
              .DoSomething()),
      AuthenticationException);

  EXPECT_EQ(identityRequests->size(), 1);
  {
    auto const& identityRequest0 = identityRequests->at(0);
    EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
    EXPECT_EQ(
        GetScopeFromClientSecretRequest(identityRequest0),
        "https%3A%2F%2Fvault.azure.net%2F.default");
  }

  EXPECT_EQ(serviceRequests->size(), 1);
  {
    auto const& serviceRequest0 = serviceRequests->at(0);
    EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN");
  }
}

TEST(ChallengeBasedAuthenticationPolicy, ScopeValidationLongerDomain)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  EXPECT_THROW(
      static_cast<void>( //
          TestKeyVaultClient(
              "https://test.vault.azure.net",
              CreateTestCredential(std::make_shared<TestHttpTransport>(
                  identityRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Ok,
                          "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN\"}",
                          {}),
                  })),
              std::make_shared<TestHttpTransport>(
                  serviceRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Unauthorized,
                          {},
                          {
                              std::make_pair<std::string, std::string>(
                                  "WWW-Authenticate",
                                  "Bearer "
                                  "authorization=\"https://login.windows.net/OriginalTenantId\","
                                  " resource=\"longer.test.vault.azure.net\""),
                          }),
                  }))
              .DoSomething()),
      AuthenticationException);

  EXPECT_EQ(identityRequests->size(), 1);
  {
    auto const& identityRequest0 = identityRequests->at(0);
    EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
    EXPECT_EQ(
        GetScopeFromClientSecretRequest(identityRequest0),
        "https%3A%2F%2Fvault.azure.net%2F.default");
  }

  EXPECT_EQ(serviceRequests->size(), 1);
  {
    auto const& serviceRequest0 = serviceRequests->at(0);
    EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN");
  }
}

TEST(ChallengeBasedAuthenticationPolicy, ScopeValidationDomainMismatch)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  EXPECT_THROW(
      static_cast<void>( //
          TestKeyVaultClient(
              "https://test.vault.azure.net",
              CreateTestCredential(std::make_shared<TestHttpTransport>(
                  identityRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Ok,
                          "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN\"}",
                          {}),
                  })),
              std::make_shared<TestHttpTransport>(
                  serviceRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Unauthorized,
                          {},
                          {
                              std::make_pair<std::string, std::string>(
                                  "WWW-Authenticate",
                                  "Bearer "
                                  "authorization=\"https://login.windows.net/OriginalTenantId\","
                                  " resource=\"vault.azure.com\""),
                          }),
                  }))
              .DoSomething()),
      AuthenticationException);

  EXPECT_EQ(identityRequests->size(), 1);
  {
    auto const& identityRequest0 = identityRequests->at(0);
    EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
    EXPECT_EQ(
        GetScopeFromClientSecretRequest(identityRequest0),
        "https%3A%2F%2Fvault.azure.net%2F.default");
  }

  EXPECT_EQ(serviceRequests->size(), 1);
  {
    auto const& serviceRequest0 = serviceRequests->at(0);
    EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN");
  }
}

TEST(ChallengeBasedAuthenticationPolicy, AuthorizationMissing)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  EXPECT_THROW(
      static_cast<void>( //
          TestKeyVaultClient(
              "https://test.vault.azure.net",
              CreateTestCredential(std::make_shared<TestHttpTransport>(
                  identityRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Ok,
                          "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN\"}",
                          {}),
                  })),
              std::make_shared<TestHttpTransport>(
                  serviceRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Unauthorized,
                          {},
                          {
                              std::make_pair<std::string, std::string>(
                                  "WWW-Authenticate", "Bearer resource=\"vault.azure.net\""),
                          }),
                  }))
              .DoSomething()),
      AuthenticationException);

  EXPECT_EQ(identityRequests->size(), 1);
  {
    auto const& identityRequest0 = identityRequests->at(0);
    EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
    EXPECT_EQ(
        GetScopeFromClientSecretRequest(identityRequest0),
        "https%3A%2F%2Fvault.azure.net%2F.default");
  }

  EXPECT_EQ(serviceRequests->size(), 1);
  {
    auto const& serviceRequest0 = serviceRequests->at(0);
    EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN");
  }
}

TEST(ChallengeBasedAuthenticationPolicy, AuthorizationEmpty)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  EXPECT_THROW(
      static_cast<void>( //
          TestKeyVaultClient(
              "https://test.vault.azure.net",
              CreateTestCredential(std::make_shared<TestHttpTransport>(
                  identityRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Ok,
                          "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN\"}",
                          {}),
                  })),
              std::make_shared<TestHttpTransport>(
                  serviceRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Unauthorized,
                          {},
                          {
                              std::make_pair<std::string, std::string>(
                                  "WWW-Authenticate",
                                  "Bearer authorization=\"\", resource=\"vault.azure.net\""),
                          }),
                  }))
              .DoSomething()),
      AuthenticationException);

  EXPECT_EQ(identityRequests->size(), 1);
  {
    auto const& identityRequest0 = identityRequests->at(0);
    EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
    EXPECT_EQ(
        GetScopeFromClientSecretRequest(identityRequest0),
        "https%3A%2F%2Fvault.azure.net%2F.default");
  }

  EXPECT_EQ(serviceRequests->size(), 1);
  {
    auto const& serviceRequest0 = serviceRequests->at(0);
    EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN");
  }
}

TEST(ChallengeBasedAuthenticationPolicy, AuthorizationInvalidUrl)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  EXPECT_THROW(
      static_cast<void>( //
          TestKeyVaultClient(
              "https://test.vault.azure.net",
              CreateTestCredential(std::make_shared<TestHttpTransport>(
                  identityRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Ok,
                          "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN\"}",
                          {}),
                  })),
              std::make_shared<TestHttpTransport>(
                  serviceRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Unauthorized,
                          {},
                          {
                              std::make_pair<std::string, std::string>(
                                  "WWW-Authenticate",
                                  "Bearer authorization=\"nonparseable_url\","
                                  " resource=\"vault.azure.net\""),
                          }),
                  }))
              .DoSomething()),
      AuthenticationException);

  EXPECT_EQ(identityRequests->size(), 1);
  {
    auto const& identityRequest0 = identityRequests->at(0);
    EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
    EXPECT_EQ(
        GetScopeFromClientSecretRequest(identityRequest0),
        "https%3A%2F%2Fvault.azure.net%2F.default");
  }

  EXPECT_EQ(serviceRequests->size(), 1);
  {
    auto const& serviceRequest0 = serviceRequests->at(0);
    EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN");
  }
}

TEST(ChallengeBasedAuthenticationPolicy, AuthorizationEmptyPath)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  EXPECT_THROW(
      static_cast<void>( //
          TestKeyVaultClient(
              "https://test.vault.azure.net",
              CreateTestCredential(std::make_shared<TestHttpTransport>(
                  identityRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Ok,
                          "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN\"}",
                          {}),
                  })),
              std::make_shared<TestHttpTransport>(
                  serviceRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Unauthorized,
                          {},
                          {
                              std::make_pair<std::string, std::string>(
                                  "WWW-Authenticate",
                                  "Bearer authorization=\"https://login.windows.net\","
                                  " resource=\"vault.azure.net\""),
                          }),
                  }))
              .DoSomething()),
      AuthenticationException);

  EXPECT_EQ(identityRequests->size(), 1);
  {
    auto const& identityRequest0 = identityRequests->at(0);
    EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
    EXPECT_EQ(
        GetScopeFromClientSecretRequest(identityRequest0),
        "https%3A%2F%2Fvault.azure.net%2F.default");
  }

  EXPECT_EQ(serviceRequests->size(), 1);
  {
    auto const& serviceRequest0 = serviceRequests->at(0);
    EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN");
  }
}

TEST(ChallengeBasedAuthenticationPolicy, AuthorizationLongerPath)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  static_cast<void>( //
      TestKeyVaultClient(
          "https://test.vault.azure.net",
          CreateTestCredential(
              std::make_shared<TestHttpTransport>(
                  identityRequests,
                  std::vector<TestResponse>{
                      TestResponse(
                          HttpStatusCode::Ok,
                          "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN1\"}",
                          {}),
                      TestResponse(
                          HttpStatusCode::Ok,
                          "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN2\"}",
                          {}),
                  }),
              {"*"}),
          std::make_shared<TestHttpTransport>(
              serviceRequests,
              std::vector<TestResponse>{
                  TestResponse(
                      HttpStatusCode::Unauthorized,
                      {},
                      {
                          std::make_pair<std::string, std::string>(
                              "WWW-Authenticate",
                              "Bearer"
                              " authorization=\"https://login.windows.net/NewTenantId/whatever\","
                              " scope=\"https://test.vault.azure.net/.default\""),
                      }),
                  TestResponse(HttpStatusCode::Ok, {}, {}),
              }))
          .DoSomething());

  EXPECT_EQ(identityRequests->size(), 2);
  {
    {
      auto const& identityRequest0 = identityRequests->at(0);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest0),
          "https%3A%2F%2Fvault.azure.net%2F.default");
    }

    {
      auto const& identityRequest1 = identityRequests->at(1);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest1), "NewTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest1),
          "https%3A%2F%2Ftest.vault.azure.net%2F.default");
    }
  }

  EXPECT_EQ(serviceRequests->size(), 2);
  {
    {
      auto const& serviceRequest0 = serviceRequests->at(0);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN1");
    }

    {
      auto const& serviceRequest1 = serviceRequests->at(1);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest1), "Bearer ACCESSTOKEN2");
    }
  }
}

TEST(ChallengeBasedAuthenticationPolicy, MultipleTimes)
{
  auto identityRequests = std::make_shared<std::vector<TestRequest>>();
  auto serviceRequests = std::make_shared<std::vector<TestRequest>>();

  TestKeyVaultClient client(
      "https://test.vault.azure.net",
      CreateTestCredential(
          std::make_shared<TestHttpTransport>(
              identityRequests,
              std::vector<TestResponse>{
                  TestResponse( // <-- DoSomething() #1
                      HttpStatusCode::Ok,
                      "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN1\"}",
                      {}),
                  TestResponse( // <-- DoSomething() #2
                      HttpStatusCode::Ok,
                      "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN2\"}",
                      {}),
                  TestResponse( // <-- DoSomething() #4
                      HttpStatusCode::Ok,
                      "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN3\"}",
                      {}),
                  TestResponse( // <-- DoSomething() #7
                      HttpStatusCode::Ok,
                      "{\"expires_in\":3600,\"access_token\":\"ACCESSTOKEN4\"}",
                      {}),
              }),
          {"*"}),
      std::make_shared<TestHttpTransport>(
          serviceRequests,
          std::vector<TestResponse>{
              // DoSomething() #1 vvvvv
              TestResponse(HttpStatusCode::Ok, {}, {}), // OriginalTenantId, TOKEN1

              // DoSomething() #2 vvvvv
              TestResponse(
                  HttpStatusCode::Unauthorized,
                  {},
                  {
                      std::make_pair<std::string, std::string>(
                          "WWW-Authenticate",
                          "Bearer"
                          " authorization=\"https://login.windows.net/NewTenantId/whatever\","
                          " scope=\"https://test.vault.azure.net/.default\""),
                  }),
              TestResponse(HttpStatusCode::Ok, {}, {}), // NewTenantId, TOKEN2

              // DoSomething() #3 vvvvv
              TestResponse(HttpStatusCode::Ok, {}, {}),

              // DoSomething() #4 vvvvv
              TestResponse(
                  HttpStatusCode::Unauthorized,
                  {},
                  {
                      std::make_pair<std::string, std::string>(
                          "WWW-Authenticate",
                          "Bearer"
                          " authorization_uri=\"https://login.windows.net/AnotherTenantId\","
                          " resource=\"https://test.vault.azure.net/\""),
                  }),
              TestResponse(HttpStatusCode::Ok, {}, {}), // AnotherTenantId (test.vault...), TOKEN3

              // DoSomething() #5 vvvvv
              TestResponse(HttpStatusCode::Ok, {}, {}),

              // DoSomething() #6 vvvvv
              TestResponse(HttpStatusCode::Ok, {}, {}),

              // DoSomething() #7 vvvvv
              TestResponse(
                  HttpStatusCode::Unauthorized,
                  {},
                  {
                      std::make_pair<std::string, std::string>(
                          "WWW-Authenticate",
                          "Bearer"
                          " authorization_uri=\"https://login.windows.net/AnotherTenantId\","
                          " resource=\"https://vault.azure.net\""),
                  }),
              TestResponse(HttpStatusCode::Ok, {}, {}), // AnotherTenantId (vault.azure...), TOKEN4

              // DoSomething() #8 vvvvv
              TestResponse(
                  HttpStatusCode::Unauthorized,
                  {},
                  {
                      std::make_pair<std::string, std::string>(
                          "WWW-Authenticate", "Bearer resource=\"https://vault.azure.net\""),
                  }), // ^^^ authorization_uri is missing - throws

              // DoSomething() #9 vvvvv
              TestResponse(HttpStatusCode::Ok, {}, {}), // AnotherTenantId (vault.azure...), TOKEN4

              // DoSomething() #10 vvvvv
              TestResponse(
                  HttpStatusCode::Unauthorized,
                  {},
                  {
                      std::make_pair<std::string, std::string>(
                          "WWW-Authenticate",
                          "Bearer"
                          " authorization_uri=\"https://login.windows.net/OriginalTenantId\","
                          " resource=\"https://vault.azure.net/\""),
                  }),
              TestResponse(HttpStatusCode::Ok, {}, {}), // OriginalTenantId, cached TOKEN2

              // DoSomething() #11 vvvvv
              TestResponse(
                  HttpStatusCode::Unauthorized,
                  {},
                  {
                      std::make_pair<std::string, std::string>(
                          "WWW-Authenticate",
                          "Bearer"
                          " authorization=\"https://login.windows.net/NewTenantId\""),
                  }), // ^^^ resource is missing - won't update token

              // DoSomething() #12 vvvvv
              TestResponse(HttpStatusCode::Ok, {}, {}), // OriginalTenantId, TOKEN5
          }));

  client.DoSomething(); // #1: Ok with defaults, authorize with TOKEN1
  client.DoSomething(); // #2: Challenge response, NewTenantId, new scope, authorize with TOKEN2
  client.DoSomething(); // #3: Ok, authorize with TOKEN2
  client.DoSomething(); // #4: Challenge response, AnotherTenantId, same scope, auth with TOKEN3
  client.DoSomething(); // #5: Ok, authorize with TOKEN3
  client.DoSomething(); // #6: Ok, authorize with TOKEN3
  client.DoSomething(); // #7: Challenge response, same TenantId, new scope, authorize with TOKEN4
  EXPECT_THROW(client.DoSomething(), AuthenticationException); // #8: Bad challenge (no TenantId)
  client.DoSomething(); // #9: Ok, keeps authorizing with TOKEN4
  client.DoSomething(); // #10: Revert back to OriginalTokenId, use cached TOKEN1
  client.DoSomething(); // #11: Attempt NewTenantId, but scope is missing
  client.DoSomething(); // #12: Ok, authorize with TOKEN1

  EXPECT_EQ(identityRequests->size(), 4);
  {
    // DoSomething() #1 vvv
    {
      auto const& identityRequest0 = identityRequests->at(0);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest0), "OriginalTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest0),
          "https%3A%2F%2Fvault.azure.net%2F.default");
    }

    // DoSomething() #2 vvv
    {
      auto const& identityRequest1 = identityRequests->at(1);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest1), "NewTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest1),
          "https%3A%2F%2Ftest.vault.azure.net%2F.default");
    }

    // DoSomething() #4 vvv
    {
      auto const& identityRequest2 = identityRequests->at(2);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest2), "AnotherTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest2),
          "https%3A%2F%2Ftest.vault.azure.net%2F.default");
    }

    // DoSomething() #7 vvv
    {
      auto const& identityRequest3 = identityRequests->at(3);
      EXPECT_EQ(GetTenantIdFromClientSecretRequest(identityRequest3), "AnotherTenantId");
      EXPECT_EQ(
          GetScopeFromClientSecretRequest(identityRequest3),
          "https%3A%2F%2Fvault.azure.net%2F.default");
    }

    // DoSomething() #10 won't make a request because the token is cached
  }

  EXPECT_EQ(serviceRequests->size(), 16);
  {
    // DoSomething() #1 vvv
    {
      auto const& serviceRequest0 = serviceRequests->at(0);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest0), "Bearer ACCESSTOKEN1");
    }

    // DoSomething() #2 vvv
    {
      auto const& serviceRequest1 = serviceRequests->at(1);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest1), "Bearer ACCESSTOKEN1");
    }

    {
      auto const& serviceRequest2 = serviceRequests->at(2);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest2), "Bearer ACCESSTOKEN2");
    }

    // DoSomething() #3 vvv
    {
      auto const& serviceRequest3 = serviceRequests->at(3);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest3), "Bearer ACCESSTOKEN2");
    }

    // DoSomething() #4 vvv
    {
      auto const& serviceRequest4 = serviceRequests->at(4);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest4), "Bearer ACCESSTOKEN2");
    }

    {
      auto const& serviceRequest5 = serviceRequests->at(5);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest5), "Bearer ACCESSTOKEN3");
    }

    // DoSomething() #5 vvv
    {
      auto const& serviceRequest6 = serviceRequests->at(6);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest6), "Bearer ACCESSTOKEN3");
    }

    // DoSomething() #6 vvv
    {
      auto const& serviceRequest7 = serviceRequests->at(7);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest7), "Bearer ACCESSTOKEN3");
    }

    // DoSomething() #7 vvv
    {
      auto const& serviceRequest8 = serviceRequests->at(8);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest8), "Bearer ACCESSTOKEN3");
    }

    {
      auto const& serviceRequest9 = serviceRequests->at(9);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest9), "Bearer ACCESSTOKEN4");
    }

    // DoSomething() #8 vvv
    {
      auto const& serviceRequest10 = serviceRequests->at(10);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest10), "Bearer ACCESSTOKEN4");
    }

    // DoSomething() #9 vvv
    {
      auto const& serviceRequest11 = serviceRequests->at(11);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest11), "Bearer ACCESSTOKEN4");
    }

    // DoSomething() #10 vvv
    {
      auto const& serviceRequest12 = serviceRequests->at(12);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest12), "Bearer ACCESSTOKEN4");
    }

    {
      auto const& serviceRequest13 = serviceRequests->at(13);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest13), "Bearer ACCESSTOKEN1");
    }

    // DoSomething() #11 vvv
    {
      auto const& serviceRequest14 = serviceRequests->at(14);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest14), "Bearer ACCESSTOKEN1");
    }

    // DoSomething() #12 vvv
    {
      auto const& serviceRequest15 = serviceRequests->at(15);
      EXPECT_EQ(GetAuthHeaderValueFromServiceRequest(serviceRequest15), "Bearer ACCESSTOKEN1");
    }
  }
}
