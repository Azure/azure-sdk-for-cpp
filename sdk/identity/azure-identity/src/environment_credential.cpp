// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT


#include "azure/identity/client_secret_credential.hpp"
#include "azure/identity/environment_credential.hpp"

#include <cstdlib>

using namespace Azure::Identity;

EnvironmentCredential::EnvironmentCredential()
{
#ifdef _MSC_VER
#pragma warning(push)
// warning C4996: 'getenv': This function or variable may be unsafe. Consider using _dupenv_s
// instead.
#pragma warning(disable : 4996)
#endif

  auto tenantId = std::getenv("AZURE_TENANT_ID");
  auto clientId = std::getenv("AZURE_CLIENT_ID");

  auto clientSecret = std::getenv("AZURE_CLIENT_SECRET");
  auto authority = std::getenv("AZURE_AUTHORITY_HOST");

  // auto username = std::getenv("AZURE_USERNAME");
  // auto password = std::getenv("AZURE_PASSWORD");
  //
  // auto clientCertificatePath = std::getenv("AZURE_CLIENT_CERTIFICATE_PATH");

#ifdef _MSC_VER
#pragma warning(pop)
#endif

  if (tenantId != nullptr && clientId != nullptr)
  {
    if (clientSecret != nullptr)
    {
      if (authority != nullptr)
      {
        m_credentialImpl.reset(
            new ClientSecretCredential(tenantId, clientId, clientSecret, authority));
      }
      else
      {
        m_credentialImpl.reset(new ClientSecretCredential(tenantId, clientId, clientSecret));
      }
    }
    // TODO: These credential types are not implemented. Uncomment when implemented.
    // else if (username != nullptr && password != nullptr)
    //{
    //  m_credentialImpl.reset(
    //      new UsernamePasswordCredential(username, password, tenantId, clientId));
    //}
    // else if (clientCertificatePath != nullptr)
    //{
    //  m_credentialImpl.reset(
    //      new ClientCertificateCredential(tenantId, clientId, clientCertificatePath));
    //}
  }
}

Azure::Core::AccessToken EnvironmentCredential::GetToken(
    Azure::Core::Context const& context,
    std::vector<std::string> const& scopes) const
{
  using namespace Azure::Core;

  if (!m_credentialImpl)
  {
    throw AuthenticationException("EnvironmentCredential authentication unavailable. "
                                  "Environment variables are not fully configured.");
  }

  return m_credentialImpl->GetToken(context, scopes);
}
