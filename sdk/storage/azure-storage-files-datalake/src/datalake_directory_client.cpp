// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_directory_client.hpp"

#include <azure/core/http/policy.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_per_retry_policy.hpp>
#include <azure/storage/common/storage_retry_policy.hpp>

#include "azure/storage/files/datalake/datalake_file_client.hpp"
#include "azure/storage/files/datalake/datalake_utilities.hpp"
#include "azure/storage/files/datalake/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  DataLakeDirectoryClient DataLakeDirectoryClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const std::string& directoryName,
      const DataLakeClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto directoryUri = std::move(parsedConnectionString.DataLakeServiceUrl);
    directoryUri.AppendPath(Storage::Details::UrlEncodePath(fileSystemName));
    directoryUri.AppendPath(Storage::Details::UrlEncodePath(directoryName));

    if (parsedConnectionString.KeyCredential)
    {
      return DataLakeDirectoryClient(
          directoryUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return DataLakeDirectoryClient(directoryUri.GetAbsoluteUrl(), options);
    }
  }

  DataLakeDirectoryClient::DataLakeDirectoryClient(
      const std::string& directoryUri,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const DataLakeClientOptions& options)
      : DataLakePathClient(directoryUri, credential, options)
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

  DataLakeDirectoryClient::DataLakeDirectoryClient(
      const std::string& directoryUri,
      std::shared_ptr<Core::TokenCredential> credential,
      const DataLakeClientOptions& options)
      : DataLakePathClient(directoryUri, credential, options)
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

  DataLakeDirectoryClient::DataLakeDirectoryClient(
      const std::string& directoryUri,
      const DataLakeClientOptions& options)
      : DataLakePathClient(directoryUri, options)
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

  DataLakeFileClient DataLakeDirectoryClient::GetFileClient(const std::string& fileName) const
  {
    auto builder = m_dfsUrl;
    builder.AppendPath(Storage::Details::UrlEncodePath(fileName));
    auto blobClient = m_blobClient;
    blobClient.m_blobUrl.AppendPath(Storage::Details::UrlEncodePath(fileName));
    auto blockBlobClient = blobClient.AsBlockBlobClient();
    return DataLakeFileClient(
        std::move(builder), std::move(blobClient), std::move(blockBlobClient), m_pipeline);
  }

  DataLakeDirectoryClient DataLakeDirectoryClient::GetSubdirectoryClient(
      const std::string& subdirectoryName) const
  {
    auto builder = m_dfsUrl;
    builder.AppendPath(Storage::Details::UrlEncodePath(subdirectoryName));
    auto blobClient = m_blobClient;
    blobClient.m_blobUrl.AppendPath(Storage::Details::UrlEncodePath(subdirectoryName));
    return DataLakeDirectoryClient(std::move(builder), std::move(blobClient), m_pipeline);
  }

  Azure::Core::Response<Models::RenameDataLakeDirectoryResult> DataLakeDirectoryClient::Rename(
      const std::string& destinationPath,
      const RenameDataLakeDirectoryOptions& options) const
  {
    Azure::Core::Nullable<std::string> destinationFileSystem = options.DestinationFileSystem;
    if (!destinationFileSystem.HasValue() || destinationFileSystem.GetValue().empty())
    {
      const auto& currentPath = m_dfsUrl.GetPath();
      std::string::const_iterator cur = currentPath.begin();
      destinationFileSystem = Details::GetSubstringTillDelimiter('/', currentPath, cur);
    }
    auto destinationDfsUri = m_dfsUrl;
    destinationDfsUri.SetPath(destinationFileSystem.GetValue() + '/' + destinationPath);

    Details::DataLakeRestClient::Path::CreateOptions protocolLayerOptions;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.Mode = options.Mode;
    protocolLayerOptions.SourceLeaseId = options.SourceAccessConditions.LeaseId;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.SourceIfMatch = options.SourceAccessConditions.IfMatch;
    protocolLayerOptions.SourceIfNoneMatch = options.SourceAccessConditions.IfNoneMatch;
    protocolLayerOptions.SourceIfModifiedSince = options.SourceAccessConditions.IfModifiedSince;
    protocolLayerOptions.SourceIfUnmodifiedSince = options.SourceAccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.RenameSource = "/" + m_dfsUrl.GetPath();
    auto result = Details::DataLakeRestClient::Path::Create(
        destinationDfsUri, *m_pipeline, options.Context, protocolLayerOptions);
    // At this point, there is not more exception thrown, meaning the rename is successful.
    Models::RenameDataLakeDirectoryResult ret;
    ret.ContinuationToken = std::move(result->ContinuationToken);
    ret.RequestId = std::move(result->RequestId);
    return Azure::Core::Response<Models::RenameDataLakeDirectoryResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteDataLakeDirectoryResult> DataLakeDirectoryClient::Delete(
      bool recursive,
      const DeleteDataLakeDirectoryOptions& options) const
  {
    DeleteDataLakePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    deleteOptions.Context = options.Context;
    deleteOptions.ContinuationToken = options.ContinuationToken;
    deleteOptions.Recursive = recursive;
    return DataLakePathClient::Delete(deleteOptions);
  }

  Azure::Core::Response<Models::DeleteDataLakeDirectoryResult> DataLakeDirectoryClient::
      DeleteIfExists(bool recursive, const DeleteDataLakeDirectoryOptions& options) const
  {
    DeleteDataLakePathOptions deleteOptions;
    deleteOptions.AccessConditions = options.AccessConditions;
    deleteOptions.Context = options.Context;
    deleteOptions.ContinuationToken = options.ContinuationToken;
    deleteOptions.Recursive = recursive;
    return DataLakePathClient::DeleteIfExists(deleteOptions);
  }

  Azure::Core::Response<Models::SetDataLakeDirectoryAccessControlRecursiveResult>
  DataLakeDirectoryClient::SetAccessControlRecursive(
      Models::PathSetAccessControlRecursiveMode mode,
      std::vector<Models::Acl> acls,
      const SetDataLakeDirectoryAccessControlRecursiveOptions& options) const
  {
    Details::DataLakeRestClient::Path::SetAccessControlRecursiveOptions protocolLayerOptions;
    protocolLayerOptions.Mode = mode;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxRecords = options.MaxRecords;
    protocolLayerOptions.ForceFlag = options.ForceFlag;
    protocolLayerOptions.Acl = Models::Acl::SerializeAcls(acls);
    return Details::DataLakeRestClient::Path::SetAccessControlRecursive(
        m_dfsUrl, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ListPathsSinglePageResult> DataLakeDirectoryClient::
      ListPathsSinglePage(bool recursive, const ListPathsSinglePageOptions& options) const
  {
    Details::DataLakeRestClient::FileSystem::ListPathsOptions protocolLayerOptions;
    protocolLayerOptions.Resource = Models::FileSystemResourceType::Filesystem;
    protocolLayerOptions.Upn = options.UserPrincipalName;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.RecursiveRequired = recursive;
    auto currentPath = m_dfsUrl.GetPath();
    // Remove the filesystem name and get directory name.
    auto firstSlashPos = currentPath.find_first_of("/");

    if (firstSlashPos == 0 || (firstSlashPos == currentPath.size() + 1U))
    {
      return Details::DataLakeRestClient::FileSystem::ListPaths(
          m_dfsUrl, *m_pipeline, options.Context, protocolLayerOptions);
    }
    else
    {
      protocolLayerOptions.Directory
          = currentPath.substr(firstSlashPos + 1U, currentPath.size() - firstSlashPos - 1U);
      auto fileSystemUrl = m_dfsUrl;
      fileSystemUrl.SetPath(currentPath.substr(
          0U, currentPath.size() - protocolLayerOptions.Directory.GetValue().size() - 1U));
      return Details::DataLakeRestClient::FileSystem::ListPaths(
          fileSystemUrl, *m_pipeline, options.Context, protocolLayerOptions);
    }
  }

}}}} // namespace Azure::Storage::Files::DataLake
