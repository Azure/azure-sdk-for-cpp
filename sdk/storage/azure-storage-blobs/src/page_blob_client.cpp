// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/page_blob_client.hpp"

#include <azure/storage/common/internal/concurrent_transfer.hpp>
#include <azure/storage/common/internal/constants.hpp>
#include <azure/storage/common/internal/file_io.hpp>
#include <azure/storage/common/internal/storage_switch_to_secondary_policy.hpp>
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
      std::shared_ptr<Core::Credentials::TokenCredential> credential,
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
      newClient.m_blobUrl.RemoveQueryParameter(_internal::HttpQuerySnapshot);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          _internal::HttpQuerySnapshot, _internal::UrlEncodeQueryParameter(snapshot));
    }
    return newClient;
  }

  PageBlobClient PageBlobClient::WithVersionId(const std::string& versionId) const
  {
    PageBlobClient newClient(*this);
    if (versionId.empty())
    {
      newClient.m_blobUrl.RemoveQueryParameter(_internal::HttpQueryVersionId);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          _internal::HttpQueryVersionId, _internal::UrlEncodeQueryParameter(versionId));
    }
    return newClient;
  }

  Azure::Response<Models::CreatePageBlobResult> PageBlobClient::Create(
      int64_t blobSize,
      const CreatePageBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::PageBlob::CreatePageBlobOptions protocolLayerOptions;
    protocolLayerOptions.BlobSize = blobSize;
    protocolLayerOptions.SequenceNumber = options.SequenceNumber;
    protocolLayerOptions.HttpHeaders = options.HttpHeaders;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.AccessTier = options.AccessTier;
    protocolLayerOptions.Tags = options.Tags;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.Value().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.Value().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.Value().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return _detail::BlobRestClient::PageBlob::Create(
        *m_pipeline, m_blobUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::CreatePageBlobResult> PageBlobClient::CreateIfNotExists(
      int64_t blobContentLength,
      const CreatePageBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    auto optionsCopy = options;
    optionsCopy.AccessConditions.IfNoneMatch = Azure::ETag::Any();
    try
    {
      return Create(blobContentLength, optionsCopy, context);
    }
    catch (StorageException& e)
    {
      if (e.StatusCode == Core::Http::HttpStatusCode::Conflict
          && e.ErrorCode == "BlobAlreadyExists")
      {
        Models::CreatePageBlobResult ret;
        ret.Created = false;
        return Azure::Response<Models::CreatePageBlobResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::UploadPagesResult> PageBlobClient::UploadPages(
      int64_t offset,
      Azure::Core::IO::BodyStream& content,
      const UploadPagesOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::PageBlob::UploadPageBlobPagesOptions protocolLayerOptions;
    protocolLayerOptions.Range.Offset = offset;
    protocolLayerOptions.Range.Length = content.Length();
    protocolLayerOptions.TransactionalContentHash = options.TransactionalContentHash;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    protocolLayerOptions.IfSequenceNumberLessThanOrEqualTo
        = options.AccessConditions.IfSequenceNumberLessThanOrEqual;
    protocolLayerOptions.IfSequenceNumberLessThan
        = options.AccessConditions.IfSequenceNumberLessThan;
    protocolLayerOptions.IfSequenceNumberEqualTo = options.AccessConditions.IfSequenceNumberEqual;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.Value().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.Value().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.Value().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return _detail::BlobRestClient::PageBlob::UploadPages(
        *m_pipeline, m_blobUrl, content, protocolLayerOptions, context);
  }

  Azure::Response<Models::UploadPagesFromUriResult> PageBlobClient::UploadPagesFromUri(
      int64_t destinationOffset,
      std::string sourceUri,
      Azure::Core::Http::HttpRange sourceRange,
      const UploadPagesFromUriOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::PageBlob::UploadPageBlobPagesFromUriOptions protocolLayerOptions;
    protocolLayerOptions.SourceUri = sourceUri;
    protocolLayerOptions.Range.Offset = destinationOffset;
    protocolLayerOptions.Range.Length = sourceRange.Length.Value();
    protocolLayerOptions.SourceRange = std::move(sourceRange);
    protocolLayerOptions.TransactionalContentHash = options.TransactionalContentHash;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    protocolLayerOptions.IfSequenceNumberLessThanOrEqualTo
        = options.AccessConditions.IfSequenceNumberLessThanOrEqual;
    protocolLayerOptions.IfSequenceNumberLessThan
        = options.AccessConditions.IfSequenceNumberLessThan;
    protocolLayerOptions.IfSequenceNumberEqualTo = options.AccessConditions.IfSequenceNumberEqual;
    protocolLayerOptions.SourceIfModifiedSince = options.SourceAccessConditions.IfModifiedSince;
    protocolLayerOptions.SourceIfUnmodifiedSince = options.SourceAccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.SourceIfMatch = options.SourceAccessConditions.IfMatch;
    protocolLayerOptions.SourceIfNoneMatch = options.SourceAccessConditions.IfNoneMatch;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.Value().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.Value().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.Value().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return _detail::BlobRestClient::PageBlob::UploadPagesFromUri(
        *m_pipeline, m_blobUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::ClearPagesResult> PageBlobClient::ClearPages(
      Azure::Core::Http::HttpRange range,
      const ClearPagesOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::PageBlob::ClearPageBlobPagesOptions protocolLayerOptions;
    protocolLayerOptions.Range = std::move(range);
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    protocolLayerOptions.IfSequenceNumberLessThanOrEqualTo
        = options.AccessConditions.IfSequenceNumberLessThanOrEqual;
    protocolLayerOptions.IfSequenceNumberLessThan
        = options.AccessConditions.IfSequenceNumberLessThan;
    protocolLayerOptions.IfSequenceNumberEqualTo = options.AccessConditions.IfSequenceNumberEqual;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.Value().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.Value().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.Value().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return _detail::BlobRestClient::PageBlob::ClearPages(
        *m_pipeline, m_blobUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::ResizePageBlobResult> PageBlobClient::Resize(
      int64_t blobSize,
      const ResizePageBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::PageBlob::ResizePageBlobOptions protocolLayerOptions;
    protocolLayerOptions.BlobSize = blobSize;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return _detail::BlobRestClient::PageBlob::Resize(
        *m_pipeline, m_blobUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::UpdateSequenceNumberResult> PageBlobClient::UpdateSequenceNumber(
      Models::SequenceNumberAction action,
      const UpdatePageBlobSequenceNumberOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::PageBlob::UpdatePageBlobSequenceNumberOptions protocolLayerOptions;
    protocolLayerOptions.Action = action;
    protocolLayerOptions.SequenceNumber = options.SequenceNumber;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return _detail::BlobRestClient::PageBlob::UpdateSequenceNumber(
        *m_pipeline, m_blobUrl, protocolLayerOptions, context);
  }

  GetPageRangesPagedResponse PageBlobClient::GetPageRanges(
      const GetPageRangesOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::PageBlob::GetPageBlobPageRangesOptions protocolLayerOptions;
    protocolLayerOptions.Range = options.Range;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    auto response = _detail::BlobRestClient::PageBlob::GetPageRanges(
        *m_pipeline, m_blobUrl, protocolLayerOptions, _internal::WithReplicaStatus(context));

    GetPageRangesPagedResponse pagedResponse;

    pagedResponse.ETag = std::move(response.Value.ETag);
    pagedResponse.LastModified = std::move(response.Value.LastModified);
    pagedResponse.BlobSize = response.Value.BlobSize;
    pagedResponse.PageRanges = std::move(response.Value.PageRanges);
    pagedResponse.m_pageBlobClient = std::make_shared<PageBlobClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.CurrentPageToken = std::string();
    pagedResponse.NextPageToken = std::string();
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  GetPageRangesDiffPagedResponse PageBlobClient::GetPageRangesDiff(
      const std::string& previousSnapshot,
      const GetPageRangesOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::PageBlob::GetPageBlobPageRangesOptions protocolLayerOptions;
    protocolLayerOptions.PreviousSnapshot = previousSnapshot;
    protocolLayerOptions.Range = options.Range;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    auto response = _detail::BlobRestClient::PageBlob::GetPageRanges(
        *m_pipeline, m_blobUrl, protocolLayerOptions, _internal::WithReplicaStatus(context));

    GetPageRangesDiffPagedResponse pagedResponse;

    pagedResponse.ETag = std::move(response.Value.ETag);
    pagedResponse.LastModified = std::move(response.Value.LastModified);
    pagedResponse.BlobSize = response.Value.BlobSize;
    pagedResponse.PageRanges = std::move(response.Value.PageRanges);
    pagedResponse.ClearRanges = std::move(response.Value.ClearRanges);
    pagedResponse.m_pageBlobClient = std::make_shared<PageBlobClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.m_previousSnapshot = previousSnapshot;
    pagedResponse.CurrentPageToken = std::string();
    pagedResponse.NextPageToken = std::string();
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  GetPageRangesDiffPagedResponse PageBlobClient::GetManagedDiskPageRangesDiff(
      const std::string& previousSnapshotUrl,
      const GetPageRangesOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::PageBlob::GetPageBlobPageRangesOptions protocolLayerOptions;
    protocolLayerOptions.PreviousSnapshotUrl = previousSnapshotUrl;
    protocolLayerOptions.Range = options.Range;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    auto response = _detail::BlobRestClient::PageBlob::GetPageRanges(
        *m_pipeline, m_blobUrl, protocolLayerOptions, _internal::WithReplicaStatus(context));

    GetPageRangesDiffPagedResponse pagedResponse;

    pagedResponse.ETag = std::move(response.Value.ETag);
    pagedResponse.LastModified = std::move(response.Value.LastModified);
    pagedResponse.BlobSize = response.Value.BlobSize;
    pagedResponse.PageRanges = std::move(response.Value.PageRanges);
    pagedResponse.ClearRanges = std::move(response.Value.ClearRanges);
    pagedResponse.m_pageBlobClient = std::make_shared<PageBlobClient>(*this);
    pagedResponse.m_operationOptions = options;
    pagedResponse.m_previousSnapshotUrl = previousSnapshotUrl;
    pagedResponse.CurrentPageToken = std::string();
    pagedResponse.NextPageToken = std::string();
    pagedResponse.RawResponse = std::move(response.RawResponse);

    return pagedResponse;
  }

  StartBlobCopyOperation PageBlobClient::StartCopyIncremental(
      const std::string& sourceUri,
      const StartBlobCopyIncrementalOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::PageBlob::StartBlobCopyIncrementalOptions protocolLayerOptions;
    protocolLayerOptions.CopySource = sourceUri;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;

    auto response = _detail::BlobRestClient::PageBlob::StartCopyIncremental(
        *m_pipeline, m_blobUrl, protocolLayerOptions, context);
    StartBlobCopyOperation res;
    res.m_rawResponse = std::move(response.RawResponse);
    res.m_blobClient = std::make_shared<BlobClient>(*this);
    return res;
  }

}}} // namespace Azure::Storage::Blobs
