// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/identity/client_secret_credential.hpp"

#include "private/token_credential_impl.hpp"
#include <azure/core/url.hpp>
#include <sstream>

using namespace Azure::Identity;
using Azure::Core::Url;

std::string const Azure::Identity::_detail::g_aadGlobalAuthority
    = "https://login.microsoftonline.com/";

ClientSecretCredential::ClientSecretCredential(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& clientSecret,
    std::string const& authorityHost,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : m_tokenCredentialImpl(new _detail::TokenCredentialImpl(options)), m_isAdfs(tenantId == "adfs")
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

ClientSecretCredential::ClientSecretCredential(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& clientSecret,
    ClientSecretCredentialOptions const& options)
    : ClientSecretCredential(tenantId, clientId, clientSecret, options.AuthorityHost, options)
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

    std::ostringstream body;
    body << m_requestBody;
    {
      auto const& scopes = tokenRequestContext.Scopes;
      if (!scopes.empty())
      {
        body << "&scope=" << TokenCredentialImpl::FormatScopes(scopes, m_isAdfs);
      }
    }

    auto request = std::make_unique<TokenCredentialImpl::TokenRequest>(
        HttpMethod::Post, m_requestUrl, body.str());

    if (m_isAdfs)
    {
      request->HttpRequest.SetHeader("Host", m_requestUrl.GetHost());
    }

    return request;
  });
}

ChallengeClientSecretCredential::ChallengeClientSecretCredential(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& clientSecret,
    std::string const& authorityHost,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : m_tokenCredentialImpl(new _detail::TokenCredentialImpl(options)), m_isAdfs(tenantId == "adfs")
{
  m_requestUrl = Url(authorityHost);
  m_requestUrl.AppendPath(tenantId);
  m_requestUrl.AppendPath(m_isAdfs ? "oauth2/token" : "oauth2/v2.0/token");

  std::ostringstream body;
  body << "grant_type=client_credentials&client_id=" << Url::Encode(clientId)
       << "&client_secret=" << Url::Encode(clientSecret);

  m_requestBody = body.str();
}

ChallengeClientSecretCredential::ChallengeClientSecretCredential(
    std::string const& tenantId,
    std::string const& clientId,
    std::string const& clientSecret,
    ClientSecretCredentialOptions const& options)
    : ChallengeClientSecretCredential(
        tenantId,
        clientId,
        clientSecret,
        options.AuthorityHost,
        options)
{
}

ChallengeClientSecretCredential::ChallengeClientSecretCredential(
    std::string tenantId,
    std::string clientId,
    std::string clientSecret,
    Core::Credentials::TokenCredentialOptions const& options)
    : ChallengeClientSecretCredential(
        tenantId,
        clientId,
        clientSecret,
        _detail::g_aadGlobalAuthority,
        options)
{
}

ChallengeClientSecretCredential::~ChallengeClientSecretCredential() = default;

Azure::Core::Credentials::AccessToken ChallengeClientSecretCredential::GetToken(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context) const
{
  return m_tokenCredentialImpl->GetToken(context, [&]() {
    using _detail::TokenCredentialImpl;
    using Azure::Core::Http::HttpMethod;

    std::ostringstream body;
    body << m_requestBody;
    {
      auto const& scopes = tokenRequestContext.Scopes;
      body << "&response_type=token";
      if (!scopes.empty())
      {
        body << "&scope=" << TokenCredentialImpl::FormatScopes(scopes, m_isAdfs);
      }
    }

    auto request = std::make_unique<TokenCredentialImpl::TokenRequest>(
        HttpMethod::Post, tokenRequestContext.AuthorizationUri.ValueOr(m_requestUrl), body.str());

    if (m_isAdfs)
    {
      request->HttpRequest.SetHeader(
          "Host", tokenRequestContext.AuthorizationUri.ValueOr(m_requestUrl).GetHost());
    }

    return request;
  });
}