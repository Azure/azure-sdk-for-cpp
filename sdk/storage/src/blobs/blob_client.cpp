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
    auto parsedConnectionString = ParseConnectionString(connectionString);

    std::string accountName;
    std::string accountKey;
    std::string blobEndpoint;
    std::string EndpointSuffix;
    std::string defaultEndpointsProtocol = ".core.windows.net";

    auto ite = parsedConnectionString.find("AccountName");
    if (ite != parsedConnectionString.end())
    {
      accountName = ite->second;
    }
    ite = parsedConnectionString.find("AccountKey");
    if (ite != parsedConnectionString.end())
    {
      accountKey = ite->second;
    }
    ite = parsedConnectionString.find("BlobEndpoint");
    if (ite != parsedConnectionString.end())
    {
      blobEndpoint = ite->second;
    }
    ite = parsedConnectionString.find("EndpointSuffix");
    if (ite != parsedConnectionString.end())
    {
      EndpointSuffix = ite->second;
    }
    ite = parsedConnectionString.find("DefaultEndpointsProtocol");
    if (ite != parsedConnectionString.end())
    {
      defaultEndpointsProtocol = ite->second;
    }

    UrlBuilder builder;
    builder.SetScheme(defaultEndpointsProtocol);
    if (!blobEndpoint.empty())
    {
      builder = UrlBuilder(blobEndpoint);
    }
    else if (!accountName.empty())
    {
      builder.SetHost(accountName + ".blob." + EndpointSuffix);
    }
    else
    {
      throw std::runtime_error("invalid connection string");
    }
    builder.AppendPath(containerName, true);
    builder.AppendPath(blobName, true);

    auto credential = std::make_shared<SharedKeyCredential>(accountName, accountKey);

    return BlobClient(builder.to_string(), credential, options);
  }

  BlobClient::BlobClient(
      const std::string& blobUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const BlobClientOptions& options)
      : m_blobUrl(blobUri)
  {
    std::vector<std::unique_ptr<Azure::Core::Http::HttpPolicy>> policies;
    for (const auto& p : options.policies)
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
    for (const auto& p : options.policies)
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
    for (const auto& p : options.policies)
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

  FlattenedDownloadProperties BlobClient::Download(const DownloadBlobOptions& options) const
  {
    BlobRestClient::Blob::DownloadOptions protocolLayerOptions;
    if (options.Offset != std::numeric_limits<decltype(options.Offset)>::max())
    {
      if (options.Length == 0)
      {
        protocolLayerOptions.Range
            = std::make_pair(options.Offset, std::numeric_limits<decltype(options.Offset)>::max());
      }
      else
      {
        protocolLayerOptions.Range
            = std::make_pair(options.Offset, options.Offset + options.Length - 1);
      }
    }
    else
    {
      protocolLayerOptions.Range
          = std::make_pair(std::numeric_limits<uint64_t>::max(), uint64_t(0));
    }
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;

    return BlobRestClient::Blob::Download(
        options.Context, *m_pipeline, m_blobUrl.to_string(), protocolLayerOptions);
  }

  BlobProperties BlobClient::GetProperties(const GetBlobPropertiesOptions& options) const
  {
    BlobRestClient::Blob::GetPropertiesOptions protocolLayerOptions;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::Blob::GetProperties(
        options.Context, *m_pipeline, m_blobUrl.to_string(), protocolLayerOptions);
  }

  BlobInfo BlobClient::SetHttpHeaders(const SetBlobHttpHeadersOptions& options) const
  {
    BlobRestClient::Blob::SetHttpHeadersOptions protocolLayerOptions;
    protocolLayerOptions.ContentType = options.ContentType;
    protocolLayerOptions.ContentEncoding = options.ContentEncoding;
    protocolLayerOptions.ContentLanguage = options.ContentLanguage;
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.CacheControl = options.CacheControl;
    protocolLayerOptions.ContentDisposition = options.ContentDisposition;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::Blob::SetHttpHeaders(
        options.Context, *m_pipeline, m_blobUrl.to_string(), protocolLayerOptions);
  }

  BlobInfo BlobClient::SetMetadata(
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
        options.Context, *m_pipeline, m_blobUrl.to_string(), protocolLayerOptions);
  }

  BasicResponse BlobClient::SetAccessTier(AccessTier Tier, const SetAccessTierOptions& options)
      const
  {
    BlobRestClient::Blob::SetAccessTierOptions protocolLayerOptions;
    protocolLayerOptions.Tier = Tier;
    protocolLayerOptions.RehydratePriority = options.RehydratePriority;
    return BlobRestClient::Blob::SetAccessTier(
        options.Context, *m_pipeline, m_blobUrl.to_string(), protocolLayerOptions);
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
        options.Context, *m_pipeline, m_blobUrl.to_string(), protocolLayerOptions);
  }

  BasicResponse BlobClient::AbortCopyFromUri(
      const std::string& copyId,
      const AbortCopyFromUriOptions& options) const
  {
    BlobRestClient::Blob::AbortCopyFromUriOptions protocolLayerOptions;
    protocolLayerOptions.CopyId = copyId;
    protocolLayerOptions.LeaseId = options.LeaseId;
    return BlobRestClient::Blob::AbortCopyFromUri(
        options.Context, *m_pipeline, m_blobUrl.to_string(), protocolLayerOptions);
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
        options.Context, *m_pipeline, m_blobUrl.to_string(), protocolLayerOptions);
  }

  BasicResponse BlobClient::Delete(const DeleteBlobOptions& options) const
  {
    BlobRestClient::Blob::DeleteOptions protocolLayerOptions;
    protocolLayerOptions.DeleteSnapshots = options.DeleteSnapshots;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::Blob::Delete(
        options.Context, *m_pipeline, m_blobUrl.to_string(), protocolLayerOptions);
  }

  BasicResponse BlobClient::Undelete(const UndeleteBlobOptions& options) const
  {
    BlobRestClient::Blob::UndeleteOptions protocolLayerOptions;
    return BlobRestClient::Blob::Undelete(
        options.Context, *m_pipeline, m_blobUrl.to_string(), protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs
