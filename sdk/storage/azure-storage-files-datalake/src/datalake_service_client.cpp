// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/files/datalake/datalake_service_client.hpp"

#include <azure/core/http/policy.hpp>
#include <azure/storage/blobs/protocol/blob_rest_client.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/files/datalake/datalake_file_system_client.hpp"
#include "azure/storage/files/datalake/datalake_utilities.hpp"
#include "azure/storage/files/datalake/version.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  namespace {
    std::vector<Models::FileSystemItem> FileSystemsFromContainerItems(
        std::vector<Blobs::Models::BlobContainerItem> items)
    {
      std::vector<Models::FileSystemItem> fileSystems;
      for (auto& item : items)
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
          fileSystem.Details.AccessType = Models::PublicAccessType(item.Details.AccessType.Get());
        }
        fileSystem.Details.HasImmutabilityPolicy = item.Details.HasImmutabilityPolicy;
        fileSystem.Details.HasLegalHold = item.Details.HasLegalHold;
        if (item.Details.LeaseDuration.HasValue())
        {
          fileSystem.Details.LeaseDuration
              = Models::LeaseDurationType((item.Details.LeaseDuration.GetValue().Get()));
        }
        fileSystem.Details.LeaseState = Models::LeaseStateType(item.Details.LeaseState.Get());
        fileSystem.Details.LeaseStatus = Models::LeaseStatusType(item.Details.LeaseStatus.Get());

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
                                      Details::GetBlobUrlFromUrl(serviceUrl),
                                      credential,
                                      Details::GetBlobClientOptions(options))
  {
    Azure::Core::Http::TelemetryPolicyOptions telemetryPolicyOptions;
    telemetryPolicyOptions.ApplicationId = options.ApplicationId;
    m_pipeline = std::make_shared<Azure::Core::Internal::Http::HttpPipeline>(
        Storage::Details::ConstructPolicies(
            std::make_unique<Azure::Core::Http::TelemetryPolicy>(
                Storage::Details::BlobServicePackageName,
                Details::Version::VersionString(),
                telemetryPolicyOptions),
            std::make_unique<Storage::Details::SharedKeyPolicy>(credential),
            options));
  }

  DataLakeServiceClient::DataLakeServiceClient(
      const std::string& serviceUrl,
      std::shared_ptr<Core::TokenCredential> credential,
      const DataLakeClientOptions& options)
      : m_serviceUrl(serviceUrl), m_blobServiceClient(
                                      Details::GetBlobUrlFromUrl(serviceUrl),
                                      credential,
                                      Details::GetBlobClientOptions(options))
  {
    Azure::Core::Http::TelemetryPolicyOptions telemetryPolicyOptions;
    telemetryPolicyOptions.ApplicationId = options.ApplicationId;
    Azure::Core::Http::TokenRequestOptions tokenOptions;
    tokenOptions.Scopes.emplace_back(Storage::Details::StorageScope);
    m_pipeline = std::make_shared<Azure::Core::Internal::Http::HttpPipeline>(
        Storage::Details::ConstructPolicies(
            std::make_unique<Azure::Core::Http::TelemetryPolicy>(
                Storage::Details::BlobServicePackageName,
                Details::Version::VersionString(),
                telemetryPolicyOptions),
            std::make_unique<Azure::Core::Http::BearerTokenAuthenticationPolicy>(
                credential, tokenOptions),
            options));
  }

  DataLakeServiceClient::DataLakeServiceClient(
      const std::string& serviceUrl,
      const DataLakeClientOptions& options)
      : m_serviceUrl(serviceUrl), m_blobServiceClient(
                                      Details::GetBlobUrlFromUrl(serviceUrl),
                                      Details::GetBlobClientOptions(options))
  {
    Azure::Core::Http::TelemetryPolicyOptions telemetryPolicyOptions;
    telemetryPolicyOptions.ApplicationId = options.ApplicationId;
    m_pipeline = std::make_shared<Azure::Core::Internal::Http::HttpPipeline>(
        Storage::Details::ConstructPolicies(
            std::make_unique<Azure::Core::Http::TelemetryPolicy>(
                Storage::Details::BlobServicePackageName,
                Details::Version::VersionString(),
                telemetryPolicyOptions),
            nullptr,
            options));
  }

  DataLakeFileSystemClient DataLakeServiceClient::GetFileSystemClient(
      const std::string& fileSystemName) const
  {
    auto builder = m_serviceUrl;
    builder.AppendPath(Storage::Details::UrlEncodePath(fileSystemName));
    return DataLakeFileSystemClient(
        builder, m_blobServiceClient.GetBlobContainerClient(fileSystemName), m_pipeline);
  }

  Azure::Core::Response<Models::ListFileSystemsSinglePageResult>
  DataLakeServiceClient::ListFileSystemsSinglePage(
      const ListFileSystemsSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    Blobs::ListBlobContainersSinglePageOptions blobOptions;
    blobOptions.Include = options.Include;
    blobOptions.Prefix = options.Prefix;
    blobOptions.ContinuationToken = options.ContinuationToken;
    blobOptions.PageSizeHint = options.PageSizeHint;
    auto result = m_blobServiceClient.ListBlobContainersSinglePage(blobOptions, context);
    auto response = Models::ListFileSystemsSinglePageResult();
    response.ContinuationToken = std::move(result->ContinuationToken);
    response.RequestId = std::move(result->RequestId);
    response.ServiceEndpoint = std::move(result->ServiceEndpoint);
    response.Prefix = std::move(result->Prefix);
    response.Items = FileSystemsFromContainerItems(std::move(result->Items));
    return Azure::Core::Response<Models::ListFileSystemsSinglePageResult>(
        std::move(response), result.ExtractRawResponse());
  }

}}}} // namespace Azure::Storage::Files::DataLake
