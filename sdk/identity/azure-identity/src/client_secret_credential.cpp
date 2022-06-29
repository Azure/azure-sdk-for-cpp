// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_secret_credential.hpp"

#include "private/token_credential_impl.hpp"

#include <sstream>

using namespace Azure::Identity;

ClientSecretCredential::ClientSecretCredential(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& clientSecret,
    std::string const& authorityHost,
    bool disableTenantDiscovery,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : m_tokenCredentialImpl(std::make_unique<_detail::TokenCredentialImpl>(options)),
      m_clientCredentialHelper(tenantId, authorityHost, disableTenantDiscovery)
{
  using Azure::Core::Url;

  std::ostringstream body;
  body << "grant_type=client_credentials&client_id=" << Url::Encode(clientId)
       << "&client_secret=" << Url::Encode(clientSecret);

  m_requestBody = body.str();
}

ClientSecretCredential::ClientSecretCredential(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& clientSecret,
    ClientSecretCredentialOptions const& options)
    : ClientSecretCredential(
        tenantId,
        clientId,
        clientSecret,
        options.AuthorityHost,
        options.DisableTenantDiscovery,
        options)
{
}

ClientSecretCredential::ClientSecretCredential(
    std::string tenantId,
    std::string clientId,
    std::string clientSecret,
    Core::Credentials::TokenCredentialOptions const& options)
    : ClientSecretCredential(
        tenantId,
        clientId,
        clientSecret,
        _detail::g_aadGlobalAuthority,
        _detail::ClientCredentialHelper::IsTenantDiscoveryDisabledByDefault(),
        options)
{
}

ClientSecretCredential::~ClientSecretCredential() = default;

Azure::Core::Credentials::AccessToken ClientSecretCredential::GetToken(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context) const
{
  return m_tokenCredentialImpl->GetToken(context, [&]() {
    using _detail::TokenCredentialImpl;
    using Azure::Core::Http::HttpMethod;

    auto const isAdfs = m_clientCredentialHelper.IsAdfs;

    std::ostringstream body;
    body << m_requestBody;
    {
      auto const& scopes = tokenRequestContext.Scopes;
      if (!scopes.empty())
      {
        body << "&scope=" << TokenCredentialImpl::FormatScopes(scopes, isAdfs);
      }
    }

    auto const requestUrl = m_clientCredentialHelper.GetRequestUrl(tokenRequestContext);
    auto request = std::make_unique<TokenCredentialImpl::TokenRequest>(
        HttpMethod::Post, requestUrl, body.str());

    if (isAdfs)
    {
      request->HttpRequest.SetHeader("Host", requestUrl.GetHost());
    }

    return request;
  });
}
