#include "azure/keyvault/shared/keyvault_shared.hpp"
#include "private/administration_constants.hpp"
#include "private/keyvault_administration_common_request.hpp"
#include "private/package_version.hpp"
#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>
#include <azure/keyvault/administration/administration_client.hpp>
#include <memory>

using namespace Azure::Security::KeyVault::Administration;
using namespace Azure::Security::KeyVault;
using namespace Azure::Security::KeyVault::_detail;

using namespace Azure;
using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Core::Http::_internal;

std::unique_ptr<Azure::Core::Http::RawResponse> AdministrationClient::SendRequest(
    Azure::Core::Http::Request& request,
    Azure::Core::Context const& context) const
{
  return Azure::Security::KeyVault::_detail::KeyVaultAdministrationCommonRequest::SendRequest(
      *m_pipeline, request, context);
}

Azure::Core::Http::Request AdministrationClient::CreateRequest(
    Azure::Core::Http::HttpMethod method,
    std::vector<std::string> const& path,
    Azure::Core::IO::BodyStream* content) const
{
  return Azure::Security::KeyVault::_detail::KeyVaultAdministrationCommonRequest::CreateRequest(
      m_vaultUrl, m_apiVersion, method, path, content);
}

Azure::Core::Http::Request AdministrationClient::ContinuationTokenRequest(
    std::vector<std::string> const& path,
    const Azure::Nullable<std::string>& NextPageToken) const
{
  if (NextPageToken)
  {
    // Using a continuation token requires to send the request to the continuation token URL instead
    // of the default URL which is used only for the first page.
    Azure::Core::Url nextPageUrl(NextPageToken.Value());
    return Azure::Core::Http::Request(HttpMethod::Get, nextPageUrl);
  }
  return CreateRequest(HttpMethod::Get, path);
}

AdministrationClient::AdministrationClient(
    std::string const& vaultUrl,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    AdministrationClientOptions options)
    : m_vaultUrl(vaultUrl), m_apiVersion(options.ApiVersion)
{
  auto apiVersion = options.ApiVersion;

  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Credentials::TokenRequestContext tokenContext;
    tokenContext.Scopes = {_internal::UrlScope::GetScopeFromUrl(m_vaultUrl)};

    perRetrypolicies.emplace_back(
        std::make_unique<BearerTokenAuthenticationPolicy>(credential, std::move(tokenContext)));
  }
  std::vector<std::unique_ptr<HttpPolicy>> perCallpolicies;

  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      options,
      _detail::KeyVaultServicePackageName,
      _detail::PackageVersion::ToString(),
      std::move(perRetrypolicies),
      std::move(perCallpolicies));
}