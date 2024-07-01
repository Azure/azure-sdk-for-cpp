// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See License.txt in the project root for license information.
// Code generated by Microsoft (R) AutoRest Code Generator.
// Changes may cause incorrect behavior and will be lost if the code is regenerated.

#include "azure/keyvault/administration/backup_restore_client.hpp"

#include "private/administration_constants.hpp"
#include "private/package_version.hpp"

#include <azure/core/exception.hpp>
#include <azure/core/http/http.hpp>
#include <azure/core/http/http_status_code.hpp>
#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/io/body_stream.hpp>
#include <azure/keyvault/shared/keyvault_challenge_based_auth.hpp>
#include <azure/keyvault/shared/keyvault_shared.hpp>

#include <cstdint>
#include <utility>

using namespace Azure::Security::KeyVault::Administration;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Core::Http::_internal;

BackupRestoreClient::BackupRestoreClient(
    std::string const& vaultUrl,
    std::shared_ptr<Core::Credentials::TokenCredential const> credential,
    BackupRestoreClientOptions options)
    : m_vaultBaseUrl(vaultUrl), m_apiVersion(options.ApiVersion)
{
  std::vector<std::unique_ptr<HttpPolicy>> perRetrypolicies;
  {
    Azure::Core::Credentials::TokenRequestContext tokenContext;
    tokenContext.Scopes = {_internal::UrlScope::GetScopeFromUrl(m_vaultBaseUrl)};

    perRetrypolicies.emplace_back(
        std::make_unique<_internal::KeyVaultChallengeBasedAuthenticationPolicy>(
            credential, std::move(tokenContext)));
  }
  std::vector<std::unique_ptr<HttpPolicy>> perCallpolicies;

  m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
      options,
      _detail::KeyVaultServicePackageName,
      _detail::PackageVersion::ToString(),
      std::move(perRetrypolicies),
      std::move(perCallpolicies));
}

Azure::Response<FullBackupOperation> BackupRestoreClient::FullBackup(
    Azure::Core::Url const& blobContainerUrl,
    SasTokenParameter const& sasToken,
    Core::Context const& context)
{
  auto url = m_vaultBaseUrl;
  url.AppendPath("backup");

  url.SetQueryParameters({{"api-version", m_apiVersion}});

  std::string jsonBody;
  {
    auto jsonRoot = Core::Json::_internal::json::object();

    jsonRoot["storageResourceUri"] = blobContainerUrl.GetAbsoluteUrl();

    if (sasToken.Token.HasValue())
    {
      jsonRoot["token"] = sasToken.Token.Value();
    }

    if (sasToken.UseManagedIdentity.HasValue())
    {
      jsonRoot["useManagedIdentity"] = sasToken.UseManagedIdentity.Value();
    }

    jsonBody = jsonRoot.dump();
  }

  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<std::uint8_t const*>(jsonBody.data()), jsonBody.length());

  Core::Http::Request request(Core::Http::HttpMethod::Post, url, &requestBody);

  request.SetHeader(HttpShared::ContentType, HttpShared::ApplicationJson);
  request.SetHeader(HttpShared::Accept, HttpShared::ApplicationJson);
  request.SetHeader("Content-Length", std::to_string(requestBody.Length()));

  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  FullBackupOperation response{};
  {
    auto const& responseBody = rawResponse->GetBody();
    if (responseBody.size() > 0)
    {
      auto const jsonRoot
          = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());

      response.Status = jsonRoot["status"].get<std::string>();

      if (jsonRoot.contains("statusDetails") && !jsonRoot["statusDetails"].is_null())
      {
        response.StatusDetails = jsonRoot["statusDetails"].get<std::string>();
      }

      response.StartTime = Core::_internal::PosixTimeConverter::PosixTimeToDateTime(
          jsonRoot["startTime"].is_string() ? std::stoll(jsonRoot["startTime"].get<std::string>())
                                            : jsonRoot["startTime"].get<std::int64_t>());

      if (jsonRoot.contains("endTime") && !jsonRoot["endTime"].is_null())
      {
        response.EndTime = Core::_internal::PosixTimeConverter::PosixTimeToDateTime(
            jsonRoot["endTime"].is_string() ? std::stoll(jsonRoot["endTime"].get<std::string>())
                                            : jsonRoot["endTime"].get<std::int64_t>());
      }

      response.JobId = jsonRoot["jobId"].get<std::string>();

      if (jsonRoot.contains("azureStorageBlobContainerUri")
          && !jsonRoot["azureStorageBlobContainerUri"].is_null())
      {
        response.AzureStorageBlobContainerUri
            = jsonRoot["azureStorageBlobContainerUri"].get<std::string>();
      }

      if (jsonRoot.contains("error") && !jsonRoot["error"].is_null())
      {
        response.Error = DeserializeKeyVaultServiceError(jsonRoot["error"]);
      }
    }
  }

  return Response<FullBackupOperation>(std::move(response), std::move(rawResponse));
}

