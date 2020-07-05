// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/blob_client.hpp"

#include "blobs/append_blob_client.hpp"
#include "blobs/block_blob_client.hpp"
#include "blobs/page_blob_client.hpp"
#include "common/common_headers_request_policy.hpp"
#include "common/shared_key_policy.hpp"
#include "common/storage_common.hpp"
#include "http/curl/curl.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobClient BlobClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& containerName,
      const std::string& blobName,
      const BlobClientOptions& options)
  {
    auto parsedConnectionString = Details::ParseConnectionString(connectionString);
    auto blobUri = std::move(parsedConnectionString.BlobServiceUri);
    blobUri.AppendPath(containerName, true);
    blobUri.AppendPath(blobName, true);

    if (parsedConnectionString.KeyCredential)
    {
      return BlobClient(blobUri.ToString(), parsedConnectionString.KeyCredential, options);
    }
    else
    {
      return BlobClient(blobUri.ToString(), options);
    }
  }

  BlobClient::BlobClient(
      const std::string& blobUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const BlobClientOptions& options)
      : m_blobUrl(blobUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    // TODO: Retry policy goes here
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<SharedKeyPolicy>(credential));
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobClient::BlobClient(
      const std::string& blobUri,
      std::shared_ptr<TokenCredential> credential,
      const BlobClientOptions& options)
      : m_blobUrl(blobUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    // TODO: Retry policy goes here
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    // not implemented yet
    unused(credential);
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlobClient::BlobClient(const std::string& blobUri, const BlobClientOptions& options)
      : m_blobUrl(blobUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.PerOperationPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    // TODO: Retry policy goes here
    for (const auto& p : options.PerRetryPolicies)
    {
      policies.emplace_back(std::unique_ptr<Azure::Core::Http::HttpPolicy>(p->Clone()));
    }
    policies.emplace_back(std::make_unique<CommonHeadersRequestPolicy>());
    policies.emplace_back(std::make_unique<Azure::Core::Http::TransportPolicy>(
        std::make_shared<Azure::Core::Http::CurlTransport>()));
    m_pipeline = std::make_shared<Azure::Core::Http::HttpPipeline>(policies);
  }

  BlockBlobClient BlobClient::GetBlockBlobClient() const { return BlockBlobClient(*this); }

  AppendBlobClient BlobClient::GetAppendBlobClient() const { return AppendBlobClient(*this); }

  PageBlobClient BlobClient::GetPageBlobClient() const { return PageBlobClient(*this); }

  BlobClient BlobClient::WithSnapshot(const std::string& snapshot) const
  {
    BlobClient newClient(*this);
    if (snapshot.empty())
    {
      newClient.m_blobUrl.RemoveQuery("snapshot");
    }
    else
    {
      newClient.m_blobUrl.AppendQuery("snapshot", snapshot);
    }
    return newClient;
  }

  BlobDownloadInfo BlobClient::Download(const DownloadBlobOptions& options) const
  {
    BlobRestClient::Blob::DownloadOptions protocolLayerOptions;
    if (options.Offset.HasValue() && options.Length.HasValue())
    {
      protocolLayerOptions.Range = std::make_pair(
          options.Offset.GetValue(), options.Offset.GetValue() + options.Length.GetValue() - 1);
    }
    else if (options.Offset.HasValue())
    {
      protocolLayerOptions.Range = std::make_pair(
          options.Offset.GetValue(),
          std::numeric_limits<std::remove_reference_t<decltype(options.Offset.GetValue())>>::max());
    }
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;

    return BlobRestClient::Blob::Download(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  BlobProperties BlobClient::GetProperties(const GetBlobPropertiesOptions& options) const
  {
    BlobRestClient::Blob::GetPropertiesOptions protocolLayerOptions;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::Blob::GetProperties(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  SetBlobHttpHeadersResponse BlobClient::SetHttpHeaders(
      BlobHttpHeaders httpHeaders,
      const SetBlobHttpHeadersOptions& options) const
  {
    BlobRestClient::Blob::SetHttpHeadersOptions protocolLayerOptions;
    protocolLayerOptions.HttpHeaders = std::move(httpHeaders);
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::Blob::SetHttpHeaders(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  SetBlobMetadataResponse BlobClient::SetMetadata(
      std::map<std::string, std::string> metadata,
      const SetBlobMetadataOptions& options) const
  {
    BlobRestClient::Blob::SetMetadataOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = std::move(metadata);
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::Blob::SetMetadata(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  SetAccessTierResponse BlobClient::SetAccessTier(
      AccessTier Tier,
      const SetAccessTierOptions& options) const
  {
    BlobRestClient::Blob::SetAccessTierOptions protocolLayerOptions;
    protocolLayerOptions.Tier = Tier;
    protocolLayerOptions.RehydratePriority = options.RehydratePriority;
    return BlobRestClient::Blob::SetAccessTier(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  BlobCopyInfo BlobClient::StartCopyFromUri(
      const std::string& sourceUri,
      const StartCopyFromUriOptions& options) const
  {
    BlobRestClient::Blob::StartCopyFromUriOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.SourceUri = sourceUri;
    protocolLayerOptions.LeaseId = options.LeaseId;
    protocolLayerOptions.SourceLeaseId = options.SourceLeaseId;
    protocolLayerOptions.Tier = options.Tier;
    protocolLayerOptions.RehydratePriority = options.RehydratePriority;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    protocolLayerOptions.SourceIfModifiedSince = options.SourceIfModifiedSince;
    protocolLayerOptions.SourceIfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.SourceIfMatch = options.SourceIfMatch;
    protocolLayerOptions.SourceIfNoneMatch = options.SourceIfNoneMatch;
    return BlobRestClient::Blob::StartCopyFromUri(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  AbortCopyBlobResponse BlobClient::AbortCopyFromUri(
      const std::string& copyId,
      const AbortCopyFromUriOptions& options) const
  {
    BlobRestClient::Blob::AbortCopyFromUriOptions protocolLayerOptions;
    protocolLayerOptions.CopyId = copyId;
    protocolLayerOptions.LeaseId = options.LeaseId;
    return BlobRestClient::Blob::AbortCopyFromUri(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  BlobSnapshotInfo BlobClient::CreateSnapshot(const CreateSnapshotOptions& options) const
  {
    BlobRestClient::Blob::CreateSnapshotOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.LeaseId = options.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::Blob::CreateSnapshot(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  DeleteBlobResponse BlobClient::Delete(const DeleteBlobOptions& options) const
  {
    BlobRestClient::Blob::DeleteOptions protocolLayerOptions;
    protocolLayerOptions.DeleteSnapshots = options.DeleteSnapshots;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::Blob::Delete(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  UndeleteBlobResponse BlobClient::Undelete(const UndeleteBlobOptions& options) const
  {
    BlobRestClient::Blob::UndeleteOptions protocolLayerOptions;
    return BlobRestClient::Blob::Undelete(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs
