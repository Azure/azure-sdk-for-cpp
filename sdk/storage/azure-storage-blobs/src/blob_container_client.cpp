// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_container_client.hpp"

#include <azure/core/http/policy.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/shared_key_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_per_retry_policy.hpp>
#include <azure/storage/common/storage_switch_to_secondary_policy.hpp>

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
    auto parsedConnectionString = Storage::_detail::ParseConnectionString(connectionString);
    auto blobContainerUrl = std::move(parsedConnectionString.BlobServiceUrl);
    blobContainerUrl.AppendPath(Storage::_detail::UrlEncodePath(blobContainerName));

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
    BlobClientOptions newOptions = options;
    newOptions.PerRetryPolicies.emplace_back(
        std::make_unique<Storage::_detail::SharedKeyPolicy>(credential));

    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(
        std::make_unique<Storage::_detail::StorageSwitchToSecondaryPolicy>(
            m_blobContainerUrl.GetHost(), newOptions.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<Storage::_detail::StoragePerRetryPolicy>());
    {
      Azure::Core::Http::_internal::ValueOptions valueOptions;
      valueOptions.HeaderValues[Storage::_detail::HttpHeaderXMsVersion] = newOptions.ApiVersion;
      perOperationPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::_internal::ValuePolicy>(valueOptions));
    }
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        newOptions,
        Storage::_detail::FileServicePackageName,
        _detail::Version::VersionString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  BlobContainerClient::BlobContainerClient(
      const std::string& blobContainerUrl,
      std::shared_ptr<Core::TokenCredential> credential,
      const BlobClientOptions& options)
      : BlobContainerClient(blobContainerUrl, options)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(
        std::make_unique<Storage::_detail::StorageSwitchToSecondaryPolicy>(
            m_blobContainerUrl.GetHost(), options.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<Storage::_detail::StoragePerRetryPolicy>());
    {
      Azure::Core::Http::TokenRequestOptions tokenOptions;
      tokenOptions.Scopes.emplace_back(Storage::_detail::StorageScope);
      perRetryPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::BearerTokenAuthenticationPolicy>(
              credential, tokenOptions));
    }
    {
      Azure::Core::Http::_internal::ValueOptions valueOptions;
      valueOptions.HeaderValues[Storage::_detail::HttpHeaderXMsVersion] = options.ApiVersion;
      perOperationPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::_internal::ValuePolicy>(valueOptions));
    }
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        Storage::_detail::FileServicePackageName,
        _detail::Version::VersionString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  BlobContainerClient::BlobContainerClient(
      const std::string& blobContainerUrl,
      const BlobClientOptions& options)
      : m_blobContainerUrl(blobContainerUrl), m_customerProvidedKey(options.CustomerProvidedKey),
        m_encryptionScope(options.EncryptionScope)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(
        std::make_unique<Storage::_detail::StorageSwitchToSecondaryPolicy>(
            m_blobContainerUrl.GetHost(), options.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<Storage::_detail::StoragePerRetryPolicy>());
    {
      Azure::Core::Http::_internal::ValueOptions valueOptions;
      valueOptions.HeaderValues[Storage::_detail::HttpHeaderXMsVersion] = options.ApiVersion;
      perOperationPolicies.emplace_back(
          std::make_unique<Azure::Core::Http::_internal::ValuePolicy>(valueOptions));
    }
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        Storage::_detail::FileServicePackageName,
        _detail::Version::VersionString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  BlobClient BlobContainerClient::GetBlobClient(const std::string& blobName) const
  {
    auto blobUrl = m_blobContainerUrl;
    blobUrl.AppendPath(Storage::_detail::UrlEncodePath(blobName));
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

  Azure::Response<Models::CreateBlobContainerResult> BlobContainerClient::Create(
      const CreateBlobContainerOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::BlobContainer::CreateBlobContainerOptions protocolLayerOptions;
    protocolLayerOptions.AccessType = options.AccessType;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.DefaultEncryptionScope = options.DefaultEncryptionScope;
    protocolLayerOptions.PreventEncryptionScopeOverride = options.PreventEncryptionScopeOverride;
    return _detail::BlobRestClient::BlobContainer::Create(
        context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Response<Models::CreateBlobContainerResult> BlobContainerClient::CreateIfNotExists(
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
        return Azure::Response<Models::CreateBlobContainerResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::DeleteBlobContainerResult> BlobContainerClient::Delete(
      const DeleteBlobContainerOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::BlobContainer::DeleteBlobContainerOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return _detail::BlobRestClient::BlobContainer::Delete(
        context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Response<Models::DeleteBlobContainerResult> BlobContainerClient::DeleteIfExists(
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
        return Azure::Response<Models::DeleteBlobContainerResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::GetBlobContainerPropertiesResult> BlobContainerClient::GetProperties(
      const GetBlobContainerPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::BlobContainer::GetBlobContainerPropertiesOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return _detail::BlobRestClient::BlobContainer::GetProperties(
        Storage::_detail::WithReplicaStatus(context),
        *m_pipeline,
        m_blobContainerUrl,
        protocolLayerOptions);
  }

  Azure::Response<Models::SetBlobContainerMetadataResult> BlobContainerClient::SetMetadata(
      Metadata metadata,
      SetBlobContainerMetadataOptions options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::BlobContainer::SetBlobContainerMetadataOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = metadata;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    return _detail::BlobRestClient::BlobContainer::SetMetadata(
        context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Response<Models::ListBlobsSinglePageResult> BlobContainerClient::ListBlobsSinglePage(
      const ListBlobsSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::BlobContainer::ListBlobsSinglePageOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Include = options.Include;
    auto response = _detail::BlobRestClient::BlobContainer::ListBlobsSinglePage(
        Storage::_detail::WithReplicaStatus(context),
        *m_pipeline,
        m_blobContainerUrl,
        protocolLayerOptions);
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

  Azure::Response<Models::ListBlobsByHierarchySinglePageResult>
  BlobContainerClient::ListBlobsByHierarchySinglePage(
      const std::string& delimiter,
      const ListBlobsSinglePageOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::BlobContainer::ListBlobsByHierarchySinglePageOptions
        protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Delimiter = delimiter;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Include = options.Include;
    auto response = _detail::BlobRestClient::BlobContainer::ListBlobsByHierarchySinglePage(
        Storage::_detail::WithReplicaStatus(context),
        *m_pipeline,
        m_blobContainerUrl,
        protocolLayerOptions);
    for (auto& i : response->Items)
    {
      if (i.VersionId.HasValue() && !i.IsCurrentVersion.HasValue())
      {
        i.IsCurrentVersion = false;
      }
    }
    return response;
  }

  Azure::Response<Models::GetBlobContainerAccessPolicyResult> BlobContainerClient::GetAccessPolicy(
      const GetBlobContainerAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::BlobContainer::GetBlobContainerAccessPolicyOptions
        protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return _detail::BlobRestClient::BlobContainer::GetAccessPolicy(
        Storage::_detail::WithReplicaStatus(context),
        *m_pipeline,
        m_blobContainerUrl,
        protocolLayerOptions);
  }

  Azure::Response<Models::SetBlobContainerAccessPolicyResult> BlobContainerClient::SetAccessPolicy(
      const SetBlobContainerAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::BlobContainer::SetBlobContainerAccessPolicyOptions
        protocolLayerOptions;
    protocolLayerOptions.AccessType = options.AccessType;
    protocolLayerOptions.SignedIdentifiers = options.SignedIdentifiers;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return _detail::BlobRestClient::BlobContainer::SetAccessPolicy(
        context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Response<Models::DeleteBlobResult> BlobContainerClient::DeleteBlob(
      const std::string& blobName,
      const DeleteBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    auto blobClient = GetBlobClient(blobName);
    return blobClient.Delete(options, context);
  }

  Azure::Response<BlockBlobClient> BlobContainerClient::UploadBlob(
      const std::string& blobName,
      Azure::IO::BodyStream* content,
      const UploadBlockBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    auto blockBlobClient = GetBlockBlobClient(blobName);
    auto response = blockBlobClient.Upload(content, options, context);
    return Azure::Response<BlockBlobClient>(
        std::move(blockBlobClient), response.ExtractRawResponse());
  }

}}} // namespace Azure::Storage::Blobs
