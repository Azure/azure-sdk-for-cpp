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

#include <array>
#include <functional>
#include <type_traits>

using namespace Azure::Identity;
using namespace Azure::Core::Credentials;

using Azure::Core::Context;
using Azure::Core::_internal::Environment;
using Azure::Core::_internal::StringExtensions;
using Azure::Core::Diagnostics::Logger;
using Azure::Identity::_detail::IdentityLog;

namespace {
constexpr auto CredentialSpecifierEnvVarName = "AZURE_TOKEN_CREDENTIALS";
} // namespace

DefaultAzureCredential::DefaultAzureCredential()
    : DefaultAzureCredential(Core::Credentials::TokenCredentialOptions{})
{
}

DefaultAzureCredential::DefaultAzureCredential(
    Core::Credentials::TokenCredentialOptions const& options)
    : DefaultAzureCredential(false, options)
{
}

DefaultAzureCredential::DefaultAzureCredential(
    bool requireCredentialSpecifierEnvVarValue,
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
  ChainedTokenCredential::Sources credentialChain;
  {
    struct CredentialInfo
    {
      bool IsProd;
      std::string CredentialName;
      std::function<std::shared_ptr<Core::Credentials::TokenCredential>(
          const Core::Credentials::TokenCredentialOptions&)>
          Create;
    };

    const auto envVarValue = Environment::GetVariable(CredentialSpecifierEnvVarName);
    const auto trimmedEnvVarValue = StringExtensions::Trim(envVarValue);

    if (requireCredentialSpecifierEnvVarValue && trimmedEnvVarValue.empty())
    {
      throw AuthenticationException(
          GetCredentialName() + ": '" + CredentialSpecifierEnvVarName
          + "' environment variable is empty.");
    }

    bool specificCred = false;
    const std::array<CredentialInfo, 4> credentials = {
        CredentialInfo{
            true,
            "EnvironmentCredential",
            [](auto options) { return std::make_shared<EnvironmentCredential>(options); }},
        CredentialInfo{
            true,
            "WorkloadIdentityCredential",
            [](auto options) { return std::make_shared<WorkloadIdentityCredential>(options); }},
        CredentialInfo{
            true,
            "ManagedIdentityCredential",
            [&specificCred](auto options) {
              return std::shared_ptr<ManagedIdentityCredential>(
                  new ManagedIdentityCredential({}, !specificCred, options));
            }},
        CredentialInfo{
            false,
            "AzureCliCredential",
            [](auto options) { return std::make_shared<AzureCliCredential>(options); }},
    };

    if (!trimmedEnvVarValue.empty())
    {
      for (const auto& cred : credentials)
      {
        if (StringExtensions::LocaleInvariantCaseInsensitiveEqual(
                trimmedEnvVarValue, cred.CredentialName))
        {
          specificCred = true;
          IdentityLog::Write(
              IdentityLog::Level::Verbose,
              GetCredentialName() + ": '" + CredentialSpecifierEnvVarName
                  + "' environment variable is set to '" + envVarValue
                  + "', therefore credential chain will only contain single credential: "
                  + cred.CredentialName + '.');
          credentialChain.emplace_back(cred.Create(options));
          break;
        }
      }
    }

    if (!specificCred)
    {
      for (const auto& cred : credentials)
      {
        if (cred.IsProd)
        {
          credentialChain.emplace_back(cred.Create(options));
        }
      }

      const auto isProd
          = StringExtensions::LocaleInvariantCaseInsensitiveEqual(trimmedEnvVarValue, "prod");

      static const auto devCredCount = std::count_if(
          credentials.begin(), credentials.end(), [](auto& cred) { return !cred.IsProd; });

      std::string devCredNames;
      {
        std::remove_const<decltype(devCredCount)>::type devCredNum = 0;
        for (const auto& cred : credentials)
        {
          if (!cred.IsProd)
          {
            if (!devCredNames.empty())
            {
              if (devCredCount == 2)
              {
                devCredNames += " and ";
              }
              else
              {
                ++devCredNum;
                devCredNames += (devCredNum < devCredCount) ? ", " : ", and ";
              }
            }

            devCredNames += cred.CredentialName;
          }
        }
      }

      const auto logMsg = GetCredentialName() + ": '" + CredentialSpecifierEnvVarName
          + "' environment variable is "
          + (envVarValue.empty() ? "not set" : ("set to '" + envVarValue + "'"))
          + ((devCredCount > 0) ? (", therefore " + devCredNames + " will " + (isProd ? "NOT " : "")
                                   + "be included in the credential chain.")
                                : ".");

      if (isProd)
      {
        IdentityLog::Write(IdentityLog::Level::Verbose, logMsg);
      }
      else if (
          trimmedEnvVarValue.empty()
          || StringExtensions::LocaleInvariantCaseInsensitiveEqual(trimmedEnvVarValue, "dev"))
      {
        IdentityLog::Write(IdentityLog::Level::Verbose, logMsg);
        for (const auto& cred : credentials)
        {
          if (!cred.IsProd)
          {
            credentialChain.emplace_back(cred.Create(options));
          }
        }
      }
      else
      {
        std::string allowedCredNames;
        for (std::size_t i = 0; i < credentials.size(); ++i)
        {
          allowedCredNames += ((i < credentials.size() - 1) ? ", '" : ", and '")
              + credentials[i].CredentialName + '\'';
        }

        throw AuthenticationException(
            GetCredentialName() + ": Invalid value '" + envVarValue + "' for the '"
            + CredentialSpecifierEnvVarName
            + "' environment variable. Allowed values are 'dev', 'prod'" + allowedCredNames
            + " (case insensitive)."
            + (requireCredentialSpecifierEnvVarValue
                   ? ""
                   : " It is also valid to not have the environment variable defined."));
      }
    }
  }

  // DefaultAzureCredential caches the selected credential, so that it can be reused on subsequent
  // calls.
  m_impl = std::make_unique<_detail::ChainedTokenCredentialImpl>(
      GetCredentialName(), std::move(credentialChain), true);
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
