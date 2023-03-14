// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_secret_credential.hpp"

#include "private/token_credential_impl.hpp"

using Azure::Identity::ClientSecretCredential;

using Azure::Core::Context;
using Azure::Core::Url;
using Azure::Core::Credentials::AccessToken;
using Azure::Core::Credentials::TokenCredentialOptions;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Http::HttpMethod;
using Azure::Identity::_detail::TokenCredentialImpl;

ClientSecretCredential::ClientSecretCredential(
    std::string tenantId,
    std::string const& clientId,
    std::string const& clientSecret,
    std::string const& authorityHost,
    TokenCredentialOptions const& options)
    : TokenCredential("ClientSecretCredential"), m_clientCredentialCore(tenantId, authorityHost),
      m_tokenCredentialImpl(std::make_unique<TokenCredentialImpl>(options)),
      m_requestBody(
          std::string("grant_type=client_credentials&client_id=") + Url::Encode(clientId)
          + "&client_secret=" + Url::Encode(clientSecret))
{
}

ClientSecretCredential::ClientSecretCredential(
    std::string tenantId,
    std::string const& clientId,
    std::string const& clientSecret,
    ClientSecretCredentialOptions const& options)
    : ClientSecretCredential(tenantId, clientId, clientSecret, options.AuthorityHost, options)
{
}

ClientSecretCredential::ClientSecretCredential(
    std::string tenantId,
    std::string const& clientId,
    std::string const& clientSecret,
    Core::Credentials::TokenCredentialOptions const& options)
    : ClientSecretCredential(
        tenantId,
        clientId,
        clientSecret,
        ClientSecretCredentialOptions{}.AuthorityHost,
        options)
{
}

ClientSecretCredential::~ClientSecretCredential() = default;

AccessToken ClientSecretCredential::GetToken(
    TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
  auto const scopesStr = m_clientCredentialCore.GetScopesString(tokenRequestContext.Scopes);

  // TokenCache::GetToken() and m_tokenCredentialImpl->GetToken() can only use the lambda argument
  // when they are being executed. They are not supposed to keep a reference to lambda argument to
  // call it later. Therefore, any capture made here will outlive the possible time frame when the
  // lambda might get called.
  return m_tokenCache.GetToken(scopesStr, tokenRequestContext.MinimumExpiration, [&]() {
    return m_tokenCredentialImpl->GetToken(context, [&]() {
      auto body = m_requestBody;

      if (!scopesStr.empty())
      {
        body += "&scope=" + scopesStr;
      }

      auto const requestUrl = m_clientCredentialCore.GetRequestUrl();

      auto request
          = std::make_unique<TokenCredentialImpl::TokenRequest>(HttpMethod::Post, requestUrl, body);

      request->HttpRequest.SetHeader("Host", requestUrl.GetHost());

      return request;
    });
  });
}
