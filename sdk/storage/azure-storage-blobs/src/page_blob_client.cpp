// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/page_blob_client.hpp"

#include <azure/storage/common/concurrent_transfer.hpp>
#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/file_io.hpp>
#include <azure/storage/common/storage_common.hpp>
#include <azure/storage/common/storage_switch_to_secondary_policy.hpp>

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
      newClient.m_blobUrl.RemoveQueryParameter(Storage::_detail::HttpQuerySnapshot);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          Storage::_detail::HttpQuerySnapshot, Storage::_detail::UrlEncodeQueryParameter(snapshot));
    }
    return newClient;
  }

  PageBlobClient PageBlobClient::WithVersionId(const std::string& versionId) const
  {
    PageBlobClient newClient(*this);
    if (versionId.empty())
    {
      newClient.m_blobUrl.RemoveQueryParameter(Storage::_detail::HttpQueryVersionId);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          Storage::_detail::HttpQueryVersionId,
          Storage::_detail::UrlEncodeQueryParameter(versionId));
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
    return _detail::BlobRestClient::PageBlob::Create(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
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
        ret.RequestId = e.RequestId;
        ret.Created = false;
        return Azure::Response<Models::CreatePageBlobResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::UploadPageBlobPagesResult> PageBlobClient::UploadPages(
      int64_t offset,
      Azure::Core::IO::BodyStream* content,
      const UploadPageBlobPagesOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::PageBlob::UploadPageBlobPagesOptions protocolLayerOptions;
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
    return _detail::BlobRestClient::PageBlob::UploadPages(
        context, *m_pipeline, m_blobUrl, content, protocolLayerOptions);
  }

  Azure::Response<Models::UploadPageBlobPagesFromUriResult> PageBlobClient::UploadPagesFromUri(
      int64_t destinationOffset,
      std::string sourceUri,
      Azure::Core::Http::HttpRange sourceRange,
      const UploadPageBlobPagesFromUriOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::PageBlob::UploadPageBlobPagesFromUriOptions protocolLayerOptions;
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
    return _detail::BlobRestClient::PageBlob::UploadPagesFromUri(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Response<Models::ClearPageBlobPagesResult> PageBlobClient::ClearPages(
      Azure::Core::Http::HttpRange range,
      const ClearPageBlobPagesOptions& options,
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
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return _detail::BlobRestClient::PageBlob::ClearPages(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
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
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return _detail::BlobRestClient::PageBlob::Resize(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Response<Models::GetPageBlobPageRangesResult> PageBlobClient::GetPageRanges(
      const GetPageBlobPageRangesOptions& options,
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
    return _detail::BlobRestClient::PageBlob::GetPageRanges(
        Storage::_detail::WithReplicaStatus(context), *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Response<Models::GetPageBlobPageRangesResult> PageBlobClient::GetPageRangesDiff(
      const std::string& previousSnapshot,
      const GetPageBlobPageRangesOptions& options,
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
    return _detail::BlobRestClient::PageBlob::GetPageRanges(
        Storage::_detail::WithReplicaStatus(context), *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Response<Models::GetPageBlobPageRangesResult> PageBlobClient::GetManagedDiskPageRangesDiff(
      const std::string& previousSnapshotUrl,
      const GetPageBlobPageRangesOptions& options,
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
    return _detail::BlobRestClient::PageBlob::GetPageRanges(
        Storage::_detail::WithReplicaStatus(context), *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  StartCopyBlobOperation PageBlobClient::StartCopyIncremental(
      const std::string& sourceUri,
      const StartCopyPageBlobIncrementalOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::PageBlob::StartCopyPageBlobIncrementalOptions protocolLayerOptions;
    protocolLayerOptions.CopySource = sourceUri;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;

    auto response = _detail::BlobRestClient::PageBlob::StartCopyIncremental(
        context, *m_pipeline, m_blobUrl, protocolLayerOptions);
    StartCopyBlobOperation res;
    res.m_rawResponse = response.ExtractRawResponse();
    res.RequestId = std::move(response->RequestId);
    res.ETag = std::move(response->ETag);
    res.LastModified = std::move(response->LastModified);
    res.CopyId = std::move(response->CopyId);
    res.CopyStatus = std::move(response->CopyStatus);
    res.VersionId = std::move(response->VersionId);
    res.m_blobClient = std::make_shared<BlobClient>(*this);
    return res;
  }

}}} // namespace Azure::Storage::Blobs
