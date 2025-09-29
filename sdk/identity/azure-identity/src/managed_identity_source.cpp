// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/managed_identity_source.hpp"

#include "private/identity_log.hpp"

#include <azure/core/http/transport.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/core/platform.hpp>

#include <chrono>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>

#include <sys/stat.h> // for stat() used to check file size

using namespace Azure::Identity::_detail;

using Azure::Core::_internal::Environment;
using Azure::Identity::_detail::IdentityLog;

namespace {
// First request for IMDS should not be taking tens of seconds - if IMDS is unavailable, we should
// fail fast. Among other reasons, this improves user experience when ManagedIdentityCredential is
// part of DefaultAzureCredential. Especially given that all the service credentials are earlier in
// the chain than the developer tool credentials, if ManagedIdentityCredential makes a request which
// takes 30 seconds to time out (host is not available), plus we make 3 retries of that request, and
// all that to figure out that IMDS is not available before moving on to AzureCliCredential, it will
// significantly worsen user experience when using DAC. Therefore, we need the timeout below (plus
// we have logic to not retry that request).
constexpr std::chrono::milliseconds ImdsFirstRequestConnectionTimeout = std::chrono::seconds{1};

std::string WithSourceAndClientIdMessage(std::string const& credSource, std::string const& clientId)
{
  std::string result = " with " + credSource + " source";
  if (!clientId.empty())
  {
    result += " and Client ID '" + clientId + '\'';
  }

  return result;
}

void PrintEnvNotSetUpMessage(std::string const& credName, std::string const& credSource)
{
  IdentityLog::Write(
      IdentityLog::Level::Verbose,
      credName + ": Environment is not set up for the credential to be created"
          + WithSourceAndClientIdMessage(credSource, {}) + '.');
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

constexpr off_t MaximumAzureArcKeySize = 4096;

#if defined(AZ_PLATFORM_WINDOWS)
constexpr char DirectorySeparator = '\\';
#else
static constexpr char DirectorySeparator = '/';
#endif

// Validates that a given Azure Arc MSI file path is valid for use.
// The specified file must:
// - be in the expected directory for the OS
// - have a .key extension
// - contain at most 4096 bytes
void ValidateArcKeyFile(std::string const& fileName)
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
    std::string const& credSource,
    std::string const& clientId)
{
  using Azure::Core::Url;
  using Azure::Core::Credentials::AuthenticationException;

  try
  {
    auto const endpointUrl = Url(url);

    IdentityLog::Write(
        IdentityLog::Level::Informational,
        credName + " will be created" + WithSourceAndClientIdMessage(credSource, clientId) + '.');

    return endpointUrl;
  }
  catch (std::invalid_argument const&)
  {
  }
  catch (std::out_of_range const&)
  {
  }

  auto const errorMessage = credName + WithSourceAndClientIdMessage(credSource, {})
      + ": Failed to create: The environment variable \'" + envVarName
      + "\' contains an invalid URL.";

  IdentityLog::Write(IdentityLog::Level::Warning, errorMessage);
  throw AuthenticationException(errorMessage);
}

template <typename T>
std::unique_ptr<ManagedIdentitySource> AppServiceManagedIdentitySource::Create(
    std::string const& credName,
    std::string const& clientId,
    std::string const& objectId,
    std::string const& resourceId,
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
        objectId,
        resourceId,
        options,
        ParseEndpointUrl(credName, msiEndpoint, endpointVarName, credSource, clientId),
        msiSecret));
  }

  PrintEnvNotSetUpMessage(credName, credSource);
  return nullptr;
}

