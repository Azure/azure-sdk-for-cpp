// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/environment_credential.hpp"

#include "credential_test_helper.hpp"
#include <azure/core/diagnostics/logger.hpp>

#include <gtest/gtest.h>

using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Http::HttpMethod;
using Azure::Identity::EnvironmentCredential;
using Azure::Identity::Test::_detail::CredentialTestHelper;

TEST(EnvironmentCredential, GetCredentialName)
{
  CredentialTestHelper::EnvironmentOverride const env({
      {"AZURE_TENANT_ID", "01234567-89ab-cdef-fedc-ba8976543210"},
      {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
      {"AZURE_CLIENT_SECRET", "CLIENTSECRET"},
      {"AZURE_AUTHORITY_HOST", ""},
      {"AZURE_USERNAME", ""},
      {"AZURE_PASSWORD", ""},
      {"AZURE_CLIENT_CERTIFICATE_PATH", ""},
  });

  EnvironmentCredential const cred;
  EXPECT_EQ(cred.GetCredentialName(), "EnvironmentCredential");
}

TEST(EnvironmentCredential, RegularClientSecretCredential)
{
  using Azure::Core::Diagnostics::Logger;
  using LogMsgVec = std::vector<std::pair<Logger::Level, std::string>>;
  LogMsgVec log;
  Logger::SetLevel(Logger::Level::Verbose);
  Logger::SetListener([&](auto lvl, auto msg) { log.push_back(std::make_pair(lvl, msg)); });

  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [&](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", "01234567-89ab-cdef-fedc-ba8976543210"},
             {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
             {"AZURE_CLIENT_SECRET", "CLIENTSECRET"},
             {"AZURE_AUTHORITY_HOST", "https://microsoft.com/"},
             {"AZURE_USERNAME", ""},
             {"AZURE_PASSWORD", ""},
             {"AZURE_CLIENT_CERTIFICATE_PATH", ""}});

        auto credential = std::make_unique<EnvironmentCredential>(options);

        EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
        EXPECT_EQ(log[0].first, Logger::Level::Verbose);
        EXPECT_EQ(
            log[0].second,
            "Identity: EnvironmentCredential: 'AZURE_TENANT_ID', 'AZURE_CLIENT_ID', "
            "'AZURE_CLIENT_SECRET', and 'AZURE_AUTHORITY_HOST' environment variables are set, so "
            "ClientSecretCredential with corresponding tenantId, clientId, clientSecret, and "
            "authorityHost gets created.");
        log.clear();

        return credential;
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});

  EXPECT_EQ(actual.Requests.size(), 1U);
  EXPECT_EQ(actual.Responses.size(), 1U);
  auto const& request = actual.Requests.at(0);
  auto const& response = actual.Responses.at(0);

  EXPECT_EQ(request.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(
      request.AbsoluteUrl,
      "https://microsoft.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    constexpr char expectedBody[]
        = "grant_type=client_credentials"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&client_secret=CLIENTSECRET"
          "&scope=https%3A%2F%2Fazure.com%2F.default"; // cspell:disable-line

    EXPECT_EQ(request.Body, expectedBody);

    EXPECT_NE(request.Headers.find("Content-Length"), request.Headers.end());
    EXPECT_EQ(request.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));
  }

  EXPECT_NE(request.Headers.find("Content-Type"), request.Headers.end());
  EXPECT_EQ(request.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response.AccessToken.Token, "ACCESSTOKEN1");

  using namespace std::chrono_literals;
  EXPECT_GE(response.AccessToken.ExpiresOn, response.EarliestExpiration + 3600s);
  EXPECT_LE(response.AccessToken.ExpiresOn, response.LatestExpiration + 3600s);

  Logger::SetListener(nullptr);
}

