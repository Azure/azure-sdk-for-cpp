// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/default_azure_credential.hpp"

#include "azure/identity/azure_cli_credential.hpp"
#include "azure/identity/environment_credential.hpp"
#include "azure/identity/managed_identity_credential.hpp"
#include "azure/identity/workload_identity_credential.hpp"
#include "private/chained_token_credential_impl.hpp"
#include "private/identity_log.hpp"

#include <azure/core/internal/environment.hpp>
#include <azure/core/internal/strings.hpp>

using namespace Azure::Identity;
using namespace Azure::Core::Credentials;

using Azure::Core::Context;
using Azure::Core::_internal::Environment;
using Azure::Core::_internal::StringExtensions;
using Azure::Core::Diagnostics::Logger;
using Azure::Identity::_detail::IdentityLog;

DefaultAzureCredential::DefaultAzureCredential(
    Core::Credentials::TokenCredentialOptions const& options)
    : TokenCredential("DefaultAzureCredential")
{
  // Initializing m_credential below and not in the member initializer list to have a specific order
  // of log messages.

  IdentityLog::Write(
      IdentityLog::Level::Verbose,
      "Creating " + GetCredentialName()
          + " which combines multiple parameterless credentials into a single one.\n"
          + GetCredentialName()
          + " is only recommended for the early stages of development, "
            "and not for usage in production environment."
            "\nOnce the developer focuses on the Credentials and Authentication aspects "
            "of their application, "
          + GetCredentialName()
          + " needs to be replaced with the credential that "
            "is the better fit for the application.");

  // Creating credentials in order to ensure the order of log messages.
  ChainedTokenCredential::Sources miSources;
  {
    miSources.emplace_back(std::make_shared<EnvironmentCredential>(options));
    miSources.emplace_back(std::make_shared<WorkloadIdentityCredential>(options));

    constexpr auto envVarName = "AZURE_TOKEN_CREDENTIALS";
    const auto envVarValue = Environment::GetVariable(envVarName);

    const auto isProd = StringExtensions::LocaleInvariantCaseInsensitiveEqual(envVarValue, "prod");
    const auto logMsg = GetCredentialName() + ": '" + envVarName + "' environment variable is "
        + (envVarValue.empty() ? "not set" : ("set to '" + envVarValue + "'"))
        + ", therefore AzureCliCredential will " + (isProd ? "NOT " : "")
        + "be included in the credential chain.";

    if (isProd)
    {
      IdentityLog::Write(IdentityLog::Level::Verbose, logMsg);
    }
    else if (
        envVarValue.empty()
        || StringExtensions::LocaleInvariantCaseInsensitiveEqual(envVarValue, "dev"))
    {
      IdentityLog::Write(IdentityLog::Level::Verbose, logMsg);
      miSources.emplace_back(std::make_shared<AzureCliCredential>(options));
    }
    else
    {
      throw AuthenticationException(
          GetCredentialName() + ": Invalid value '" + envVarValue + "' for the '" + envVarName
          + "' environment variable. Allowed values are 'dev' and 'prod' (case insensitive). "
            "It is also allowed to not have the environment variable defined.");
    }

    miSources.emplace_back(std::make_shared<ManagedIdentityCredential>(options));
  }

  // DefaultAzureCredential caches the selected credential, so that it can be reused on subsequent
  // calls.
  m_impl = std::make_unique<_detail::ChainedTokenCredentialImpl>(
      GetCredentialName(), std::move(miSources), true);
}

DefaultAzureCredential::~DefaultAzureCredential() = default;

AccessToken DefaultAzureCredential::GetToken(
    TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
  try
  {
    return m_impl->GetToken(GetCredentialName(), tokenRequestContext, context);
  }
  catch (AuthenticationException const&)
  {
    throw AuthenticationException(
        "Failed to get token from " + GetCredentialName()
        + ".\nSee Azure::Core::Diagnostics::Logger for details "
          "(https://aka.ms/azsdk/cpp/identity/troubleshooting).");
  }
}
