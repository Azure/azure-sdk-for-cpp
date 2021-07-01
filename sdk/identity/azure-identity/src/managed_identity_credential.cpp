// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/managed_identity_credential.hpp"
#include "private/managed_identity_source.hpp"

using namespace Azure::Identity;

namespace {
std::unique_ptr<_detail::ManagedIdentitySource> CreateManagedIdentitySource(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
{
  using namespace Azure::Core::Credentials;
  using namespace Azure::Identity::_detail;
  static std::unique_ptr<ManagedIdentitySource> (*managedIdentitySourceCreate[])(
      std::string const& clientId, TokenCredentialOptions const& options)
      = {AppServiceManagedIdentitySource::Create,
         CloudShellManagedIdentitySource::Create,
         AzureArcManagedIdentitySource::Create,
         ImdsManagedIdentitySource::Create};

  for (auto create : managedIdentitySourceCreate)
  {
    if (auto source = create(clientId, options))
    {
      return source;
    }
  }

  throw AuthenticationException(
      "ManagedIdentityCredential authentication unavailable. No Managed Identity endpoint found.");
}
} // namespace

ManagedIdentityCredential::~ManagedIdentityCredential() = default;

ManagedIdentityCredential::ManagedIdentityCredential(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : m_managedIdentitySource(CreateManagedIdentitySource(clientId, options))
{
}

Azure::Core::Credentials::AccessToken ManagedIdentityCredential::GetToken(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context) const
{
  return m_managedIdentitySource->GetToken(tokenRequestContext, context);
}