TEST(EnvironmentCredential, AzureStackClientSecretCredential)
{
  using Azure::Core::Diagnostics::Logger;
  using LogMsgVec = std::vector<std::pair<Logger::Level, std::string>>;
  LogMsgVec log;
  Logger::SetLevel(Logger::Level::Informational); // Setting it to Informational instead of Verbose
                                                  // will result in less detailed log message.
  Logger::SetListener([&](auto lvl, auto msg) { log.push_back(std::make_pair(lvl, msg)); });

  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [&](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", "adfs"},
             {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
             {"AZURE_CLIENT_SECRET", "CLIENTSECRET"},
             {"AZURE_AUTHORITY_HOST", "https://microsoft.com/"},
             {"AZURE_USERNAME", ""},
             {"AZURE_PASSWORD", ""},
             {"AZURE_CLIENT_CERTIFICATE_PATH", ""}});

        auto credential = std::make_unique<EnvironmentCredential>(options);

        EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
        EXPECT_EQ(log[0].first, Logger::Level::Informational);
        EXPECT_EQ(
            log[0].second,
            "Identity: EnvironmentCredential gets created with ClientSecretCredential.");
        log.clear();

        return credential;
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});

  EXPECT_EQ(actual.Requests.size(), 1U);
  EXPECT_EQ(actual.Responses.size(), 1U);
  auto const& request = actual.Requests.at(0);
  auto const& response = actual.Responses.at(0);

  EXPECT_EQ(request.AbsoluteUrl, "https://microsoft.com/adfs/oauth2/token");

  {
    constexpr char expectedBody[] = "grant_type=client_credentials"
                                    "&client_id=fedcba98-7654-3210-0123-456789abcdef"
                                    "&client_secret=CLIENTSECRET"
                                    "&scope=https%3A%2F%2Fazure.com"; // cspell:disable-line

    EXPECT_EQ(request.Body, expectedBody);

    EXPECT_NE(request.Headers.find("Content-Length"), request.Headers.end());
    EXPECT_EQ(request.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));
  }

  EXPECT_NE(request.Headers.find("Content-Type"), request.Headers.end());
  EXPECT_EQ(request.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_NE(request.Headers.find("Host"), request.Headers.end());
  EXPECT_EQ(request.Headers.at("Host"), "microsoft.com");

  EXPECT_EQ(response.AccessToken.Token, "ACCESSTOKEN1");

  using namespace std::chrono_literals;
  EXPECT_GE(response.AccessToken.ExpiresOn, response.EarliestExpiration + 3600s);
  EXPECT_LE(response.AccessToken.ExpiresOn, response.LatestExpiration + 3600s);

  Logger::SetListener(nullptr);
}

TEST(EnvironmentCredential, Unavailable)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  using Azure::Core::Diagnostics::Logger;
  using LogMsgVec = std::vector<std::pair<Logger::Level, std::string>>;
  LogMsgVec log;
  Logger::SetLevel(Logger::Level::Verbose);
  Logger::SetListener([&](auto lvl, auto msg) { log.push_back(std::make_pair(lvl, msg)); });

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [&](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", ""},
             {"AZURE_CLIENT_ID", ""},
             {"AZURE_CLIENT_SECRET", ""},
             {"AZURE_AUTHORITY_HOST", ""},
             {"AZURE_USERNAME", ""},
             {"AZURE_PASSWORD", ""},
             {"AZURE_CLIENT_CERTIFICATE_PATH", ""}});

        auto credential = std::make_unique<EnvironmentCredential>(options);

        EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
        EXPECT_EQ(log[0].first, Logger::Level::Warning);
        EXPECT_EQ(
            log[0].second,
            "Identity: EnvironmentCredential was not initialized with underlying credential: Both "
            "'AZURE_TENANT_ID' and 'AZURE_CLIENT_ID', and at least one of 'AZURE_CLIENT_SECRET', "
            "'AZURE_CLIENT_CERTIFICATE_PATH' needs to be set. Additionally, "
            "'AZURE_AUTHORITY_HOST' could be set to override the default authority host."
            " Currently:\n"
            " * 'AZURE_TENANT_ID' is NOT set\n"
            " * 'AZURE_CLIENT_ID' is NOT set\n"
            " * 'AZURE_CLIENT_SECRET' is NOT set\n"
            " * 'AZURE_CLIENT_CERTIFICATE_PATH' is NOT set\n"
            " * 'AZURE_AUTHORITY_HOST' is NOT set\n");
        log.clear();

        return credential;
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"},
      [&](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);

        EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
        EXPECT_EQ(log[0].first, Logger::Level::Warning);
        EXPECT_EQ(
            log[0].second,
            "Identity: EnvironmentCredential authentication unavailable. "
            "See earlier EnvironmentCredential log messages for details.");
        log.clear();

        return token;
      }));

  Logger::SetListener(nullptr);
}

