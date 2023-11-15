// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/default_azure_credential.hpp"
#include "credential_test_helper.hpp"

#include <azure/core/diagnostics/logger.hpp>

#include <../src/private/chained_token_credential_impl.hpp>
#include <gtest/gtest.h>

using Azure::Identity::DefaultAzureCredential;

using Azure::Core::Context;
using Azure::Core::Credentials::AccessToken;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenCredential;
using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Diagnostics::Logger;
using Azure::Identity::Test::_detail::CredentialTestHelper;

namespace {
class TestCredential : public TokenCredential {
private:
  std::string m_token;

public:
  TestCredential(std::string token = "") : TokenCredential("TestCredential"), m_token(token) {}

  mutable bool WasInvoked = false;

  AccessToken GetToken(TokenRequestContext const&, Context const&) const override
  {
    WasInvoked = true;

    if (m_token.empty())
    {
      throw AuthenticationException("Test Error");
    }

    AccessToken token;
    token.Token = m_token;
    return token;
  }
};
} // namespace

TEST(DefaultAzureCredential, GetCredentialName)
{
  CredentialTestHelper::EnvironmentOverride const env({
      {"AZURE_TENANT_ID", "01234567-89ab-cdef-fedc-ba8976543210"},
      {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
      {"AZURE_CLIENT_SECRET", "CLIENTSECRET"},
      {"AZURE_AUTHORITY_HOST", ""},
      {"AZURE_FEDERATED_TOKEN_FILE", "azure-identity-test.pem"},
      {"AZURE_USERNAME", ""},
      {"AZURE_PASSWORD", ""},
      {"AZURE_CLIENT_CERTIFICATE_PATH", ""},
      {"MSI_ENDPOINT", ""},
      {"MSI_SECRET", ""},
      {"IDENTITY_ENDPOINT", "https://visualstudio.com/"},
      {"IMDS_ENDPOINT", ""},
      {"IDENTITY_HEADER", "CLIENTSECRET"},
      {"IDENTITY_SERVER_THUMBPRINT", ""},
  });

  DefaultAzureCredential const cred;
  EXPECT_EQ(cred.GetCredentialName(), "DefaultAzureCredential");
}

TEST(DefaultAzureCredential, CachingCredential)
{
  auto c1 = std::make_shared<TestCredential>();
  auto c2 = std::make_shared<TestCredential>("Token2");
  DefaultAzureCredential cred;

  cred.m_impl = std::make_unique<Azure::Identity::_detail::ChainedTokenCredentialImpl>(
      "Test DAC", Azure::Identity::ChainedTokenCredential::Sources{c1, c2}, true);

  EXPECT_FALSE(c1->WasInvoked);
  EXPECT_FALSE(c2->WasInvoked);

  auto token = cred.GetToken({}, {});
  EXPECT_EQ(token.Token, "Token2");

  EXPECT_TRUE(c1->WasInvoked);
  EXPECT_TRUE(c2->WasInvoked);

  // We expect default azure credential to cache the selected credential which was successful
  // and only try that one, rather than going through the entire list again.
  c1->WasInvoked = false;
  c1->WasInvoked = false;

  token = cred.GetToken({}, {});
  EXPECT_EQ(token.Token, "Token2");

  EXPECT_FALSE(c1->WasInvoked);
  EXPECT_TRUE(c2->WasInvoked);

  // Only the 2nd credential in the list should get invoked, which is c1, since that's the cached
  // index.
  c1->WasInvoked = false;
  c2->WasInvoked = false;

  cred.m_impl->m_sources = Azure::Identity::ChainedTokenCredential::Sources{c2, c1, c2};

  // We don't expect c2 to ever be used here.
  EXPECT_THROW(static_cast<void>(cred.GetToken({}, {})), AuthenticationException);

  EXPECT_TRUE(c1->WasInvoked);
  EXPECT_FALSE(c2->WasInvoked);

  // Caching is per instance of the DefaultAzureCredential and not global.
  c1->WasInvoked = false;
  c2->WasInvoked = false;

  DefaultAzureCredential cred1;
  cred1.m_impl = std::make_unique<Azure::Identity::_detail::ChainedTokenCredentialImpl>(
      "Test DAC", Azure::Identity::ChainedTokenCredential::Sources{c1, c2}, true);

  DefaultAzureCredential cred2;
  cred2.m_impl = std::make_unique<Azure::Identity::_detail::ChainedTokenCredentialImpl>(
      "Test DAC", Azure::Identity::ChainedTokenCredential::Sources{c2, c1}, true);

  // The first credential in the list, c2, got called and cached on cred2.
  token = cred2.GetToken({}, {});
  EXPECT_EQ(token.Token, "Token2");

  EXPECT_FALSE(c1->WasInvoked);
  EXPECT_TRUE(c2->WasInvoked);

  // cred1 is unaffected by cred2 and both c1 and c2 are called, in order.
  token = cred1.GetToken({}, {});
  EXPECT_EQ(token.Token, "Token2");

  EXPECT_TRUE(c1->WasInvoked);
  EXPECT_TRUE(c2->WasInvoked);
}

TEST(DefaultAzureCredential, LogMessages)
{
  using LogMsgVec = std::vector<std::pair<Logger::Level, std::string>>;
  LogMsgVec log;
  Logger::SetLevel(Logger::Level::Verbose);
  Logger::SetListener([&](auto lvl, auto msg) { log.push_back(std::make_pair(lvl, msg)); });

  CredentialTestHelper::SimulateTokenRequest(
      [&](auto transport) {
        TokenCredentialOptions options;
        options.Transport.Transport = transport;

        CredentialTestHelper::EnvironmentOverride const env({
            {"AZURE_TENANT_ID", "01234567-89ab-cdef-fedc-ba8976543210"},
            {"AZURE_CLIENT_ID", "fedcba98-7654-3210-0123-456789abcdef"},
            {"AZURE_CLIENT_SECRET", "CLIENTSECRET"},
            {"AZURE_AUTHORITY_HOST", "https://microsoft.com/"},
            {"AZURE_FEDERATED_TOKEN_FILE", "azure-identity-test.pem"},
            {"AZURE_USERNAME", ""},
            {"AZURE_PASSWORD", ""},
            {"AZURE_CLIENT_CERTIFICATE_PATH", ""},
            {"MSI_ENDPOINT", ""},
            {"MSI_SECRET", ""},
            {"IDENTITY_ENDPOINT", ""},
            {"IMDS_ENDPOINT", ""},
            {"IDENTITY_HEADER", ""},
            {"IDENTITY_SERVER_THUMBPRINT", ""},
        });

        auto credential = std::make_unique<DefaultAzureCredential>(options);

        EXPECT_EQ(log.size(), LogMsgVec::size_type(11));

        EXPECT_EQ(log[0].first, Logger::Level::Verbose);
        EXPECT_EQ(
            log[0].second,
            "Identity: Creating DefaultAzureCredential which combines "
            "mutiple parameterless credentials into a single one."
            "\nDefaultAzureCredential is only recommended for the early stages of development, "
            "and not for usage in production environment."
            "\nOnce the developer focuses on the Credentials and Authentication aspects of their "
            "application, DefaultAzureCredential needs to be replaced with the credential that "
            "is the better fit for the application.");

        EXPECT_EQ(log[1].first, Logger::Level::Informational);
        EXPECT_EQ(
            log[1].second,
            "Identity: EnvironmentCredential gets created with ClientSecretCredential.");

        EXPECT_EQ(log[2].first, Logger::Level::Verbose);
        EXPECT_EQ(
            log[2].second,
            "Identity: EnvironmentCredential: 'AZURE_TENANT_ID', 'AZURE_CLIENT_ID', "
            "'AZURE_CLIENT_SECRET', and 'AZURE_AUTHORITY_HOST' environment variables are set, so "
            "ClientSecretCredential with corresponding tenantId, clientId, clientSecret, and "
            "authorityHost gets created.");

        EXPECT_EQ(log[3].first, Logger::Level::Informational);
        EXPECT_EQ(log[3].second, "Identity: WorkloadIdentityCredential was created successfully.");

        EXPECT_EQ(log[5].first, Logger::Level::Verbose);
        EXPECT_EQ(
            log[5].second,
            "Identity: ManagedIdentityCredential: Environment is not set up for the credential "
            "to be created with App Service 2019 source.");

        EXPECT_EQ(log[6].first, Logger::Level::Verbose);
        EXPECT_EQ(
            log[6].second,
            "Identity: ManagedIdentityCredential: Environment is not set up for the credential "
            "to be created with App Service 2017 source.");

        EXPECT_EQ(log[7].first, Logger::Level::Verbose);
        EXPECT_EQ(
            log[7].second,
            "Identity: ManagedIdentityCredential: Environment is not set up for the credential "
            "to be created with Cloud Shell source.");

        EXPECT_EQ(log[8].first, Logger::Level::Verbose);
        EXPECT_EQ(
            log[8].second,
            "Identity: ManagedIdentityCredential: Environment is not set up for the credential "
            "to be created with Azure Arc source.");

        EXPECT_EQ(log[9].first, Logger::Level::Informational);
        EXPECT_EQ(
            log[9].second,
            "Identity: ManagedIdentityCredential will be created "
            "with Azure Instance Metadata Service source."
            "\nSuccessful creation does not guarantee further successful token retrieval.");

        EXPECT_EQ(log[4].first, Logger::Level::Informational);
        EXPECT_EQ(
            log[4].second,
            "Identity: AzureCliCredential created."
            "\nSuccessful creation does not guarantee further successful token retrieval.");

        EXPECT_EQ(log[10].first, Logger::Level::Informational);
        EXPECT_EQ(
            log[10].second,
            "Identity: DefaultAzureCredential: Created with the following credentials: "
            "EnvironmentCredential, WorkloadIdentityCredential, AzureCliCredential, "
            "ManagedIdentityCredential.");

        log.clear();

        return credential;
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});

  EXPECT_EQ(
      log.size(),
      LogMsgVec::size_type(5)); // Request and retry policies will get their messages here as well.

  EXPECT_EQ(log[3].first, Logger::Level::Informational);
  EXPECT_EQ(
      log[3].second,
      "Identity: DefaultAzureCredential: Successfully got token from EnvironmentCredential. This "
      "credential will be reused for subsequent calls.");

  EXPECT_EQ(log[4].first, Logger::Level::Verbose);
  EXPECT_EQ(
      log[4].second,
      "Identity: DefaultAzureCredential: Saved this credential at index 0 for subsequent calls.");

  Logger::SetListener(nullptr);
}
