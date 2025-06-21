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

class LogMessages : public ::testing::TestWithParam<std::string> {
};

INSTANTIATE_TEST_SUITE_P(
    DefaultAzureCredential,
    LogMessages,
    ::testing::Values(
        "",
        " ",
        "dev",
        "DeV",
        "dEv ",
        " DEV  ",
        "prod",
        "pRoD",
        " PrOd",
        "d ev",
        "production"));

TEST_P(LogMessages, )
{
  const auto azTokenCredsEnvVarValue = GetParam();
  if (azTokenCredsEnvVarValue == "d ev" || azTokenCredsEnvVarValue == "production")
  {
    CredentialTestHelper::EnvironmentOverride const env(
        {{"AZURE_TOKEN_CREDENTIALS", azTokenCredsEnvVarValue}});

    EXPECT_THROW(
        static_cast<void>(std::make_unique<DefaultAzureCredential>()), AuthenticationException);
  }
  else
  {
    const auto isDev = azTokenCredsEnvVarValue == "dev" || azTokenCredsEnvVarValue == "DeV"
        || azTokenCredsEnvVarValue == "dEv " || azTokenCredsEnvVarValue == " DEV  "
        || azTokenCredsEnvVarValue == "" || azTokenCredsEnvVarValue == " ";

    using LogMsgVec = std::vector<std::pair<Logger::Level, std::string>>;
    LogMsgVec log;
    Logger::SetLevel(Logger::Level::Verbose);
    Logger::SetListener([&](auto lvl, auto msg) { log.push_back(std::make_pair(lvl, msg)); });

    try
    {
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
                {"AZURE_TOKEN_CREDENTIALS", azTokenCredsEnvVarValue},
            });

            auto credential = std::make_unique<DefaultAzureCredential>(options);

            EXPECT_EQ(log.size(), LogMsgVec::size_type(isDev ? 12 : 11));
            LogMsgVec::size_type i = 0;

            EXPECT_EQ(log.at(i).first, Logger::Level::Verbose);
            EXPECT_EQ(
                log.at(i).second,
                "Identity: Creating DefaultAzureCredential which combines "
                "multiple parameterless credentials into a single one."
                "\nDefaultAzureCredential is only recommended for the early stages of development, "
                "and not for usage in production environment."
                "\nOnce the developer focuses on the Credentials "
                "and Authentication aspects of their application, "
                "DefaultAzureCredential needs to be replaced with the credential that "
                "is the better fit for the application.");

            ++i;
            EXPECT_EQ(log.at(i).first, Logger::Level::Informational);
            EXPECT_EQ(
                log.at(i).second,
                "Identity: EnvironmentCredential gets created with ClientSecretCredential.");

            ++i;
            EXPECT_EQ(log.at(i).first, Logger::Level::Verbose);
            EXPECT_EQ(
                log.at(i).second,
                "Identity: EnvironmentCredential: 'AZURE_TENANT_ID', 'AZURE_CLIENT_ID', "
                "'AZURE_CLIENT_SECRET', and 'AZURE_AUTHORITY_HOST' environment variables "
                "are set, so ClientSecretCredential with corresponding "
                "tenantId, clientId, clientSecret, and authorityHost gets created.");

            ++i;
            EXPECT_EQ(log.at(i).first, Logger::Level::Informational);
            EXPECT_EQ(
                log.at(i).second, "Identity: WorkloadIdentityCredential was created successfully.");

            ++i;
            EXPECT_EQ(log.at(i).first, Logger::Level::Verbose);
            EXPECT_EQ(
                log.at(i).second,
                "Identity: ManagedIdentityCredential: Environment is not set up "
                "for the credential to be created with App Service 2019 source.");

            ++i;
            EXPECT_EQ(log.at(i).first, Logger::Level::Verbose);
            EXPECT_EQ(
                log.at(i).second,
                "Identity: ManagedIdentityCredential: Environment is not set up "
                "for the credential to be created with App Service 2017 source.");

            ++i;
            EXPECT_EQ(log.at(i).first, Logger::Level::Verbose);
            EXPECT_EQ(
                log.at(i).second,
                "Identity: ManagedIdentityCredential: Environment is not set up "
                "for the credential to be created with Cloud Shell source.");

            ++i;
            EXPECT_EQ(log.at(i).first, Logger::Level::Verbose);
            EXPECT_EQ(
                log.at(i).second,
                "Identity: ManagedIdentityCredential: Environment is not set up "
                "for the credential to be created with Azure Arc source.");

            ++i;
            EXPECT_EQ(log.at(i).first, Logger::Level::Informational);
            EXPECT_EQ(
                log.at(i).second,
                "Identity: ManagedIdentityCredential will be created "
                "with Azure Instance Metadata Service source."
                "\nSuccessful creation does not guarantee further successful token retrieval.");

            {
              const auto variableSetWording = azTokenCredsEnvVarValue.empty()
                  ? "not set"
                  : ("set to '" + azTokenCredsEnvVarValue + "'");

              const auto beIncludedWording = isDev ? "" : "NOT ";

              ++i;
              EXPECT_EQ(log.at(i).first, Logger::Level::Verbose);
              EXPECT_EQ(
                  log.at(i).second,
                  "Identity: DefaultAzureCredential: "
                  "'AZURE_TOKEN_CREDENTIALS' environment variable is "
                      + variableSetWording + ", therefore AzureCliCredential will "
                      + beIncludedWording + "be included in the credential chain.");
            }

            if (isDev)
            {
              ++i;
              EXPECT_EQ(log.at(i).first, Logger::Level::Informational);
              EXPECT_EQ(
                  log.at(i).second,
                  "Identity: AzureCliCredential created."
                  "\nSuccessful creation does not guarantee further successful token retrieval.");
            }

            ++i;
            EXPECT_EQ(log.at(i).first, Logger::Level::Informational);
            EXPECT_EQ(
                log.at(i).second,
                std::string(
                    "Identity: DefaultAzureCredential: Created with the following credentials: "
                    "EnvironmentCredential, WorkloadIdentityCredential, ManagedIdentityCredential")
                    + (isDev ? ", AzureCliCredential" : "") + ".");

            ++i;
            EXPECT_EQ(i, log.size());

            log.clear();

            return credential;
          },
          {{"https://azure.com/.default"}},
          {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});

      EXPECT_EQ(
          log.size(),
          LogMsgVec::size_type(
              5)); // Request and retry policies will get their messages here as well.

      LogMsgVec::size_type i = 3;
      EXPECT_EQ(log.at(i).first, Logger::Level::Informational);
      EXPECT_EQ(
          log.at(i).second,
          "Identity: DefaultAzureCredential: Successfully got token from EnvironmentCredential. "
          "This credential will be reused for subsequent calls.");

      ++i;
      EXPECT_EQ(log.at(i).first, Logger::Level::Verbose);
      EXPECT_EQ(
          log.at(i).second,
          "Identity: DefaultAzureCredential: "
          "Saved this credential at index 0 for subsequent calls.");

      ++i;
      EXPECT_EQ(i, log.size());
    }
    catch (...)
    {
      Logger::SetListener(nullptr);
      throw;
    }

    Logger::SetListener(nullptr);
  }
}

