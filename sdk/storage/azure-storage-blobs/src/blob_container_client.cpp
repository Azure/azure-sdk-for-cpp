// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_container_client.hpp"

#include <azure/core/http/policies/policy.hpp>
#include <azure/storage/common/crypt.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/shared_key_policy.hpp>
#include <azure/storage/common/internal/storage_per_retry_policy.hpp>
#include <azure/storage/common/internal/storage_service_version_policy.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_exception.hpp>

#include "azure/storage/blobs/append_blob_client.hpp"
#include "azure/storage/blobs/block_blob_client.hpp"
#include "azure/storage/blobs/page_blob_client.hpp"

#include "private/package_version.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobContainerClient BlobContainerClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& blobContainerName,
      const BlobClientOptions& options)
  {
    auto parsedConnectionString = _internal::ParseConnectionString(connectionString);
    auto blobContainerUrl = std::move(parsedConnectionString.BlobServiceUrl);
    blobContainerUrl.AppendPath(_internal::UrlEncodePath(blobContainerName));

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
        std::make_unique<_internal::SharedKeyPolicy>(credential));

    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_blobContainerUrl.GetHost(), newOptions.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(newOptions.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        newOptions,
        _internal::BlobServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  BlobContainerClient::BlobContainerClient(
      const std::string& blobContainerUrl,
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
      const BlobClientOptions& options)
      : BlobContainerClient(blobContainerUrl, options)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_blobContainerUrl.GetHost(), options.SecondaryHostForRetryReads));
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
        _internal::BlobServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  BlobContainerClient::BlobContainerClient(
      const std::string& blobContainerUrl,
      const BlobClientOptions& options)
      : m_blobContainerUrl(blobContainerUrl), m_customerProvidedKey(options.CustomerProvidedKey),
        m_encryptionScope(options.EncryptionScope)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perRetryPolicies;
    std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> perOperationPolicies;
    perRetryPolicies.emplace_back(std::make_unique<_internal::StorageSwitchToSecondaryPolicy>(
        m_blobContainerUrl.GetHost(), options.SecondaryHostForRetryReads));
    perRetryPolicies.emplace_back(std::make_unique<_internal::StoragePerRetryPolicy>());
    perOperationPolicies.emplace_back(
        std::make_unique<_internal::StorageServiceVersionPolicy>(options.ApiVersion));
    m_pipeline = std::make_shared<Azure::Core::Http::_internal::HttpPipeline>(
        options,
        _internal::BlobServicePackageName,
        _detail::PackageVersion::ToString(),
        std::move(perRetryPolicies),
        std::move(perOperationPolicies));
  }

  BlobClient BlobContainerClient::GetBlobClient(const std::string& blobName) const
  {
    auto blobUrl = m_blobContainerUrl;
    blobUrl.AppendPath(_internal::UrlEncodePath(blobName));
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
    _detail::BlobContainerClient::CreateBlobContainerOptions protocolLayerOptions;
    protocolLayerOptions.Access = options.AccessType;
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(options.Metadata.begin(), options.Metadata.end());
    protocolLayerOptions.DefaultEncryptionScope = options.DefaultEncryptionScope;
    protocolLayerOptions.PreventEncryptionScopeOverride = options.PreventEncryptionScopeOverride;
    return _detail::BlobContainerClient::Create(
        *m_pipeline, m_blobContainerUrl, protocolLayerOptions, context);
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
    _detail::BlobContainerClient::DeleteBlobContainerOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return _detail::BlobContainerClient::Delete(
        *m_pipeline, m_blobContainerUrl, protocolLayerOptions, context);
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
        ret.Deleted = false;
        return Azure::Response<Models::DeleteBlobContainerResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::BlobContainerProperties> BlobContainerClient::GetProperties(
      const GetBlobContainerPropertiesOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobContainerClient::GetBlobContainerPropertiesOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return _detail::BlobContainerClient::GetProperties(
        *m_pipeline,
        m_blobContainerUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));
  }

  Azure::Response<Models::SetBlobContainerMetadataResult> BlobContainerClient::SetMetadata(
      Metadata metadata,
      SetBlobContainerMetadataOptions options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobContainerClient::SetBlobContainerMetadataOptions protocolLayerOptions;
    protocolLayerOptions.Metadata
        = std::map<std::string, std::string>(metadata.begin(), metadata.end());
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    return _detail::BlobContainerClient::SetMetadata(
        *m_pipeline, m_blobContainerUrl, protocolLayerOptions, context);
  }

  ListBlobsPagedResponse BlobContainerClient::ListBlobs(
      const ListBlobsOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobContainerClient::ListBlobContainerBlobsOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Marker = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Include = options.Include;
    auto response = _detail::BlobContainerClient::ListBlobs(
        *m_pipeline,
        m_blobContainerUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));
    for (auto& i : response.Value.Items)
    {
      if (i.Details.AccessTier.HasValue() && !i.Details.IsAccessTierInferred.HasValue())
      {
        i.Details.IsAccessTierInferred = false;
      }
      if (i.VersionId.HasValue())
      {
        if (!i.HasVersionsOnly.HasValue())
        {
          i.HasVersionsOnly = false;
        }
        if (!i.IsCurrentVersion.HasValue())
        {
          i.IsCurrentVersion = false;
        }
      }
      if (i.BlobType == Models::BlobType::AppendBlob && !i.Details.IsSealed.HasValue())
      {
        i.Details.IsSealed = false;
      }
      if (i.Details.CopyStatus.HasValue() && !i.Details.IsIncrementalCopy.HasValue())
      {
        i.Details.IsIncrementalCopy = false;
      }
      {
        std::map<std::string, std::vector<Models::ObjectReplicationRule>> orPropertiesMap;
        for (auto& policy : i.Details.ObjectReplicationSourceProperties)
        {
          for (auto& rule : policy.Rules)
          {
            auto underscorePos = rule.RuleId.find('_', 3);
            std::string policyId
                = std::string(rule.RuleId.begin() + 3, rule.RuleId.begin() + underscorePos);
            std::string ruleId = rule.RuleId.substr(underscorePos + 1);
            rule.RuleId = ruleId;
            orPropertiesMap[policyId].emplace_back(std::move(rule));
          }
        }
        i.Details.ObjectReplicationSourceProperties.clear();
        for (auto& property : orPropertiesMap)
        {
          Models::ObjectReplicationPolicy policy;
          policy.PolicyId = property.first;
          policy.Rules = std::move(property.second);
          i.Details.ObjectReplicationSourceProperties.emplace_back(std::move(policy));
        }
      }
    }

    ListBlobsPagedResponse pagedResponse;
    pagedResponse.ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    pagedResponse.BlobContainerName = std::move(response.Value.BlobContainerName);
    pagedResponse.Prefix = std::move(response.Value.Prefix);
    pagedResponse.Blobs = std::move(response.Value.Items);
    pagedResponse.m_blobContainerClient = std::make_shared<BlobContainerClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken;
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  ListBlobsByHierarchyPagedResponse BlobContainerClient::ListBlobsByHierarchy(
      const std::string& delimiter,
      const ListBlobsOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobContainerClient::ListBlobContainerBlobsByHierarchyOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Delimiter = delimiter;
    protocolLayerOptions.Marker = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.PageSizeHint;
    protocolLayerOptions.Include = options.Include;
    auto response = _detail::BlobContainerClient::ListBlobsByHierarchy(
        *m_pipeline,
        m_blobContainerUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));
    for (auto& i : response.Value.Items)
    {
      if (i.Details.AccessTier.HasValue() && !i.Details.IsAccessTierInferred.HasValue())
      {
        i.Details.IsAccessTierInferred = false;
      }
      if (i.VersionId.HasValue())
      {
        if (!i.HasVersionsOnly.HasValue())
        {
          i.HasVersionsOnly = false;
        }
        if (!i.IsCurrentVersion.HasValue())
        {
          i.IsCurrentVersion = false;
        }
      }
      if (i.BlobType == Models::BlobType::AppendBlob && !i.Details.IsSealed.HasValue())
      {
        i.Details.IsSealed = false;
      }
      if (i.Details.CopyStatus.HasValue() && !i.Details.IsIncrementalCopy.HasValue())
      {
        i.Details.IsIncrementalCopy = false;
      }
      {
        std::map<std::string, std::vector<Models::ObjectReplicationRule>> orPropertiesMap;
        for (auto& policy : i.Details.ObjectReplicationSourceProperties)
        {
          for (auto& rule : policy.Rules)
          {
            auto underscorePos = rule.RuleId.find('_', 3);
            std::string policyId
                = std::string(rule.RuleId.begin() + 3, rule.RuleId.begin() + underscorePos);
            std::string ruleId = rule.RuleId.substr(underscorePos + 1);
            rule.RuleId = ruleId;
            orPropertiesMap[policyId].emplace_back(std::move(rule));
          }
        }
        i.Details.ObjectReplicationSourceProperties.clear();
        for (auto& property : orPropertiesMap)
        {
          Models::ObjectReplicationPolicy policy;
          policy.PolicyId = property.first;
          policy.Rules = std::move(property.second);
          i.Details.ObjectReplicationSourceProperties.emplace_back(std::move(policy));
        }
      }
    }

    ListBlobsByHierarchyPagedResponse pagedResponse;

    pagedResponse.ServiceEndpoint = std::move(response.Value.ServiceEndpoint);
    pagedResponse.BlobContainerName = std::move(response.Value.BlobContainerName);
    pagedResponse.Prefix = std::move(response.Value.Prefix);
    pagedResponse.Delimiter = std::move(response.Value.Delimiter);
    pagedResponse.Blobs = std::move(response.Value.Items);
    pagedResponse.BlobPrefixes = std::move(response.Value.BlobPrefixes);
    pagedResponse.m_blobContainerClient = std::make_shared<BlobContainerClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.m_delimiter = delimiter;
    pagedResponse.CurrentPageToken = options.ContinuationToken.ValueOr(std::string());
    pagedResponse.NextPageToken = response.Value.ContinuationToken;
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  Azure::Response<Models::BlobContainerAccessPolicy> BlobContainerClient::GetAccessPolicy(
      const GetBlobContainerAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobContainerClient::GetBlobContainerAccessPolicyOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return _detail::BlobContainerClient::GetAccessPolicy(
        *m_pipeline,
        m_blobContainerUrl,
        protocolLayerOptions,
        _internal::WithReplicaStatus(context));
  }

  Azure::Response<Models::SetBlobContainerAccessPolicyResult> BlobContainerClient::SetAccessPolicy(
      const SetBlobContainerAccessPolicyOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobContainerClient::SetBlobContainerAccessPolicyOptions protocolLayerOptions;
    protocolLayerOptions.Access = options.AccessType;
    protocolLayerOptions.ContainerAcl = options.SignedIdentifiers;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return _detail::BlobContainerClient::SetAccessPolicy(
        *m_pipeline, m_blobContainerUrl, protocolLayerOptions, context);
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
      Azure::Core::IO::BodyStream& content,
      const UploadBlockBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    auto blockBlobClient = GetBlockBlobClient(blobName);
    auto response = blockBlobClient.Upload(content, options, context);
    return Azure::Response<BlockBlobClient>(
        std::move(blockBlobClient), std::move(response.RawResponse));
  }

}}} // namespace Azure::Storage::Blobs