TEST(EnvironmentCredential, ClientSecretDefaultAuthority)
{
  using Azure::Core::Diagnostics::Logger;
  using LogMsgVec = std::vector<std::pair<Logger::Level, std::string>>;
  LogMsgVec log;
  Logger::SetLevel(Logger::Level::Verbose);
  Logger::SetListener([&](auto lvl, auto msg) { log.push_back(std::make_pair(lvl, msg)); });

  auto const actual = CredentialTestHelper::SimulateTokenRequest(
      [&](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", "01234567-89ab-cdef-fedc-ba8976543210"},
             {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
             {"AZURE_CLIENT_SECRET", "CLIENTSECRET"},
             {"AZURE_AUTHORITY_HOST", ""},
             {"AZURE_USERNAME", ""},
             {"AZURE_PASSWORD", ""},
             {"AZURE_CLIENT_CERTIFICATE_PATH", ""}});

        auto credential = std::make_unique<EnvironmentCredential>(options);

        EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
        EXPECT_EQ(log[0].first, Logger::Level::Verbose);
        EXPECT_EQ(
            log[0].second,
            "Identity: EnvironmentCredential: 'AZURE_TENANT_ID', 'AZURE_CLIENT_ID', and "
            "'AZURE_CLIENT_SECRET' environment variables are set, so ClientSecretCredential with "
            "corresponding tenantId, clientId, and clientSecret gets created.");
        log.clear();

        return credential;
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});

  EXPECT_EQ(actual.Requests.size(), 1U);
  EXPECT_EQ(actual.Responses.size(), 1U);
  auto const& request = actual.Requests.at(0);
  auto const& response = actual.Responses.at(0);

  EXPECT_EQ(request.HttpMethod, HttpMethod::Post);

  EXPECT_EQ(
      request.AbsoluteUrl,
      "https://login.microsoftonline.com/01234567-89ab-cdef-fedc-ba8976543210/oauth2/v2.0/token");

  {
    constexpr char expectedBody[]
        = "grant_type=client_credentials"
          "&client_id=fedcba98-7654-3210-0123-456789abcdef"
          "&client_secret=CLIENTSECRET"
          "&scope=https%3A%2F%2Fazure.com%2F.default"; // cspell:disable-line

    EXPECT_EQ(request.Body, expectedBody);

    EXPECT_NE(request.Headers.find("Content-Length"), request.Headers.end());
    EXPECT_EQ(request.Headers.at("Content-Length"), std::to_string(sizeof(expectedBody) - 1));
  }

  EXPECT_NE(request.Headers.find("Content-Type"), request.Headers.end());
  EXPECT_EQ(request.Headers.at("Content-Type"), "application/x-www-form-urlencoded");

  EXPECT_EQ(response.AccessToken.Token, "ACCESSTOKEN1");

  using namespace std::chrono_literals;
  EXPECT_GE(response.AccessToken.ExpiresOn, response.EarliestExpiration + 3600s);
  EXPECT_LE(response.AccessToken.ExpiresOn, response.LatestExpiration + 3600s);

  Logger::SetListener(nullptr);
}

TEST(EnvironmentCredential, ClientSecretNoTenantId)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  using Azure::Core::Diagnostics::Logger;
  using LogMsgVec = std::vector<std::pair<Logger::Level, std::string>>;
  LogMsgVec log;
  Logger::SetLevel(Logger::Level::Verbose);
  Logger::SetListener([&](auto lvl, auto msg) { log.push_back(std::make_pair(lvl, msg)); });

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [&](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", ""},
             {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
             {"AZURE_CLIENT_SECRET", "CLIENTSECRET"},
             {"AZURE_AUTHORITY_HOST", "https://microsoft.com/"},
             {"AZURE_USERNAME", ""},
             {"AZURE_PASSWORD", ""},
             {"AZURE_CLIENT_CERTIFICATE_PATH", ""}});

        auto credential = std::make_unique<EnvironmentCredential>(options);

        EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
        EXPECT_EQ(log[0].first, Logger::Level::Warning);
        EXPECT_EQ(
            log[0].second,
            "Identity: EnvironmentCredential was not initialized with underlying credential: Both "
            "'AZURE_TENANT_ID' and 'AZURE_CLIENT_ID', and at least one of 'AZURE_CLIENT_SECRET', "
            "'AZURE_CLIENT_CERTIFICATE_PATH' needs to be set. Additionally, "
            "'AZURE_AUTHORITY_HOST' could be set to override the default authority host."
            " Currently:\n"
            " * 'AZURE_TENANT_ID' is NOT set\n"
            " * 'AZURE_CLIENT_ID' is set\n"
            " * 'AZURE_CLIENT_SECRET' is set\n"
            " * 'AZURE_CLIENT_CERTIFICATE_PATH' is NOT set\n"
            " * 'AZURE_AUTHORITY_HOST' is set\n");
        log.clear();

        return credential;
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"},
      [&](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);

        EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
        EXPECT_EQ(log[0].first, Logger::Level::Warning);
        EXPECT_EQ(
            log[0].second,
            "Identity: EnvironmentCredential authentication unavailable. "
            "See earlier EnvironmentCredential log messages for details.");
        log.clear();

        return token;
      }));

  Logger::SetListener(nullptr);
}

