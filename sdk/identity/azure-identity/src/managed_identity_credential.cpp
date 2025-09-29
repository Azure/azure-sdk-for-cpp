// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/managed_identity_credential.hpp"

#include "private/managed_identity_source.hpp"

using namespace Azure::Identity::_detail;
using namespace Azure::Identity;

namespace {
std::unique_ptr<_detail::ManagedIdentitySource> CreateManagedIdentitySource(
    std::string const& credentialName,
    std::string const& clientId,
    std::string const& objectId,
    std::string const& resourceId,
    Azure::Core::Credentials::TokenCredentialOptions const& options,
    bool isProbeEnabled)
{
  using namespace Azure::Core::Credentials;
  using namespace Azure::Identity::_detail;
  static std::unique_ptr<ManagedIdentitySource> (*managedIdentitySourceCreate[])(
      std::string const& credName,
      std::string const& clientId,
      std::string const& objectId,
      std::string const& resourceId,
      TokenCredentialOptions const& options,
      bool isProbeEnabled)
      = {AppServiceV2019ManagedIdentitySource::Create,
         AppServiceV2017ManagedIdentitySource::Create,
         CloudShellManagedIdentitySource::Create,
         AzureArcManagedIdentitySource::Create,
         ImdsManagedIdentitySource::Create};

  // IMDS ManagedIdentity, which comes last in the list, will never return nullptr from Create().
  // For that reason, it is not possible to cover that execution branch in tests.
  for (auto create : managedIdentitySourceCreate)
  {
    if (auto source
        = create(credentialName, clientId, objectId, resourceId, options, isProbeEnabled))
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
    : ManagedIdentityCredential(clientId, false, options)
{
}

ManagedIdentityCredential::ManagedIdentityCredential(
    char const* clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : ManagedIdentityCredential(std::string(clientId), options)
{
}

ManagedIdentityCredential::ManagedIdentityCredential(
    std::string const& clientId,
    bool isProbeEnabled,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : TokenCredential("ManagedIdentityCredential")
{
  m_managedIdentitySource
      = CreateManagedIdentitySource(GetCredentialName(), clientId, {}, {}, options, isProbeEnabled);
}

ManagedIdentityCredential::ManagedIdentityCredential(
    Azure::Identity::ManagedIdentityCredentialOptions const& options)
    : TokenCredential("ManagedIdentityCredential")
{
  ManagedIdentityIdKind idType = options.IdentityId.GetManagedIdentityIdKind();
  switch (idType)
  {
    case ManagedIdentityIdKind::SystemAssigned:
      m_managedIdentitySource = CreateManagedIdentitySource(
          GetCredentialName(), {}, {}, {}, options, options.IsProbeEnabled);
      break;
    case ManagedIdentityIdKind::ClientId:
      m_managedIdentitySource = CreateManagedIdentitySource(
          GetCredentialName(), options.IdentityId.GetId(), {}, {}, options, options.IsProbeEnabled);
      break;
    case ManagedIdentityIdKind::ObjectId:
      m_managedIdentitySource = CreateManagedIdentitySource(
          GetCredentialName(), {}, options.IdentityId.GetId(), {}, options, options.IsProbeEnabled);
      break;
    case ManagedIdentityIdKind::ResourceId:
      m_managedIdentitySource = CreateManagedIdentitySource(
          GetCredentialName(), {}, {}, options.IdentityId.GetId(), options, options.IsProbeEnabled);
      break;
    default:
      throw std::invalid_argument(
          "The ManagedIdentityIdKind in the options is not set to one of the valid values.");
      break;
  }
}

ManagedIdentityCredential::ManagedIdentityCredential(
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : ManagedIdentityCredential(std::string(), false, options)
{
}

ManagedIdentityCredential::ManagedIdentityCredential(
    bool isProbeEnabled,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : ManagedIdentityCredential(std::string(), isProbeEnabled, options)
{
}

Azure::Core::Credentials::AccessToken ManagedIdentityCredential::GetToken(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context) const
{
  return m_managedIdentitySource->GetToken(tokenRequestContext, context);
}
