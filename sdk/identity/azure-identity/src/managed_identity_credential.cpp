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
      = {AppServiceV2019ManagedIdentitySource::Create,
         AppServiceV2017ManagedIdentitySource::Create,
         CloudShellManagedIdentitySource::Create,
         AzureArcManagedIdentitySource::Create,
         ImdsManagedIdentitySource::Create};

  // IMDS ManagedIdentity, which comes last in the list, will never return nullptr from Create().
  // For that reason, it is not possible to cover that execution branch in tests.
  for (auto create : managedIdentitySourceCreate) // LCOV_EXCL_LINE
  {
    if (auto source = create(clientId, options))
    {
      return source;
    }
  }

  // LCOV_EXCL_START
  throw AuthenticationException(
      "ManagedIdentityCredential authentication unavailable. No Managed Identity endpoint found.");
  // LCOV_EXCL_STOP
}
} // namespace

ManagedIdentityCredential::~ManagedIdentityCredential() = default;

ManagedIdentityCredential::ManagedIdentityCredential(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : m_managedIdentitySource(CreateManagedIdentitySource(clientId, options))
{
}

ManagedIdentityCredential::ManagedIdentityCredential(
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : ManagedIdentityCredential(std::string(), options)
{
}

Azure::Core::Credentials::AccessToken ManagedIdentityCredential::GetToken(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context) const
{
  return m_managedIdentitySource->GetToken(tokenRequestContext, context);
}