// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/managed_identity_source.hpp"

#include "private/environment.hpp"

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>

using namespace Azure::Identity::_detail;

Azure::Core::Url ManagedIdentitySource::ParseEndpointUrl(
    std::string const& url,
    char const* envVarName)
{
  using Azure::Core::Url;
  using Azure::Core::Credentials::AuthenticationException;

  try
  {
    return Url(url);
  }
  catch (std::invalid_argument const&)
  {
  }
  catch (std::out_of_range const&)
  {
  }

  throw AuthenticationException(
      std::string("The environment variable ") + envVarName + " contains an invalid URL.");
}

std::unique_ptr<ManagedIdentitySource> AppServiceManagedIdentitySource::Create(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
{
  constexpr auto EndpointVarName = "MSI_ENDPOINT";
  auto msiEndpoint = Environment::GetVariable(EndpointVarName);
  auto msiSecret = Environment::GetVariable("MSI_SECRET");

  return (msiEndpoint.empty() || msiSecret.empty())
      ? nullptr
      : std::unique_ptr<ManagedIdentitySource>(new AppServiceManagedIdentitySource(
          clientId, options, ParseEndpointUrl(msiEndpoint, EndpointVarName), msiSecret));
}

AppServiceManagedIdentitySource::AppServiceManagedIdentitySource(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options,
    Azure::Core::Url endpointUrl,
    std::string const& secret)
    : ManagedIdentitySource(options),
      m_request(Azure::Core::Http::HttpMethod::Get, std::move(endpointUrl))
{
  {
    using Azure::Core::Url;
    auto& url = m_request.GetUrl();

    url.AppendQueryParameter("api-version", "2017-09-01");

    if (!clientId.empty())
    {
      url.AppendQueryParameter("clientid", clientId);
    }
  }

  m_request.SetHeader("secret", secret);
}

Azure::Core::Credentials::AccessToken AppServiceManagedIdentitySource::GetToken(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context) const
{
  return TokenCredentialImpl::GetToken(context, [&]() {
    auto request = std::make_unique<TokenRequest>(m_request);
    {
      auto const& scopes = tokenRequestContext.Scopes;
      if (!scopes.empty())
      {
        request->HttpRequest.GetUrl().AppendQueryParameter("resource", FormatScopes(scopes, true));
      }
    }

    return request;
  });
}

std::unique_ptr<ManagedIdentitySource> CloudShellManagedIdentitySource::Create(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
{
  constexpr auto EndpointVarName = "MSI_ENDPOINT";
  auto msiEndpoint = Environment::GetVariable(EndpointVarName);

  return (msiEndpoint.empty())
      ? nullptr
      : std::unique_ptr<ManagedIdentitySource>(new CloudShellManagedIdentitySource(
          clientId, options, ParseEndpointUrl(msiEndpoint, EndpointVarName)));
}

CloudShellManagedIdentitySource::CloudShellManagedIdentitySource(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options,
    Azure::Core::Url endpointUrl)
    : ManagedIdentitySource(options), m_url(std::move(endpointUrl))
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
  return TokenCredentialImpl::GetToken(context, [&]() {
    using Azure::Core::Url;
    using Azure::Core::Http::HttpMethod;

    std::string resource;
    {
      auto const& scopes = tokenRequestContext.Scopes;
      if (!scopes.empty())
      {
        resource = "resource=" + FormatScopes(scopes, true);
        if (!m_body.empty())
        {
          resource += "&";
        }
      }
    }

    auto request = std::make_unique<TokenRequest>(HttpMethod::Post, m_url, resource + m_body);
    request->HttpRequest.SetHeader("Metadata", "true");

    return request;
  });
}

std::unique_ptr<ManagedIdentitySource> AzureArcManagedIdentitySource::Create(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
{
  using Azure::Core::Credentials::AuthenticationException;

  constexpr auto EndpointVarName = "IDENTITY_ENDPOINT";
  auto identityEndpoint = Environment::GetVariable(EndpointVarName);

  if (identityEndpoint.empty() || Environment::GetVariable("IMDS_ENDPOINT").empty())
  {
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
      options, ParseEndpointUrl(identityEndpoint, EndpointVarName)));
}

AzureArcManagedIdentitySource::AzureArcManagedIdentitySource(
    Azure::Core::Credentials::TokenCredentialOptions const& options,
    Azure::Core::Url endpointUrl)
    : ManagedIdentitySource(options), m_url(std::move(endpointUrl))
{

  m_url.AppendQueryParameter("api-version", "2019-11-01");
}

Azure::Core::Credentials::AccessToken AzureArcManagedIdentitySource::GetToken(
    Azure::Core::Credentials::TokenRequestContext const& tokenRequestContext,
    Azure::Core::Context const& context) const
{
  auto const createRequest = [&]() {
    using Azure::Core::Http::HttpMethod;
    using Azure::Core::Http::Request;

    auto request = std::make_unique<TokenRequest>(Request(HttpMethod::Get, m_url));
    {
      auto& httpRequest = request->HttpRequest;
      httpRequest.SetHeader("Metadata", "true");
      {
        auto const& scopes = tokenRequestContext.Scopes;
        if (!scopes.empty())
        {
          httpRequest.GetUrl().AppendQueryParameter("resource", FormatScopes(scopes, true));
        }
      }
    }

    return request;
  };

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
          throw AuthenticationException("The WWW-Authenticate header in the response from Azure "
                                        "Arc Managed Identity Endpoint "
                                        "did not match the expected format.");
        }

        auto request = createRequest();
        std::ifstream secretFile(challenge.substr(eq + 1));
        request->HttpRequest.SetHeader(
            "Authorization",
            "Basic "
                + std::string(
                    std::istreambuf_iterator<char>(secretFile), std::istreambuf_iterator<char>()));

        return request;
      });
}

std::unique_ptr<ManagedIdentitySource> ImdsManagedIdentitySource::Create(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
{
  return std::unique_ptr<ManagedIdentitySource>(new ImdsManagedIdentitySource(clientId, options));
}

ImdsManagedIdentitySource::ImdsManagedIdentitySource(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : ManagedIdentitySource(options),
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
  return TokenCredentialImpl::GetToken(context, [&]() {
    auto request = std::make_unique<TokenRequest>(m_request);
    {
      auto const& scopes = tokenRequestContext.Scopes;
      if (!scopes.empty())
      {
        request->HttpRequest.GetUrl().AppendQueryParameter("resource", FormatScopes(scopes, true));
      }
    }

    return request;
  });
}
