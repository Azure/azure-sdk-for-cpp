// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/keyvault/shared/keyvault_shared.hpp"
#include "private/administration_constants.hpp"
#include "private/keyvault_settings_common_request.hpp"
#include "private/package_version.hpp"
#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/internal/json/json_optional.hpp>
#include <azure/core/internal/json/json_serializable.hpp>
#include <azure/keyvault/administration/settings_client.hpp>
#include <memory>

using namespace Azure::Security::KeyVault::Administration;
using namespace Azure::Security::KeyVault::Administration::_detail;
using namespace Azure::Security::KeyVault;
using namespace Azure::Security::KeyVault::_detail;

using namespace Azure;
using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Core::Http::_internal;

using namespace Azure::Core::Json::_internal;

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
    std::string const& settingName,
    UpdateSettingOptions const& options,
    const Azure::Core::Context& context) const
{
  std::string jsonBody;
  {
    auto jsonRoot = Azure::Core::Json::_internal::json::object();
    jsonRoot[ValueField] = options.Value;
    jsonBody = jsonRoot.dump();
  }
  Azure::Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<const uint8_t*>(jsonBody.data()), jsonBody.length());

  auto request = CreateRequest(HttpMethod::Patch, {SettingPathName, settingName}, &requestBody);
  auto pRawResponse = m_pipeline->Send(request, context);
  auto httpStatusCode = pRawResponse->GetStatusCode();
  if (httpStatusCode != Azure::Core::Http::HttpStatusCode::Ok)
  {
    throw Azure::Core::RequestFailedException(pRawResponse);
  }
  Setting response = ParseSetting(pRawResponse->GetBody());
  return Azure::Response<Setting>(std::move(response), std::move(pRawResponse));
}

Azure::Response<Setting> KeyVaultSettingsClient::GetSetting(
    std::string const& settingName,
    const Azure::Core::Context& context) const
{
  auto request = CreateRequest(HttpMethod::Get, {SettingPathName, settingName});
  auto pRawResponse = m_pipeline->Send(request, context);
  auto httpStatusCode = pRawResponse->GetStatusCode();
  if (httpStatusCode != Azure::Core::Http::HttpStatusCode::Ok)
  {
    throw Azure::Core::RequestFailedException(pRawResponse);
  }
  Setting response = ParseSetting(pRawResponse->GetBody());
  return Azure::Response<Setting>(std::move(response), std::move(pRawResponse));
}

Azure::Response<SettingsListResult> KeyVaultSettingsClient::GetSettings(
    const Azure::Core::Context& context) const
{
  auto request = CreateRequest(Azure::Core::Http::HttpMethod::Get, {SettingPathName});
  auto pRawResponse = m_pipeline->Send(request, context);

  auto httpStatusCode = pRawResponse->GetStatusCode();
  if (httpStatusCode != Azure::Core::Http::HttpStatusCode::Ok)
  {
    throw Azure::Core::RequestFailedException(pRawResponse);
  }
  SettingsListResult response;
  {
    const auto& responseBody = pRawResponse->GetBody();
    auto jsonRoot = json::parse(responseBody);
    auto settingsArray = jsonRoot[SettingNodeName];
    for (const auto& setting : settingsArray)
    {
      auto const settingString = setting.dump(); 
      Setting parsedSetting = ParseSetting(std::vector<uint8_t>(settingString.begin(), settingString.end()));
      response.Value.emplace_back(std::move(parsedSetting));
    }
  }
  return Azure::Response<SettingsListResult>(std::move(response), std::move(pRawResponse));
}

Setting KeyVaultSettingsClient::ParseSetting(std::vector<uint8_t> const& responseBody) const
{
  Setting response;
  {
    auto jsonRoot
        = Azure::Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());
    response.Name = jsonRoot[NameField].get<std::string>();
    response.Value = jsonRoot[ValueField].get<std::string>();
    if (jsonRoot.count(TypeField) != 0)
    {
      response.Type = SettingTypeEnum(jsonRoot[TypeField].get<std::string>());
    }
  }
  return response;
}