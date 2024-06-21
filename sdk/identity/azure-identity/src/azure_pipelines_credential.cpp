// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "azure/identity/azure_pipelines_credential.hpp"

#include "private/identity_log.hpp"
#include "private/package_version.hpp"
#include "private/tenant_id_resolver.hpp"
#include "private/token_credential_impl.hpp"

#include <azure/core/internal/json/json.hpp>

using Azure::Identity::AzurePipelinesCredential;
using Azure::Identity::AzurePipelinesCredentialOptions;

using Azure::Core::Context;
using Azure::Core::Url;
using Azure::Core::_internal::StringExtensions;
using Azure::Core::Credentials::AccessToken;
using Azure::Core::Credentials::AuthenticationException;
using Azure::Core::Credentials::TokenRequestContext;
using Azure::Core::Http::HttpMethod;
using Azure::Core::Http::HttpStatusCode;
using Azure::Core::Http::RawResponse;
using Azure::Core::Http::Request;
using Azure::Core::Http::_internal::HttpPipeline;
using Azure::Core::Json::_internal::json;
using Azure::Identity::_detail::IdentityLog;
using Azure::Identity::_detail::PackageVersion;
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

AzurePipelinesCredential::AzurePipelinesCredential(
    std::string tenantId,
    std::string clientId,
    std::string serviceConnectionId,
    std::string systemAccessToken,
    AzurePipelinesCredentialOptions const& options)
    : TokenCredential("AzurePipelinesCredential"), m_serviceConnectionId(serviceConnectionId),
      m_systemAccessToken(systemAccessToken),
      m_clientCredentialCore(tenantId, options.AuthorityHost, options.AdditionallyAllowedTenants),
      m_httpPipeline(HttpPipeline(options, "identity", PackageVersion::ToString(), {}, {}))
{
  m_oidcRequestUrl = _detail::DefaultOptionValues::GetOidcRequestUrl();

  bool isTenantIdValid = IsValidTenantId(tenantId);
  if (!isTenantIdValid)
  {
    IdentityLog::Write(
        IdentityLog::Level::Warning,
        "Invalid tenant ID provided  for " + GetCredentialName()
            + ". The tenant ID must be a non-empty string containing only alphanumeric characters, "
              "periods, or hyphens. You can locate your tenant ID by following the instructions "
              "listed here: https://learn.microsoft.com/partner-center/find-ids-and-domain-names");
  }
  if (clientId.empty())
  {
    IdentityLog::Write(
        IdentityLog::Level::Warning, "No client ID specified for " + GetCredentialName() + ".");
  }
  if (serviceConnectionId.empty())
  {
    IdentityLog::Write(
        IdentityLog::Level::Warning,
        "No service connection ID specified for " + GetCredentialName() + ".");
  }
  if (systemAccessToken.empty())
  {
    IdentityLog::Write(
        IdentityLog::Level::Warning,
        "No system access token specified for " + GetCredentialName() + ".");
  }
  if (m_oidcRequestUrl.empty())
  {
    IdentityLog::Write(
        IdentityLog::Level::Warning,
        "No value for environment variable '" + Azure::Identity::_detail::OidcRequestUrlEnvVarName
            + "' needed by " + GetCredentialName() + ". This should be set by Azure Pipelines.");
  }

  if (isTenantIdValid && !clientId.empty() && !serviceConnectionId.empty()
      && !systemAccessToken.empty() && !m_oidcRequestUrl.empty())
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
        IdentityLog::Level::Warning,
        "Azure Pipelines environment is not set up for the " + GetCredentialName()
            + " credential to work.");
  }
}

Request AzurePipelinesCredential::CreateOidcRequestMessage() const
{
  const std::string oidcApiVersion = "7.1";

  Url requestUrl = Url(
      m_oidcRequestUrl + "?api-version=" + Url::Encode(oidcApiVersion)
      + "&serviceConnectionId=" + Url::Encode(m_serviceConnectionId));
  Request request = Request(HttpMethod::Post, requestUrl);
  request.SetHeader("content-type", "application/json");
  request.SetHeader("authorization", "Bearer " + m_systemAccessToken);

  return request;
}

std::string AzurePipelinesCredential::GetOidcTokenResponse(
    std::unique_ptr<RawResponse> const& response,
    std::string responseBody) const
{
  auto const statusCode = response->GetStatusCode();
  if (statusCode != HttpStatusCode::Ok)
  {
    // Include the response because its body, if any, probably contains an error message.
    // OK responses aren't included with errors because they probably contain secrets.

    std::string message = GetCredentialName() + " : "
        + std::to_string(static_cast<std::underlying_type<decltype(statusCode)>::type>(statusCode))
        + " (" + response->GetReasonPhrase()
        + ") response from the OIDC endpoint. Check service connection ID and Pipeline "
          "configuration.\n\n"
        + responseBody;
    IdentityLog::Write(IdentityLog::Level::Verbose, message);

    throw AuthenticationException(message);
  }

  json parsedJson;
  try
  {
    parsedJson = Azure::Core::Json::_internal::json::parse(responseBody);
  }
  catch (json::exception const&)
  {
    std::string message = GetCredentialName() + " : Cannot parse the response string as JSON.";
    IdentityLog::Write(IdentityLog::Level::Verbose, message);

    throw AuthenticationException(message);
  }

  const std::string oidcTokenPropertyName = "oidcToken";
  if (!parsedJson.contains(oidcTokenPropertyName) || !parsedJson[oidcTokenPropertyName].is_string())
  {
    std::string message = GetCredentialName()
        + " : OIDC token not found in response. \nSee Azure::Core::Diagnostics::Logger for details "
          "(https://aka.ms/azsdk/cpp/identity/troubleshooting).";
    IdentityLog::Write(IdentityLog::Level::Verbose, message);
    throw AuthenticationException(message);
  }
  return parsedJson[oidcTokenPropertyName].get<std::string>();
}

AzurePipelinesCredential::~AzurePipelinesCredential() = default;

std::string AzurePipelinesCredential::GetAssertion(Context const& context) const
{
  Azure::Core::Http::Request oidcRequest = CreateOidcRequestMessage();
  std::unique_ptr<RawResponse> response = m_httpPipeline.Send(oidcRequest, context);

  if (!response)
  {
    throw AuthenticationException(
        GetCredentialName() + " couldn't send OIDC token request: null response.");
  }

  auto const bodyStream = response->ExtractBodyStream();
  auto const bodyVec = bodyStream ? bodyStream->ReadToEnd(context) : response->GetBody();
  auto const responseBody
      = std::string(reinterpret_cast<char const*>(bodyVec.data()), bodyVec.size());

  return GetOidcTokenResponse(response, responseBody);
}

AccessToken AzurePipelinesCredential::GetToken(
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
        AuthUnavailable + "Azure Pipelines environment is not set up correctly.");
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

      // Get the request url before calling GetAssertion to validate the authority host scheme.
      // This is to avoid making a request to the OIDC endpoint if the authority host scheme is
      // invalid.
      auto const requestUrl = m_clientCredentialCore.GetRequestUrl(tenantId);

      const std::string assertion = GetAssertion(context);

      body += "&client_assertion=" + Azure::Core::Url::Encode(assertion);

      auto request
          = std::make_unique<TokenCredentialImpl::TokenRequest>(HttpMethod::Post, requestUrl, body);

      request->HttpRequest.SetHeader("Host", requestUrl.GetHost());

      return request;
    });
  });
}
