// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/managed_identity_source.hpp"

#include <azure/core/internal/environment.hpp>

#include <azure/core/internal/diagnostics/log.hpp>

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>

using namespace Azure::Identity::_detail;

using Azure::Core::_internal::Environment;
using Azure::Core::Diagnostics::Logger;
using Azure::Core::Diagnostics::_internal::Log;

namespace {
std::string const IdentityPrefix = "Identity: ";
std::string const CredPrefix = "ManagedIdentityCredential";

std::string WithSourceMessage(std::string const& credSource)
{
  return std::string(" with ") + credSource + " source";
}

void PrintEnvNotSetUpMessage(std::string const& credSource)
{
  auto const logLevel = Logger::Level::Verbose;
  if (Log::ShouldWrite(logLevel))
  {
    Log::Write(
        logLevel,
        IdentityPrefix + CredPrefix + ": Environment is not set up for the credential to be created"
            + WithSourceMessage(credSource) + '.');
  }
}
} // namespace

Azure::Core::Url ManagedIdentitySource::ParseEndpointUrl(
    std::string const& url,
    char const* envVarName,
    std::string const credSource)
{
  using Azure::Core::Url;
  using Azure::Core::Credentials::AuthenticationException;

  try
  {
    auto const endpointUrl = Url(url);

    auto const logLevel = Logger::Level::Informational;
    if (Log::ShouldWrite(logLevel))
    {
      Log::Write(
          logLevel,
          IdentityPrefix + CredPrefix + " will be created" + WithSourceMessage(credSource) + '.');
    }

    return endpointUrl;
  }
  catch (std::invalid_argument const&)
  {
  }
  catch (std::out_of_range const&)
  {
  }

  auto const errorMessage = CredPrefix + WithSourceMessage(credSource)
      + ": Failed to create: The environment variable \'" + envVarName
      + "\' contains an invalid URL.";

  auto const logLevel = Logger::Level::Warning;
  if (Log::ShouldWrite(logLevel))
  {
    Log::Write(logLevel, IdentityPrefix + errorMessage);
  }

  throw AuthenticationException(errorMessage);
}

template <typename T>
std::unique_ptr<ManagedIdentitySource> AppServiceManagedIdentitySource::Create(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options,
    char const* endpointVarName,
    char const* secretVarName,
    char const* appServiceVersion)
{
  auto const msiEndpoint = Environment::GetVariable(endpointVarName);
  auto const msiSecret = Environment::GetVariable(secretVarName);

  auto const credSource = std::string("App Service ") + appServiceVersion;

  if (!msiEndpoint.empty() && !msiSecret.empty())
  {
    return std::unique_ptr<ManagedIdentitySource>(new T(
        clientId, options, ParseEndpointUrl(msiEndpoint, endpointVarName, credSource), msiSecret));
  }

  PrintEnvNotSetUpMessage(credSource);
  return nullptr;
}

AppServiceManagedIdentitySource::AppServiceManagedIdentitySource(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options,
    Azure::Core::Url endpointUrl,
    std::string const& secret,
    std::string const& apiVersion,
    std::string const& secretHeaderName,
    std::string const& clientIdHeaderName)
    : ManagedIdentitySource(clientId, endpointUrl.GetHost(), options),
      m_request(Azure::Core::Http::HttpMethod::Get, std::move(endpointUrl))
{
  {
    using Azure::Core::Url;
    auto& url = m_request.GetUrl();

    url.AppendQueryParameter("api-version", apiVersion);

    if (!clientId.empty())
    {
      url.AppendQueryParameter(clientIdHeaderName, clientId);
    }
  }

  m_request.SetHeader(secretHeaderName, secret);
}

Azure::Core::Credentials::AccessToken AppServiceManagedIdentitySource::GetToken(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context) const
{
  std::string scopesStr;
  {
    auto const& scopes = tokenRequestContext.Scopes;
    if (!scopes.empty())
    {
      scopesStr = TokenCredentialImpl::FormatScopes(scopes, true);
    }
  }

  // TokenCache::GetToken() and TokenCredentialImpl::GetToken() can only use the lambda argument
  // when they are being executed. They are not supposed to keep a reference to lambda argument to
  // call it later. Therefore, any capture made here will outlive the possible time frame when the
  // lambda might get called.
  return m_tokenCache.GetToken(scopesStr, tokenRequestContext.MinimumExpiration, [&]() {
    return TokenCredentialImpl::GetToken(context, [&]() {
      auto request = std::make_unique<TokenRequest>(m_request);

      if (!scopesStr.empty())
      {
        request->HttpRequest.GetUrl().AppendQueryParameter("resource", scopesStr);
      }

      return request;
    });
  });
}

