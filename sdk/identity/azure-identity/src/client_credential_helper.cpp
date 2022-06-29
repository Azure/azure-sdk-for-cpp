// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/detail/client_credential_helper.hpp"

#include <azure/core/internal/environment.hpp>
#include <azure/core/internal/strings.hpp>

using Azure::Core::Url;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Identity::_detail::ClientCredentialHelper;
using Azure::Identity::_detail::g_aadGlobalAuthority;

std::string const g_aadGlobalAuthority = "https://login.microsoftonline.com/";

ClientCredentialHelper::ClientCredentialHelper(
    std::string tenantId,
    std::string const& authorityHost,
    bool disableTenantDiscovery)
    : m_tenantId(tenantId), m_authorityHost(Url(authorityHost)),
      m_disableTenantDiscovery(disableTenantDiscovery), IsAdfs(tenantId == "adfs")
{
}

Url ClientCredentialHelper::GetRequestUrl(TokenRequestContext const& tokenRequestContext) const
{
  auto const& contextAuthUrl = tokenRequestContext.AuthorizationUrl;
  auto const& contextTenantId = tokenRequestContext.TenantId;

  auto isMultitenant = !(m_disableTenantDiscovery || IsAdfs || contextTenantId.empty());

  auto requestUrl = isMultitenant ? contextAuthUrl : m_authorityHost;
  requestUrl.AppendPath(isMultitenant ? contextTenantId : m_tenantId);
  requestUrl.AppendPath(IsAdfs ? "oauth2/token" : "oauth2/v2.0/token");

  return requestUrl;
}

namespace {
bool IsMultiTenantAuthDisabled()
{
  using Azure::Core::_internal::Environment;
  using Azure::Core::_internal::StringExtensions;

  auto envVar = Environment::GetVariable("AZURE_IDENTITY_DISABLE_MULTITENANTAUTH");
  return (envVar == "1" || StringExtensions::LocaleInvariantCaseInsensitiveEqual(envVar, "true"));
}
} // namespace

bool ClientCredentialHelper::IsTenantDiscoveryDisabledByDefault()
{
  static bool const isTenantDiscoveryDisabled = IsMultiTenantAuthDisabled();
  return isTenantDiscoveryDisabled;
}
