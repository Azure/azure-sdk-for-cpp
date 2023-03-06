// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/default_azure_credential.hpp"

#include "credential_test_helper.hpp"
#include <azure/core/diagnostics/logger.hpp>

#include <gtest/gtest.h>

using Azure::Identity::DefaultAzureCredential;

using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Diagnostics::Logger;
using Azure::Identity::Test::_detail::CredentialTestHelper;

namespace {
} // namespace

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

        EXPECT_EQ(log.size(), LogMsgVec::size_type(9));

        EXPECT_EQ(log[0].first, Logger::Level::Verbose);
        EXPECT_EQ(
            log[0].second,
            "Identity: Creating DefaultAzureCredential which combines mutiple parameterless "
            "credentials "
            "into a single one (by using ChainedTokenCredential)."
            "\nDefaultAzureCredential is only recommended for the early stages of development, "
            "and not for usage in production environment."
            "\nOnce the developer focuses on the Credentials and Authentication aspects of their "
            "application, DefaultAzureCredential needs to be replaced with the credential that "
            "is the better fit for the application.");

        EXPECT_EQ(log[1].first, Logger::Level::Verbose);
        EXPECT_EQ(
            log[1].second,
            "Identity: EnvironmentCredential: 'AZURE_TENANT_ID', 'AZURE_CLIENT_ID', "
            "'AZURE_CLIENT_SECRET', and 'AZURE_AUTHORITY_HOST' environment variables are set, so "
            "ClientSecretCredential with corresponding tenantId, clientId, clientSecret, and "
            "authorityHost gets created.");

        EXPECT_EQ(log[2].first, Logger::Level::Informational);
        EXPECT_EQ(
            log[2].second,
            "Identity: AzureCliCredential created."
            "\nSuccessful creation does not guarantee further successful token retrieval.");

        EXPECT_EQ(log[3].first, Logger::Level::Verbose);
        EXPECT_EQ(
            log[3].second,
            "Identity: ManagedIdentityCredential: Environment is not set up for the credential "
            "to be created with App Service 2019 source.");

        EXPECT_EQ(log[4].first, Logger::Level::Verbose);
        EXPECT_EQ(
            log[4].second,
            "Identity: ManagedIdentityCredential: Environment is not set up for the credential "
            "to be created with App Service 2017 source.");

        EXPECT_EQ(log[5].first, Logger::Level::Verbose);
        EXPECT_EQ(
            log[5].second,
            "Identity: ManagedIdentityCredential: Environment is not set up for the credential "
            "to be created with Cloud Shell source.");

        EXPECT_EQ(log[6].first, Logger::Level::Verbose);
        EXPECT_EQ(
            log[6].second,
            "Identity: ManagedIdentityCredential: Environment is not set up for the credential "
            "to be created with Azure Arc source.");

        EXPECT_EQ(log[7].first, Logger::Level::Informational);
        EXPECT_EQ(
            log[7].second,
            "Identity: ManagedIdentityCredential will be created "
            "with Azure Instance Metadata Service source."
            "\nSuccessful creation does not guarantee further successful token retrieval.");

        EXPECT_EQ(log[8].first, Logger::Level::Informational);
        EXPECT_EQ(
            log[8].second,
            "Identity: DefaultAzureCredential: Created ChainedTokenCredential "
            "with the following credentials: "
            "EnvironmentCredential, AzureCliCredential, ManagedIdentityCredential.");

        log.clear();

        return credential;
      },
      {{"https://azure.com/.default"}},
      {"{\"expires_in\":3600, \"access_token\":\"ACCESSTOKEN1\"}"});

  EXPECT_EQ(
      log.size(),
      LogMsgVec::size_type(4)); // Request and retry policies will get their messages here as well.

  EXPECT_EQ(log[3].first, Logger::Level::Informational);
  EXPECT_EQ(
      log[3].second,
      "Identity: DefaultAzureCredential -> ChainedTokenCredential: "
      "Successfully got token from EnvironmentCredential.");

  Logger::SetListener(nullptr);
}
