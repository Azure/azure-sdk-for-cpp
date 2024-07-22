// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/managed_identity_source.hpp"

#include "private/identity_log.hpp"

#include <azure/core/internal/environment.hpp>
#include <azure/core/platform.hpp>

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>

#include <sys/stat.h> // for stat() used to check file size

using namespace Azure::Identity::_detail;

using Azure::Core::_internal::Environment;
using Azure::Identity::_detail::IdentityLog;

namespace {
std::string WithSourceMessage(std::string const& credSource)
{
  return " with " + credSource + " source";
}

void PrintEnvNotSetUpMessage(std::string const& credName, std::string const& credSource)
{
  IdentityLog::Write(
      IdentityLog::Level::Verbose,
      credName + ": Environment is not set up for the credential to be created"
          + WithSourceMessage(credSource) + '.');
}

// ExpectedArcKeyDirectory returns the directory expected to contain Azure Arc keys.
std::string ExpectedArcKeyDirectory()
{
  using Azure::Core::Credentials::AuthenticationException;

#if defined(AZ_PLATFORM_LINUX)
  return "/var/opt/azcmagent/tokens";
#elif defined(AZ_PLATFORM_WINDOWS)
  const std::string programDataPath{
      Azure::Core::_internal::Environment::GetVariable("ProgramData")};
  if (programDataPath.empty())
  {
    throw AuthenticationException("Unable to get ProgramData folder path.");
  }
  return programDataPath + "\\AzureConnectedMachineAgent\\Tokens";
#else
  throw AuthenticationException("Unsupported OS. Arc supports only Linux and Windows.");
#endif
}

static constexpr off_t MaximumAzureArcKeySize = 4096;

#if defined(AZ_PLATFORM_WINDOWS)
static constexpr char DirectorySeparator = '\\';
#else
static constexpr char DirectorySeparator = '/';
#endif

// Validates that a given Azure Arc MSI file path is valid for use.
// The specified file must:
// - be in the expected directory for the OS
// - have a .key extension
// - contain at most 4096 bytes
void ValidateArcKeyFile(std::string fileName)
{
  using Azure::Core::Credentials::AuthenticationException;

  std::string directory;
  const size_t lastSlashIndex = fileName.rfind(DirectorySeparator);
  if (std::string::npos != lastSlashIndex)
  {
    directory = fileName.substr(0, lastSlashIndex);
  }
  if (directory != ExpectedArcKeyDirectory() || fileName.size() < 5
      || fileName.substr(fileName.size() - 4) != ".key")
  {
    throw AuthenticationException(
        "The file specified in the 'WWW-Authenticate' header in the response from Azure Arc "
        "Managed Identity Endpoint has an unexpected file path.");
  }

  struct stat s;
  if (!stat(fileName.c_str(), &s))
  {
    if (s.st_size > MaximumAzureArcKeySize)
    {
      throw AuthenticationException(
          "The file specified in the 'WWW-Authenticate' header in the response from Azure Arc "
          "Managed Identity Endpoint is larger than 4096 bytes.");
    }
  }
  else
  {
    throw AuthenticationException("Failed to get file size for '" + fileName + "'.");
  }
}
} // namespace

Azure::Core::Url ManagedIdentitySource::ParseEndpointUrl(
    std::string const& credName,
    std::string const& url,
    char const* envVarName,
    std::string const& credSource)
{
  using Azure::Core::Url;
  using Azure::Core::Credentials::AuthenticationException;

  try
  {
    auto const endpointUrl = Url(url);

    IdentityLog::Write(
        IdentityLog::Level::Informational,
        credName + " will be created" + WithSourceMessage(credSource) + '.');

    return endpointUrl;
  }
  catch (std::invalid_argument const&)
  {
  }
  catch (std::out_of_range const&)
  {
  }

  auto const errorMessage = credName + WithSourceMessage(credSource)
      + ": Failed to create: The environment variable \'" + envVarName
      + "\' contains an invalid URL.";

  IdentityLog::Write(IdentityLog::Level::Warning, errorMessage);
  throw AuthenticationException(errorMessage);
}

