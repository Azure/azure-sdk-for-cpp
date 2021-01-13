// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/page_blob_client.hpp"

#include <azure/storage/common/concurrent_transfer.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/file_io.hpp>
#include <azure/storage/common/storage_common.hpp>

namespace Azure { namespace Storage { namespace Blobs {

  PageBlobClient PageBlobClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& blobContainerName,
      const std::string& blobName,
      const BlobClientOptions& options)
  {
    PageBlobClient newClient(BlobClient::CreateFromConnectionString(
        connectionString, blobContainerName, blobName, options));
    return newClient;
  }

  PageBlobClient::PageBlobClient(
      const std::string& blobUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const BlobClientOptions& options)
      : BlobClient(blobUrl, std::move(credential), options)
  {
  }

  PageBlobClient::PageBlobClient(
      const std::string& blobUrl,
      std::shared_ptr<Core::TokenCredential> credential,
      const BlobClientOptions& options)
      : BlobClient(blobUrl, std::move(credential), options)
  {
  }

  PageBlobClient::PageBlobClient(const std::string& blobUrl, const BlobClientOptions& options)
      : BlobClient(blobUrl, options)
  {
  }

  PageBlobClient::PageBlobClient(BlobClient blobClient) : BlobClient(std::move(blobClient)) {}

  PageBlobClient PageBlobClient::WithSnapshot(const std::string& snapshot) const
  {
    PageBlobClient newClient(*this);
    if (snapshot.empty())
    {
      newClient.m_blobUrl.RemoveQueryParameter(Storage::Details::HttpQuerySnapshot);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          Storage::Details::HttpQuerySnapshot, Storage::Details::UrlEncodeQueryParameter(snapshot));
    }
    return newClient;
  }

  PageBlobClient PageBlobClient::WithVersionId(const std::string& versionId) const
  {
    PageBlobClient newClient(*this);
    if (versionId.empty())
    {
      newClient.m_blobUrl.RemoveQueryParameter(Storage::Details::HttpQueryVersionId);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          Storage::Details::HttpQueryVersionId,
          Storage::Details::UrlEncodeQueryParameter(versionId));
    }
    return newClient;
  }

  Azure::Core::Response<Models::CreatePageBlobResult> PageBlobClient::Create(
      int64_t blobContentLength,
      const CreatePageBlobOptions& options) const
  {
    Details::BlobRestClient::PageBlob::CreatePageBlobOptions protocolLayerOptions;
    protocolLayerOptions.BlobContentLength = blobContentLength;
    protocolLayerOptions.SequenceNumber = options.SequenceNumber;
    protocolLayerOptions.HttpHeaders = options.HttpHeaders;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.Tier = options.Tier;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return Details::BlobRestClient::PageBlob::Create(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::CreatePageBlobResult> PageBlobClient::CreateIfNotExists(
      int64_t blobContentLength,
      const CreatePageBlobOptions& options) const
  {
    auto optionsCopy = options;
    optionsCopy.AccessConditions.IfNoneMatch = ETagWildcard;
    try
    {
      return Create(blobContentLength, optionsCopy);
    }
    catch (StorageException& e)
    {
      if (e.StatusCode == Core::Http::HttpStatusCode::Conflict
          && e.ErrorCode == "BlobAlreadyExists")
      {
        Models::CreatePageBlobResult ret;
        ret.Created = false;
        return Azure::Core::Response<Models::CreatePageBlobResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::UploadPageBlobPagesResult> PageBlobClient::UploadPages(
      int64_t offset,
      Azure::Core::Http::BodyStream* content,
      const UploadPageBlobPagesOptions& options) const
  {
    Details::BlobRestClient::PageBlob::UploadPageBlobPagesOptions protocolLayerOptions;
    protocolLayerOptions.Range.Offset = offset;
    protocolLayerOptions.Range.Length = content->Length();
    protocolLayerOptions.TransactionalContentHash = options.TransactionalContentHash;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return Details::BlobRestClient::PageBlob::UploadPages(
        options.Context, *m_pipeline, m_blobUrl, content, protocolLayerOptions);
  }

  Azure::Core::Response<Models::UploadPageBlobPagesFromUriResult>
  PageBlobClient::UploadPagesFromUri(
      int64_t destinationOffset,
      std::string sourceUri,
      Azure::Core::Http::Range sourceRange,
      const UploadPageBlobPagesFromUriOptions& options) const
  {
    Details::BlobRestClient::PageBlob::UploadPageBlobPagesFromUriOptions protocolLayerOptions;
    protocolLayerOptions.SourceUri = sourceUri;
    protocolLayerOptions.Range.Offset = destinationOffset;
    protocolLayerOptions.Range.Length = sourceRange.Length.GetValue();
    protocolLayerOptions.SourceRange = std::move(sourceRange);
    protocolLayerOptions.TransactionalContentHash = options.TransactionalContentHash;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return Details::BlobRestClient::PageBlob::UploadPagesFromUri(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ClearPageBlobPagesResult> PageBlobClient::ClearPages(
      Azure::Core::Http::Range range,
      const ClearPageBlobPagesOptions& options) const
  {
    Details::BlobRestClient::PageBlob::ClearPageBlobPagesOptions protocolLayerOptions;
    protocolLayerOptions.Range = std::move(range);
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return Details::BlobRestClient::PageBlob::ClearPages(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::ResizePageBlobResult> PageBlobClient::Resize(
      int64_t blobContentLength,
      const ResizePageBlobOptions& options) const
  {
    Details::BlobRestClient::PageBlob::ResizePageBlobOptions protocolLayerOptions;
    protocolLayerOptions.BlobContentLength = blobContentLength;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return Details::BlobRestClient::PageBlob::Resize(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetPageBlobPageRangesResult> PageBlobClient::GetPageRanges(
      const GetPageBlobPageRangesOptions& options) const
  {
    Details::BlobRestClient::PageBlob::GetPageBlobPageRangesOptions protocolLayerOptions;
    protocolLayerOptions.Range = options.Range;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return Details::BlobRestClient::PageBlob::GetPageRanges(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetPageBlobPageRangesResult> PageBlobClient::GetPageRangesDiff(
      const std::string& previousSnapshot,
      const GetPageBlobPageRangesOptions& options) const
  {
    Details::BlobRestClient::PageBlob::GetPageBlobPageRangesOptions protocolLayerOptions;
    protocolLayerOptions.PreviousSnapshot = previousSnapshot;
    protocolLayerOptions.Range = options.Range;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return Details::BlobRestClient::PageBlob::GetPageRanges(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::GetPageBlobPageRangesResult>
  PageBlobClient::GetManagedDiskPageRangesDiff(
      const std::string& previousSnapshotUrl,
      const GetPageBlobPageRangesOptions& options) const
  {
    Details::BlobRestClient::PageBlob::GetPageBlobPageRangesOptions protocolLayerOptions;
    protocolLayerOptions.PreviousSnapshotUrl = previousSnapshotUrl;
    protocolLayerOptions.Range = options.Range;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return Details::BlobRestClient::PageBlob::GetPageRanges(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::StartCopyPageBlobIncrementalResult>
  PageBlobClient::StartCopyIncremental(
      const std::string& sourceUri,
      const StartCopyPageBlobIncrementalOptions& options) const
  {
    Details::BlobRestClient::PageBlob::StartCopyPageBlobIncrementalOptions protocolLayerOptions;
    protocolLayerOptions.CopySource = sourceUri;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return Details::BlobRestClient::PageBlob::StartCopyIncremental(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs
