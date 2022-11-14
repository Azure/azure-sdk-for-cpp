//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/environment_credential.hpp"
#include "azure/identity/client_certificate_credential.hpp"
#include "azure/identity/client_secret_credential.hpp"

#include <azure/core/internal/environment.hpp>

using namespace Azure::Identity;

EnvironmentCredential::EnvironmentCredential(
    Azure::Core::Credentials::TokenCredentialOptions options)
{
  using Azure::Core::_internal::Environment;

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
        using namespace Azure::Core::Credentials;
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
      m_credentialImpl.reset(
          new ClientCertificateCredential(tenantId, clientId, clientCertificatePath, options));
    }
  }
}

Azure::Core::Credentials::AccessToken EnvironmentCredential::GetToken(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context) const
{
  using namespace Azure::Core::Credentials;

  if (!m_credentialImpl)
  {
    throw AuthenticationException("EnvironmentCredential authentication unavailable. "
                                  "Environment variables are not fully configured.");
  }

  return m_credentialImpl->GetToken(tokenRequestContext, context);
}
