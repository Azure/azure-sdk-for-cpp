// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_container_client.hpp"

#include <azure/core/http/policy.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>

#include "azure/storage/blobs/append_blob_client.hpp"
#include "azure/storage/blobs/block_blob_client.hpp"
#include "azure/storage/blobs/page_blob_client.hpp"
#include "azure/storage/blobs/version.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobContainerClient BlobContainerClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& blobContainerName,
      const BlobClientOptions& options)
  {
    auto parsedConnectionString = Storage::Details::ParseConnectionString(connectionString);
    auto blobContainerUrl = std::move(parsedConnectionString.BlobServiceUrl);
    blobContainerUrl.AppendPath(Storage::Details::UrlEncodePath(blobContainerName));

    if (parsedConnectionString.KeyCredential)
    {
      return BlobContainerClient(
          blobContainerUrl.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return BlobContainerClient(blobContainerUrl.GetAbsoluteUrl(), options);
    }
  }

  BlobContainerClient::BlobContainerClient(
      const std::string& blobContainerUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const BlobClientOptions& options)
      : BlobContainerClient(blobContainerUrl, options)
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

  BlobContainerClient::BlobContainerClient(
      const std::string& blobContainerUrl,
      std::shared_ptr<Core::TokenCredential> credential,
      const BlobClientOptions& options)
      : BlobContainerClient(blobContainerUrl, options)
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

  BlobContainerClient::BlobContainerClient(
      const std::string& blobContainerUrl,
      const BlobClientOptions& options)
      : m_blobContainerUrl(blobContainerUrl), m_customerProvidedKey(options.CustomerProvidedKey),
        m_encryptionScope(options.EncryptionScope)
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

  BlobClient BlobContainerClient::GetBlobClient(const std::string& blobName) const
  {
    auto blobUrl = m_blobContainerUrl;
    blobUrl.AppendPath(Storage::Details::UrlEncodePath(blobName));
    return BlobClient(std::move(blobUrl), m_pipeline, m_customerProvidedKey, m_encryptionScope);
  }

  BlockBlobClient BlobContainerClient::GetBlockBlobClient(const std::string& blobName) const
  {
    return GetBlobClient(blobName).AsBlockBlobClient();
  }

  AppendBlobClient BlobContainerClient::GetAppendBlobClient(const std::string& blobName) const
  {
    return GetBlobClient(blobName).AsAppendBlobClient();
  }

  PageBlobClient BlobContainerClient::GetPageBlobClient(const std::string& blobName) const
  {
    return GetBlobClient(blobName).AsPageBlobClient();
  }

  Azure::Core::Response<Models::CreateBlobContainerResult> BlobContainerClient::Create(
      const CreateBlobContainerOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::BlobContainer::CreateBlobContainerOptions protocolLayerOptions;
    protocolLayerOptions.AccessType = options.AccessType;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.DefaultEncryptionScope = options.DefaultEncryptionScope;
    protocolLayerOptions.PreventEncryptionScopeOverride = options.PreventEncryptionScopeOverride;
    return Details::BlobRestClient::BlobContainer::Create(
        context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::CreateBlobContainerResult> BlobContainerClient::CreateIfNotExists(
      const CreateBlobContainerOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Create(options, context);
    }
    catch (StorageException& e)
    {
      if (e.StatusCode == Core::Http::HttpStatusCode::Conflict
          && e.ErrorCode == "ContainerAlreadyExists")
      {
        Models::CreateBlobContainerResult ret;
        ret.RequestId = e.RequestId;
        ret.Created = false;
        return Azure::Core::Response<Models::CreateBlobContainerResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::DeleteBlobContainerResult> BlobContainerClient::Delete(
      const DeleteBlobContainerOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::BlobContainer::DeleteBlobContainerOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return Details::BlobRestClient::BlobContainer::Delete(
        context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::DeleteBlobContainerResult> BlobContainerClient::DeleteIfExists(
      const DeleteBlobContainerOptions& options,
      const Azure::Core::Context& context) const
  {
    try
    {
      return Delete(options, context);
    }
    catch (StorageException& e)
    {
      if (e.StatusCode == Core::Http::HttpStatusCode::NotFound
          && e.ErrorCode == "ContainerNotFound")
      {
        Models::DeleteBlobContainerResult ret;
        ret.RequestId = e.RequestId;
        ret.Deleted = false;
        return Azure::Core::Response<Models::DeleteBlobContainerResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::GetBlobContainerPropertiesResult>
  BlobContainerClient::GetProperties(
      const GetBlobContainerPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::BlobContainer::GetBlobContainerPropertiesOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return Details::BlobRestClient::BlobContainer::GetProperties(
        context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetBlobContainerMetadataResult> BlobContainerClient::SetMetadata(
      Metadata metadata,
      SetBlobContainerMetadataOptions options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::BlobContainer::SetBlobContainerMetadataOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = metadata;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    return Details::BlobRestClient::BlobContainer::SetMetadata(
        context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ListBlobsSinglePageResult> BlobContainerClient::ListBlobsSinglePage(
      const ListBlobsSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::BlobContainer::ListBlobsSinglePageOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Include = options.Include;
    auto response = Details::BlobRestClient::BlobContainer::ListBlobsSinglePage(
        context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
    for (auto& i : response->Items)
    {
      if (i.Details.Tier.HasValue() && !i.Details.IsAccessTierInferred.HasValue())
      {
        i.Details.IsAccessTierInferred = false;
      }
      if (i.VersionId.HasValue() && !i.IsCurrentVersion.HasValue())
      {
        i.IsCurrentVersion = false;
      }
      if (i.BlobType == Models::BlobType::AppendBlob && !i.Details.IsSealed)
      {
        i.Details.IsSealed = false;
      }
    }
    return response;
  }

  Azure::Core::Response<Models::ListBlobsByHierarchySinglePageResult>
  BlobContainerClient::ListBlobsByHierarchySinglePage(
      const std::string& delimiter,
      const ListBlobsSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::BlobContainer::ListBlobsByHierarchySinglePageOptions
        protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Delimiter = delimiter;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Include = options.Include;
    auto response = Details::BlobRestClient::BlobContainer::ListBlobsByHierarchySinglePage(
        context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
    for (auto& i : response->Items)
    {
      if (i.VersionId.HasValue() && !i.IsCurrentVersion.HasValue())
      {
        i.IsCurrentVersion = false;
      }
    }
    return response;
  }

  Azure::Core::Response<Models::GetBlobContainerAccessPolicyResult>
  BlobContainerClient::GetAccessPolicy(
      const GetBlobContainerAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::BlobContainer::GetBlobContainerAccessPolicyOptions
        protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return Details::BlobRestClient::BlobContainer::GetAccessPolicy(
        context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetBlobContainerAccessPolicyResult>
  BlobContainerClient::SetAccessPolicy(
      const SetBlobContainerAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    Details::BlobRestClient::BlobContainer::SetBlobContainerAccessPolicyOptions
        protocolLayerOptions;
    protocolLayerOptions.AccessType = options.AccessType;
    protocolLayerOptions.SignedIdentifiers = options.SignedIdentifiers;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return Details::BlobRestClient::BlobContainer::SetAccessPolicy(
        context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<void> BlobContainerClient::DeleteBlob(
      const std::string& blobName,
      const DeleteBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    auto blobClient = GetBlobClient(blobName);
    auto response = blobClient.Delete(options, context);
    return Azure::Core::Response<void>(response.ExtractRawResponse());
  }

  Azure::Core::Response<BlockBlobClient> BlobContainerClient::UploadBlob(
      const std::string& blobName,
      Azure::Core::Http::BodyStream* content,
      const UploadBlockBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    auto blockBlobClient = GetBlockBlobClient(blobName);
    auto response = blockBlobClient.Upload(content, options, context);
    return Azure::Core::Response<BlockBlobClient>(
        std::move(blockBlobClient), response.ExtractRawResponse());
  }

}}} // namespace Azure::Storage::Blobs
