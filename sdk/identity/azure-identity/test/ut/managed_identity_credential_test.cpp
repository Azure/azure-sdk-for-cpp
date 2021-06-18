// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/managed_identity_credential.hpp"

#include "credential_test_helper.hpp"

#include <gtest/gtest.h>

using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Http::HttpMethod;
using Azure::Core::Http::HttpStatusCode;
using Azure::Identity::ManagedIdentityCredential;
using Azure::Identity::Test::_detail::CredentialTestHelper;

TEST(ManagedIdentityCredential, AppService)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com/"},
            {"MSI_SECRET", "CLIENTSECRET"},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Get);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://microsoft.com"
      "?api-version=2017-09-01"
      "&resource=https%3A%2F%2Fazure.com");

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://microsoft.com"
      "?api-version=2017-09-01"
      "&resource=https%3A%2F%2Foutlook.com");

  EXPECT_TRUE(request0.Body.empty());
  EXPECT_TRUE(request1.Body.empty());

  {
    EXPECT_NE(request0.Headers.find("secret"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("secret"), "CLIENTSECRET");

    EXPECT_NE(request1.Headers.find("secret"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("secret"), "CLIENTSECRET");
  }

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GT(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LT(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GT(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LT(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

TEST(ManagedIdentityCredential, AppServiceClientId)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com/"},
            {"MSI_SECRET", "CLIENTSECRET"},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(
            "fedcba98-7654-3210-0123-456789abcdef", options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Get);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://microsoft.com"
      "?api-version=2017-09-01"
      "&clientid=fedcba98-7654-3210-0123-456789abcdef"
      "&resource=https%3A%2F%2Fazure.com");

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://microsoft.com"
      "?api-version=2017-09-01"
      "&clientid=fedcba98-7654-3210-0123-456789abcdef"
      "&resource=https%3A%2F%2Foutlook.com");

  EXPECT_TRUE(request0.Body.empty());
  EXPECT_TRUE(request1.Body.empty());

  {
    EXPECT_NE(request0.Headers.find("secret"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("secret"), "CLIENTSECRET");

    EXPECT_NE(request1.Headers.find("secret"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("secret"), "CLIENTSECRET");
  }

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GT(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LT(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GT(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LT(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

TEST(ManagedIdentityCredential, AppServiceInvalidUrl)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  using Azure::Core::Credentials::AuthenticationException;
  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com:INVALID/"},
            {"MSI_SECRET", "CLIENTSECRET"},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        std::unique_ptr<ManagedIdentityCredential const> appServiceManagedIdentityCredential;
        EXPECT_THROW(
            appServiceManagedIdentityCredential
            = std::make_unique<ManagedIdentityCredential>(options),
            AuthenticationException);

        return appServiceManagedIdentityCredential;
      },
      {},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"}));
}

TEST(ManagedIdentityCredential, CloudShell)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com/"},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(request0.AbsoluteUrl, "https://microsoft.com");
  EXPECT_EQ(request1.AbsoluteUrl, "https://microsoft.com");

  EXPECT_EQ(request0.Body, "resource=https%3A%2F%2Fazure.com");
  EXPECT_EQ(request1.Body, "resource=https%3A%2F%2Foutlook.com");

  {
    EXPECT_NE(request0.Headers.find("Metadata"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("Metadata"), "true");

    EXPECT_NE(request1.Headers.find("Metadata"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Metadata"), "true");
  }

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GT(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LT(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GT(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LT(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

TEST(ManagedIdentityCredential, CloudShellClientId)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com/"},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(
            "fedcba98-7654-3210-0123-456789abcdef", options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Post);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(request0.AbsoluteUrl, "https://microsoft.com");
  EXPECT_EQ(request1.AbsoluteUrl, "https://microsoft.com");

  EXPECT_EQ(
      request0.Body,
      "resource=https%3A%2F%2Fazure.com&client_id=fedcba98-7654-3210-0123-456789abcdef");

  EXPECT_EQ(
      request1.Body,
      "resource=https%3A%2F%2Foutlook.com&client_id=fedcba98-7654-3210-0123-456789abcdef");

  {
    EXPECT_NE(request0.Headers.find("Metadata"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("Metadata"), "true");

    EXPECT_NE(request1.Headers.find("Metadata"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Metadata"), "true");
  }

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GT(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LT(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GT(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LT(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

TEST(ManagedIdentityCredential, CloudShellInvalidUrl)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", "https://microsoft.com:INVALID/"},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        std::unique_ptr<ManagedIdentityCredential const> cloudShellManagedIdentityCredential;
        EXPECT_THROW(
            cloudShellManagedIdentityCredential
            = std::make_unique<ManagedIdentityCredential>(options),
            AuthenticationException);

        return cloudShellManagedIdentityCredential;
      },
      {},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"}));
}

TEST(ManagedIdentityCredential, AzureArc)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}},
      {{HttpStatusCode::Unauthorized, "", {{"WWW-Authenticate", "ABC=SECRET1"}}},
       {HttpStatusCode::Ok, "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}", {}},
       {HttpStatusCode::Unauthorized, "", {{"WWW-Authenticate", "XYZ=SECRET2"}}},
       {HttpStatusCode::Ok, "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}", {}}});

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);
  auto const& request2 = actual.Requests.at(2);
  auto const& request3 = actual.Requests.at(3);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request2.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request3.HttpMethod, HttpMethod::Get);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "https://visualstudio.com?api-version=2019-11-01&resource=https%3A%2F%2Fazure.com");

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "https://visualstudio.com?api-version=2019-11-01&resource=https%3A%2F%2Fazure.com");

  EXPECT_EQ(
      request2.AbsoluteUrl,
      "https://visualstudio.com?api-version=2019-11-01&resource=https%3A%2F%2Foutlook.com");

  EXPECT_EQ(
      request3.AbsoluteUrl,
      "https://visualstudio.com?api-version=2019-11-01&resource=https%3A%2F%2Foutlook.com");

  EXPECT_TRUE(request0.Body.empty());
  EXPECT_TRUE(request1.Body.empty());
  EXPECT_TRUE(request2.Body.empty());
  EXPECT_TRUE(request3.Body.empty());

  {
    EXPECT_NE(request0.Headers.find("Metadata"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("Metadata"), "true");

    EXPECT_NE(request1.Headers.find("Metadata"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Metadata"), "true");

    EXPECT_NE(request2.Headers.find("Metadata"), request2.Headers.end());
    EXPECT_EQ(request2.Headers.at("Metadata"), "true");

    EXPECT_NE(request3.Headers.find("Metadata"), request3.Headers.end());
    EXPECT_EQ(request3.Headers.at("Metadata"), "true");
  }

  {
    EXPECT_EQ(request0.Headers.find("Authorization"), request0.Headers.end());

    EXPECT_NE(request1.Headers.find("Authorization"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Authorization"), "Basic SECRET1");

    EXPECT_EQ(request2.Headers.find("Authorization"), request2.Headers.end());

    EXPECT_NE(request3.Headers.find("Authorization"), request3.Headers.end());
    EXPECT_EQ(request3.Headers.at("Authorization"), "Basic SECRET2");
  }

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GT(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LT(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GT(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LT(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

TEST(ManagedIdentityCredential, AzureArcClientId)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        std::unique_ptr<ManagedIdentityCredential const> azureArcManagedIdentityCredential;
        EXPECT_THROW(
            azureArcManagedIdentityCredential = std::make_unique<ManagedIdentityCredential>(
                "fedcba98-7654-3210-0123-456789abcdef", options),
            AuthenticationException);

        return azureArcManagedIdentityCredential;
      },
      {},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"}));
}

TEST(ManagedIdentityCredential, AzureArcAuthHeaderMissing)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}},
      {{HttpStatusCode::Unauthorized, "", {}},
       {HttpStatusCode::Ok, "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}", {}}},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}

TEST(ManagedIdentityCredential, AzureArcAuthHeaderNoEquals)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}},
      {{HttpStatusCode::Unauthorized, "", {{"WWW-Authenticate", "ABCSECRET1"}}},
       {HttpStatusCode::Ok, "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}", {}}},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}

TEST(ManagedIdentityCredential, AzureArcAuthHeaderTwoEquals)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}},
      {{HttpStatusCode::Unauthorized, "", {{"WWW-Authenticate", "ABC=SECRET1=SECRET2"}}},
       {HttpStatusCode::Ok, "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}", {}}},
      [](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);
        return token;
      }));
}

TEST(ManagedIdentityCredential, AzureArcInvalidUrl)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", "https://visualstudio.com:INVALID/"},
            {"IMDS_ENDPOINT", "https://xbox.com/"},
            {"IDENTITY_HEADER", "CLIENTSECRET"},
            {"IDENTITY_SERVER_THUMBPRINT", "0123456789abcdef0123456789abcdef01234567"},
        });

        std::unique_ptr<ManagedIdentityCredential const> azureArcManagedIdentityCredential;
        EXPECT_THROW(
            azureArcManagedIdentityCredential
            = std::make_unique<ManagedIdentityCredential>(options),
            AuthenticationException);

        return azureArcManagedIdentityCredential;
      },
      {},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"}));
}

TEST(ManagedIdentityCredential, Imds)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", ""},
            {"IMDS_ENDPOINT", ""},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", ""},
        });

        return std::make_unique<ManagedIdentityCredential>(options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Get);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "http://169.254.169.254/metadata/identity/oauth2/token"
      "?api-version=2018-02-01"
      "&resource=https%3A%2F%2Fazure.com");

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "http://169.254.169.254/metadata/identity/oauth2/token"
      "?api-version=2018-02-01"
      "&resource=https%3A%2F%2Foutlook.com");

  EXPECT_TRUE(request0.Body.empty());
  EXPECT_TRUE(request1.Body.empty());

  {
    EXPECT_NE(request0.Headers.find("Metadata"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("Metadata"), "true");

    EXPECT_NE(request1.Headers.find("Metadata"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Metadata"), "true");
  }

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GT(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LT(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GT(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LT(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}

TEST(ManagedIdentityCredential, ImdsClientId)
{
  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", ""},
            {"IMDS_ENDPOINT", ""},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", ""},
        });

        return std::make_unique<ManagedIdentityCredential>(
            "fedcba98-7654-3210-0123-456789abcdef", options);
      },
      {{{"https://azure.com/.default"}}, {{"https://outlook.com/.default"}}},
      std::vector<std::string>{
          "{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}",
          "{\"expires_in\":7200, \"access_token\":\"ACCESSTOKEN2\"}"});

  auto const& request0 = actual.Requests.at(0);
  auto const& request1 = actual.Requests.at(1);

  auto const& response0 = actual.Responses.at(0);
  auto const& response1 = actual.Responses.at(1);

  EXPECT_EQ(request0.HttpMethod, HttpMethod::Get);
  EXPECT_EQ(request1.HttpMethod, HttpMethod::Get);

  EXPECT_EQ(
      request0.AbsoluteUrl,
      "http://169.254.169.254/metadata/identity/oauth2/token"
      "?api-version=2018-02-01"
      "&client_id=fedcba98-7654-3210-0123-456789abcdef"
      "&resource=https%3A%2F%2Fazure.com");

  EXPECT_EQ(
      request1.AbsoluteUrl,
      "http://169.254.169.254/metadata/identity/oauth2/token"
      "?api-version=2018-02-01"
      "&client_id=fedcba98-7654-3210-0123-456789abcdef"
      "&resource=https%3A%2F%2Foutlook.com");

  EXPECT_TRUE(request0.Body.empty());
  EXPECT_TRUE(request1.Body.empty());

  {
    EXPECT_NE(request0.Headers.find("Metadata"), request0.Headers.end());
    EXPECT_EQ(request0.Headers.at("Metadata"), "true");

    EXPECT_NE(request1.Headers.find("Metadata"), request1.Headers.end());
    EXPECT_EQ(request1.Headers.at("Metadata"), "true");
  }

  EXPECT_EQ(response0.AccessToken.Token, "ACCESSTOKEN1");
  EXPECT_EQ(response1.AccessToken.Token, "ACCESSTOKEN2");

  using namespace std::chrono_literals;
  EXPECT_GT(response0.AccessToken.ExpiresOn, response0.EarliestExpiration + 3600s);
  EXPECT_LT(response0.AccessToken.ExpiresOn, response0.LatestExpiration + 3600s);

  EXPECT_GT(response1.AccessToken.ExpiresOn, response1.EarliestExpiration + 7200s);
  EXPECT_LT(response1.AccessToken.ExpiresOn, response1.LatestExpiration + 7200s);
}
