// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_service_client.hpp"

#include "azure/core/credentials.hpp"
#include "azure/core/http/curl/curl.hpp"
#include "azure/storage/blobs/protocol/blob_rest_client.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_credential.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"
#include "azure/storage/common/storage_retry_policy.hpp"
#include "azure/storage/files/datalake/datalake_file_system_client.hpp"
#include "azure/storage/files/datalake/datalake_utilities.hpp"
#include "azure/storage/files/datalake/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {
  namespace {
    Blobs::BlobServiceClientOptions GetBlobServiceClientOptions(const ServiceClientOptions& options)
    {
      Blobs::BlobServiceClientOptions blobOptions;
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

    std::vector<FileSystem> FileSystemsFromContainerItems(
        const std::vector<Blobs::BlobContainerItem>& items)
    {
      std::vector<FileSystem> fileSystems;
      for (const auto& item : items)
      {
        FileSystem fileSystem;
        fileSystem.ETag = item.ETag;
        fileSystem.Name = item.Name;
        fileSystem.LastModified = item.LastModified;
        fileSystems.emplace_back(std::move(fileSystem));
      }
      return fileSystems;
    }
  } // namespace

  ServiceClient ServiceClient::CreateFromConnectionString(
      const std::string& connectionString,
      const ServiceClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto serviceUri = std::move(parsedConnectionString.DataLakeServiceUri);

    if (parsedConnectionString.KeyCredential)
    {
      return ServiceClient(
          serviceUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return ServiceClient(serviceUri.GetAbsoluteUrl(), options);
    }
  }

  ServiceClient::ServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const ServiceClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(serviceUri)), m_blobServiceClient(
                                                             Details::GetBlobUriFromUri(serviceUri),
                                                             credential,
                                                             GetBlobServiceClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_DatalakeServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    StorageRetryWithSecondaryOptions dfsRetryOptions = options.RetryOptions;
    dfsRetryOptions.SecondaryHostForRetryReads
        = Details::GetDfsUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(dfsRetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  ServiceClient::ServiceClient(
      const std::string& serviceUri,
      std::shared_ptr<Identity::ClientSecretCredential> credential,
      const ServiceClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(serviceUri)), m_blobServiceClient(
                                                             Details::GetBlobUriFromUri(serviceUri),
                                                             credential,
                                                             GetBlobServiceClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_DatalakeServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    StorageRetryWithSecondaryOptions dfsRetryOptions = options.RetryOptions;
    dfsRetryOptions.SecondaryHostForRetryReads
        = Details::GetDfsUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(dfsRetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Core::BearerTokenAuthenticationPolicy>(
        credential, Azure::Storage::Details::c_StorageScope));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  ServiceClient::ServiceClient(const std::string& serviceUri, const ServiceClientOptions& options)
      : m_dfsUri(Details::GetDfsUriFromUri(serviceUri)), m_blobServiceClient(
                                                             Details::GetBlobUriFromUri(serviceUri),
                                                             GetBlobServiceClientOptions(options))
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Azure::Storage::Details::c_DatalakeServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    StorageRetryWithSecondaryOptions dfsRetryOptions = options.RetryOptions;
    dfsRetryOptions.SecondaryHostForRetryReads
        = Details::GetDfsUriFromUri(options.RetryOptions.SecondaryHostForRetryReads);
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(dfsRetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  FileSystemClient ServiceClient::GetFileSystemClient(const std::string& fileSystemName) const
  {
    auto builder = m_dfsUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(fileSystemName));
    return FileSystemClient(
        builder, m_blobServiceClient.GetBlobContainerClient(fileSystemName), m_pipeline);
  }

  Azure::Core::Response<ListFileSystemsSegmentResult> ServiceClient::ListFileSystemsSegement(
      const ListFileSystemsSegmentOptions& options) const
  {
    Blobs::ListContainersSegmentOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.Prefix = options.Prefix;
    blobOptions.ContinuationToken = options.ContinuationToken;
    blobOptions.MaxResults = options.MaxResults;
    auto result = m_blobServiceClient.ListBlobContainersSegment(blobOptions);
    auto response = ListFileSystemsSegmentResult();
    response.ContinuationToken = result->ContinuationToken.empty() ? response.ContinuationToken
                                                                   : result->ContinuationToken;
    response.Filesystems = FileSystemsFromContainerItems(result->Items);
    return Azure::Core::Response<ListFileSystemsSegmentResult>(
        std::move(response), result.ExtractRawResponse());
  }

}}}} // namespace Azure::Storage::Files::DataLake