TEST(EnvironmentCredential, ClientSecretNoClientId)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  using Azure::Core::Diagnostics::Logger;
  using LogMsgVec = std::vector<std::pair<Logger::Level, std::string>>;
  LogMsgVec log;
  Logger::SetLevel(Logger::Level::Warning); // Setting it to Warning instead of Verbose will result
                                            // in less detailed Warning message.
  Logger::SetListener([&](auto lvl, auto msg) { log.push_back(std::make_pair(lvl, msg)); });

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [&](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", "01234567-89ab-cdef-fedc-ba8976543210"},
             {"AZURE_CLIENT_ID", ""},
             {"AZURE_CLIENT_SECRET", "CLIENTSECRET"},
             {"AZURE_AUTHORITY_HOST", "https://microsoft.com/"},
             {"AZURE_USERNAME", ""},
             {"AZURE_PASSWORD", ""},
             {"AZURE_CLIENT_CERTIFICATE_PATH", ""}});

        auto credential = std::make_unique<EnvironmentCredential>(options);

        EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
        EXPECT_EQ(log[0].first, Logger::Level::Warning);
        EXPECT_EQ(
            log[0].second,
            "Identity: EnvironmentCredential was not initialized with underlying credential.");
        log.clear();

        return credential;
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"},
      [&](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);

        EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
        EXPECT_EQ(log[0].first, Logger::Level::Warning);
        EXPECT_EQ(
            log[0].second,
            "Identity: EnvironmentCredential authentication unavailable. "
            "See earlier EnvironmentCredential log messages for details.");
        log.clear();

        return token;
      }));

  Logger::SetListener(nullptr);
}

TEST(EnvironmentCredential, ClientSecretNoClientSecret)
{
  using Azure::Core::Credentials::AccessToken;
  using Azure::Core::Credentials::AuthenticationException;

  using Azure::Core::Diagnostics::Logger;
  using LogMsgVec = std::vector<std::pair<Logger::Level, std::string>>;
  LogMsgVec log;
  Logger::SetLevel(Logger::Level::Verbose);
  Logger::SetListener([&](auto lvl, auto msg) { log.push_back(std::make_pair(lvl, msg)); });

  static_cast<void>(CredentialTestHelper::SimulateTokenRequest(
      [&](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env(
            {{"AZURE_TENANT_ID", "01234567-89ab-cdef-fedc-ba8976543210"},
             {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
             {"AZURE_CLIENT_SECRET", ""},
             {"AZURE_AUTHORITY_HOST", "https://microsoft.com/"},
             {"AZURE_USERNAME", ""},
             {"AZURE_PASSWORD", ""},
             {"AZURE_CLIENT_CERTIFICATE_PATH", ""}});

        auto credential = std::make_unique<EnvironmentCredential>(options);

        EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
        EXPECT_EQ(log[0].first, Logger::Level::Warning);
        EXPECT_EQ(
            log[0].second,
            "Identity: EnvironmentCredential was not initialized with underlying credential: Both "
            "'AZURE_TENANT_ID' and 'AZURE_CLIENT_ID', and at least one of 'AZURE_CLIENT_SECRET', "
            "'AZURE_CLIENT_CERTIFICATE_PATH' needs to be set. Additionally, "
            "'AZURE_AUTHORITY_HOST' could be set to override the default authority host."
            " Currently:\n"
            " * 'AZURE_TENANT_ID' is set\n"
            " * 'AZURE_CLIENT_ID' is set\n"
            " * 'AZURE_CLIENT_SECRET' is NOT set\n"
            " * 'AZURE_CLIENT_CERTIFICATE_PATH' is NOT set\n"
            " * 'AZURE_AUTHORITY_HOST' is set\n");
        log.clear();

        return credential;
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"},
      [&](auto& credential, auto& tokenRequestContext, auto& context) {
        AccessToken token;
        EXPECT_THROW(
            token = credential.GetToken(tokenRequestContext, context), AuthenticationException);

        EXPECT_EQ(log.size(), LogMsgVec::size_type(1));
        EXPECT_EQ(log[0].first, Logger::Level::Warning);
        EXPECT_EQ(
            log[0].second,
            "Identity: EnvironmentCredential authentication unavailable. "
            "See earlier EnvironmentCredential log messages for details.");
        log.clear();

        return token;
      }));

  Logger::SetListener(nullptr);
}
