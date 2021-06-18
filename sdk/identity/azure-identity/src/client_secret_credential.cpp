// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_secret_credential.hpp"

#include <sstream>

using namespace Azure::Identity;

std::string const Azure::Identity::_detail::g_aadGlobalAuthority
    = "https://login.microsoftonline.com/";

ClientSecretCredential::ClientSecretCredential(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& clientSecret,
    std::string const& authorityHost,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : _detail::TokenCredentialImpl(options), m_isAdfs(tenantId == "adfs")
{
  using Azure::Core::Url;
  m_requestUrl = Url(authorityHost);
  m_requestUrl.AppendPath(tenantId);
  m_requestUrl.AppendPath(m_isAdfs ? "oauth2/token" : "oauth2/v2.0/token");

  std::ostringstream body;
  body << "grant_type=client_credentials&client_id=" << Url::Encode(clientId)
       << "&client_secret=" << Url::Encode(clientSecret);

  m_requestBody = body.str();
}

std::unique_ptr<Azure::Identity::_detail::TokenCredentialImpl::TokenRequest>
ClientSecretCredential::CreateRequest(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext) const
{
  using Azure::Core::Http::HttpMethod;

  std::ostringstream body;
  body << m_requestBody;
  {
    auto const& scopes = tokenRequestContext.Scopes;
    if (!scopes.empty())
    {
      body << "&scope=" << FormatScopes(scopes, m_isAdfs);
    }
  }

  auto request = std::make_unique<TokenRequest>(HttpMethod::Post, m_requestUrl, body.str());
  if (m_isAdfs)
  {
    request->HttpRequest.SetHeader("Host", m_requestUrl.GetHost());
  }

  return request;
}
