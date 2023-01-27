//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/detail/client_credential_core.hpp"

#include "private/token_credential_impl.hpp"

#include <utility>

using Azure::Identity::_detail::ClientCredentialCore;

using Azure::Core::Url;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Identity::_detail::TokenCredentialImpl;

decltype(ClientCredentialCore::AadGlobalAuthority) ClientCredentialCore::AadGlobalAuthority
    = "https://login.microsoftonline.com/";

ClientCredentialCore::ClientCredentialCore(std::string tenantId, std::string const& authorityHost)
    : m_authorityHost(Url(authorityHost)), m_tenantId(std::move(tenantId))
{
  // ADFS is the Active Directory Federation Service, a tenant ID that is used in Azure Stack.
  m_isAdfs = m_tenantId == "adfs";
}

Url ClientCredentialCore::GetRequestUrl() const
{
  auto requestUrl = m_authorityHost;
  requestUrl.AppendPath(m_tenantId);
  requestUrl.AppendPath(m_isAdfs ? "oauth2/token" : "oauth2/v2.0/token");

  return requestUrl;
}

std::string ClientCredentialCore::GetScopesString(decltype(TokenRequestContext::Scopes)
                                                      const& scopes) const
{
  return scopes.empty() ? std::string() : TokenCredentialImpl::FormatScopes(scopes, m_isAdfs);
}
