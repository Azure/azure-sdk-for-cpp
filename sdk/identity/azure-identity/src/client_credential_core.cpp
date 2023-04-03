// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/detail/client_credential_core.hpp"

#include "private/tenant_id_resolver.hpp"
#include "private/token_credential_impl.hpp"

#include <utility>

using Azure::Identity::_detail::ClientCredentialCore;

using Azure::Core::Url;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Identity::_detail::TenantIdResolver;
using Azure::Identity::_detail::TokenCredentialImpl;

decltype(ClientCredentialCore::AadGlobalAuthority) ClientCredentialCore::AadGlobalAuthority
    = "https://login.microsoftonline.com/";

ClientCredentialCore::ClientCredentialCore(
    std::string tenantId,
    std::string const& authorityHost,
    std::vector<std::string> additionallyAllowedTenants)
    : m_additionallyAllowedTenants(std::move(additionallyAllowedTenants)),
      m_authorityHost(Url(authorityHost)), m_tenantId(std::move(tenantId))
{
}

Url ClientCredentialCore::GetRequestUrl(std::string const& tenantId) const
{
  auto requestUrl = m_authorityHost;
  requestUrl.AppendPath(tenantId);
  requestUrl.AppendPath(TenantIdResolver::IsAdfs(tenantId) ? "oauth2/token" : "oauth2/v2.0/token");

  return requestUrl;
}

std::string ClientCredentialCore::GetScopesString(
    std::string const& tenantId,
    decltype(TokenRequestContext::Scopes) const& scopes) const
{
  return scopes.empty()
      ? std::string()
      : TokenCredentialImpl::FormatScopes(scopes, TenantIdResolver::IsAdfs(tenantId));
}
