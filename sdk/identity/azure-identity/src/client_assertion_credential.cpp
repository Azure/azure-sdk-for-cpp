// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/client_assertion_credential.hpp"

#include "private/identity_log.hpp"
#include "private/package_version.hpp"
#include "private/tenant_id_resolver.hpp"
#include "private/token_credential_impl.hpp"

#include <azure/core/internal/json/json.hpp>

using Azure::Identity::ClientAssertionCredential;
using Azure::Identity::ClientAssertionCredentialOptions;

using Azure::Core::Context;
using Azure::Core::Url;
using Azure::Core::_internal::StringExtensions;
using Azure::Core::Credentials::AccessToken;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Http::HttpMethod;
using Azure::Identity::_detail::IdentityLog;
using Azure::Identity::_detail::TenantIdResolver;
using Azure::Identity::_detail::TokenCredentialImpl;

namespace {
bool IsValidTenantId(std::string const& tenantId)
{
  const std::string allowedChars = ".-";
  if (tenantId.empty())
  {
    return false;
  }
  for (auto const c : tenantId)
  {
    if (allowedChars.find(c) != std::string::npos)
    {
      continue;
    }
    if (!StringExtensions::IsAlphaNumeric(c))
    {
      return false;
    }
  }
  return true;
}
} // namespace

ClientAssertionCredential::ClientAssertionCredential(
    std::string tenantId,
    std::string clientId,
    std::function<std::string(Context const&)> assertionCallback,
    ClientAssertionCredentialOptions const& options)
    : TokenCredential("ClientAssertionCredential"),
      m_assertionCallback(std::move(assertionCallback)),
      m_clientCredentialCore(tenantId, options.AuthorityHost, options.AdditionallyAllowedTenants)
{
  bool isTenantIdValid = IsValidTenantId(tenantId);
  if (!isTenantIdValid)
  {
    IdentityLog::Write(
        IdentityLog::Level::Warning,
        GetCredentialName()
            + ": Invalid tenant ID provided. The tenant ID must be a non-empty string containing "
              "only alphanumeric characters, periods, or hyphens. You can locate your tenant ID by "
              "following the instructions listed here: "
              "https://learn.microsoft.com/partner-center/find-ids-and-domain-names");
  }
  if (clientId.empty())
  {
    IdentityLog::Write(
        IdentityLog::Level::Warning, GetCredentialName() + ": No client ID specified.");
  }
  if (!m_assertionCallback)
  {
    IdentityLog::Write(
        IdentityLog::Level::Warning,
        GetCredentialName()
            + ": The assertionCallback must be a valid function that returns assertions.");
  }

  if (isTenantIdValid && !clientId.empty() && m_assertionCallback)
  {
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
    // Rather than throwing an exception in the ctor, following the pattern in existing credentials
    // to log the errors, and defer throwing an exception to the first call of GetToken(). This is
    // primarily needed for credentials that are part of the DefaultAzureCredential, which this
    // credential is not intended for.
    IdentityLog::Write(
        IdentityLog::Level::Warning, GetCredentialName() + " was not initialized correctly.");
  }
}

ClientAssertionCredential::~ClientAssertionCredential() = default;

AccessToken ClientAssertionCredential::GetToken(
    TokenRequestContext const& tokenRequestContext,
    Context const& context) const
{
  if (!m_tokenCredentialImpl)
  {
    auto const AuthUnavailable = GetCredentialName() + " authentication unavailable. ";

    IdentityLog::Write(
        IdentityLog::Level::Warning,
        AuthUnavailable + "See earlier " + GetCredentialName() + " log messages for details.");

    throw AuthenticationException(AuthUnavailable);
  }

  auto const tenantId = TenantIdResolver::Resolve(
      m_clientCredentialCore.GetTenantId(),
      tokenRequestContext,
      m_clientCredentialCore.GetAdditionallyAllowedTenants());

  auto const scopesStr
      = m_clientCredentialCore.GetScopesString(tenantId, tokenRequestContext.Scopes);

  // TokenCache::GetToken() and m_tokenCredentialImpl->GetToken() can only use the lambda
  // argument when they are being executed. They are not supposed to keep a reference to lambda
  // argument to call it later. Therefore, any capture made here will outlive the possible time
  // frame when the lambda might get called.
  return m_tokenCache.GetToken(scopesStr, tenantId, tokenRequestContext.MinimumExpiration, [&]() {
    return m_tokenCredentialImpl->GetToken(context, false, [&]() {
      auto body = m_requestBody;
      if (!scopesStr.empty())
      {
        body += "&scope=" + scopesStr;
      }

      // Get the request url before calling m_assertionCallback to validate the authority host
      // scheme (GetRequestUrl() will throw if validation fails). This is to avoid calling the
      // assertion callback if the authority host scheme is invalid.
      auto const requestUrl = m_clientCredentialCore.GetRequestUrl(tenantId);

      const std::string assertion = m_assertionCallback(context);

      body += "&client_assertion=" + Azure::Core::Url::Encode(assertion);

      auto request
          = std::make_unique<TokenCredentialImpl::TokenRequest>(HttpMethod::Post, requestUrl, body);

      request->HttpRequest.SetHeader("Host", requestUrl.GetHost());

      return request;
    });
  });
}