AppServiceManagedIdentitySource::AppServiceManagedIdentitySource(
    std::string const& clientId,
    std::string const& objectId,
    std::string const& resourceId,
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

    // Only one of clientId, objectId, or resourceId will be set to a non-empty value.
    // AppService uses mi_res_id, and not msi_res_id:
    // https://learn.microsoft.com/azure/app-service/overview-managed-identity?tabs=portal%2Chttp#rest-endpoint-reference
    // Based on the App Service documentation, using principal_id for the query parameter name here
    // instead of object_id (which is used as an alias).
    if (!clientId.empty())
    {
      url.AppendQueryParameter(clientIdHeaderName, clientId);
    }
    else if (!objectId.empty())
    {
      url.AppendQueryParameter("principal_id", objectId);
    }
    else if (!resourceId.empty())
    {
      url.AppendQueryParameter("mi_res_id", resourceId);
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
    std::string const& objectId,
    std::string const& resourceId,
    bool useProbeRequest,
    Core::Credentials::TokenCredentialOptions const& options)
{
  static_cast<void>(useProbeRequest);
  return AppServiceManagedIdentitySource::Create<AppServiceV2017ManagedIdentitySource>(
      credName, clientId, objectId, resourceId, options, "MSI_ENDPOINT", "MSI_SECRET", "2017");
}

std::unique_ptr<ManagedIdentitySource> AppServiceV2019ManagedIdentitySource::Create(
    std::string const& credName,
    std::string const& clientId,
    std::string const& objectId,
    std::string const& resourceId,
    bool useProbeRequest,
    Core::Credentials::TokenCredentialOptions const& options)
{
  static_cast<void>(useProbeRequest);
  return AppServiceManagedIdentitySource::Create<AppServiceV2019ManagedIdentitySource>(
      credName,
      clientId,
      objectId,
      resourceId,
      options,
      "IDENTITY_ENDPOINT",
      "IDENTITY_HEADER",
      "2019");
}

std::unique_ptr<ManagedIdentitySource> CloudShellManagedIdentitySource::Create(
    std::string const& credName,
    std::string const& clientId,
    std::string const& objectId,
    std::string const& resourceId,
    bool useProbeRequest,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
{
  static_cast<void>(useProbeRequest);
  using Azure::Core::Credentials::AuthenticationException;

  constexpr auto EndpointVarName = "MSI_ENDPOINT";
  auto msiEndpoint = Environment::GetVariable(EndpointVarName);

  std::string const credSource = "Cloud Shell";

  if (!msiEndpoint.empty())
  {
    if (!clientId.empty() || !objectId.empty() || !resourceId.empty())
    {
      throw AuthenticationException(
          "User-assigned managed identities are not supported in Cloud Shell environments. Omit "
          "the clientId, objectId, or resourceId when constructing the ManagedIdentityCredential.");
    }

    return std::unique_ptr<ManagedIdentitySource>(new CloudShellManagedIdentitySource(
        clientId,
        options,
        ParseEndpointUrl(credName, msiEndpoint, EndpointVarName, credSource, clientId)));
  }

  PrintEnvNotSetUpMessage(credName, credSource);
  return nullptr;
}

CloudShellManagedIdentitySource::CloudShellManagedIdentitySource(
    std::string const& clientId,
    Azure::Core::Credentials::TokenCredentialOptions const& options,
    Azure::Core::Url endpointUrl)
    : ManagedIdentitySource(clientId, endpointUrl.GetHost(), options), m_url(std::move(endpointUrl))
{
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
      }

      auto request = std::make_unique<TokenRequest>(HttpMethod::Post, m_url, resource);
      request->HttpRequest.SetHeader("Metadata", "true");

      return request;
    });
  });
}