Azure::Response<FullBackupOperation> BackupRestoreClient::FullBackupStatus(
    std::string const& jobId,
    Core::Context const& context)
{
  auto url = m_vaultBaseUrl;
  url.AppendPath("backup");
  url.AppendPath(!jobId.empty() ? Core::Url::Encode(jobId) : "null");
  url.AppendPath("pending");

  url.SetQueryParameters({{"api-version", m_apiVersion}});

  Core::Http::Request request(Core::Http::HttpMethod::Get, url);
  request.SetHeader(HttpShared::ContentType, HttpShared::ApplicationJson);
  request.SetHeader(HttpShared::Accept, HttpShared::ApplicationJson);

  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  FullBackupOperation response{};
  {
    auto const& responseBody = rawResponse->GetBody();
    if (responseBody.size() > 0)
    {
      auto const jsonRoot
          = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());

      response.Status = jsonRoot["status"].get<std::string>();

      if (jsonRoot.contains("statusDetails") && !jsonRoot["statusDetails"].is_null())
      {
        response.StatusDetails = jsonRoot["statusDetails"].get<std::string>();
      }

      response.StartTime = Core::_internal::PosixTimeConverter::PosixTimeToDateTime(
          jsonRoot["startTime"].is_string() ? std::stoll(jsonRoot["startTime"].get<std::string>())
                                            : jsonRoot["startTime"].get<std::int64_t>());

      if (jsonRoot.contains("endTime") && !jsonRoot["endTime"].is_null())
      {
        response.EndTime = Core::_internal::PosixTimeConverter::PosixTimeToDateTime(
            jsonRoot["endTime"].is_string() ? std::stoll(jsonRoot["endTime"].get<std::string>())
                                            : jsonRoot["endTime"].get<std::int64_t>());
      }

      response.JobId = jsonRoot["jobId"].get<std::string>();

      if (jsonRoot.contains("azureStorageBlobContainerUri")
          && !jsonRoot["azureStorageBlobContainerUri"].is_null())
      {
        response.AzureStorageBlobContainerUri
            = jsonRoot["azureStorageBlobContainerUri"].get<std::string>();
      }

      if (jsonRoot.contains("error") && !jsonRoot["error"].is_null())
      {
        response.Error = DeserializeKeyVaultServiceError(jsonRoot["error"]);
      }
    }
  }

  return Response<FullBackupOperation>(std::move(response), std::move(rawResponse));
}

