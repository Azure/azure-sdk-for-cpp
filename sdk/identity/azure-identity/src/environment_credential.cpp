// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/environment_credential.hpp"
#include "azure/identity/client_certificate_credential.hpp"
#include "azure/identity/client_secret_credential.hpp"

#include <azure/core/azure_assert.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/internal/environment.hpp>

#include <utility>
#include <vector>

using Azure::Identity::EnvironmentCredential;

using Azure::Core::Context;
using Azure::Core::_internal::Environment;
using Azure::Core::Credentials::AccessToken;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Diagnostics::Logger;
using Azure::Core::Diagnostics::_internal::Log;

namespace {
auto const AzureTenantIdEnvVarName = "AZURE_TENANT_ID";
auto const AzureClientIdEnvVarName = "AZURE_CLIENT_ID";
auto const AzureClientSecretEnvVarName = "AZURE_CLIENT_SECRET";
auto const AzureAuthorityHostEnvVarName = "AZURE_AUTHORITY_HOST";
auto const AzureClientCertificatePathEnvVarName = "AZURE_CLIENT_CERTIFICATE_PATH";

std::string const LogMsgPrefix = "Identity: EnvironmentCredential";

void PrintCredentialCreationLogMessage(
    std::vector<std::pair<char const*, char const*>> const& envVarsToParams,
    char const* credThatGetsCreated);
} // namespace

EnvironmentCredential::EnvironmentCredential(TokenCredentialOptions options)
{
  auto tenantId = Environment::GetVariable(AzureTenantIdEnvVarName);
  auto clientId = Environment::GetVariable(AzureClientIdEnvVarName);

  auto clientSecret = Environment::GetVariable(AzureClientSecretEnvVarName);
  auto authority = Environment::GetVariable(AzureAuthorityHostEnvVarName);

  auto clientCertificatePath = Environment::GetVariable(AzureClientCertificatePathEnvVarName);

  if (!tenantId.empty() && !clientId.empty())
  {
    if (!clientSecret.empty())
    {
      if (!authority.empty())
      {
        PrintCredentialCreationLogMessage(
            {
                {AzureTenantIdEnvVarName, "tenantId"},
                {AzureClientIdEnvVarName, "clientId"},
                {AzureClientSecretEnvVarName, "clientSecret"},
                {AzureAuthorityHostEnvVarName, "authorityHost"},
            },
            "ClientSecretCredential");

        ClientSecretCredentialOptions clientSecretCredentialOptions;
        static_cast<TokenCredentialOptions&>(clientSecretCredentialOptions) = options;
        clientSecretCredentialOptions.AuthorityHost = authority;

        m_credentialImpl.reset(new ClientSecretCredential(
            tenantId, clientId, clientSecret, clientSecretCredentialOptions));
      }
      else
      {
        PrintCredentialCreationLogMessage(
            {
                {AzureTenantIdEnvVarName, "tenantId"},
                {AzureClientIdEnvVarName, "clientId"},
                {AzureClientSecretEnvVarName, "clientSecret"},
            },
            "ClientSecretCredential");

        m_credentialImpl.reset(
            new ClientSecretCredential(tenantId, clientId, clientSecret, options));
      }
    }
    else if (!clientCertificatePath.empty())
    {
      if (!authority.empty())
      {
        PrintCredentialCreationLogMessage(
            {
                {AzureTenantIdEnvVarName, "tenantId"},
                {AzureClientIdEnvVarName, "clientId"},
                {AzureClientCertificatePathEnvVarName, "clientCertificatePath"},
                {AzureAuthorityHostEnvVarName, "authorityHost"},
            },
            "ClientCertificateCredential");

        ClientCertificateCredentialOptions clientCertificateCredentialOptions;
        static_cast<TokenCredentialOptions&>(clientCertificateCredentialOptions) = options;
        clientCertificateCredentialOptions.AuthorityHost = authority;

        m_credentialImpl.reset(new ClientCertificateCredential(
            tenantId, clientId, clientCertificatePath, clientCertificateCredentialOptions));
      }
      else
      {
        PrintCredentialCreationLogMessage(
            {
                {AzureTenantIdEnvVarName, "tenantId"},
                {AzureClientIdEnvVarName, "clientId"},
                {AzureClientCertificatePathEnvVarName, "clientCertificatePath"},
            },
            "ClientCertificateCredential");

        m_credentialImpl.reset(
            new ClientCertificateCredential(tenantId, clientId, clientCertificatePath, options));
      }
    }
  }

  if (!m_credentialImpl)
  {
    auto const logLevel = Logger::Level::Warning;
    if (Log::ShouldWrite(logLevel))
    {
      std::string const basicMessage
          = LogMsgPrefix + " was not initialized with underlying credential";

      if (!Log::ShouldWrite(Logger::Level::Verbose))
      {
        Log::Write(logLevel, basicMessage + '.');
      }
      else
      {
        std::string logMsg = basicMessage + ": Both '" + AzureTenantIdEnvVarName + "' and '"
            + AzureClientIdEnvVarName + "', and at least one of '" + AzureClientSecretEnvVarName
            + "', '" + AzureClientCertificatePathEnvVarName + "' needs to be set. Additionally, '"
            + AzureAuthorityHostEnvVarName
            + "' could be set to override the default authority host. Currently:\n";

        std::pair<char const*, bool> envVarStatus[] = {
            {AzureTenantIdEnvVarName, !tenantId.empty()},
            {AzureClientIdEnvVarName, !clientId.empty()},
            {AzureClientSecretEnvVarName, !clientSecret.empty()},
            {AzureClientCertificatePathEnvVarName, !clientCertificatePath.empty()},
            {AzureAuthorityHostEnvVarName, !authority.empty()},
        };
        for (auto const status : envVarStatus)
        {
          logMsg += std::string(" * '") + status.first + "' " + "is"
              + (status.second ? " " : " NOT ") + "set\n";
        }

        Log::Write(logLevel, logMsg);
      }
    }
  }
}

