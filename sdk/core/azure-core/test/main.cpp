// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/http.hpp>
#include <internal/credentials_internal.hpp>

#include "gtest/gtest.h"

#include <string>
#include <vector>

using namespace Azure::Core;

TEST(Http_Request, getters)
{
  Http::HttpMethod httpMethod = Http::HttpMethod::Get;
  std::string url = "http://test.url.com";
  Http::Request req(httpMethod, url);

  // EXPECT_PRED works better than just EQ because it will print values in log
  EXPECT_PRED2(
      [](Http::HttpMethod a, Http::HttpMethod b) { return a == b; }, req.getMethod(), httpMethod);
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, req.getEncodedUrl(), url);
  /* EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getBodyStream(),
      Http::BodyStream::null);
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getBodyBuffer(),
      Http::BodyBuffer::null); */

  uint8_t buffer[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
  auto bufferBody = Http::BodyBuffer(buffer, sizeof(buffer));
  Http::Request requestWithBody(httpMethod, url, &bufferBody);

  EXPECT_PRED2(
      [](Http::HttpMethod a, Http::HttpMethod b) { return a == b; },
      requestWithBody.getMethod(),
      httpMethod);
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; }, requestWithBody.getEncodedUrl(), url);
  /* EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      requestWithBody.getBodyStream(),
      Http::BodyStream::null); */

  // body with buffer
  auto body = requestWithBody.getBodyBuffer();
  ASSERT_EQ(body->_bodyBufferSize, 10);
  for (auto i = 0; i < 10; i++)
  {
    ASSERT_EQ(body->_bodyBuffer[i], i);
  }

  EXPECT_NO_THROW(req.addHeader("name", "value"));
  EXPECT_NO_THROW(req.addHeader("name2", "value2"));

  auto headers = req.getHeaders();

  EXPECT_TRUE(headers.count("name"));
  EXPECT_TRUE(headers.count("name2"));
  EXPECT_FALSE(headers.count("newHeader"));

  auto value = headers.find("name");
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value->second, "value");
  auto value2 = headers.find("name2");
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value2->second, "value2");

  // now add to retry headers
  req.startRetry();
  // same headers first, then one new
  EXPECT_NO_THROW(req.addHeader("name", "retryValue"));
  EXPECT_NO_THROW(req.addHeader("name2", "retryValue2"));
  EXPECT_NO_THROW(req.addHeader("newHeader", "new"));

  headers = req.getHeaders();

  EXPECT_TRUE(headers.count("name"));
  EXPECT_TRUE(headers.count("name2"));
  EXPECT_TRUE(headers.count("newHeader"));

  value = headers.find("name");
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value->second, "retryValue");
  value2 = headers.find("name2");
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value2->second, "retryValue2");
  auto value3 = headers.find("newHeader");
  EXPECT_PRED2([](std::string a, std::string b) { return a == b; }, value3->second, "new");
}

