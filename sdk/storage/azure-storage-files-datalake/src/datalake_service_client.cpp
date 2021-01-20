// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_service_client.hpp"

#include <azure/core/http/policy.hpp>
#include <azure/storage/blobs/protocol/blob_rest_client.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_credential.hpp>
#include <azure/storage/common/storage_per_retry_policy.hpp>
#include <azure/storage/common/storage_retry_policy.hpp>

#include "azure/storage/files/datalake/datalake_file_system_client.hpp"
#include "azure/storage/files/datalake/datalake_utilities.hpp"
#include "azure/storage/files/datalake/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {
  namespace {
    Blobs::BlobClientOptions GetBlobServiceClientOptions(const DataLakeClientOptions& options)
    {
      Blobs::BlobClientOptions blobOptions;
      for (const auto& p : options.PerOperationPolicies)
      {
        blobOptions.PerOperationPolicies.emplace_back(p->Clone());
      }
      for (const auto& p : options.PerRetryPolicies)
      {
        blobOptions.PerRetryPolicies.emplace_back(p->Clone());
      }
      blobOptions.RetryOptions = options.RetryOptions;
      blobOptions.RetryOptions.SecondaryHostForRetryReads
          = Details::GetBlobUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
      return blobOptions;
    }

    std::vector<Models::FileSystem> FileSystemsFromContainerItems(
        const std::vector<Blobs::Models::BlobContainerItem>& items)
    {
      std::vector<Models::FileSystem> fileSystems;
      for (const auto& item : items)
      {
        Models::FileSystem fileSystem;
        fileSystem.ETag = item.ETag;
        fileSystem.Name = item.Name;
        fileSystem.LastModified = item.LastModified;
        fileSystems.emplace_back(std::move(fileSystem));
      }
      return fileSystems;
    }
  } // namespace

  DataLakeServiceClient DataLakeServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const DataLakeClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto serviceUri = std::move(parsedConnectionString.DataLakeServiceUrl);

    if (parsedConnectionString.KeyCredential)
    {
      return DataLakeServiceClient(
          serviceUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return DataLakeServiceClient(serviceUri.GetAbsoluteUrl(), options);
    }
  }

  DataLakeServiceClient::DataLakeServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const DataLakeClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(serviceUri)), m_blobServiceClient(
                                                             Details::GetBlobUriFromUri(serviceUri),
                                                             credential,
                                                             GetBlobServiceClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::DatalakeServicePackageName, Details::Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    StorageRetryWithSecondaryOptions dfsRetryOptions = options.RetryOptions;
    dfsRetryOptions.SecondaryHostForRetryReads
        = Details::GetDfsUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<Storage::Details::StorageRetryPolicy>(dfsRetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Storage::Details::SharedKeyPolicy>(credential));
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  DataLakeServiceClient::DataLakeServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<Core::TokenCredential> credential,
      const DataLakeClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(serviceUri)), m_blobServiceClient(
                                                             Details::GetBlobUriFromUri(serviceUri),
                                                             credential,
                                                             GetBlobServiceClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::DatalakeServicePackageName, Details::Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    StorageRetryWithSecondaryOptions dfsRetryOptions = options.RetryOptions;
    dfsRetryOptions.SecondaryHostForRetryReads
        = Details::GetDfsUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<Storage::Details::StorageRetryPolicy>(dfsRetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Core::Http::BearerTokenAuthenticationPolicy>(
        credential, Azure::Storage::Details::StorageScope));
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  DataLakeServiceClient::DataLakeServiceClient(
      const std::string& serviceUri,
      const DataLakeClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(serviceUri)), m_blobServiceClient(
                                                             Details::GetBlobUriFromUri(serviceUri),
                                                             GetBlobServiceClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::DatalakeServicePackageName, Details::Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    StorageRetryWithSecondaryOptions dfsRetryOptions = options.RetryOptions;
    dfsRetryOptions.SecondaryHostForRetryReads
        = Details::GetDfsUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<Storage::Details::StorageRetryPolicy>(dfsRetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  DataLakeFileSystemClient DataLakeServiceClient::GetFileSystemClient(
      const std::string& fileSystemName) const
  {
    auto builder = m_dfsUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(fileSystemName));
    return DataLakeFileSystemClient(
        builder, m_blobServiceClient.GetBlobContainerClient(fileSystemName), m_pipeline);
  }

  Azure::Core::Response<Models::ListFileSystemsSinglePageResult>
  DataLakeServiceClient::ListFileSystemsSinglePage(
      const ListFileSystemsSinglePageOptions& options) const
  {
    Blobs::ListBlobContainersSinglePageOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.Prefix = options.Prefix;
    blobOptions.ContinuationToken = options.ContinuationToken;
    blobOptions.PageSizeHint = options.PageSizeHint;
    auto result = m_blobServiceClient.ListBlobContainersSinglePage(blobOptions);
    auto response = Models::ListFileSystemsSinglePageResult();
    response.ContinuationToken = std::move(result->ContinuationToken);
    response.Filesystems = FileSystemsFromContainerItems(result->Items);
    return Azure::Core::Response<Models::ListFileSystemsSinglePageResult>(
        std::move(response), result.ExtractRawResponse());
  }

}}}} // namespace Azure::Storage::Files::DataLake