template <typename T>
std::unique_ptr<ManagedIdentitySource> AppServiceManagedIdentitySource::Create(
    std::string const& credName,
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
        clientId,
        options,
        ParseEndpointUrl(credName, msiEndpoint, endpointVarName, credSource),
        msiSecret));
  }

  PrintEnvNotSetUpMessage(credName, credSource);
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
  return m_tokenCache.GetToken(scopesStr, {}, tokenRequestContext.MinimumExpiration, [&]() {
    return TokenCredentialImpl::GetToken(context, true, [&]() {
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
    std::string const& credName,
    std::string const& clientId,
    Core::Credentials::TokenCredentialOptions const& options)
{
  return AppServiceManagedIdentitySource::Create<AppServiceV2017ManagedIdentitySource>(
      credName, clientId, options, "MSI_ENDPOINT", "MSI_SECRET", "2017");
}

std::unique_ptr<ManagedIdentitySource> AppServiceV2019ManagedIdentitySource::Create(
    std::string const& credName,
    std::string const& clientId,
    Core::Credentials::TokenCredentialOptions const& options)
{
  return AppServiceManagedIdentitySource::Create<AppServiceV2019ManagedIdentitySource>(
      credName, clientId, options, "IDENTITY_ENDPOINT", "IDENTITY_HEADER", "2019");
}

std::unique_ptr<ManagedIdentitySource> CloudShellManagedIdentitySource::Create(
    std::string const& credName,
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
{
  constexpr auto EndpointVarName = "MSI_ENDPOINT";
  auto msiEndpoint = Environment::GetVariable(EndpointVarName);

  std::string const CredSource = "Cloud Shell";

  if (!msiEndpoint.empty())
  {
    return std::unique_ptr<ManagedIdentitySource>(new CloudShellManagedIdentitySource(
        clientId, options, ParseEndpointUrl(credName, msiEndpoint, EndpointVarName, CredSource)));
  }

  PrintEnvNotSetUpMessage(credName, CredSource);
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
  return m_tokenCache.GetToken(scopesStr, {}, tokenRequestContext.MinimumExpiration, [&]() {
    return TokenCredentialImpl::GetToken(context, true, [&]() {
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
    std::string const& credName,
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
{
  using Azure::Core::Credentials::AuthenticationException;

  constexpr auto EndpointVarName = "IDENTITY_ENDPOINT";
  auto identityEndpoint = Environment::GetVariable(EndpointVarName);

  std::string const credSource = "Azure Arc";

  if (identityEndpoint.empty() || Environment::GetVariable("IMDS_ENDPOINT").empty())
  {
    PrintEnvNotSetUpMessage(credName, credSource);
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
      options, ParseEndpointUrl(credName, identityEndpoint, EndpointVarName, credSource)));
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
  return m_tokenCache.GetToken(scopesStr, {}, tokenRequestContext.MinimumExpiration, [&]() {
    return TokenCredentialImpl::GetToken(
        context,
        true,
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
                "Did not receive expected 'WWW-Authenticate' header "
                "in the response from Azure Arc Managed Identity Endpoint.");
          }

          constexpr auto ChallengeValueSeparator = '=';
          auto const& challenge = authHeader->second;
          auto eq = challenge.find(ChallengeValueSeparator);
          if (eq == std::string::npos
              || challenge.find(ChallengeValueSeparator, eq + 1) != std::string::npos)
          {
            throw AuthenticationException(
                "The 'WWW-Authenticate' header in the response from Azure Arc "
                "Managed Identity Endpoint did not match the expected format.");
          }

          auto request = createRequest();

          const std::string fileName = challenge.substr(eq + 1);
          ValidateArcKeyFile(fileName);

          std::ifstream secretFile(fileName);
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
    std::string const& credName,
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
{
  IdentityLog::Write(
      IdentityLog::Level::Informational,
      credName + " will be created" + WithSourceMessage("Azure Instance Metadata Service")
          + ".\nSuccessful creation does not guarantee further successful token retrieval.");

  std::string imdsHost = "http://169.254.169.254";
  std::string customImdsHost = Environment::GetVariable("AZURE_IMDS_CUSTOM_AUTHORITY_HOST");
  if (!customImdsHost.empty())
  {
    IdentityLog::Write(
        IdentityLog::Level::Informational, "Custom IMDS host is set to: " + customImdsHost);
    imdsHost = customImdsHost;
  }
  Azure::Core::Url imdsUrl(imdsHost);
  imdsUrl.AppendPath("/metadata/identity/oauth2/token");

  return std::unique_ptr<ManagedIdentitySource>(
      new ImdsManagedIdentitySource(clientId, imdsUrl, options));
}

ImdsManagedIdentitySource::ImdsManagedIdentitySource(
    std::string const& clientId,
    Azure::Core::Url const& imdsUrl,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : ManagedIdentitySource(clientId, std::string(), options),
      m_request(Azure::Core::Http::HttpMethod::Get, Azure::Core::Url(imdsUrl))
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
  return m_tokenCache.GetToken(scopesStr, {}, tokenRequestContext.MinimumExpiration, [&]() {
    return TokenCredentialImpl::GetToken(context, true, [&]() {
      auto request = std::make_unique<TokenRequest>(m_request);

      if (!scopesStr.empty())
      {
        request->HttpRequest.GetUrl().AppendQueryParameter("resource", scopesStr);
      }

      return request;
    });
  });
}
