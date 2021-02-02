// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_file_system_client.hpp"

#include <azure/core/http/policy.hpp>
#include <azure/storage/blobs/protocol/blob_rest_client.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_per_retry_policy.hpp>
#include <azure/storage/common/storage_retry_policy.hpp>

#include "azure/storage/files/datalake/datalake_constants.hpp"
#include "azure/storage/files/datalake/datalake_directory_client.hpp"
#include "azure/storage/files/datalake/datalake_file_client.hpp"
#include "azure/storage/files/datalake/datalake_path_client.hpp"
#include "azure/storage/files/datalake/datalake_utilities.hpp"
#include "azure/storage/files/datalake/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {
  namespace {
    Blobs::BlobClientOptions GetBlobContainerClientOptions(const DataLakeClientOptions& options)
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
  } // namespace

  DataLakeFileSystemClient DataLakeFileSystemClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const DataLakeClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto fileSystemUri = std::move(parsedConnectionString.DataLakeServiceUrl);
    fileSystemUri.AppendPath(Storage::Details::UrlEncodePath(fileSystemName));

    if (parsedConnectionString.KeyCredential)
    {
      return DataLakeFileSystemClient(
          fileSystemUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return DataLakeFileSystemClient(fileSystemUri.GetAbsoluteUrl(), options);
    }
  }

  DataLakeFileSystemClient::DataLakeFileSystemClient(
      const std::string& fileSystemUri,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const DataLakeClientOptions& options)
      : m_dfsUrl(Details::GetDfsUrlFromUrl(fileSystemUri)),
        m_blobContainerClient(
            Details::GetBlobUrlFromUrl(fileSystemUri),
            credential,
            GetBlobContainerClientOptions(options))
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

  DataLakeFileSystemClient::DataLakeFileSystemClient(
      const std::string& fileSystemUri,
      std::shared_ptr<Core::TokenCredential> credential,
      const DataLakeClientOptions& options)
      : m_dfsUrl(Details::GetDfsUrlFromUrl(fileSystemUri)),
        m_blobContainerClient(
            Details::GetBlobUrlFromUrl(fileSystemUri),
            credential,
            GetBlobContainerClientOptions(options))
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

    {
      Azure::Core::Http::TokenRequestOptions const tokenOptions
          = {{Storage::Details::StorageScope}};

      policies.emplace_back(std::make_unique<Azure::Core::Http::BearerTokenAuthenticationPolicy>(
          credential, tokenOptions));
    }

    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  DataLakeFileSystemClient::DataLakeFileSystemClient(
      const std::string& fileSystemUri,
      const DataLakeClientOptions& options)
      : m_dfsUrl(Details::GetDfsUrlFromUrl(fileSystemUri)),
        m_blobContainerClient(
            Details::GetBlobUrlFromUrl(fileSystemUri),
            GetBlobContainerClientOptions(options))
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

  DataLakePathClient DataLakeFileSystemClient::GetPathClient(const std::string& path) const
  {
    auto builder = m_dfsUrl;
    builder.AppendPath(Storage::Details::UrlEncodePath(path));
    return DataLakePathClient(builder, m_blobContainerClient.GetBlobClient(path), m_pipeline);
  }

  DataLakeFileClient DataLakeFileSystemClient::GetFileClient(const std::string& fileName) const
  {

    auto builder = m_dfsUrl;
    builder.AppendPath(Storage::Details::UrlEncodePath(fileName));
    auto blobClient = m_blobContainerClient.GetBlobClient(fileName);
    auto blockBlobClient = blobClient.AsBlockBlobClient();
    return DataLakeFileClient(
        std::move(builder), std::move(blobClient), std::move(blockBlobClient), m_pipeline);
  }

  DataLakeDirectoryClient DataLakeFileSystemClient::GetDirectoryClient(
      const std::string& directoryName) const
  {
    auto builder = m_dfsUrl;
    builder.AppendPath(Storage::Details::UrlEncodePath(directoryName));
    return DataLakeDirectoryClient(
        builder, m_blobContainerClient.GetBlobClient(directoryName), m_pipeline);
  }

  Azure::Core::Response<Models::CreateDataLakeFileSystemResult> DataLakeFileSystemClient::Create(
      const CreateDataLakeFileSystemOptions& options) const
  {
    Blobs::CreateBlobContainerOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.Metadata = options.Metadata;
    if (options.AccessType == Models::PublicAccessType::FileSystem)
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType::BlobContainer;
    }
    else if (options.AccessType == Models::PublicAccessType::Path)
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType::Blob;
    }
    else if (options.AccessType == Models::PublicAccessType::None)
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType::None;
    }
    else
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType(options.AccessType.Get());
    }
    auto result = m_blobContainerClient.Create(blobOptions);
    Models::CreateDataLakeFileSystemResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.Created = true;
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::CreateDataLakeFileSystemResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::CreateDataLakeFileSystemResult>
  DataLakeFileSystemClient::CreateIfNotExists(const CreateDataLakeFileSystemOptions& options) const
  {
    try
    {
      return Create(options);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == Details::ContainerAlreadyExists)
      {
        Models::CreateDataLakeFileSystemResult ret;
        ret.Created = false;
        return Azure::Core::Response<Models::CreateDataLakeFileSystemResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::DeleteDataLakeFileSystemResult> DataLakeFileSystemClient::Delete(
      const DeleteDataLakeFileSystemOptions& options) const
  {
    Blobs::DeleteBlobContainerOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobContainerClient.Delete(blobOptions);
    Models::DeleteDataLakeFileSystemResult ret;
    ret.Deleted = true;
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::DeleteDataLakeFileSystemResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteDataLakeFileSystemResult>
  DataLakeFileSystemClient::DeleteIfExists(const DeleteDataLakeFileSystemOptions& options) const
  {
    try
    {
      return Delete(options);
    }
    catch (StorageException& e)
    {
      if (e.ErrorCode == Details::ContainerNotFound)
      {
        Models::DeleteDataLakeFileSystemResult ret;
        ret.Deleted = false;
        return Azure::Core::Response<Models::DeleteDataLakeFileSystemResult>(
            ret, std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::GetDataLakeFileSystemPropertiesResult>
  DataLakeFileSystemClient::GetProperties(
      const GetDataLakeFileSystemPropertiesOptions& options) const
  {
    Blobs::GetBlobContainerPropertiesOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobContainerClient.GetProperties(blobOptions);
    Models::GetDataLakeFileSystemPropertiesResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.Metadata = std::move(result->Metadata);
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::GetDataLakeFileSystemPropertiesResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::SetDataLakeFileSystemMetadataResult>
  DataLakeFileSystemClient::SetMetadata(
      Storage::Metadata metadata,
      const SetDataLakeFileSystemMetadataOptions& options) const
  {
    Blobs::SetBlobContainerMetadataOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    if (options.AccessConditions.IfUnmodifiedSince.HasValue())
    {
      std::abort();
    }
    auto result = m_blobContainerClient.SetMetadata(std::move(metadata), blobOptions);
    Models::SetDataLakeFileSystemMetadataResult ret;
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::SetDataLakeFileSystemMetadataResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::ListPathsSinglePageResult> DataLakeFileSystemClient::
      ListPathsSinglePage(bool recursive, const ListPathsSinglePageOptions& options) const
  {
    Details::DataLakeRestClient::FileSystem::ListPathsOptions protocolLayerOptions;
    protocolLayerOptions.Resource = Models::FileSystemResourceType::Filesystem;
    protocolLayerOptions.Upn = options.UserPrincipalName;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.RecursiveRequired = recursive;
    return Details::DataLakeRestClient::FileSystem::ListPaths(
        m_dfsUrl, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetDataLakeFileSystemAccessPolicyResult>
  DataLakeFileSystemClient::GetAccessPolicy(
      const GetDataLakeFileSystemAccessPolicyOptions& options) const
  {
    Blobs::GetBlobContainerAccessPolicyOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    auto result = m_blobContainerClient.GetAccessPolicy(blobOptions);
    Models::GetDataLakeFileSystemAccessPolicyResult ret;
    if (result->AccessType == Blobs::Models::PublicAccessType::BlobContainer)
    {
      ret.AccessType = Models::PublicAccessType::FileSystem;
    }
    else if (result->AccessType == Blobs::Models::PublicAccessType::Blob)
    {
      ret.AccessType = Models::PublicAccessType::Path;
    }
    else if (result->AccessType == Blobs::Models::PublicAccessType::None)
    {
      ret.AccessType = Models::PublicAccessType::None;
    }
    else
    {
      ret.AccessType = Models::PublicAccessType(result->AccessType.Get());
    }
    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.SignedIdentifiers = std::move(result->SignedIdentifiers);
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::GetDataLakeFileSystemAccessPolicyResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::SetDataLakeFileSystemAccessPolicyResult>
  DataLakeFileSystemClient::SetAccessPolicy(
      const SetDataLakeFileSystemAccessPolicyOptions& options) const
  {
    Blobs::SetBlobContainerAccessPolicyOptions blobOptions;
    blobOptions.Context = options.Context;
    blobOptions.AccessConditions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    blobOptions.AccessConditions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    blobOptions.AccessConditions.LeaseId = options.AccessConditions.LeaseId;
    blobOptions.SignedIdentifiers = options.SignedIdentifiers;
    if (options.AccessType == Models::PublicAccessType::FileSystem)
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType::BlobContainer;
    }
    else if (options.AccessType == Models::PublicAccessType::Path)
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType::Blob;
    }
    else if (options.AccessType == Models::PublicAccessType::None)
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType::None;
    }
    else
    {
      blobOptions.AccessType = Blobs::Models::PublicAccessType(options.AccessType.Get());
    }
    auto result = m_blobContainerClient.SetAccessPolicy(blobOptions);
    Models::SetDataLakeFileSystemAccessPolicyResult ret;

    ret.ETag = std::move(result->ETag);
    ret.LastModified = std::move(result->LastModified);
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::SetDataLakeFileSystemAccessPolicyResult>(
        std::move(ret), result.ExtractRawResponse());
  }

}}}} // namespace Azure::Storage::Files::DataLake