Azure::Response<RestoreOperation> BackupRestoreClient::FullRestore(
    Azure::Core::Url const& blobContainerUrl,
    std::string folderToRestore,
    SasTokenParameter const& sasToken,
    Core::Context const& context)
{
  auto url = m_vaultBaseUrl;
  url.AppendPath("restore");

  url.SetQueryParameters({{"api-version", m_apiVersion}});

  std::string jsonBody;
  {
    auto jsonRoot = Core::Json::_internal::json::object();

    jsonRoot["sasTokenParameters"]["storageResourceUri"] = blobContainerUrl.GetAbsoluteUrl();

    if (sasToken.Token.HasValue())
    {
      jsonRoot["sasTokenParameters"]["token"] = sasToken.Token.Value();
    }

    if (sasToken.UseManagedIdentity.HasValue())
    {
      jsonRoot["sasTokenParameters"]["useManagedIdentity"] = sasToken.UseManagedIdentity.Value();
    }

    jsonRoot["folderToRestore"] = folderToRestore;

    jsonBody = jsonRoot.dump();
  }

  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<std::uint8_t const*>(jsonBody.data()), jsonBody.length());

  Core::Http::Request request(Core::Http::HttpMethod::Put, url, &requestBody);

  request.SetHeader("Content-Type", "application/json");
  request.SetHeader("Content-Length", std::to_string(requestBody.Length()));

  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  RestoreOperation response{};
  {
    auto const& responseBody = rawResponse->GetBody();
    if (responseBody.size() > 0)
    {
      auto const jsonRoot
          = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());

      response.Status = jsonRoot["status"].get<std::string>();

      if (jsonRoot.contains("statusDetails") && !jsonRoot["statusDetails"].is_null())
      {
        response.StatusDetails = jsonRoot["statusDetails"].get<std::string>();
      }

      response.JobId = jsonRoot["jobId"].get<std::string>();

      response.StartTime = Core::_internal::PosixTimeConverter::PosixTimeToDateTime(
          jsonRoot["startTime"].is_string() ? std::stoll(jsonRoot["startTime"].get<std::string>())
                                            : jsonRoot["startTime"].get<std::int64_t>());

      if (jsonRoot.contains("endTime") && !jsonRoot["endTime"].is_null())
      {
        response.EndTime = Core::_internal::PosixTimeConverter::PosixTimeToDateTime(
            jsonRoot["endTime"].is_string() ? std::stoll(jsonRoot["endTime"].get<std::string>())
                                            : jsonRoot["endTime"].get<std::int64_t>());
      }

      if (jsonRoot.contains("error") && !jsonRoot["error"].is_null())
      {
        response.Error = DeserializeKeyVaultServiceError(jsonRoot["error"]);
      }
    }
  }

  return Response<RestoreOperation>(std::move(response), std::move(rawResponse));
}

Azure::Response<RestoreOperation> BackupRestoreClient::RestoreStatus(
    std::string const& jobId,
    Core::Context const& context)
{
  auto url = m_vaultBaseUrl;
  url.AppendPath("restore");
  url.AppendPath(!jobId.empty() ? Core::Url::Encode(jobId) : "null");
  url.AppendPath("pending");

  url.SetQueryParameters({{"api-version", "7.5"}});

  Core::Http::Request request(Core::Http::HttpMethod::Get, url);

  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Ok)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  RestoreOperation response{};
  {
    auto const& responseBody = rawResponse->GetBody();
    if (responseBody.size() > 0)
    {
      auto const jsonRoot
          = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());

      response.Status = jsonRoot["status"].get<std::string>();

      if (jsonRoot.contains("statusDetails") && !jsonRoot["statusDetails"].is_null())
      {
        response.StatusDetails = jsonRoot["statusDetails"].get<std::string>();
      }

      response.JobId = jsonRoot["jobId"].get<std::string>();

      response.StartTime = Core::_internal::PosixTimeConverter::PosixTimeToDateTime(
          jsonRoot["startTime"].is_string() ? std::stoll(jsonRoot["startTime"].get<std::string>())
                                            : jsonRoot["startTime"].get<std::int64_t>());

      if (jsonRoot.contains("endTime") && !jsonRoot["endTime"].is_null())
      {
        response.EndTime = Core::_internal::PosixTimeConverter::PosixTimeToDateTime(
            jsonRoot["endTime"].is_string() ? std::stoll(jsonRoot["endTime"].get<std::string>())
                                            : jsonRoot["endTime"].get<std::int64_t>());
      }

      if (jsonRoot.contains("error") && !jsonRoot["error"].is_null())
      {
        response.Error = DeserializeKeyVaultServiceError(jsonRoot["error"]);
      }
    }
  }

  return Response<RestoreOperation>(std::move(response), std::move(rawResponse));
}

