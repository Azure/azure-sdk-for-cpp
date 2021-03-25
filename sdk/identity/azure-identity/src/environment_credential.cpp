// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/environment_credential.hpp"
#include "azure/identity/client_secret_credential.hpp"

#include <azure/core/internal/environment.hpp>
#include <azure/core/platform.hpp>

#include <cstdlib>
#include <utility>

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <windows.h>
#endif

using namespace Azure::Identity;

EnvironmentCredential::EnvironmentCredential(EnvironmentCredentialOptions options)
    : m_options(std::move(options))
{
#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.
  auto tenantId = Core::_internal::Environment::Get("AZURE_TENANT_ID");
  auto clientId = Core::_internal::Environment::Get("AZURE_CLIENT_ID");

  auto clientSecret = Core::_internal::Environment::Get("AZURE_CLIENT_SECRET");
  auto authority = Core::_internal::Environment::Get("AZURE_AUTHORITY_HOST");

  // auto username = Core::_internal::Environment::Get("AZURE_USERNAME");
  // auto password = Core::_internal::Environment::Get("AZURE_PASSWORD");
  //
  // auto clientCertificatePath =
  // Core::_internal::Environment::Get("AZURE_CLIENT_CERTIFICATE_PATH");

  if (tenantId != nullptr && clientId != nullptr)
  {
    if (clientSecret != nullptr)
    {
      ClientSecretCredentialOptions clientSecretCredentialOptions;
      static_cast<Core::_internal::ClientOptions&>(clientSecretCredentialOptions) = m_options;
      if (authority != nullptr)
      {
        clientSecretCredentialOptions.AuthorityHost = authority;
      }

      m_credentialImpl.reset(new ClientSecretCredential(
          tenantId, clientId, clientSecret, clientSecretCredentialOptions));
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
#endif
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
