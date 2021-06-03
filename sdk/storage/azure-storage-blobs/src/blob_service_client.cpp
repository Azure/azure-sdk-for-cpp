// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_service_client.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

#include "private/package_version.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobServiceClient BlobServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const BlobClientOptions& options)
  {
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
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
        std::make_unique<_internal::SharedKeyPolicy>(credential));

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_serviceUrl.GetHost(), newOptions.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(newOptions.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        newOptions,
        _internal::BlobServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  BlobServiceClient::BlobServiceClient(
      const std::string& serviceUrl,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const BlobClientOptions& options)
      : BlobServiceClient(serviceUrl, options)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_serviceUrl.GetHost(), options.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    {
      Azure::Core::Credentials::TokenRequestContext tokenContext;
      tokenContext.Scopes.emplace_back(_internal::StorageScope);
      perRetryPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::Policies::_internal::BearerTokenAuthenticationPolicy>(
              credential, tokenContext));
    }
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        _internal::BlobServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  BlobServiceClient::BlobServiceClient(
      const std::string& serviceUrl,
      const BlobClientOptions& options)
      : m_serviceUrl(serviceUrl), m_customerProvidedKey(options.CustomerProvidedKey),
        m_encryptionScope(options.EncryptionScope)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_serviceUrl.GetHost(), options.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        _internal::BlobServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  BlobContainerClient BlobServiceClient::GetBlobContainerClient(
      const std::string& blobContainerName) const
  {
    auto blobContainerUrl = m_serviceUrl;
    blobContainerUrl.AppendPath(_internal::UrlEncodePath(blobContainerName));
    return BlobContainerClient(
        std::move(blobContainerUrl), m_pipeline, m_customerProvidedKey, m_encryptionScope);
  }

  ListBlobContainersPagedResponse BlobServiceClient::ListBlobContainers(
      const ListBlobContainersOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::Service::ListBlobContainersOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    if (options.ContinuationToken.HasValue() && !options.ContinuationToken.Value().empty())
    {
      protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    }
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Include = options.Include;
    auto response = _detail::BlobRestClient::Service::ListBlobContainers(
        *m_pipeline, m_serviceUrl, protocolLayerOptions, _internal::WithReplicaStatus(context));

    ListBlobContainersPagedResponse pagedResponse;
    pagedResponse.ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    pagedResponse.Prefix = std::move(response.Value.Prefix);
    pagedResponse.BlobContainers = std::move(response.Value.Items);
    pagedResponse.m_blobServiceClient = std::make_shared<BlobServiceClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken;
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  Azure::Response<Models::UserDelegationKey> BlobServiceClient::GetUserDelegationKey(
      const Azure::DateTime& expiresOn,
      const GetUserDelegationKeyOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::Service::GetUserDelegationKeyOptions protocolLayerOptions;
    protocolLayerOptions.StartsOn = options.StartsOn;
    protocolLayerOptions.ExpiresOn = expiresOn;
    return _detail::BlobRestClient::Service::GetUserDelegationKey(
        *m_pipeline, m_serviceUrl, protocolLayerOptions, _internal::WithReplicaStatus(context));
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
        *m_pipeline, m_serviceUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::BlobServiceProperties> BlobServiceClient::GetProperties(
      const GetServicePropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::BlobRestClient::Service::GetServicePropertiesOptions protocolLayerOptions;
    return _detail::BlobRestClient::Service::GetProperties(
        *m_pipeline, m_serviceUrl, protocolLayerOptions, _internal::WithReplicaStatus(context));
  }

  Azure::Response<Models::AccountInfo> BlobServiceClient::GetAccountInfo(
      const GetAccountInfoOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::BlobRestClient::Service::GetAccountInfoOptions protocolLayerOptions;
    return _detail::BlobRestClient::Service::GetAccountInfo(
        *m_pipeline, m_serviceUrl, protocolLayerOptions, _internal::WithReplicaStatus(context));
  }

  Azure::Response<Models::ServiceStatistics> BlobServiceClient::GetStatistics(
      const GetBlobServiceStatisticsOptions& options,
      const Azure::Core::Context& context) const
  {
    (void)options;
    _detail::BlobRestClient::Service::GetServiceStatisticsOptions protocolLayerOptions;
    return _detail::BlobRestClient::Service::GetStatistics(
        *m_pipeline, m_serviceUrl, protocolLayerOptions, context);
  }

  FindBlobsByTagsPagedResponse BlobServiceClient::FindBlobsByTags(
      const std::string& tagFilterSqlExpression,
      const FindBlobsByTagsOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::Service::FindBlobsByTagsOptions protocolLayerOptions;
    protocolLayerOptions.Where = tagFilterSqlExpression;
    if (options.ContinuationToken.HasValue() && !options.ContinuationToken.Value().empty())
    {
      protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    }
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    auto response = _detail::BlobRestClient::Service::FindBlobsByTags(
        *m_pipeline, m_serviceUrl, protocolLayerOptions, _internal::WithReplicaStatus(context));

    FindBlobsByTagsPagedResponse pagedResponse;
    pagedResponse.ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    pagedResponse.TaggedBlobs = std::move(response.Value.Items);
    pagedResponse.m_blobServiceClient = std::make_shared<BlobServiceClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.m_tagFilterSqlExpression = tagFilterSqlExpression;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken;
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  Azure::Response<BlobContainerClient> BlobServiceClient::CreateBlobContainer(
      const std::string& blobContainerName,
      const CreateBlobContainerOptions& options,
      const Azure::Core::Context& context) const
  {
    auto blobContainerClient = GetBlobContainerClient(blobContainerName);
    auto response = blobContainerClient.Create(options, context);
    return Azure::Response<BlobContainerClient>(
        std::move(blobContainerClient), std::move(response.RawResponse));
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
    (void)options;

    auto blobContainerClient = GetBlobContainerClient(deletedBlobContainerName);

    _detail::BlobRestClient::BlobContainer::UndeleteBlobContainerOptions protocolLayerOptions;
    protocolLayerOptions.DeletedBlobContainerName = deletedBlobContainerName;
    protocolLayerOptions.DeletedBlobContainerVersion = deletedBlobContainerVersion;
    auto response = _detail::BlobRestClient::BlobContainer::Undelete(
        *m_pipeline, Azure::Core::Url(blobContainerClient.GetUrl()), protocolLayerOptions, context);

    return Azure::Response<BlobContainerClient>(
        std::move(blobContainerClient), std::move(response.RawResponse));
  }

}}} // namespace Azure::Storage::Blobs