TEST(Http_Request, query_parameter)
{
  Http::HttpMethod httpMethod = Http::HttpMethod::Put;
  std::string url = "http://test.com";
  Http::Request req(httpMethod, url);

  EXPECT_NO_THROW(req.addQueryParameter("query", "value"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getEncodedUrl(),
      url + "?query=value");

  std::string url_with_query = "http://test.com?query=1";
  Http::Request req_with_query(httpMethod, url_with_query);

  // ignore if adding same query parameter key that is already in url
  EXPECT_NO_THROW(req_with_query.addQueryParameter("query", "value"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req_with_query.getEncodedUrl(),
      url_with_query);

  // retry query params testing
  req.startRetry();
  // same query parameter should override previous
  EXPECT_NO_THROW(req.addQueryParameter("query", "retryValue"));

  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getEncodedUrl(),
      url + "?query=retryValue");
}

TEST(Http_Request, add_path)
{
  Http::HttpMethod httpMethod = Http::HttpMethod::Post;
  std::string url = "http://test.com";
  Http::Request req(httpMethod, url);

  EXPECT_NO_THROW(req.addPath("path"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; }, req.getEncodedUrl(), url + "/path");

  EXPECT_NO_THROW(req.addQueryParameter("query", "value"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getEncodedUrl(),
      url + "/path?query=value");

  EXPECT_NO_THROW(req.addPath("path2"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getEncodedUrl(),
      url + "/path/path2?query=value");

  EXPECT_NO_THROW(req.addPath("path3"));
  EXPECT_PRED2(
      [](std::string a, std::string b) { return a == b; },
      req.getEncodedUrl(),
      url + "/path/path2/path3?query=value");
}

class Azure::Core::Credentials::Details::CredentialTest : public ClientSecretCredential
{
public:
  CredentialTest(
      std::string const& tenantId,
      std::string const& clientId,
      std::string const& clientSecret)
      : ClientSecretCredential(tenantId, clientId, clientSecret)
  {
  }

  std::string NewTokenString;
  std::chrono::system_clock::time_point NewExpiration;
  bool IsExpired;

  std::string GetTenantId() const
  {
    return this->ClientSecretCredential::m_clientSecret->m_tenantId;
  }

  std::string GetClientId() const
  {
    return this->ClientSecretCredential::m_clientSecret->m_clientId;
  }

  std::string GetClientSecret() const
  {
    return this->ClientSecretCredential::m_clientSecret->m_clientSecret;
  }

  std::string GetScopes() const { return this->ClientSecretCredential::m_clientSecret->m_scopes; }

  bool IsTokenPtrNull() const { return !this->TokenCredential::m_token; }

  std::string GetTokenString() const { return this->TokenCredential::m_token->m_tokenString; }

  std::chrono::system_clock::time_point GetExpiration() const
  {
    return this->TokenCredential::m_token->m_expiresAt;
  }

private:
  void RefreshToken(
      std::string& newTokenString,
      std::chrono::system_clock::time_point& newExpiration) override
  {
    newTokenString = this->NewTokenString;
    newExpiration = this->NewExpiration;
  }

  bool IsTokenExpired(std::chrono::system_clock::time_point const&) const override
  {
    return this->IsExpired;
  }
};

TEST(Credential, ClientSecretCredential)
{
  // Client Secret credential properties
  std::string const tenantId = "tenantId";
  std::string const clientId = "clientId";
  std::string const clientSecret = "clientSecret";

  Credentials::Details::CredentialTest clientSecretCredential(tenantId, clientId, clientSecret);

  EXPECT_EQ(clientSecretCredential.GetTenantId(), tenantId);
  EXPECT_EQ(clientSecretCredential.GetClientId(), clientId);
  EXPECT_EQ(clientSecretCredential.GetClientSecret(), clientSecret);

  // Token credential
  {
    auto const emptyString = std::string();
    auto const defaultTime = std::chrono::system_clock::time_point();
    {
      // Default values
      {
        EXPECT_EQ(clientSecretCredential.IsTokenPtrNull(), true);
      }

      {
        // Set scopes
        std::string const scopes = "scope";
        {
          Credentials::Credential::Internal::SetScopes(clientSecretCredential, scopes);
          EXPECT_EQ(clientSecretCredential.IsTokenPtrNull(), true);
        }

        // Get token
        {
          std::string const olderToken = "olderToken";
          std::string const newToken = "newToken";
          auto const olderTime = defaultTime + std::chrono::minutes(10);
          auto const newTime = olderTime + std::chrono::minutes(10);

          {
            clientSecretCredential.IsExpired = true;
            clientSecretCredential.NewTokenString = olderToken;
            clientSecretCredential.NewExpiration = olderTime;

            auto const tokenReceived
                = Credentials::TokenCredential::Internal::GetToken(clientSecretCredential);

            EXPECT_EQ(clientSecretCredential.IsTokenPtrNull(), false);
            EXPECT_EQ(tokenReceived, olderToken);
            EXPECT_EQ(clientSecretCredential.GetTokenString(), olderToken);
            EXPECT_EQ(clientSecretCredential.GetScopes(), scopes);
            EXPECT_EQ(clientSecretCredential.GetExpiration(), olderTime);
          }

          // Attemp to get the token when it is not expired yet
          {
            clientSecretCredential.IsExpired = false;
            clientSecretCredential.NewTokenString = newToken;
            clientSecretCredential.NewExpiration = newTime;

            auto const tokenReceived
                = Credentials::TokenCredential::Internal::GetToken(clientSecretCredential);

            EXPECT_EQ(tokenReceived, olderToken);
            EXPECT_EQ(clientSecretCredential.GetTokenString(), olderToken);
            EXPECT_EQ(clientSecretCredential.GetScopes(), scopes);
            EXPECT_EQ(clientSecretCredential.GetExpiration(), olderTime);
          }

          // Attempt to get token after it expired
          {
            clientSecretCredential.IsExpired = true;

            auto const tokenReceived
                = Credentials::TokenCredential::Internal::GetToken(clientSecretCredential);

            EXPECT_EQ(tokenReceived, newToken);
            EXPECT_EQ(clientSecretCredential.GetTokenString(), newToken);
            EXPECT_EQ(clientSecretCredential.GetScopes(), scopes);
            EXPECT_EQ(clientSecretCredential.GetExpiration(), newTime);

            clientSecretCredential.IsExpired = false;
          }

          // Setting the very same scopes set earlier does not reset token
          {
            std::string const scopesCopy
                = scopes.substr(0, scopes.length() / 2) + scopes.substr(scopes.length() / 2);

            {
              auto const scopesPtr = scopes.c_str();
              auto const scopesCopyPtr = scopesCopy.c_str();
              EXPECT_NE(scopesPtr, scopesCopyPtr);
              EXPECT_EQ(scopes, scopesCopy);
            }
            

            Credentials::Credential::Internal::SetScopes(clientSecretCredential, scopesCopy);

            EXPECT_EQ(clientSecretCredential.GetTenantId(), tenantId);
            EXPECT_EQ(clientSecretCredential.GetClientId(), clientId);
            EXPECT_EQ(clientSecretCredential.GetClientSecret(), clientSecret);

            auto const tokenReceived
                = Credentials::TokenCredential::Internal::GetToken(clientSecretCredential);

            EXPECT_EQ(tokenReceived, newToken);
            EXPECT_EQ(clientSecretCredential.GetTokenString(), newToken);
            EXPECT_EQ(clientSecretCredential.GetScopes(), scopes);
            EXPECT_EQ(clientSecretCredential.GetExpiration(), newTime);
          }

          // Updating scopes does reset the token
          {
            clientSecretCredential.IsExpired = false;

            std::string const anotherScopes = "anotherScopes";
            std::string const anotherToken = "anotherToken";
            auto const anotherTime = newTime + std::chrono::minutes(10);

            clientSecretCredential.NewTokenString = anotherToken;
            clientSecretCredential.NewExpiration = anotherTime;

            auto tokenReceived
                = Credentials::TokenCredential::Internal::GetToken(clientSecretCredential);

            EXPECT_EQ(tokenReceived, newToken);
            EXPECT_EQ(clientSecretCredential.GetTokenString(), newToken);
            EXPECT_EQ(clientSecretCredential.GetScopes(), scopes);
            EXPECT_EQ(clientSecretCredential.GetExpiration(), newTime);

            Credentials::Credential::Internal::SetScopes(
                clientSecretCredential, std::string(anotherScopes));


            EXPECT_EQ(clientSecretCredential.GetTenantId(), tenantId);
            EXPECT_EQ(clientSecretCredential.GetClientId(), clientId);
            EXPECT_EQ(clientSecretCredential.GetClientSecret(), clientSecret);
            EXPECT_EQ(clientSecretCredential.GetScopes(), anotherScopes);
            EXPECT_EQ(clientSecretCredential.IsTokenPtrNull(), true);

            tokenReceived
                = Credentials::TokenCredential::Internal::GetToken(clientSecretCredential);

            EXPECT_EQ(clientSecretCredential.IsTokenPtrNull(), false);
            EXPECT_EQ(tokenReceived, anotherToken);
            EXPECT_EQ(clientSecretCredential.GetTokenString(), anotherToken);
            EXPECT_EQ(clientSecretCredential.GetScopes(), anotherScopes);
            EXPECT_EQ(clientSecretCredential.GetExpiration(), anotherTime);
          }
        }
      }
    }
  }
}