struct SpecificCredentialInfo
{
  std::string CredentialName;
  std::string EnvVarValue;
  size_t ExpectedLogMsgCount;
};

class LogMessagesForSpecificCredential : public ::testing::TestWithParam<SpecificCredentialInfo> {
};

INSTANTIATE_TEST_SUITE_P(
    DefaultAzureCredential,
    LogMessagesForSpecificCredential,
    ::testing::Values(
        SpecificCredentialInfo{"EnvironmentCredential", "eNvIrOnMeNt", 5},
        SpecificCredentialInfo{"WorkloadIdentityCredential", "workloadidentity", 4},
        SpecificCredentialInfo{"ManagedIdentityCredential", "MANAGEDIDENTITY", 8},
        SpecificCredentialInfo{"AzureCliCredential", "  AzureCLI ", 4}));

TEST_P(LogMessagesForSpecificCredential, )
{
  const auto specificCredentialInfo = GetParam();

  using LogMsgVec = std::vector<std::pair<Logger::Level, std::string>>;
  LogMsgVec log;
  Logger::SetLevel(Logger::Level::Verbose);
  Logger::SetListener([&](auto lvl, auto msg) { log.push_back(std::make_pair(lvl, msg)); });

  try
  {
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
        {"AZURE_TOKEN_CREDENTIALS", specificCredentialInfo.EnvVarValue},
    });

    static_cast<void>(std::make_unique<DefaultAzureCredential>());

    EXPECT_EQ(log.size(), specificCredentialInfo.ExpectedLogMsgCount);
    LogMsgVec::size_type i = 0;

    EXPECT_EQ(log.at(i).first, Logger::Level::Verbose);
    EXPECT_EQ(
        log.at(i).second,
        "Identity: Creating DefaultAzureCredential which combines "
        "multiple parameterless credentials into a single one."
        "\nDefaultAzureCredential is only recommended for the early stages of development, "
        "and not for usage in production environment."
        "\nOnce the developer focuses on the Credentials "
        "and Authentication aspects of their application, "
        "DefaultAzureCredential needs to be replaced with the credential that "
        "is the better fit for the application.");

    ++i;
    EXPECT_EQ(log.at(i).first, Logger::Level::Verbose);
    EXPECT_EQ(
        log.at(i).second,
        "Identity: DefaultAzureCredential: "
        "'AZURE_TOKEN_CREDENTIALS' environment variable is set to '"
            + specificCredentialInfo.EnvVarValue
            + "', therefore credential chain will only contain single credential: "
            + specificCredentialInfo.CredentialName + '.');

    if (specificCredentialInfo.CredentialName == "EnvironmentCredential")
    {
      ++i;
      EXPECT_EQ(log.at(i).first, Logger::Level::Informational);
      EXPECT_EQ(
          log.at(i).second,
          "Identity: EnvironmentCredential gets created with ClientSecretCredential.");

      ++i;
      EXPECT_EQ(log.at(i).first, Logger::Level::Verbose);
      EXPECT_EQ(
          log.at(i).second,
          "Identity: EnvironmentCredential: 'AZURE_TENANT_ID', 'AZURE_CLIENT_ID', "
          "'AZURE_CLIENT_SECRET', and 'AZURE_AUTHORITY_HOST' environment variables "
          "are set, so ClientSecretCredential with corresponding "
          "tenantId, clientId, clientSecret, and authorityHost gets created.");
    }
    else if (specificCredentialInfo.CredentialName == "WorkloadIdentityCredential")
    {
      ++i;
      EXPECT_EQ(log.at(i).first, Logger::Level::Informational);
      EXPECT_EQ(log.at(i).second, "Identity: WorkloadIdentityCredential was created successfully.");
    }
    else if (specificCredentialInfo.CredentialName == "ManagedIdentityCredential")
    {
      ++i;
      EXPECT_EQ(log.at(i).first, Logger::Level::Verbose);
      EXPECT_EQ(
          log.at(i).second,
          "Identity: ManagedIdentityCredential: Environment is not set up "
          "for the credential to be created with App Service 2019 source.");

      ++i;
      EXPECT_EQ(log.at(i).first, Logger::Level::Verbose);
      EXPECT_EQ(
          log.at(i).second,
          "Identity: ManagedIdentityCredential: Environment is not set up "
          "for the credential to be created with App Service 2017 source.");

      ++i;
      EXPECT_EQ(log.at(i).first, Logger::Level::Verbose);
      EXPECT_EQ(
          log.at(i).second,
          "Identity: ManagedIdentityCredential: Environment is not set up "
          "for the credential to be created with Cloud Shell source.");

      ++i;
      EXPECT_EQ(log.at(i).first, Logger::Level::Verbose);
      EXPECT_EQ(
          log.at(i).second,
          "Identity: ManagedIdentityCredential: Environment is not set up "
          "for the credential to be created with Azure Arc source.");

      ++i;
      EXPECT_EQ(log.at(i).first, Logger::Level::Informational);
      EXPECT_EQ(
          log.at(i).second,
          "Identity: ManagedIdentityCredential will be created "
          "with Azure Instance Metadata Service source."
          "\nSuccessful creation does not guarantee further successful token retrieval.");
    }
    else if (specificCredentialInfo.CredentialName == "AzureCliCredential")
    {
      ++i;
      EXPECT_EQ(log.at(i).first, Logger::Level::Informational);
      EXPECT_EQ(
          log.at(i).second,
          "Identity: AzureCliCredential created."
          "\nSuccessful creation does not guarantee further successful token retrieval.");
    }

    ++i;
    EXPECT_EQ(log.at(i).first, Logger::Level::Informational);
    EXPECT_EQ(
        log.at(i).second,
        std::string(
            "Identity: DefaultAzureCredential: Created with the following credentials: "
            + specificCredentialInfo.CredentialName + '.'));

    ++i;
    EXPECT_EQ(i, log.size());

    log.clear();
  }
  catch (...)
  {
    Logger::SetListener(nullptr);
    throw;
  }

  Logger::SetListener(nullptr);
}
