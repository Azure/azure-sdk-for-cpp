// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/blob_container_client.hpp"

#include "azure/core/credentials.hpp"
#include "azure/core/http/curl/curl.hpp"
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
      const std::string& containerName,
      const BlobContainerClientOptions& options)
  {
    auto parsedConnectionString = Details::ParseConnectionString(connectionString);
    auto containerUri = std::move(parsedConnectionString.BlobServiceUri);
    containerUri.AppendPath(Details::UrlEncodePath(containerName));

    if (parsedConnectionString.KeyCredential)
    {
      return BlobContainerClient(
          containerUri.GetAbsoluteUrl(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return BlobContainerClient(containerUri.GetAbsoluteUrl(), options);
    }
  }

  BlobContainerClient::BlobContainerClient(
      const std::string& containerUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const BlobContainerClientOptions& options)
      : BlobContainerClient(containerUri, options)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Details::c_BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(options.RetryOptions));
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

  BlobContainerClient::BlobContainerClient(
      const std::string& containerUri,
      std::shared_ptr<Identity::ClientSecretCredential> credential,
      const BlobContainerClientOptions& options)
      : BlobContainerClient(containerUri, options)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Details::c_BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Core::BearerTokenAuthenticationPolicy>(
        credential, Details::c_StorageScope));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobContainerClient::BlobContainerClient(
      const std::string& containerUri,
      const BlobContainerClientOptions& options)
      : m_containerUrl(containerUri), m_customerProvidedKey(options.CustomerProvidedKey),
        m_encryptionScope(options.EncryptionScope)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<Azure::Core::Http::TelemetryPolicy>(
        Details::c_BlobServicePackageName, Version::VersionString()));
    policies.emplace_back(std::make_unique<Azure::Core::Http::RequestIdPolicy>());
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StorageRetryPolicy>(options.RetryOptions));
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(p->Clone());
    }
    policies.emplace_back(std::make_unique<StoragePerRetryPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobClient BlobContainerClient::GetBlobClient(const std::string& blobName) const
  {
    auto blobUri = m_containerUrl;
    blobUri.AppendPath(Details::UrlEncodePath(blobName));
    return BlobClient(std::move(blobUri), m_pipeline, m_customerProvidedKey, m_encryptionScope);
  }

  BlockBlobClient BlobContainerClient::GetBlockBlobClient(const std::string& blobName) const
  {
    return GetBlobClient(blobName).GetBlockBlobClient();
  }

  AppendBlobClient BlobContainerClient::GetAppendBlobClient(const std::string& blobName) const
  {
    return GetBlobClient(blobName).GetAppendBlobClient();
  }

  PageBlobClient BlobContainerClient::GetPageBlobClient(const std::string& blobName) const
  {
    return GetBlobClient(blobName).GetPageBlobClient();
  }

  Azure::Core::Response<CreateContainerResult> BlobContainerClient::Create(
      const CreateContainerOptions& options) const
  {
    BlobRestClient::Container::CreateContainerOptions protocolLayerOptions;
    protocolLayerOptions.AccessType = options.AccessType;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.DefaultEncryptionScope = options.DefaultEncryptionScope;
    protocolLayerOptions.PreventEncryptionScopeOverride = options.PreventEncryptionScopeOverride;
    return BlobRestClient::Container::Create(
        options.Context, *m_pipeline, m_containerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<DeleteContainerResult> BlobContainerClient::Delete(
      const DeleteContainerOptions& options) const
  {
    BlobRestClient::Container::DeleteContainerOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return BlobRestClient::Container::Delete(
        options.Context, *m_pipeline, m_containerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<UndeleteContainerResult> BlobContainerClient::Undelete(
      const std::string& deletedContainerName,
      const std::string& deletedContainerVersion,
      const UndeleteContainerOptions& options) const
  {
    BlobRestClient::Container::UndeleteContainerOptions protocolLayerOptions;
    protocolLayerOptions.DeletedContainerName = deletedContainerName;
    protocolLayerOptions.DeletedContainerVersion = deletedContainerVersion;
    return BlobRestClient::Container::Undelete(
        options.Context, *m_pipeline, m_containerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<GetContainerPropertiesResult> BlobContainerClient::GetProperties(
      const GetContainerPropertiesOptions& options) const
  {
    BlobRestClient::Container::GetContainerPropertiesOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return BlobRestClient::Container::GetProperties(
        options.Context, *m_pipeline, m_containerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<SetContainerMetadataResult> BlobContainerClient::SetMetadata(
      std::map<std::string, std::string> metadata,
      SetContainerMetadataOptions options) const
  {
    BlobRestClient::Container::SetContainerMetadataOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = metadata;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    if (options.AccessConditions.IfUnmodifiedSince.HasValue())
    {
      // Strangely enough, this operation doesn't support If-Unmodified-Since while it does support
      // If-Modified-Since
      throw std::runtime_error("this operation doesn't support unmodified since access condition.");
    }
    return BlobRestClient::Container::SetMetadata(
        options.Context, *m_pipeline, m_containerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<ListBlobsFlatSegmentResult> BlobContainerClient::ListBlobsFlatSegment(
      const ListBlobsSegmentOptions& options) const
  {
    BlobRestClient::Container::ListBlobsFlatSegmentOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Include = options.Include;
    auto response = BlobRestClient::Container::ListBlobsFlat(
        options.Context, *m_pipeline, m_containerUrl, protocolLayerOptions);
    for (auto& i : response->Items)
    {
      if (i.VersionId.HasValue() && !i.IsCurrentVersion.HasValue())
      {
        i.IsCurrentVersion = false;
      }
    }
    return response;
  }

  Azure::Core::Response<ListBlobsByHierarchySegmentResult>
  BlobContainerClient::ListBlobsByHierarchySegment(
      const std::string& delimiter,
      const ListBlobsSegmentOptions& options) const
  {
    BlobRestClient::Container::ListBlobsByHierarchySegmentOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Delimiter = delimiter;
    protocolLayerOptions.ContinuationToken = options.ContinuationToken;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Include = options.Include;
    auto response = BlobRestClient::Container::ListBlobsByHierarchy(
        options.Context, *m_pipeline, m_containerUrl, protocolLayerOptions);
    for (auto& i : response->Items)
    {
      if (i.VersionId.HasValue() && !i.IsCurrentVersion.HasValue())
      {
        i.IsCurrentVersion = false;
      }
    }
    return response;
  }

  Azure::Core::Response<GetContainerAccessPolicyResult> BlobContainerClient::GetAccessPolicy(
      const GetContainerAccessPolicyOptions& options) const
  {
    BlobRestClient::Container::GetContainerAccessPolicyOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    return BlobRestClient::Container::GetAccessPolicy(
        options.Context, *m_pipeline, m_containerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<SetContainerAccessPolicyResult> BlobContainerClient::SetAccessPolicy(
      const SetContainerAccessPolicyOptions& options) const
  {
    BlobRestClient::Container::SetContainerAccessPolicyOptions protocolLayerOptions;
    protocolLayerOptions.AccessType = options.AccessType;
    protocolLayerOptions.SignedIdentifiers = options.SignedIdentifiers;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    return BlobRestClient::Container::SetAccessPolicy(
        options.Context, *m_pipeline, m_containerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<AcquireContainerLeaseResult> BlobContainerClient::AcquireLease(
      const std::string& proposedLeaseId,
      int32_t duration,
      const AcquireContainerLeaseOptions& options) const
  {
    BlobRestClient::Container::AcquireContainerLeaseOptions protocolLayerOptions;
    protocolLayerOptions.ProposedLeaseId = proposedLeaseId;
    protocolLayerOptions.LeaseDuration = duration;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    return BlobRestClient::Container::AcquireLease(
        options.Context, *m_pipeline, m_containerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<RenewContainerLeaseResult> BlobContainerClient::RenewLease(
      const std::string& leaseId,
      const RenewContainerLeaseOptions& options) const
  {
    BlobRestClient::Container::RenewContainerLeaseOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = leaseId;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    return BlobRestClient::Container::RenewLease(
        options.Context, *m_pipeline, m_containerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<ReleaseContainerLeaseResult> BlobContainerClient::ReleaseLease(
      const std::string& leaseId,
      const ReleaseContainerLeaseOptions& options) const
  {
    BlobRestClient::Container::ReleaseContainerLeaseOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = leaseId;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    return BlobRestClient::Container::ReleaseLease(
        options.Context, *m_pipeline, m_containerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<ChangeContainerLeaseResult> BlobContainerClient::ChangeLease(
      const std::string& leaseId,
      const std::string& proposedLeaseId,
      const ChangeContainerLeaseOptions& options) const
  {
    BlobRestClient::Container::ChangeContainerLeaseOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = leaseId;
    protocolLayerOptions.ProposedLeaseId = proposedLeaseId;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    return BlobRestClient::Container::ChangeLease(
        options.Context, *m_pipeline, m_containerUrl, protocolLayerOptions);
  }

  Azure::Core::Response<BreakContainerLeaseResult> BlobContainerClient::BreakLease(
      const BreakContainerLeaseOptions& options) const
  {
    BlobRestClient::Container::BreakContainerLeaseOptions protocolLayerOptions;
    protocolLayerOptions.BreakPeriod = options.BreakPeriod;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    return BlobRestClient::Container::BreakLease(
        options.Context, *m_pipeline, m_containerUrl, protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs
