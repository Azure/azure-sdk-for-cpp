// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/workload_identity_credential.hpp"

#include "private/identity_log.hpp"
#include "private/tenant_id_resolver.hpp"
#include "private/token_credential_impl.hpp"

#include <azure/core/internal/environment.hpp>

#include <fstream>
#include <streambuf>

using Azure::Identity::WorkloadIdentityCredential;

using Azure::Core::Context;
using Azure::Core::Url;
using Azure::Core::_internal::Environment;
using Azure::Core::Credentials::AccessToken;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Http::HttpMethod;
using Azure::Identity::_detail::IdentityLog;
using Azure::Identity::_detail::TenantIdResolver;
using Azure::Identity::_detail::TokenCredentialImpl;

WorkloadIdentityCredential::WorkloadIdentityCredential(
    WorkloadIdentityCredentialOptions const& options)
    : TokenCredential("WorkloadIdentityCredential"), m_clientCredentialCore(
                                                         options.TenantId,
                                                         options.AuthorityHost,
                                                         options.AdditionallyAllowedTenants)
{
  std::string tenantId = options.TenantId;
  std::string clientId = options.ClientId;
  std::string authorityHost = options.AuthorityHost;
  m_tokenFilePath = options.TokenFilePath;

  if (!tenantId.empty() && !clientId.empty() && !m_tokenFilePath.empty())
  {
    m_clientCredentialCore = Azure::Identity::_detail::ClientCredentialCore(
        tenantId, authorityHost, options.AdditionallyAllowedTenants);
    m_tokenCredentialImpl = std::make_unique<TokenCredentialImpl>(options);
    m_requestBody
        = std::string(
              "grant_type=client_credentials"
              "&client_assertion_type="
              "urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer" // cspell:disable-line
              "&client_id=")
        + Url::Encode(clientId);

    IdentityLog::Write(
        IdentityLog::Level::Informational, GetCredentialName() + " was created successfully.");
  }
  else
  {
    IdentityLog::Write(
        IdentityLog::Level::Warning,
        "Azure Kubernetes environment is not set up for the " + GetCredentialName()
            + " credential to work.");
  }
}

WorkloadIdentityCredential::WorkloadIdentityCredential(
    Core::Credentials::TokenCredentialOptions const& options)
    : TokenCredential("WorkloadIdentityCredential"),
      m_clientCredentialCore("", "", std::vector<std::string>())
{
  std::string const tenantId = _detail::DefaultOptionValues::GetTenantId();
  std::string const clientId = _detail::DefaultOptionValues::GetClientId();
  m_tokenFilePath = _detail::DefaultOptionValues::GetFederatedTokenFile();

  if (!tenantId.empty() && !clientId.empty() && !m_tokenFilePath.empty())
  {
    std::string const authorityHost = _detail::DefaultOptionValues::GetAuthorityHost();

    m_clientCredentialCore = Azure::Identity::_detail::ClientCredentialCore(
        tenantId, authorityHost, std::vector<std::string>());
    m_tokenCredentialImpl = std::make_unique<TokenCredentialImpl>(options);
    m_requestBody
        = std::string(
              "grant_type=client_credentials"
              "&client_assertion_type="
              "urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer" // cspell:disable-line
              "&client_id=")
        + Url::Encode(clientId);

    IdentityLog::Write(
        IdentityLog::Level::Informational, GetCredentialName() + " was created successfully.");
  }
  else
  {
    IdentityLog::Write(
        IdentityLog::Level::Warning,
        "Azure Kubernetes environment is not set up for the " + GetCredentialName()
            + " credential to work.");
  }
}

WorkloadIdentityCredential::~WorkloadIdentityCredential() = default;

AccessToken WorkloadIdentityCredential::GetToken(
    TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
  if (!m_tokenCredentialImpl)
  {
    auto const AuthUnavailable = GetCredentialName() + " authentication unavailable. ";

    IdentityLog::Write(
        IdentityLog::Level::Warning,
        AuthUnavailable + "See earlier " + GetCredentialName() + " log messages for details.");

    throw AuthenticationException(
        AuthUnavailable + "Azure Kubernetes environment is not set up correctly.");
  }

  auto const tenantId = TenantIdResolver::Resolve(
      m_clientCredentialCore.GetTenantId(),
      tokenRequestContext,
      m_clientCredentialCore.GetAdditionallyAllowedTenants());

  auto const scopesStr
      = m_clientCredentialCore.GetScopesString(tenantId, tokenRequestContext.Scopes);

  // TokenCache::GetToken() and m_tokenCredentialImpl->GetToken() can only use the lambda argument
  // when they are being executed. They are not supposed to keep a reference to lambda argument to
  // call it later. Therefore, any capture made here will outlive the possible time frame when the
  // lambda might get called.
  return m_tokenCache.GetToken(scopesStr, tenantId, tokenRequestContext.MinimumExpiration, [&]() {
    return m_tokenCredentialImpl->GetToken(context, [&]() {
      auto body = m_requestBody;
      if (!scopesStr.empty())
      {
        body += "&scope=" + scopesStr;
      }

      auto const requestUrl = m_clientCredentialCore.GetRequestUrl(tenantId);

      // Read the specified file's content, which is expected to be a Kubernetes service account
      // token. Kubernetes is responsible for updating the file as service account tokens expire.
      std::ifstream azureFederatedTokenFile(m_tokenFilePath);
      std::string assertion(
          (std::istreambuf_iterator<char>(azureFederatedTokenFile)),
          std::istreambuf_iterator<char>());

      body += "&client_assertion=" + Azure::Core::Url::Encode(assertion);

      auto request
          = std::make_unique<TokenCredentialImpl::TokenRequest>(HttpMethod::Post, requestUrl, body);

      request->HttpRequest.SetHeader("Host", requestUrl.GetHost());

      return request;
    });
  });
}
