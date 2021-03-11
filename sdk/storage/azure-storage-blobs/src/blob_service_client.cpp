// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_service_client.hpp"

#include <azure/core/http/policy.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_per_retry_policy.hpp>
#include <azure/storage/common/storage_switch_to_secondary_policy.hpp>

#include "azure/storage/blobs/version.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobServiceClient BlobServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const BlobClientOptions& options)
  {
    auto parsedConnectionString = Storage::_detail::ParseConnectionString(connectionString);
    auto serviceUrl = std::move(parsedConnectionString.BlobServiceUrl);

    if (parsedConnectionString.KeyCredential)
    {
      return BlobServiceClient(
          serviceUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return BlobServiceClient(serviceUrl.GetAbsoluteUrl(), options);
    }
  }

  BlobServiceClient::BlobServiceClient(
      const std::string& serviceUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const BlobClientOptions& options)
      : BlobServiceClient(serviceUrl, options)
  {
    BlobClientOptions newOptions = options;
    newOptions.PerRetryPolicies.emplace_back(
        std::make_unique<Storage::_detail::SharedKeyPolicy>(credential));

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(
        std::make_unique<Storage::_detail::StorageSwitchToSecondaryPolicy>(
            m_serviceUrl.GetHost(), newOptions.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<Storage::_detail::StoragePerRetryPolicy>());
    {
      Azure::Core::Http::_internal::ValueOptions valueOptions;
      valueOptions.HeaderValues[Storage::_detail::HttpHeaderXMsVersion] = newOptions.ApiVersion;
      perOperationPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::_internal::ValuePolicy>(valueOptions));
    }
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        newOptions,
        Storage::_detail::FileServicePackageName,
        PackageVersion::VersionString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  BlobServiceClient::BlobServiceClient(
      const std::string& serviceUrl,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const BlobClientOptions& options)
      : BlobServiceClient(serviceUrl, options)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(
        std::make_unique<Storage::_detail::StorageSwitchToSecondaryPolicy>(
            m_serviceUrl.GetHost(), options.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<Storage::_detail::StoragePerRetryPolicy>());
    {
      Azure::Core::Credentials::TokenRequestContext tokenContext;
      tokenContext.Scopes.emplace_back(Storage::_detail::StorageScope);
      perRetryPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::BearerTokenAuthenticationPolicy>(
              credential, tokenContext));
    }
    {
      Azure::Core::Http::_internal::ValueOptions valueOptions;
      valueOptions.HeaderValues[Storage::_detail::HttpHeaderXMsVersion] = options.ApiVersion;
      perOperationPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::_internal::ValuePolicy>(valueOptions));
    }
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        Storage::_detail::FileServicePackageName,
        PackageVersion::VersionString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  BlobServiceClient::BlobServiceClient(
      const std::string& serviceUrl,
      const BlobClientOptions& options)
      : m_serviceUrl(serviceUrl), m_customerProvidedKey(options.CustomerProvidedKey),
        m_encryptionScope(options.EncryptionScope)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(
        std::make_unique<Storage::_detail::StorageSwitchToSecondaryPolicy>(
            m_serviceUrl.GetHost(), options.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<Storage::_detail::StoragePerRetryPolicy>());
    {
      Azure::Core::Http::_internal::ValueOptions valueOptions;
      valueOptions.HeaderValues[Storage::_detail::HttpHeaderXMsVersion] = options.ApiVersion;
      perOperationPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::_internal::ValuePolicy>(valueOptions));
    }
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        Storage::_detail::FileServicePackageName,
        PackageVersion::VersionString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  BlobContainerClient BlobServiceClient::GetBlobContainerClient(
      const std::string& blobContainerName) const
  {
    auto blobContainerUrl = m_serviceUrl;
    blobContainerUrl.AppendPath(Storage::_detail::UrlEncodePath(blobContainerName));
    return BlobContainerClient(
        std::move(blobContainerUrl), m_pipeline, m_customerProvidedKey, m_encryptionScope);
  }

  Azure::Response<Models::ListBlobContainersSinglePageResult>
  BlobServiceClient::ListBlobContainersSinglePage(
      const ListBlobContainersSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::Service::ListBlobContainersSinglePageOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Include = options.Include;
    return _detail::BlobRestClient::Service::ListBlobContainersSinglePage(
        Storage::_detail::WithReplicaStatus(context),
        *m_pipeline,
        m_serviceUrl,
        protocolLayerOptions);
  }

  Azure::Response<Models::GetUserDelegationKeyResult> BlobServiceClient::GetUserDelegationKey(
      const Azure::DateTime& expiresOn,
      const GetUserDelegationKeyOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::Service::GetUserDelegationKeyOptions protocolLayerOptions;
    protocolLayerOptions.StartsOn = options.startsOn;
    protocolLayerOptions.ExpiresOn = expiresOn;
    return _detail::BlobRestClient::Service::GetUserDelegationKey(
        Storage::_detail::WithReplicaStatus(context),
        *m_pipeline,
        m_serviceUrl,
        protocolLayerOptions);
  }

  Azure::Response<Models::SetServicePropertiesResult> BlobServiceClient::SetProperties(
      Models::BlobServiceProperties properties,
      const SetServicePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::BlobRestClient::Service::SetServicePropertiesOptions protocolLayerOptions;
    protocolLayerOptions.Properties = std::move(properties);
    return _detail::BlobRestClient::Service::SetProperties(
        context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

  Azure::Response<Models::GetServicePropertiesResult> BlobServiceClient::GetProperties(
      const GetServicePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::BlobRestClient::Service::GetServicePropertiesOptions protocolLayerOptions;
    return _detail::BlobRestClient::Service::GetProperties(
        Storage::_detail::WithReplicaStatus(context),
        *m_pipeline,
        m_serviceUrl,
        protocolLayerOptions);
  }

  Azure::Response<Models::GetAccountInfoResult> BlobServiceClient::GetAccountInfo(
      const GetAccountInfoOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::BlobRestClient::Service::GetAccountInfoOptions protocolLayerOptions;
    return _detail::BlobRestClient::Service::GetAccountInfo(
        Storage::_detail::WithReplicaStatus(context),
        *m_pipeline,
        m_serviceUrl,
        protocolLayerOptions);
  }

  Azure::Response<Models::GetServiceStatisticsResult> BlobServiceClient::GetStatistics(
      const GetBlobServiceStatisticsOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::BlobRestClient::Service::GetServiceStatisticsOptions protocolLayerOptions;
    return _detail::BlobRestClient::Service::GetStatistics(
        context, *m_pipeline, m_serviceUrl, protocolLayerOptions);
  }

  Azure::Response<Models::FindBlobsByTagsSinglePageResult>
  BlobServiceClient::FindBlobsByTagsSinglePage(
      const std::string& tagFilterSqlExpression,
      const FindBlobsByTagsSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::Service::FindBlobsByTagsSinglePageOptions protocolLayerOptions;
    protocolLayerOptions.Where = tagFilterSqlExpression;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    return _detail::BlobRestClient::Service::FindBlobsByTagsSinglePage(
        Storage::_detail::WithReplicaStatus(context),
        *m_pipeline,
        m_serviceUrl,
        protocolLayerOptions);
  }

  Azure::Response<BlobContainerClient> BlobServiceClient::CreateBlobContainer(
      const std::string& blobContainerName,
      const CreateBlobContainerOptions& options,
      const Azure::Core::Context& context) const
  {
    auto blobContainerClient = GetBlobContainerClient(blobContainerName);
    auto response = blobContainerClient.Create(options, context);
    return Azure::Response<BlobContainerClient>(
        std::move(blobContainerClient), response.ExtractRawResponse());
  }

  Azure::Response<Models::DeleteBlobContainerResult> BlobServiceClient::DeleteBlobContainer(
      const std::string& blobContainerName,
      const DeleteBlobContainerOptions& options,
      const Azure::Core::Context& context) const
  {
    auto blobContainerClient = GetBlobContainerClient(blobContainerName);
    return blobContainerClient.Delete(options, context);
  }

  Azure::Response<BlobContainerClient> BlobServiceClient::UndeleteBlobContainer(
      const std::string deletedBlobContainerName,
      const std::string deletedBlobContainerVersion,
      const UndeleteBlobContainerOptions& options,
      const Azure::Core::Context& context) const
  {
    std::string destinationBlobContainerName = options.DestinationBlobContainerName.HasValue()
        ? options.DestinationBlobContainerName.GetValue()
        : deletedBlobContainerName;
    auto blobContainerClient = GetBlobContainerClient(destinationBlobContainerName);

    _detail::BlobRestClient::BlobContainer::UndeleteBlobContainerOptions protocolLayerOptions;
    protocolLayerOptions.DeletedBlobContainerName = deletedBlobContainerName;
    protocolLayerOptions.DeletedBlobContainerVersion = deletedBlobContainerVersion;
    auto response = _detail::BlobRestClient::BlobContainer::Undelete(
        context, *m_pipeline, Azure::Core::Url(blobContainerClient.GetUrl()), protocolLayerOptions);

    return Azure::Response<BlobContainerClient>(
        std::move(blobContainerClient), response.ExtractRawResponse());
  }

}}} // namespace Azure::Storage::Blobs