AccessToken EnvironmentCredential::GetToken(
    TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
  if (!m_credentialImpl)
  {
    std::string const AuthUnavailable = LogMsgPrefix + " authentication unavailable. ";

    {
      auto const logLevel = Logger::Level::Warning;
      if (Log::ShouldWrite(logLevel))
      {
        Log::Write(
            logLevel,
            AuthUnavailable + "See earlier EnvironmentCredential log messages for details.");
      }
    }

    throw AuthenticationException(
        AuthUnavailable + "Environment variables are not fully configured.");
  }

  return m_credentialImpl->GetToken(tokenRequestContext, context);
}

namespace {
void PrintCredentialCreationLogMessage(
    std::vector<std::pair<char const*, char const*>> const& envVarsToParams,
    char const* credThatGetsCreated)
{
  if (!Log::ShouldWrite(Logger::Level::Verbose))
  {
    if (Log::ShouldWrite(Logger::Level::Informational))
    {
      Log::Write(
          Logger::Level::Informational,
          LogMsgPrefix + " gets created with " + credThatGetsCreated + '.');
    }

    return;
  }

  auto const envVarsToParamsSize = envVarsToParams.size();

  // LCOV_EXCL_START
  AZURE_ASSERT(envVarsToParamsSize > 1);
  // LCOV_EXCL_STOP

  std::string const Tick = "'";
  std::string const Comma = ", ";
  std::string const TickComma = Tick + Comma;
  std::string const And = "and ";

  std::string envVars;
  std::string credParams;
  for (auto i = 0; i < envVarsToParamsSize - 1;
       ++i) // not iterating over the last element for ", and".
  {
    envVars += Tick + envVarsToParams[i].first + TickComma;
    credParams += envVarsToParams[i].second + Comma;
  }

  envVars += And + Tick + envVarsToParams.back().first + Tick;
  credParams += And + envVarsToParams.back().second;

  Log::Write(
      Logger::Level::Verbose,
      LogMsgPrefix + ": " + envVars + " environment variables are set, so " + credThatGetsCreated
          + " with corresponding " + credParams + " gets created.");
}
} // namespace
