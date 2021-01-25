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
          = Details::GetBlobUrlFromUrl(options.RetryOptions.SecondaryHostForRetryReads);
      return blobOptions;
    }

    std::vector<Models::FileSystemItem> FileSystemsFromContainerItems(
        std::vector<Blobs::Models::BlobContainerItem> items)
    {
      std::vector<Models::FileSystemItem> fileSystems;
      for (auto& item : items)
      {
        Models::FileSystemItem fileSystem;
        fileSystem.Name = std::move(item.Name);
        fileSystem.ETag = std::move(item.ETag);
        fileSystem.LastModified = std::move(item.LastModified);
        fileSystem.Metadata = std::move(item.Metadata);
        if (item.AccessType == Blobs::Models::PublicAccessType::BlobContainer)
        {
          fileSystem.AccessType = Models::PublicAccessType::FileSystem;
        }
        else if (item.AccessType == Blobs::Models::PublicAccessType::Blob)
        {
          fileSystem.AccessType = Models::PublicAccessType::Path;
        }
        else if (item.AccessType == Blobs::Models::PublicAccessType::Private)
        {
          fileSystem.AccessType = Models::PublicAccessType::None;
        }
        else
        {
          fileSystem.AccessType = Models::PublicAccessType(item.AccessType.Get());
        }
        fileSystem.HasImmutabilityPolicy = item.HasImmutabilityPolicy;
        fileSystem.HasLegalHold = item.HasLegalHold;
        if (item.LeaseDuration.HasValue())
        {
          fileSystem.LeaseDuration
              = Models::LeaseDurationType((item.LeaseDuration.GetValue().Get()));
        }
        fileSystem.LeaseState = Models::LeaseStateType(item.LeaseState.Get());
        fileSystem.LeaseStatus = Models::LeaseStatusType(item.LeaseStatus.Get());

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
      : m_dfsUrl(Details::GetDfsUrlFromUrl(serviceUri)), m_blobServiceClient(
                                                             Details::GetBlobUrlFromUrl(serviceUri),
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
        = Details::GetDfsUrlFromUrl(options.RetryOptions.SecondaryHostForRetryReads);
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
      : m_dfsUrl(Details::GetDfsUrlFromUrl(serviceUri)), m_blobServiceClient(
                                                             Details::GetBlobUrlFromUrl(serviceUri),
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
        = Details::GetDfsUrlFromUrl(options.RetryOptions.SecondaryHostForRetryReads);
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
      : m_dfsUrl(Details::GetDfsUrlFromUrl(serviceUri)), m_blobServiceClient(
                                                             Details::GetBlobUrlFromUrl(serviceUri),
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
        = Details::GetDfsUrlFromUrl(options.RetryOptions.SecondaryHostForRetryReads);
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
    auto builder = m_dfsUrl;
    builder.AppendPath(Storage::Details::UrlEncodePath(fileSystemName));
    return DataLakeFileSystemClient(
        builder, m_blobServiceClient.GetBlobContainerClient(fileSystemName), m_pipeline);
  }

  Azure::Core::Response<Models::ListFileSystemsSinglePageResult>
  DataLakeServiceClient::ListFileSystemsSinglePage(
      const ListFileSystemsSinglePageOptions& options) const
  {
    Blobs::ListBlobContainersSinglePageOptions blobOptions;
    blobOptions.Include = options.Include;
    blobOptions.Context = options.Context;
    blobOptions.Prefix = options.Prefix;
    blobOptions.ContinuationToken = options.ContinuationToken;
    blobOptions.PageSizeHint = options.PageSizeHint;
    auto result = m_blobServiceClient.ListBlobContainersSinglePage(blobOptions);
    auto response = Models::ListFileSystemsSinglePageResult();
    response.ContinuationToken = std::move(result->ContinuationToken);
    response.PreviousContinuationToken = std::move(result->PreviousContinuationToken);
    response.RequestId = std::move(result->RequestId);
    response.ServiceEndpoint = std::move(result->ServiceEndpoint);
    response.Prefix = std::move(result->Prefix);
    response.Items = FileSystemsFromContainerItems(std::move(result->Items));
    return Azure::Core::Response<Models::ListFileSystemsSinglePageResult>(
        std::move(response), result.ExtractRawResponse());
  }

}}}} // namespace Azure::Storage::Files::DataLake
