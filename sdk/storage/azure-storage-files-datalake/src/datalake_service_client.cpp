// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_service_client.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/files/datalake/datalake_file_system_client.hpp"
#include "private/datalake_utilities.hpp"
#include "private/package_version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  DataLakeServiceClient DataLakeServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const DataLakeClientOptions& options)
  {
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
    auto serviceUrl = std::move(parsedConnectionString.DataLakeServiceUrl);

    if (parsedConnectionString.KeyCredential)
    {
      return DataLakeServiceClient(
          serviceUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return DataLakeServiceClient(serviceUrl.GetAbsoluteUrl(), options);
    }
  }

  DataLakeServiceClient::DataLakeServiceClient(
      const std::string& serviceUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const DataLakeClientOptions& options)
      : m_serviceUrl(serviceUrl), m_blobServiceClient(
                                      _detail::GetBlobUrlFromUrl(serviceUrl),
                                      credential,
                                      _detail::GetBlobClientOptions(options)),
        m_customerProvidedKey(options.CustomerProvidedKey)
  {
    DataLakeClientOptions newOptions = options;
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
        _internal::DatalakeServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  DataLakeServiceClient::DataLakeServiceClient(
      const std::string& serviceUrl,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const DataLakeClientOptions& options)
      : m_serviceUrl(serviceUrl), m_blobServiceClient(
                                      _detail::GetBlobUrlFromUrl(serviceUrl),
                                      credential,
                                      _detail::GetBlobClientOptions(options)),
        m_customerProvidedKey(options.CustomerProvidedKey)
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
        _internal::DatalakeServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  DataLakeServiceClient::DataLakeServiceClient(
      const std::string& serviceUrl,
      const DataLakeClientOptions& options)
      : m_serviceUrl(serviceUrl), m_blobServiceClient(
                                      _detail::GetBlobUrlFromUrl(serviceUrl),
                                      _detail::GetBlobClientOptions(options)),
        m_customerProvidedKey(options.CustomerProvidedKey)
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
        _internal::DatalakeServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  DataLakeFileSystemClient DataLakeServiceClient::GetFileSystemClient(
      const std::string& fileSystemName) const
  {
    auto builder = m_serviceUrl;
    builder.AppendPath(_internal::UrlEncodePath(fileSystemName));
    return DataLakeFileSystemClient(
        builder,
        m_blobServiceClient.GetBlobContainerClient(fileSystemName),
        m_pipeline,
        m_customerProvidedKey);
  }

  ListFileSystemsPagedResponse DataLakeServiceClient::ListFileSystems(
      const ListFileSystemsOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::ListBlobContainersOptions blobOptions;
    blobOptions.Include = options.Include;
    blobOptions.Prefix = options.Prefix;
    blobOptions.ContinuationToken = options.ContinuationToken;
    blobOptions.PageSizeHint = options.PageSizeHint;
    auto blobPagedResponse = m_blobServiceClient.ListBlobContainers(blobOptions, context);

    ListFileSystemsPagedResponse pagedResponse;

    pagedResponse.ServiceEndpoint = std::move(blobPagedResponse.ServiceEndpoint);
    pagedResponse.Prefix = std::move(blobPagedResponse.Prefix);
    for (auto& item : blobPagedResponse.BlobContainers)
    {
      Models::FileSystemItem fileSystem;
      fileSystem.Name = std::move(item.Name);
      fileSystem.Details.ETag = std::move(item.Details.ETag);
      fileSystem.Details.LastModified = std::move(item.Details.LastModified);
      fileSystem.Details.Metadata = std::move(item.Details.Metadata);
      if (item.Details.AccessType == Blobs::Models::PublicAccessType::BlobContainer)
      {
        fileSystem.Details.AccessType = Models::PublicAccessType::FileSystem;
      }
      else if (item.Details.AccessType == Blobs::Models::PublicAccessType::Blob)
      {
        fileSystem.Details.AccessType = Models::PublicAccessType::Path;
      }
      else if (item.Details.AccessType == Blobs::Models::PublicAccessType::None)
      {
        fileSystem.Details.AccessType = Models::PublicAccessType::None;
      }
      else
      {
        fileSystem.Details.AccessType
            = Models::PublicAccessType(item.Details.AccessType.ToString());
      }
      fileSystem.Details.HasImmutabilityPolicy = item.Details.HasImmutabilityPolicy;
      fileSystem.Details.HasLegalHold = item.Details.HasLegalHold;
      fileSystem.Details.LeaseDuration = std::move(item.Details.LeaseDuration);
      fileSystem.Details.LeaseState = std::move(item.Details.LeaseState);
      fileSystem.Details.LeaseStatus = std::move(item.Details.LeaseStatus);
      fileSystem.Details.DefaultEncryptionScope = std::move(item.Details.DefaultEncryptionScope);
      fileSystem.Details.PreventEncryptionScopeOverride
          = item.Details.PreventEncryptionScopeOverride;

      pagedResponse.FileSystems.emplace_back(std::move(fileSystem));
    }
    pagedResponse.m_dataLakeServiceClient = std::make_shared<DataLakeServiceClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = std::move(blobPagedResponse.CurrentPageToken);
    pagedResponse.NextPageToken = std::move(blobPagedResponse.NextPageToken);
    pagedResponse.RawResponse = std::move(blobPagedResponse.RawResponse);

    return pagedResponse;
  }

}}}} // namespace Azure::Storage::Files::DataLake