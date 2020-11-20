// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_directory_client.hpp"

#include "azure/core/credentials.hpp"
#include "azure/core/http/policy.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/crypt.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"
#include "azure/storage/common/storage_retry_policy.hpp"
#include "azure/storage/files/datalake/datalake_file_client.hpp"
#include "azure/storage/files/datalake/datalake_utilities.hpp"
#include "azure/storage/files/datalake/version.hpp"

#include <limits>
#include <utility> //std::pair

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  DirectoryClient DirectoryClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& fileSystemName,
      const std::string& path,
      const DataLakeClientOptions& options)
  {
    auto parsedConnectionString = Azure::Storage::Details::ParseConnectionString(connectionString);
    auto directoryUri = std::move(parsedConnectionString.DataLakeServiceUrl);
    directoryUri.AppendPath(Storage::Details::UrlEncodePath(fileSystemName));
    directoryUri.AppendPath(Storage::Details::UrlEncodePath(path));

    if (parsedConnectionString.KeyCredential)
    {
      return DirectoryClient(
          directoryUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return DirectoryClient(directoryUri.GetAbsoluteUrl(), options);
    }
  }

  DirectoryClient::DirectoryClient(
      const std::string& directoryUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const DataLakeClientOptions& options)
      : PathClient(directoryUri, credential, options)
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
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  DirectoryClient::DirectoryClient(
      const std::string& directoryUri,
      std::shared_ptr<Identity::ClientSecretCredential> credential,
      const DataLakeClientOptions& options)
      : PathClient(directoryUri, credential, options)
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
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  DirectoryClient::DirectoryClient(
      const std::string& directoryUri,
      const DataLakeClientOptions& options)
      : PathClient(directoryUri, options)
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
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  FileClient DirectoryClient::GetFileClient(const std::string& path) const
  {
    auto builder = m_dfsUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(path));
    auto blobClient = m_blobClient;
    blobClient.m_blobUrl.AppendPath(Storage::Details::UrlEncodePath(path));
    auto blockBlobClient = blobClient.AsBlockBlobClient();
    return FileClient(
        std::move(builder), std::move(blobClient), std::move(blockBlobClient), m_pipeline);
  }

  DirectoryClient DirectoryClient::GetSubDirectoryClient(const std::string& path) const
  {
    auto builder = m_dfsUri;
    builder.AppendPath(Storage::Details::UrlEncodePath(path));
    auto blobClient = m_blobClient;
    blobClient.m_blobUrl.AppendPath(Storage::Details::UrlEncodePath(path));
    return DirectoryClient(std::move(builder), std::move(blobClient), m_pipeline);
  }

  Azure::Core::Response<Models::RenameDirectoryResult> DirectoryClient::Rename(
      const std::string& destinationPath,
      const RenameDirectoryOptions& options) const
  {
    Azure::Core::Nullable<std::string> destinationFileSystem = options.DestinationFileSystem;
    if (!destinationFileSystem.HasValue() || destinationFileSystem.GetValue().empty())
    {
      const auto& currentPath = m_dfsUri.GetPath();
      std::string::const_iterator cur = currentPath.begin();
      destinationFileSystem = Details::GetSubstringTillDelimiter('/', currentPath, cur);
    }
    auto destinationDfsUri = m_dfsUri;
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
    protocolLayerOptions.RenameSource = "/" + m_dfsUri.GetPath();
    auto result = Details::DataLakeRestClient::Path::Create(
        destinationDfsUri, *m_pipeline, options.Context, protocolLayerOptions);
    // At this point, there is not more exception thrown, meaning the rename is successful.
    auto ret = Models::RenameDirectoryResult();
    ret.ContinuationToken = std::move(result->ContinuationToken);
    return Azure::Core::Response<Models::RenameDirectoryResult>(
        std::move(ret), result.ExtractRawResponse());
  }

  Azure::Core::Response<Models::DeleteDirectoryResult> DirectoryClient::Delete(
      bool recursive,
      const DeleteDirectoryOptions& options) const
  {
    Details::DataLakeRestClient::Path::DeleteOptions protocolLayerOptions;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.LeaseIdOptional = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.RecursiveOptional = recursive;
    return Details::DataLakeRestClient::Path::Delete(
        m_dfsUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetDirectoryAccessControlRecursiveResult>
  DirectoryClient::SetAccessControlRecursive(
      Models::PathSetAccessControlRecursiveMode mode,
      std::vector<Models::Acl> acls,
      const SetDirectoryAccessControlRecursiveOptions& options) const
  {
    Details::DataLakeRestClient::Path::SetAccessControlRecursiveOptions protocolLayerOptions;
    protocolLayerOptions.Mode = mode;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxRecords = options.MaxRecords;
    protocolLayerOptions.ForceFlag = options.ForceFlag;
    protocolLayerOptions.Acl = Models::Acl::SerializeAcls(acls);
    return Details::DataLakeRestClient::Path::SetAccessControlRecursive(
        m_dfsUri, *m_pipeline, options.Context, protocolLayerOptions);
  }

}}}} // namespace Azure::Storage::Files::DataLake