std::unique_ptr<ManagedIdentitySource> AppServiceV2017ManagedIdentitySource::Create(
    std::string const& clientId,
    Core::Credentials::TokenCredentialOptions const& options)
{
  return AppServiceManagedIdentitySource::Create<AppServiceV2017ManagedIdentitySource>(
      clientId, options, "MSI_ENDPOINT", "MSI_SECRET", "2017");
}

std::unique_ptr<ManagedIdentitySource> AppServiceV2019ManagedIdentitySource::Create(
    std::string const& clientId,
    Core::Credentials::TokenCredentialOptions const& options)
{
  return AppServiceManagedIdentitySource::Create<AppServiceV2019ManagedIdentitySource>(
      clientId, options, "IDENTITY_ENDPOINT", "IDENTITY_HEADER", "2019");
}

std::unique_ptr<ManagedIdentitySource> CloudShellManagedIdentitySource::Create(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
{
  constexpr auto EndpointVarName = "MSI_ENDPOINT";
  auto msiEndpoint = Environment::GetVariable(EndpointVarName);

  std::string const credSource = "Cloud Shell";

  if (!msiEndpoint.empty())
  {
    return std::unique_ptr<ManagedIdentitySource>(new CloudShellManagedIdentitySource(
        clientId, options, ParseEndpointUrl(msiEndpoint, EndpointVarName, credSource)));
  }

  PrintEnvNotSetUpMessage(credSource);
  return nullptr;
}

CloudShellManagedIdentitySource::CloudShellManagedIdentitySource(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options,
    Azure::Core::Url endpointUrl)
    : ManagedIdentitySource(clientId, endpointUrl.GetHost(), options), m_url(std::move(endpointUrl))
{
  using Azure::Core::Url;
  if (!clientId.empty())
  {
    m_body = std::string("client_id=" + Url::Encode(clientId));
  }
}

Azure::Core::Credentials::AccessToken CloudShellManagedIdentitySource::GetToken(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context) const
{
  std::string scopesStr;
  {
    auto const& scopes = tokenRequestContext.Scopes;
    if (!scopes.empty())
    {
      scopesStr = TokenCredentialImpl::FormatScopes(scopes, true);
    }
  }

  // TokenCache::GetToken() and TokenCredentialImpl::GetToken() can only use the lambda argument
  // when they are being executed. They are not supposed to keep a reference to lambda argument to
  // call it later. Therefore, any capture made here will outlive the possible time frame when the
  // lambda might get called.
  return m_tokenCache.GetToken(scopesStr, tokenRequestContext.MinimumExpiration, [&]() {
    return TokenCredentialImpl::GetToken(context, [&]() {
      using Azure::Core::Url;
      using Azure::Core::Http::HttpMethod;

      std::string resource;

      if (!scopesStr.empty())
      {
        resource = "resource=" + scopesStr;
        if (!m_body.empty())
        {
          resource += "&";
        }
      }

      auto request = std::make_unique<TokenRequest>(HttpMethod::Post, m_url, resource + m_body);
      request->HttpRequest.SetHeader("Metadata", "true");

      return request;
    });
  });
}

std::unique_ptr<ManagedIdentitySource> AzureArcManagedIdentitySource::Create(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
{
  using Azure::Core::Credentials::AuthenticationException;

  constexpr auto EndpointVarName = "IDENTITY_ENDPOINT";
  auto identityEndpoint = Environment::GetVariable(EndpointVarName);

  std::string const credSource = "Azure Arc";

  if (identityEndpoint.empty() || Environment::GetVariable("IMDS_ENDPOINT").empty())
  {
    PrintEnvNotSetUpMessage(credSource);
    return nullptr;
  }

  if (!clientId.empty())
  {
    throw AuthenticationException(
        "User assigned identity is not supported by the Azure Arc Managed Identity Endpoint. "
        "To authenticate with the system assigned identity, omit the client ID "
        "when constructing the ManagedIdentityCredential.");
  }

  return std::unique_ptr<ManagedIdentitySource>(new AzureArcManagedIdentitySource(
      options, ParseEndpointUrl(identityEndpoint, EndpointVarName, credSource)));
}

AzureArcManagedIdentitySource::AzureArcManagedIdentitySource(
    Azure::Core::Credentials::TokenCredentialOptions const& options,
    Azure::Core::Url endpointUrl)
    : ManagedIdentitySource(std::string(), endpointUrl.GetHost(), options),
      m_url(std::move(endpointUrl))
{

  m_url.AppendQueryParameter("api-version", "2019-11-01");
}

Azure::Core::Credentials::AccessToken AzureArcManagedIdentitySource::GetToken(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context) const
{
  std::string scopesStr;
  {
    auto const& scopes = tokenRequestContext.Scopes;
    if (!scopes.empty())
    {
      scopesStr = TokenCredentialImpl::FormatScopes(scopes, true);
    }
  }

  auto const createRequest = [&]() {
    using Azure::Core::Http::HttpMethod;
    using Azure::Core::Http::Request;

    auto request = std::make_unique<TokenRequest>(Request(HttpMethod::Get, m_url));
    {
      auto& httpRequest = request->HttpRequest;
      httpRequest.SetHeader("Metadata", "true");

      if (!scopesStr.empty())
      {
        httpRequest.GetUrl().AppendQueryParameter("resource", scopesStr);
      }
    }

    return request;
  };

  // TokenCache::GetToken() and TokenCredentialImpl::GetToken() can only use the lambda argument
  // when they are being executed. They are not supposed to keep a reference to lambda argument to
  // call it later. Therefore, any capture made here will outlive the possible time frame when the
  // lambda might get called.
  return m_tokenCache.GetToken(scopesStr, tokenRequestContext.MinimumExpiration, [&]() {
    return TokenCredentialImpl::GetToken(
        context,
        createRequest,
        [&](auto const statusCode, auto const& response) -> std::unique_ptr<TokenRequest> {
          using Core::Credentials::AuthenticationException;
          using Core::Http::HttpStatusCode;

          if (statusCode != HttpStatusCode::Unauthorized)
          {
            return nullptr;
          }

          auto const& headers = response.GetHeaders();
          auto authHeader = headers.find("WWW-Authenticate");
          if (authHeader == headers.end())
          {
            throw AuthenticationException(
                "Did not receive expected WWW-Authenticate header "
                "in the response from Azure Arc Managed Identity Endpoint.");
          }

          constexpr auto ChallengeValueSeparator = '=';
          auto const& challenge = authHeader->second;
          auto eq = challenge.find(ChallengeValueSeparator);
          if (eq == std::string::npos
              || challenge.find(ChallengeValueSeparator, eq + 1) != std::string::npos)
          {
            throw AuthenticationException(
                "The WWW-Authenticate header in the response from Azure Arc "
                "Managed Identity Endpoint did not match the expected format.");
          }

          auto request = createRequest();
          std::ifstream secretFile(challenge.substr(eq + 1));
          request->HttpRequest.SetHeader(
              "Authorization",
              "Basic "
                  + std::string(
                      std::istreambuf_iterator<char>(secretFile),
                      std::istreambuf_iterator<char>()));

          return request;
        });
  });
}

std::unique_ptr<ManagedIdentitySource> ImdsManagedIdentitySource::Create(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
{
  auto const logLevel = Logger::Level::Informational;
  if (Log::ShouldWrite(logLevel))
  {
    Log::Write(
        logLevel,
        IdentityPrefix + CredPrefix + " will be created"
            + WithSourceMessage("Azure Instance Metadata Service")
            + ".\nSuccessful creation does not guarantee further successful token retrieval.");
  }

  return std::unique_ptr<ManagedIdentitySource>(new ImdsManagedIdentitySource(clientId, options));
}

ImdsManagedIdentitySource::ImdsManagedIdentitySource(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : ManagedIdentitySource(clientId, std::string(), options),
      m_request(
          Azure::Core::Http::HttpMethod::Get,
          Azure::Core::Url("http://169.254.169.254/metadata/identity/oauth2/token"))
{
  {
    using Azure::Core::Url;
    auto& url = m_request.GetUrl();

    url.AppendQueryParameter("api-version", "2018-02-01");

    if (!clientId.empty())
    {
      url.AppendQueryParameter("client_id", clientId);
    }
  }

  m_request.SetHeader("Metadata", "true");
}

Azure::Core::Credentials::AccessToken ImdsManagedIdentitySource::GetToken(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context) const
{
  std::string scopesStr;
  {
    auto const& scopes = tokenRequestContext.Scopes;
    if (!scopes.empty())
    {
      scopesStr = TokenCredentialImpl::FormatScopes(scopes, true);
    }
  }

  // TokenCache::GetToken() and TokenCredentialImpl::GetToken() can only use the lambda argument
  // when they are being executed. They are not supposed to keep a reference to lambda argument to
  // call it later. Therefore, any capture made here will outlive the possible time frame when the
  // lambda might get called.
  return m_tokenCache.GetToken(scopesStr, tokenRequestContext.MinimumExpiration, [&]() {
    return TokenCredentialImpl::GetToken(context, [&]() {
      auto request = std::make_unique<TokenRequest>(m_request);

      if (!scopesStr.empty())
      {
        request->HttpRequest.GetUrl().AppendQueryParameter("resource", scopesStr);
      }

      return request;
    });
  });
}