Azure::Response<SelectiveKeyRestoreOperation> BackupRestoreClient::SelectiveKeyRestore(
    std::string const& keyName,
    Azure::Core::Url const& blobContainerUrl,
    std::string folderToRestore,
    SasTokenParameter const& sasToken,
    Core::Context const& context)
{
  auto url = m_vaultBaseUrl;
  url.AppendPath("keys");
  url.AppendPath(!keyName.empty() ? Core::Url::Encode(keyName) : "null");
  url.AppendPath("restore");

  url.SetQueryParameters({{"api-version", "7.5"}});

  std::string jsonBody;
  {
    auto jsonRoot = Core::Json::_internal::json::object();

    jsonRoot["sasTokenParameters"]["storageResourceUri"] = blobContainerUrl.GetAbsoluteUrl();

    if (sasToken.Token.HasValue())
    {
      jsonRoot["sasTokenParameters"]["token"] = sasToken.Token.Value();
    }

    if (sasToken.UseManagedIdentity.HasValue())
    {
      jsonRoot["sasTokenParameters"]["useManagedIdentity"] = sasToken.UseManagedIdentity.Value();
    }

    jsonRoot["folder"] = folderToRestore;

    jsonBody = jsonRoot.dump();
  }

  Core::IO::MemoryBodyStream requestBody(
      reinterpret_cast<std::uint8_t const*>(jsonBody.data()), jsonBody.length());

  Core::Http::Request request(Core::Http::HttpMethod::Put, url, &requestBody);

  request.SetHeader("Content-Type", "application/json");
  request.SetHeader("Content-Length", std::to_string(requestBody.Length()));

  auto rawResponse = m_pipeline->Send(request, context);
  auto const httpStatusCode = rawResponse->GetStatusCode();

  if (httpStatusCode != Core::Http::HttpStatusCode::Accepted)
  {
    throw Core::RequestFailedException(rawResponse);
  }

  SelectiveKeyRestoreOperation response{};
  {
    auto const& responseBody = rawResponse->GetBody();
    if (responseBody.size() > 0)
    {
      auto const jsonRoot
          = Core::Json::_internal::json::parse(responseBody.begin(), responseBody.end());

      response.Status = jsonRoot["status"].get<std::string>();

      if (jsonRoot.contains("statusDetails") && !jsonRoot["statusDetails"].is_null())
      {
        response.StatusDetails = jsonRoot["statusDetails"].get<std::string>();
      }

      response.JobId = jsonRoot["jobId"].get<std::string>();

      response.StartTime = Core::_internal::PosixTimeConverter::PosixTimeToDateTime(
          jsonRoot["startTime"].is_string() ? std::stoll(jsonRoot["startTime"].get<std::string>())
                                            : jsonRoot["startTime"].get<std::int64_t>());
      if (jsonRoot.contains("error") && !jsonRoot["error"].is_null())
      {
        response.Error = DeserializeKeyVaultServiceError(jsonRoot["error"]);
      }
      if (jsonRoot.contains("endTime") && !jsonRoot["endTime"].is_null())
      {
        response.EndTime = Core::_internal::PosixTimeConverter::PosixTimeToDateTime(
            jsonRoot["endTime"].is_string() ? std::stoll(jsonRoot["endTime"].get<std::string>())
                                            : jsonRoot["endTime"].get<std::int64_t>());
      }
    }
  }

  return Response<SelectiveKeyRestoreOperation>(std::move(response), std::move(rawResponse));
}

KeyVaultServiceError BackupRestoreClient::DeserializeKeyVaultServiceError(
    Azure::Core::Json::_internal::json errorFragment)
{
  KeyVaultServiceError result;
  if (errorFragment.contains("code"))
  {
    result.Code = errorFragment["code"].get<std::string>();
  }
  if (errorFragment.contains("message"))
  {
    result.Message = errorFragment["message"].get<std::string>();
  }
  if (errorFragment.contains("innererror"))
  {
    result.InnerError = std::make_unique<KeyVaultServiceError>(
        DeserializeKeyVaultServiceError(errorFragment["innererror"]));
  }
  return result;
}
