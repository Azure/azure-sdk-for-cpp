// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/page_blob_client.hpp"

#include "common/storage_common.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  PageBlobClient PageBlobClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& containerName,
      const std::string& blobName,
      const PageBlobClientOptions& options)
  {
    PageBlobClient newClient(
        BlobClient::CreateFromConnectionString(connectionString, containerName, blobName, options));
    return newClient;
  }

  PageBlobClient::PageBlobClient(
      const std::string& blobUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const PageBlobClientOptions& options)
      : BlobClient(blobUri, std::move(credential), options)
  {
  }

  PageBlobClient::PageBlobClient(
      const std::string& blobUri,
      std::shared_ptr<TokenCredential> credential,
      const PageBlobClientOptions& options)
      : BlobClient(blobUri, std::move(credential), options)
  {
  }

  PageBlobClient::PageBlobClient(const std::string& blobUri, const PageBlobClientOptions& options)
      : BlobClient(blobUri, options)
  {
  }

  PageBlobClient::PageBlobClient(BlobClient blobClient) : BlobClient(std::move(blobClient)) {}

  PageBlobClient PageBlobClient::WithSnapshot(const std::string& snapshot) const
  {
    PageBlobClient newClient(*this);
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

  BlobContentInfo PageBlobClient::Create(
      int64_t blobContentLength,
      const CreatePageBlobOptions& options)
  {
    BlobRestClient::PageBlob::CreateOptions protocolLayerOptions;
    protocolLayerOptions.BlobContentLength = blobContentLength;
    protocolLayerOptions.SequenceNumber = options.SequenceNumber;
    protocolLayerOptions.HttpHeaders = options.HttpHeaders;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.Tier = options.Tier;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::PageBlob::Create(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  PageInfo PageBlobClient::UploadPages(
      Azure::Core::Http::BodyStream& content,
      int64_t offset,
      const UploadPagesOptions& options)
  {
    BlobRestClient::PageBlob::UploadPagesOptions protocolLayerOptions;
    protocolLayerOptions.Range = std::make_pair(offset, offset + content.Length() - 1);
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.ContentCRC64 = options.ContentCRC64;
    protocolLayerOptions.LeaseId = options.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::PageBlob::UploadPages(
        options.Context, *m_pipeline, m_blobUrl.ToString(), content, protocolLayerOptions);
  }

  PageInfo PageBlobClient::UploadPagesFromUri(
      std::string sourceUri,
      int64_t sourceOffset,
      int64_t sourceLength,
      int64_t destinationoffset,
      const UploadPagesFromUriOptions& options)
  {
    BlobRestClient::PageBlob::UploadPagesFromUriOptions protocolLayerOptions;
    protocolLayerOptions.SourceUri = sourceUri;
    protocolLayerOptions.SourceRange
        = std::make_pair(sourceOffset, sourceOffset + sourceLength - 1);
    protocolLayerOptions.Range
        = std::make_pair(destinationoffset, destinationoffset + sourceLength - 1);
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.ContentCRC64 = options.ContentCRC64;
    protocolLayerOptions.LeaseId = options.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::PageBlob::UploadPagesFromUri(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  PageInfo PageBlobClient::ClearPages(
      int64_t offset,
      int64_t length,
      const ClearPagesOptions& options)
  {
    BlobRestClient::PageBlob::ClearPagesOptions protocolLayerOptions;
    protocolLayerOptions.Range = std::make_pair(offset, offset + length - 1);
    protocolLayerOptions.LeaseId = options.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::PageBlob::ClearPages(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  PageBlobInfo PageBlobClient::Resize(
      int64_t blobContentLength,
      const ResizePageBlobOptions& options)
  {
    BlobRestClient::PageBlob::ResizeOptions protocolLayerOptions;
    protocolLayerOptions.BlobContentLength = blobContentLength;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::PageBlob::Resize(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  PageRangesInfo PageBlobClient::GetPageRanges(const GetPageRangesOptions& options)
  {
    BlobRestClient::PageBlob::GetPageRangesOptions protocolLayerOptions;
    protocolLayerOptions.PreviousSnapshot = options.PreviousSnapshot;
    protocolLayerOptions.PreviousSnapshotUrl = options.PreviousSnapshotUrl;
    if (options.Offset.HasValue() && options.Length.HasValue())
    {
      protocolLayerOptions.Range = std::make_pair(
          options.Offset.GetValue(), options.Offset.GetValue() + options.Length.GetValue() - 1);
    }
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    auto protocolLayerResponse = BlobRestClient::PageBlob::GetPageRanges(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);

    PageRangesInfo ret;
    ret.RequestId = std::move(protocolLayerResponse.RequestId);
    ret.Date = std::move(protocolLayerResponse.Date);
    ret.Version = std::move(protocolLayerResponse.Version);
    ret.ClientRequestId = std::move(protocolLayerResponse.ClientRequestId);
    ret.ETag = std::move(protocolLayerResponse.ETag);
    ret.LastModified = std::move(protocolLayerResponse.LastModified);
    ret.BlobContentLength = protocolLayerResponse.BlobContentLength;
    for (const auto& range : protocolLayerResponse.PageRanges)
    {
      ret.PageRanges.emplace_back(PageRange{range.first, range.second - range.first + 1});
    }
    for (const auto& range : protocolLayerResponse.ClearRanges)
    {
      ret.ClearRanges.emplace_back(PageRange{range.first, range.second - range.first + 1});
    }
    return ret;
  }

  BlobCopyInfo PageBlobClient::StartCopyIncremental(
      const std::string& sourceUri,
      const IncrementalCopyPageBlobOptions& options)
  {
    BlobRestClient::PageBlob::CopyIncrementalOptions protocolLayerOptions;
    protocolLayerOptions.CopySource = sourceUri;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::PageBlob::CopyIncremental(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs
