// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/workload_identity_credential.hpp"

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
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Http::HttpMethod;
using Azure::Identity::_detail::TenantIdResolver;
using Azure::Identity::_detail::TokenCredentialImpl;

namespace {
constexpr auto AzureTenantIdEnvVarName = "AZURE_TENANT_ID";
constexpr auto AzureClientIdEnvVarName = "AZURE_CLIENT_ID";
constexpr auto AzureAuthorityHostEnvVarName = "AZURE_AUTHORITY_HOST";
constexpr auto AzureFederatedTokenFileEnvVarName = "AZURE_FEDERATED_TOKEN_FILE";
} // namespace

WorkloadIdentityCredential::WorkloadIdentityCredential(
    std::string const& tenantIdOptions,
    std::string const& clientIdOptions,
    std::string const& authorityHostOptions,
    std::string const& tokenFilePathOptions,
    std::vector<std::string> additionallyAllowedTenants,
    Core::Credentials::TokenCredentialOptions const& options)
    : TokenCredential("WorkloadIdentityCredential"),
      m_clientCredentialCore(tenantIdOptions, authorityHostOptions, additionallyAllowedTenants)
{
  std::string tenantId;
  std::string clientId;
  std::string authorityHost;

  if (tenantIdOptions.empty())
  {
    tenantId = Environment::GetVariable(AzureTenantIdEnvVarName);
    if (tenantId.empty())
    {
      throw std::runtime_error(
          "No tenant ID specified. Check pod configuration or set TenantID in the options.");
    }
  }
  if (clientIdOptions.empty())
  {
    clientId = Environment::GetVariable(AzureClientIdEnvVarName);
    if (clientId.empty())
    {
      throw std::runtime_error(
          "No client ID specified. Check pod configuration or set ClientID in the options.");
    }
  }
  if (authorityHostOptions.empty())
  {
    authorityHost = Environment::GetVariable(AzureAuthorityHostEnvVarName);
    if (authorityHost.empty())
    {
      authorityHost = _detail::ClientCredentialCore::AadGlobalAuthority;
    }
  }
  if (tokenFilePathOptions.empty())
  {
    m_tokenFilePath = Environment::GetVariable(AzureFederatedTokenFileEnvVarName);
    if (m_tokenFilePath.empty())
    {
      throw std::runtime_error(
          "No token file specified. Check pod configuration or set TokenFilePath in the options.");
    }
  }

  m_clientCredentialCore = Azure::Identity::_detail::ClientCredentialCore(
      tenantId, authorityHost, additionallyAllowedTenants);
  m_tokenCredentialImpl = std::make_unique<TokenCredentialImpl>(options);
  m_requestBody
      = std::string(
            "grant_type=client_credentials"
            "&client_assertion_type="
            "urn%3Aietf%3Aparams%3Aoauth%3Aclient-assertion-type%3Ajwt-bearer" // cspell:disable-line
            "&client_id=")
      + Url::Encode(clientId);
}

WorkloadIdentityCredential::WorkloadIdentityCredential(
    WorkloadIdentityCredentialOptions const& options)
    : WorkloadIdentityCredential(
        options.TenantId,
        options.ClientId,
        options.AuthorityHost,
        options.TokenFilePath,
        options.AdditionallyAllowedTenants,
        options)
{
}

WorkloadIdentityCredential::WorkloadIdentityCredential(
    Core::Credentials::TokenCredentialOptions const& options)
    : WorkloadIdentityCredential(
        WorkloadIdentityCredentialOptions{}.TenantId,
        WorkloadIdentityCredentialOptions{}.ClientId,
        WorkloadIdentityCredentialOptions{}.AuthorityHost,
        WorkloadIdentityCredentialOptions{}.TokenFilePath,
        WorkloadIdentityCredentialOptions{}.AdditionallyAllowedTenants,
        options)
{
}

WorkloadIdentityCredential::~WorkloadIdentityCredential() = default;

AccessToken WorkloadIdentityCredential::GetToken(
    TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
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
