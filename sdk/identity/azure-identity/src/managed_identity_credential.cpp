// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/managed_identity_credential.hpp"

#include "private/managed_identity_source.hpp"

using namespace Azure::Identity;

namespace {
std::unique_ptr<_detail::ManagedIdentitySource> CreateManagedIdentitySource(
    std::string const& credentialName,
    std::string const& clientId,
    std::string const& objectId,
    std::string const& resourceId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
{
  using namespace Azure::Core::Credentials;
  using namespace Azure::Identity::_detail;
  static std::unique_ptr<ManagedIdentitySource> (*managedIdentitySourceCreate[])(
      std::string const& credName,
      std::string const& clientId,
      std::string const& objectId,
      std::string const& resourceId,
      TokenCredentialOptions const& options)
      = {AppServiceV2019ManagedIdentitySource::Create,
         AppServiceV2017ManagedIdentitySource::Create,
         CloudShellManagedIdentitySource::Create,
         AzureArcManagedIdentitySource::Create,
         ImdsManagedIdentitySource::Create};

  // IMDS ManagedIdentity, which comes last in the list, will never return nullptr from Create().
  // For that reason, it is not possible to cover that execution branch in tests.
  for (auto create : managedIdentitySourceCreate)
  {
    if (auto source = create(credentialName, clientId, objectId, resourceId, options))
    {
      return source;
    }
  }

  throw AuthenticationException(
      credentialName + " authentication unavailable. No Managed Identity endpoint found.");
}
} // namespace

ManagedIdentityCredential::~ManagedIdentityCredential() = default;

ManagedIdentityCredential::ManagedIdentityCredential(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : TokenCredential("ManagedIdentityCredential")
{
  m_managedIdentitySource
      = CreateManagedIdentitySource(GetCredentialName(), clientId, {}, {}, options);
}

ManagedIdentityCredential::ManagedIdentityCredential(
    ManagedIdentityCredentialOptions const& options)
    : TokenCredential("ManagedIdentityCredential")
{
  int numOptionsSet = 0;
  if (!options.ClientId.empty())
  {
    numOptionsSet++;
  }
  if (!options.ObjectId.empty())
  {
    numOptionsSet++;
  }
  if (!options.ResourceId.ToString().empty())
  {
    numOptionsSet++;
  }
  if (numOptionsSet > 1)
  {
    throw std::invalid_argument("Only one of ClientId, ObjectId, or ResourceId can be set in "
                                "ManagedIdentityCredentialOptions.");
  }
  m_managedIdentitySource = CreateManagedIdentitySource(
      GetCredentialName(),
      options.ClientId,
      options.ObjectId,
      options.ResourceId.ToString(),
      options);
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
