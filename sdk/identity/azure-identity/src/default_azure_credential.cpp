// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/default_azure_credential.hpp"

#include "azure/identity/azure_cli_credential.hpp"
#include "azure/identity/environment_credential.hpp"
#include "azure/identity/managed_identity_credential.hpp"

#include "azure/core/internal/diagnostics/log.hpp"

using namespace Azure::Identity;
using namespace Azure::Core::Credentials;

using Azure::Core::Context;
using Azure::Core::Diagnostics::Logger;
using Azure::Core::Diagnostics::_internal::Log;

namespace {
constexpr auto IdentityPrefix = "Identity: ";
} // namespace

DefaultAzureCredential::DefaultAzureCredential(TokenCredentialOptions const& options)
    : TokenCredential("DefaultAzureCredential")
{
  // Initializing m_credential below and not in the member initializer list to have a specific order
  // of log messages.
  auto const logLevel = Logger::Level::Verbose;
  if (Log::ShouldWrite(logLevel))
  {
    Log::Write(
        logLevel,
        std::string(IdentityPrefix) + "Creating " + GetCredentialName()
            + " which combines mutiple parameterless credentials "
              "into a single one (by using ChainedTokenCredential).\n"
            + GetCredentialName()
            + " is only recommended for the early stages of development, "
              "and not for usage in production environment."
              "\nOnce the developer focuses on the Credentials and Authentication aspects "
              "of their application, "
            + GetCredentialName()
            + " needs to be replaced with the credential that "
              "is the better fit for the application.");
  }

  // Creating credentials in order to ensure the order of log messages.
  auto const envCred = std::make_shared<EnvironmentCredential>(options);
  auto const azCliCred = std::make_shared<AzureCliCredential>(options);
  auto const managedIdentityCred = std::make_shared<ManagedIdentityCredential>(options);

  // Using the ChainedTokenCredential's private constructor for more detailed log messages.
  m_credentials.reset(new ChainedTokenCredential(
      ChainedTokenCredential::Sources{envCred, azCliCred, managedIdentityCred},
      GetCredentialName())); // extra arg for the ChainedTokenCredential's private constructor.
}

DefaultAzureCredential::~DefaultAzureCredential() = default;

AccessToken DefaultAzureCredential::GetToken(
    TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
  try
  {
    return m_credentials->GetToken(tokenRequestContext, context);
  }
  catch (AuthenticationException const&)
  {
    throw AuthenticationException(
        "Failed to get token from " + GetCredentialName()
        + ".\nSee Azure::Core::Diagnostics::Logger for details "
          "(https://aka.ms/azsdkcpp/identity/troubleshooting).");
  }
}