std::unique_ptr<ManagedIdentitySource> AzureArcManagedIdentitySource::Create(
    std::string const& credName,
    std::string const& clientId,
    std::string const& objectId,
    std::string const& resourceId,
    bool useProbeRequest,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
{
  static_cast<void>(useProbeRequest);
  using Azure::Core::Credentials::AuthenticationException;

  constexpr auto EndpointVarName = "IDENTITY_ENDPOINT";
  auto identityEndpoint = Environment::GetVariable(EndpointVarName);

  std::string const credSource = "Azure Arc";

  if (identityEndpoint.empty() || Environment::GetVariable("IMDS_ENDPOINT").empty())
  {
    PrintEnvNotSetUpMessage(credName, credSource);
    return nullptr;
  }

  if (!clientId.empty() || !objectId.empty() || !resourceId.empty())
  {
    throw AuthenticationException(
        "User assigned identity is not supported by the Azure Arc Managed Identity Endpoint. "
        "To authenticate with the system assigned identity, omit the client, object, or resource "
        "ID when constructing the ManagedIdentityCredential.");
  }

  return std::unique_ptr<ManagedIdentitySource>(new AzureArcManagedIdentitySource(
      options,
      ParseEndpointUrl(credName, identityEndpoint, EndpointVarName, credSource, clientId)));
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
    std::string const& objectId,
    std::string const& resourceId,
    bool useProbeRequest,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
{
  const std::string ImdsName = "Azure Instance Metadata Service";

  IdentityLog::Write(
      IdentityLog::Level::Informational,
      credName + " will be created" + WithSourceAndClientIdMessage(ImdsName, clientId)
          + ".\nSuccessful creation does not guarantee further successful token retrieval.");

  // https://learn.microsoft.com/azure/virtual-machines/instance-metadata-service
  // IMDS is a REST API that's available at a well-known, non-routable IP address
  // (169.254.169.254). You can only access it from within the VM. Communication between the VM
  // and IMDS never leaves the host.
  // 'AZURE_POD_IDENTITY_AUTHORITY_HOST' environment variable allows user to override the
  // authority host for IMDS. This is consistent with other language SDKs.
  Core::Url imdsUrl{"http://169.254.169.254"};
  constexpr auto ImdsEndpointEnvVarName = "AZURE_POD_IDENTITY_AUTHORITY_HOST";
  const auto imdsEndpointEnvVarValue = Environment::GetVariable(ImdsEndpointEnvVarName);
  if (!imdsEndpointEnvVarValue.empty())
  {
    IdentityLog::Write(
        IdentityLog::Level::Verbose,
        credName + WithSourceAndClientIdMessage(ImdsName, {}) + ": '" + ImdsEndpointEnvVarName
            + "' environment variable is set, so customized authority host ('"
            + imdsEndpointEnvVarValue + "') will be used.");

    imdsUrl = Core::Url{imdsEndpointEnvVarValue};
  }
  imdsUrl.SetPath("metadata/identity/oauth2/token");

  return std::unique_ptr<ManagedIdentitySource>(new ImdsManagedIdentitySource(
      clientId, objectId, resourceId, imdsUrl, options, useProbeRequest));
}

ImdsManagedIdentitySource::ImdsManagedIdentitySource(
    std::string const& clientId,
    std::string const& objectId,
    std::string const& resourceId,
    Azure::Core::Url const& imdsUrl,
    bool useProbeRequest,
    Azure::Core::Credentials::TokenCredentialOptions const& options)
    : ManagedIdentitySource(clientId, std::string(), options),
      m_request(Azure::Core::Http::HttpMethod::Get, imdsUrl)
{
  {
    auto& url = m_request.GetUrl();

    url.AppendQueryParameter("api-version", "2018-02-01");

    // Only one of clientId, objectId, or resourceId will be set to a non-empty value.
    // IMDS uses msi_res_id, and not mi_res_id:
    // https://learn.microsoft.com/entra/identity/managed-identities-azure-resources/how-to-use-vm-token#get-a-token-using-http
    if (!clientId.empty())
    {
      url.AppendQueryParameter("client_id", clientId);
    }
    else if (!objectId.empty())
    {
      url.AppendQueryParameter("object_id", objectId);
    }
    else if (!resourceId.empty())
    {
      url.AppendQueryParameter("msi_res_id", resourceId);
    }
  }

  m_request.SetHeader("Metadata", "true");

  Core::Credentials::TokenCredentialOptions firstRequestOptions = options;
  firstRequestOptions.Retry.MaxRetries = 0;
  m_firstRequestPipeline = std::make_unique<TokenCredentialImpl>(firstRequestOptions);
  m_firstRequestSucceeded = !useProbeRequest;
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
    std::function<std::unique_ptr<TokenRequest>()> const& createRequest = [&]() {
      auto request = std::make_unique<TokenRequest>(m_request);

      if (!scopesStr.empty())
      {
        request->HttpRequest.GetUrl().AppendQueryParameter("resource", scopesStr);
      }

      return request;
    };

    if (!m_firstRequestSucceeded)
    {
      std::unique_lock<std::mutex> lock(m_firstRequestMutex);
      if (!m_firstRequestSucceeded)
      {
        const auto token = m_firstRequestPipeline->GetToken(
            context.WithValue(
                Core::Http::_internal::HttpConnectionTimeout, ImdsFirstRequestConnectionTimeout),
            true,
            createRequest);

        m_firstRequestSucceeded = true;

        lock.unlock();
        m_firstRequestPipeline.reset();

        return token;
      }
    }

    return TokenCredentialImpl::GetToken(context, true, createRequest);
  });
}
