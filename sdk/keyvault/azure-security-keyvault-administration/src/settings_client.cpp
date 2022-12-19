#include "azure/keyvault/shared/keyvault_shared.hpp"
#include "private/administration_constants.hpp"
#include "private/keyvault_settings_common_request.hpp"
#include "private/package_version.hpp"
#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>
#include <azure/keyvault/administration/settings_client.hpp>
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
std::unique_ptr<Azure::Core::Http::RawResponse> KeyVaultSettingsClient::SendRequest(
    Azure::Core::Http::Request& request,
    Azure::Core::Context const& context) const
{
  return Azure::Security::KeyVault::_detail::KeyVaultSettingsCommonRequest::SendRequest(
      *m_pipeline, request, context);
}

Azure::Core::Http::Request KeyVaultSettingsClient::CreateRequest(
    Azure::Core::Http::HttpMethod method,
    std::vector<std::string> const& path,
    Azure::Core::IO::BodyStream* content) const
{
  return Azure::Security::KeyVault::_detail::KeyVaultSettingsCommonRequest::CreateRequest(
      m_vaultUrl, m_apiVersion, method, path, content);
}

Azure::Core::Http::Request KeyVaultSettingsClient::ContinuationTokenRequest(
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

KeyVaultSettingsClient::KeyVaultSettingsClient(
    std::string const& vaultUrl,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    KeyVaultSettingsClientOptions options)
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


  Azure::Response<Setting> KeyVaultSettingsClient::UpdateSetting(
    const UpdateSettingOptions& options,
    const Azure::Core::Context& context)
{
  std::string jsonBody;
  {
    auto jsonRoot = Azure::Core::Json::_internal::json::object();
    jsonRoot["value"] = options.parameters.value;
    jsonBody = jsonRoot.dump();
  }
  Azure::Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<const uint8_t*>(jsonBody.data()), jsonBody.length());
  auto url = m_vaultUrl;
      url.SetQueryParameters({{"apiVersion", "7.4-preview.1"}});
  auto request
      = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Patch, url, &requestBody);
  request.SetHeader("Content-Type", "application/json");
  request.SetHeader("Content-Length", std::to_string(requestBody.Length()));
  request.GetUrl().AppendQueryParameter("api-version", "7.4-preview.1");
  auto pRawResponse = m_pipeline->Send(request, context);
  auto httpStatusCode = pRawResponse->GetStatusCode();
  if (httpStatusCode != Azure::Core::Http::HttpStatusCode::Ok)
  {
    throw Azure::Core::RequestFailedException(pRawResponse);
  }
  Setting response;
  {
    const auto& responseBody = pRawResponse->GetBody();
    auto jsonRoot
        = Azure::Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());
    response.name = jsonRoot["name"].get<std::string>();
    response.value = jsonRoot["value"].get<std::string>();
    if (jsonRoot.count("type") != 0)
    {
      response.type = SettingTypeEnum(jsonRoot["type"].get<std::string>());
    }
  }
  return Azure::Response<Setting>(std::move(response), std::move(pRawResponse));
}

Azure::Response<Setting> KeyVaultSettingsClient::GetSetting(const Azure::Core::Context& context) const
{
  auto url = m_vaultUrl;
  url.SetQueryParameters({{"apiVersion", "7.4-preview.1"}});
  auto request = Azure::Core::Http::Request(Azure::Core::Http::HttpMethod::Get, url);
  request.GetUrl().AppendQueryParameter("api-version", "7.4-preview.1");
  auto pRawResponse = m_pipeline->Send(request, context);
  auto httpStatusCode = pRawResponse->GetStatusCode();
  if (httpStatusCode != Azure::Core::Http::HttpStatusCode::Ok)
  {
    throw Azure::Core::RequestFailedException(pRawResponse);
  }
  Setting response;
  {
    const auto& responseBody = pRawResponse->GetBody();
    auto jsonRoot
        = Azure::Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());
    response.name = jsonRoot["name"].get<std::string>();
    response.value = jsonRoot["value"].get<std::string>();
    if (jsonRoot.count("type") != 0)
    {
      response.type = SettingTypeEnum(jsonRoot["type"].get<std::string>());
    }
  }
  return Azure::Response<Setting>(std::move(response), std::move(pRawResponse));
}

Azure::Response<SettingsListResult> KeyVaultSettingsClient::GetSettings(
    const Azure::Core::Context& context) const
{
  auto request = CreateRequest(Azure::Core::Http::HttpMethod::Get, {"settings"});
  auto pRawResponse = m_pipeline->Send(request, context);
  auto httpStatusCode = pRawResponse->GetStatusCode();
  if (httpStatusCode != Azure::Core::Http::HttpStatusCode::Ok)
  {
    throw Azure::Core::RequestFailedException(pRawResponse);
  }
  SettingsListResult response;
  {
    const auto& responseBody = pRawResponse->GetBody();
    auto jsonRoot
        = Azure::Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());
    for (const auto& var0 : jsonRoot.count("value") != 0 && jsonRoot["value"].is_array()
             ? jsonRoot["value"]
             : Azure::Core::Json::_internal::json::array())
    {
      Setting vectorElement2;
      vectorElement2.name = var0["name"].get<std::string>();
      vectorElement2.value = var0["value"].get<std::string>();
      if (var0.count("type") != 0)
      {
        vectorElement2.type = SettingTypeEnum(var0["type"].get<std::string>());
      }
      response.value.emplace_back(std::move(vectorElement2));
    }
  }
  return Azure::Response<SettingsListResult>(std::move(response), std::move(pRawResponse));
}