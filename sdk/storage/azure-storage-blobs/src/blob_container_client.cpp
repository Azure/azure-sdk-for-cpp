// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_container_client.hpp"

#include "azure/core/credentials.hpp"
#include "azure/core/http/policy.hpp"
#include "azure/storage/blobs/append_blob_client.hpp"
#include "azure/storage/blobs/block_blob_client.hpp"
#include "azure/storage/blobs/page_blob_client.hpp"
#include "azure/storage/blobs/version.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/shared_key_policy.hpp"
#include "azure/storage/common/storage_common.hpp"
#include "azure/storage/common/storage_per_retry_policy.hpp"

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
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Storage::Details::BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Storage::Details::StorageRetryPolicy>(options.RetryOptions));
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

  BlobContainerClient::BlobContainerClient(
      const std::string& blobContainerUrl,
      std::shared_ptr<Core::TokenCredential> credential,
      const BlobClientOptions& options)
      : BlobContainerClient(blobContainerUrl, options)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Storage::Details::BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Storage::Details::StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Core::BearerTokenAuthenticationPolicy>(
        credential, Storage::Details::StorageScope));
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobContainerClient::BlobContainerClient(
      const std::string& blobContainerUrl,
      const BlobClientOptions& options)
      : m_blobContainerUrl(blobContainerUrl), m_customerProvidedKey(options.CustomerProvidedKey),
        m_encryptionScope(options.EncryptionScope)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Storage::Details::BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(
        std::make_unique<Storage::Details::StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<Storage::Details::StoragePerRetryPolicy>());
    policies.emplace_back(
        std::make_unique<Azure::Core::Http::TransportPolicy>(options.TransportPolicyOptions));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
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
      const CreateBlobContainerOptions& options) const
  {
    Details::BlobRestClient::BlobContainer::CreateBlobContainerOptions protocolLayerOptions;
    protocolLayerOptions.AccessType = options.AccessType;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.DefaultEncryptionScope = options.DefaultEncryptionScope;
    protocolLayerOptions.PreventEncryptionScopeOverride = options.PreventEncryptionScopeOverride;
    return Details::BlobRestClient::BlobContainer::Create(
        options.Context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::DeleteBlobContainerResult> BlobContainerClient::Delete(
      const DeleteBlobContainerOptions& options) const
  {
    Details::BlobRestClient::BlobContainer::DeleteBlobContainerOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return Details::BlobRestClient::BlobContainer::Delete(
        options.Context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::UndeleteBlobContainerResult> BlobContainerClient::Undelete(
      const std::string& deletedBlobContainerName,
      const std::string& deletedBlobContainerVersion,
      const UndeleteBlobContainerOptions& options) const
  {
    Details::BlobRestClient::BlobContainer::UndeleteBlobContainerOptions protocolLayerOptions;
    protocolLayerOptions.DeletedBlobContainerName = deletedBlobContainerName;
    protocolLayerOptions.DeletedBlobContainerVersion = deletedBlobContainerVersion;
    return Details::BlobRestClient::BlobContainer::Undelete(
        options.Context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetBlobContainerPropertiesResult>
  BlobContainerClient::GetProperties(const GetBlobContainerPropertiesOptions& options) const
  {
    Details::BlobRestClient::BlobContainer::GetBlobContainerPropertiesOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return Details::BlobRestClient::BlobContainer::GetProperties(
        options.Context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetBlobContainerMetadataResult> BlobContainerClient::SetMetadata(
      Metadata metadata,
      SetBlobContainerMetadataOptions options) const
  {
    Details::BlobRestClient::BlobContainer::SetBlobContainerMetadataOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = metadata;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    if (options.AccessConditions.IfUnmodifiedSince.HasValue())
    {
      // Strangely enough, this operation doesn't support If-Unmodified-Since while it does support
      // If-Modified-Since
      throw std::runtime_error("this operation doesn't support unmodified since access condition.");
    }
    return Details::BlobRestClient::BlobContainer::SetMetadata(
        options.Context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ListBlobsFlatSegmentResult>
  BlobContainerClient::ListBlobsFlatSegment(const ListBlobsSegmentOptions& options) const
  {
    Details::BlobRestClient::BlobContainer::ListBlobsFlatSegmentOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Include = options.Include;
    auto response = Details::BlobRestClient::BlobContainer::ListBlobsFlat(
        options.Context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
    for (auto& i : response->Items)
    {
      if (i.VersionId.HasValue() && !i.IsCurrentVersion.HasValue())
      {
        i.IsCurrentVersion = false;
      }
    }
    return response;
  }

  Azure::Core::Response<Models::ListBlobsByHierarchySegmentResult>
  BlobContainerClient::ListBlobsByHierarchySegment(
      const std::string& delimiter,
      const ListBlobsSegmentOptions& options) const
  {
    Details::BlobRestClient::BlobContainer::ListBlobsByHierarchySegmentOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Delimiter = delimiter;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Include = options.Include;
    auto response = Details::BlobRestClient::BlobContainer::ListBlobsByHierarchy(
        options.Context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
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
  BlobContainerClient::GetAccessPolicy(const GetBlobContainerAccessPolicyOptions& options) const
  {
    Details::BlobRestClient::BlobContainer::GetBlobContainerAccessPolicyOptions
        protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return Details::BlobRestClient::BlobContainer::GetAccessPolicy(
        options.Context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SetBlobContainerAccessPolicyResult>
  BlobContainerClient::SetAccessPolicy(const SetBlobContainerAccessPolicyOptions& options) const
  {
    Details::BlobRestClient::BlobContainer::SetBlobContainerAccessPolicyOptions
        protocolLayerOptions;
    protocolLayerOptions.AccessType = options.AccessType;
    protocolLayerOptions.SignedIdentifiers = options.SignedIdentifiers;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return Details::BlobRestClient::BlobContainer::SetAccessPolicy(
        options.Context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::AcquireBlobContainerLeaseResult> BlobContainerClient::AcquireLease(
      const std::string& proposedLeaseId,
      int32_t duration,
      const AcquireBlobContainerLeaseOptions& options) const
  {
    Details::BlobRestClient::BlobContainer::AcquireBlobContainerLeaseOptions protocolLayerOptions;
    protocolLayerOptions.ProposedLeaseId = proposedLeaseId;
    protocolLayerOptions.LeaseDuration = duration;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return Details::BlobRestClient::BlobContainer::AcquireLease(
        options.Context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::RenewBlobContainerLeaseResult> BlobContainerClient::RenewLease(
      const std::string& leaseId,
      const RenewBlobContainerLeaseOptions& options) const
  {
    Details::BlobRestClient::BlobContainer::RenewBlobContainerLeaseOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = leaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return Details::BlobRestClient::BlobContainer::RenewLease(
        options.Context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ReleaseBlobContainerLeaseResult> BlobContainerClient::ReleaseLease(
      const std::string& leaseId,
      const ReleaseBlobContainerLeaseOptions& options) const
  {
    Details::BlobRestClient::BlobContainer::ReleaseBlobContainerLeaseOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = leaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return Details::BlobRestClient::BlobContainer::ReleaseLease(
        options.Context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ChangeBlobContainerLeaseResult> BlobContainerClient::ChangeLease(
      const std::string& leaseId,
      const std::string& proposedLeaseId,
      const ChangeBlobContainerLeaseOptions& options) const
  {
    Details::BlobRestClient::BlobContainer::ChangeBlobContainerLeaseOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = leaseId;
    protocolLayerOptions.ProposedLeaseId = proposedLeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return Details::BlobRestClient::BlobContainer::ChangeLease(
        options.Context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::BreakBlobContainerLeaseResult> BlobContainerClient::BreakLease(
      const BreakBlobContainerLeaseOptions& options) const
  {
    Details::BlobRestClient::BlobContainer::BreakBlobContainerLeaseOptions protocolLayerOptions;
    protocolLayerOptions.BreakPeriod = options.BreakPeriod;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return Details::BlobRestClient::BlobContainer::BreakLease(
        options.Context, *m_pipeline, m_blobContainerUrl, protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs
