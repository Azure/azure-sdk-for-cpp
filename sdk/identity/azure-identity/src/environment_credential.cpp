// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/environment_credential.hpp"
#include "azure/identity/client_certificate_credential.hpp"
#include "azure/identity/client_secret_credential.hpp"

#include <azure/core/internal/environment.hpp>

using Azure::Identity::EnvironmentCredential;

using Azure::Core::Context;
using Azure::Core::_internal::Environment;
using Azure::Core::Credentials::AccessToken;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Credentials::TokenRequestContext;

EnvironmentCredential::EnvironmentCredential(TokenCredentialOptions options)
{
  auto tenantId = Environment::GetVariable("AZURE_TENANT_ID");
  auto clientId = Environment::GetVariable("AZURE_CLIENT_ID");

  auto clientSecret = Environment::GetVariable("AZURE_CLIENT_SECRET");
  auto authority = Environment::GetVariable("AZURE_AUTHORITY_HOST");

  // auto username = Environment::GetVariable("AZURE_USERNAME");
  // auto password = Environment::GetVariable("AZURE_PASSWORD");

  auto clientCertificatePath = Environment::GetVariable("AZURE_CLIENT_CERTIFICATE_PATH");

  if (!tenantId.empty() && !clientId.empty())
  {
    if (!clientSecret.empty())
    {
      if (!authority.empty())
      {
        ClientSecretCredentialOptions clientSecretCredentialOptions;
        static_cast<TokenCredentialOptions&>(clientSecretCredentialOptions) = options;
        clientSecretCredentialOptions.AuthorityHost = authority;

        m_credentialImpl.reset(new ClientSecretCredential(
            tenantId, clientId, clientSecret, clientSecretCredentialOptions));
      }
      else
      {
        m_credentialImpl.reset(
            new ClientSecretCredential(tenantId, clientId, clientSecret, options));
      }
    }
    // TODO: UsernamePasswordCredential is not implemented. Uncomment when implemented.
    // else if (!username.empty() && !password.empty())
    // {
    //   m_credentialImpl.reset(
    //       new UsernamePasswordCredential(tenantId, clientId, username, password, options));
    // }
    else if (!clientCertificatePath.empty())
    {
      if (!authority.empty())
      {
        ClientCertificateCredentialOptions clientCertificateCredentialOptions;
        static_cast<TokenCredentialOptions&>(clientCertificateCredentialOptions) = options;
        clientCertificateCredentialOptions.AuthorityHost = authority;

        m_credentialImpl.reset(new ClientCertificateCredential(
            tenantId, clientId, clientCertificatePath, clientCertificateCredentialOptions));
      }
      else
      {
        m_credentialImpl.reset(
            new ClientCertificateCredential(tenantId, clientId, clientCertificatePath, options));
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
    throw AuthenticationException("EnvironmentCredential authentication unavailable. "
                                  "Environment variables are not fully configured.");
  }

  return m_credentialImpl->GetToken(tokenRequestContext, context);
}
