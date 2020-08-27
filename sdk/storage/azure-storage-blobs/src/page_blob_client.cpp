// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/page_blob_client.hpp"

#include "azure/storage/common/concurrent_transfer.hpp"
#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/file_io.hpp"
#include "azure/storage/common/storage_common.hpp"

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
      std::shared_ptr<Core::Credentials::ClientSecretCredential> credential,
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
      newClient.m_blobUrl.RemoveQuery(Details::c_HttpQuerySnapshot);
    }
    else
    {
      newClient.m_blobUrl.AppendQuery(Details::c_HttpQuerySnapshot, snapshot);
    }
    return newClient;
  }

  PageBlobClient PageBlobClient::WithVersionId(const std::string& versionId) const
  {
    PageBlobClient newClient(*this);
    if (versionId.empty())
    {
      newClient.m_blobUrl.RemoveQuery(Details::c_HttpQueryVersionId);
    }
    else
    {
      newClient.m_blobUrl.AppendQuery(Details::c_HttpQueryVersionId, versionId);
    }
    return newClient;
  }

  Azure::Core::Response<CreatePageBlobResult> PageBlobClient::Create(
      int64_t blobContentLength,
      const CreatePageBlobOptions& options) const
  {
    BlobRestClient::PageBlob::CreatePageBlobOptions protocolLayerOptions;
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
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return BlobRestClient::PageBlob::Create(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  Azure::Core::Response<UploadPageBlobPagesResult> PageBlobClient::UploadPages(
      Azure::Core::Http::BodyStream* content,
      int64_t offset,
      const UploadPageBlobPagesOptions& options) const
  {
    BlobRestClient::PageBlob::UploadPageBlobPagesOptions protocolLayerOptions;
    protocolLayerOptions.Range = std::make_pair(offset, offset + content->Length() - 1);
    protocolLayerOptions.ContentMd5 = options.ContentMd5;
    protocolLayerOptions.ContentCrc64 = options.ContentCrc64;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return BlobRestClient::PageBlob::UploadPages(
        options.Context, *m_pipeline, m_blobUrl.ToString(), content, protocolLayerOptions);
  }

  Azure::Core::Response<UploadPageBlobPagesFromUriResult> PageBlobClient::UploadPagesFromUri(
      std::string sourceUri,
      int64_t sourceOffset,
      int64_t sourceLength,
      int64_t destinationoffset,
      const UploadPageBlobPagesFromUriOptions& options) const
  {
    BlobRestClient::PageBlob::UploadPageBlobPagesFromUriOptions protocolLayerOptions;
    protocolLayerOptions.SourceUri = sourceUri;
    protocolLayerOptions.SourceRange
        = std::make_pair(sourceOffset, sourceOffset + sourceLength - 1);
    protocolLayerOptions.Range
        = std::make_pair(destinationoffset, destinationoffset + sourceLength - 1);
    protocolLayerOptions.ContentMd5 = options.ContentMd5;
    protocolLayerOptions.ContentCrc64 = options.ContentCrc64;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return BlobRestClient::PageBlob::UploadPagesFromUri(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  Azure::Core::Response<ClearPageBlobPagesResult> PageBlobClient::ClearPages(
      int64_t offset,
      int64_t length,
      const ClearPageBlobPagesOptions& options) const
  {
    BlobRestClient::PageBlob::ClearPageBlobPagesOptions protocolLayerOptions;
    protocolLayerOptions.Range = std::make_pair(offset, offset + length - 1);
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return BlobRestClient::PageBlob::ClearPages(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  Azure::Core::Response<UploadPageBlobFromResult> PageBlobClient::UploadFrom(
      const uint8_t* buffer,
      std::size_t bufferSize,
      const UploadPageBlobFromOptions& options) const
  {
    BlobRestClient::PageBlob::CreatePageBlobOptions createOptions;
    createOptions.BlobContentLength = bufferSize;
    createOptions.HttpHeaders = options.HttpHeaders;
    createOptions.Metadata = options.Metadata;
    createOptions.Tier = options.Tier;
    if (m_customerProvidedKey.HasValue())
    {
      createOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      createOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      createOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    createOptions.EncryptionScope = m_encryptionScope;
    auto createResult = BlobRestClient::PageBlob::Create(
        options.Context, *m_pipeline, m_blobUrl.ToString(), createOptions);

    constexpr int64_t c_defaultChunkSize = 8 * 1024 * 1024;
    int64_t chunkSize
        = options.ChunkSize.HasValue() ? options.ChunkSize.GetValue() : c_defaultChunkSize;

    auto uploadPageFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      unused(chunkId, numChunks);
      Azure::Core::Http::MemoryBodyStream contentStream(buffer + offset, length);
      UploadPageBlobPagesOptions uploadPagesOptions;
      uploadPagesOptions.Context = options.Context;
      UploadPages(&contentStream, offset, uploadPagesOptions);
    };

    Details::ConcurrentTransfer(0, bufferSize, chunkSize, options.Concurrency, uploadPageFunc);

    UploadPageBlobFromResult result;
    result.ServerEncrypted = createResult->ServerEncrypted;
    result.EncryptionKeySha256 = createResult->EncryptionKeySha256;
    result.EncryptionScope = createResult->EncryptionScope;
    return Azure::Core::Response<UploadPageBlobFromResult>(
        std::move(result),
        std::make_unique<Azure::Core::Http::RawResponse>(std::move(createResult.GetRawResponse())));
  }

  Azure::Core::Response<UploadPageBlobFromResult> PageBlobClient::UploadFrom(
      const std::string& file,
      const UploadPageBlobFromOptions& options) const
  {
    Details::FileReader fileReader(file);

    BlobRestClient::PageBlob::CreatePageBlobOptions createOptions;
    createOptions.BlobContentLength = fileReader.GetFileSize();
    createOptions.HttpHeaders = options.HttpHeaders;
    createOptions.Metadata = options.Metadata;
    createOptions.Tier = options.Tier;
    if (m_customerProvidedKey.HasValue())
    {
      createOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      createOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      createOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    createOptions.EncryptionScope = m_encryptionScope;
    auto createResult = BlobRestClient::PageBlob::Create(
        options.Context, *m_pipeline, m_blobUrl.ToString(), createOptions);

    constexpr int64_t c_defaultChunkSize = 8 * 1024 * 1024;
    int64_t chunkSize
        = options.ChunkSize.HasValue() ? options.ChunkSize.GetValue() : c_defaultChunkSize;

    auto uploadPageFunc = [&](int64_t offset, int64_t length, int64_t chunkId, int64_t numChunks) {
      unused(chunkId, numChunks);
      Azure::Core::Http::FileBodyStream contentStream(fileReader.GetHandle(), offset, length);
      UploadPageBlobPagesOptions uploadPagesOptions;
      uploadPagesOptions.Context = options.Context;
      UploadPages(&contentStream, offset, uploadPagesOptions);
    };

    Details::ConcurrentTransfer(
        0, fileReader.GetFileSize(), chunkSize, options.Concurrency, uploadPageFunc);

    UploadPageBlobFromResult result;
    result.ServerEncrypted = createResult->ServerEncrypted;
    result.EncryptionKeySha256 = createResult->EncryptionKeySha256;
    result.EncryptionScope = createResult->EncryptionScope;
    return Azure::Core::Response<UploadPageBlobFromResult>(
        std::move(result),
        std::make_unique<Azure::Core::Http::RawResponse>(std::move(createResult.GetRawResponse())));
  }

  Azure::Core::Response<ResizePageBlobResult> PageBlobClient::Resize(
      int64_t blobContentLength,
      const ResizePageBlobOptions& options) const
  {
    BlobRestClient::PageBlob::ResizePageBlobOptions protocolLayerOptions;
    protocolLayerOptions.BlobContentLength = blobContentLength;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    if (m_customerProvidedKey.HasValue())
    {
      protocolLayerOptions.EncryptionKey = m_customerProvidedKey.GetValue().Key;
      protocolLayerOptions.EncryptionKeySha256 = m_customerProvidedKey.GetValue().KeyHash;
      protocolLayerOptions.EncryptionAlgorithm = m_customerProvidedKey.GetValue().Algorithm;
    }
    protocolLayerOptions.EncryptionScope = m_encryptionScope;
    return BlobRestClient::PageBlob::Resize(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  Azure::Core::Response<GetPageBlobPageRangesResult> PageBlobClient::GetPageRanges(
      const GetPageBlobPageRangesOptions& options) const
  {
    BlobRestClient::PageBlob::GetPageBlobPageRangesOptions protocolLayerOptions;
    protocolLayerOptions.PreviousSnapshot = options.PreviousSnapshot;
    protocolLayerOptions.PreviousSnapshotUrl = options.PreviousSnapshotUrl;
    if (options.Offset.HasValue() && options.Length.HasValue())
    {
      protocolLayerOptions.Range = std::make_pair(
          options.Offset.GetValue(), options.Offset.GetValue() + options.Length.GetValue() - 1);
    }
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    auto protocolLayerResponse = BlobRestClient::PageBlob::GetPageRanges(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);

    GetPageBlobPageRangesResult ret;
    ret.ETag = std::move(protocolLayerResponse->ETag);
    ret.LastModified = std::move(protocolLayerResponse->LastModified);
    ret.BlobContentLength = protocolLayerResponse->BlobContentLength;
    for (const auto& range : protocolLayerResponse->PageRanges)
    {
      ret.PageRanges.emplace_back(PageRange{range.first, range.second - range.first + 1});
    }
    for (const auto& range : protocolLayerResponse->ClearRanges)
    {
      ret.ClearRanges.emplace_back(PageRange{range.first, range.second - range.first + 1});
    }
    return Azure::Core::Response<GetPageBlobPageRangesResult>(
        std::move(ret),
        std::make_unique<Azure::Core::Http::RawResponse>(
            std::move(protocolLayerResponse.GetRawResponse())));
  }

  Azure::Core::Response<StartCopyPageBlobIncrementalResult> PageBlobClient::StartCopyIncremental(
      const std::string& sourceUri,
      const StartCopyPageBlobIncrementalOptions& options) const
  {
    BlobRestClient::PageBlob::StartCopyPageBlobIncrementalOptions protocolLayerOptions;
    protocolLayerOptions.CopySource = sourceUri;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    return BlobRestClient::PageBlob::StartCopyIncremental(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs
