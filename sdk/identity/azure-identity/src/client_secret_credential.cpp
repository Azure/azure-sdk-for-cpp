// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_secret_credential.hpp"

#include "private/token_credential_impl.hpp"

#include <sstream>
#include <utility>

using namespace Azure::Identity;

std::string const Azure::Identity::_detail::g_aadGlobalAuthority
    = "https://login.microsoftonline.com/";

ClientSecretCredential::ClientSecretCredential(
    std::string tenantId,
    std::string clientId,
    std::string const& clientSecret,
    std::string authorityHost,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : m_tokenCredentialImpl(std::make_unique<_detail::TokenCredentialImpl>(options)),
      m_tenantId(std::move(tenantId)), m_clientId(std::move(clientId)),
      m_authorityHost(std::move(authorityHost))
{
  using Azure::Core::Url;

  m_isAdfs = (m_tenantId == "adfs");

  m_requestUrl = Url(m_authorityHost);
  m_requestUrl.AppendPath(m_tenantId);
  m_requestUrl.AppendPath(m_isAdfs ? "oauth2/token" : "oauth2/v2.0/token");

  std::ostringstream body;
  body << "grant_type=client_credentials&client_id=" << Url::Encode(m_clientId)
       << "&client_secret=" << Url::Encode(clientSecret);

  m_requestBody = body.str();
}

ClientSecretCredential::ClientSecretCredential(
    std::string tenantId,
    std::string clientId,
    std::string const& clientSecret,
    ClientSecretCredentialOptions const& options)
    : ClientSecretCredential(tenantId, clientId, clientSecret, options.AuthorityHost, options)
{
}

ClientSecretCredential::ClientSecretCredential(
    std::string tenantId,
    std::string clientId,
    std::string const& clientSecret,
    Core::Credentials::TokenCredentialOptions const& options)
    : ClientSecretCredential(
        tenantId,
        clientId,
        clientSecret,
        _detail::g_aadGlobalAuthority,
        options)
{
}

ClientSecretCredential::~ClientSecretCredential() = default;

Azure::Core::Credentials::AccessToken ClientSecretCredential::GetToken(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context) const
{
  using _detail::TokenCredentialImpl;

  std::string scopesStr;
  {
    auto const& scopes = tokenRequestContext.Scopes;
    if (!scopes.empty())
    {
      scopesStr = TokenCredentialImpl::FormatScopes(scopes, m_isAdfs);
    }
  }

  // TokenCache::GetToken() and m_tokenCredentialImpl->GetToken() can only use the lambda argument
  // when they are being executed. They are not supposed to keep a reference to lambda argument to
  // call it later. Therefore, any capture made here will outlive the possible time frame when the
  // lambda might get called.
  return m_tokenCache.GetToken(scopesStr, tokenRequestContext.MinimumExpiration, [&]() {
    return m_tokenCredentialImpl->GetToken(context, [&]() {
      using Azure::Core::Http::HttpMethod;

      std::ostringstream body;
      body << m_requestBody;

      if (!scopesStr.empty())
      {
        body << "&scope=" << scopesStr;
      }

      auto request = std::make_unique<TokenCredentialImpl::TokenRequest>(
          HttpMethod::Post, m_requestUrl, body.str());

      request->HttpRequest.SetHeader("Host", m_requestUrl.GetHost());

      return request;
    });
  });
}
