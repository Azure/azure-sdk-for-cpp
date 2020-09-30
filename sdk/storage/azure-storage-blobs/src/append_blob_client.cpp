// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/append_blob_client.hpp"

#include "azure/storage/common/constants.hpp"
#include "azure/storage/common/storage_common.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  AppendBlobClient AppendBlobClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& containerName,
      const std::string& blobName,
      const AppendBlobClientOptions& options)
  {
    AppendBlobClient newClient(
        BlobClient::CreateFromConnectionString(connectionString, containerName, blobName, options));
    return newClient;
  }

  AppendBlobClient::AppendBlobClient(
      const std::string& blobUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const AppendBlobClientOptions& options)
      : BlobClient(blobUri, std::move(credential), options)
  {
  }

  AppendBlobClient::AppendBlobClient(
      const std::string& blobUri,
      std::shared_ptr<Core::Credentials::ClientSecretCredential> credential,
      const AppendBlobClientOptions& options)
      : BlobClient(blobUri, std::move(credential), options)
  {
  }

  AppendBlobClient::AppendBlobClient(
      const std::string& blobUri,
      const AppendBlobClientOptions& options)
      : BlobClient(blobUri, options)
  {
  }

  AppendBlobClient::AppendBlobClient(BlobClient blobClient) : BlobClient(std::move(blobClient)) {}

  AppendBlobClient AppendBlobClient::WithSnapshot(const std::string& snapshot) const
  {
    AppendBlobClient newClient(*this);
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

  AppendBlobClient AppendBlobClient::WithVersionId(const std::string& versionId) const
  {
    AppendBlobClient newClient(*this);
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

  Azure::Core::Response<CreateAppendBlobResult> AppendBlobClient::Create(
      const CreateAppendBlobOptions& options) const
  {
    BlobRestClient::AppendBlob::CreateAppendBlobOptions protocolLayerOptions;
    protocolLayerOptions.HttpHeaders = options.HttpHeaders;
    protocolLayerOptions.Metadata = options.Metadata;
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
    return BlobRestClient::AppendBlob::Create(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<AppendBlockResult> AppendBlobClient::AppendBlock(
      Azure::Core::Http::BodyStream* content,
      const AppendBlockOptions& options) const
  {
    BlobRestClient::AppendBlob::AppendBlockOptions protocolLayerOptions;
    protocolLayerOptions.TransactionalContentMd5 = options.TransactionalContentMd5;
    protocolLayerOptions.TransactionalContentCrc64 = options.TransactionalContentCrc64;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.MaxSize = options.AccessConditions.MaxSize;
    protocolLayerOptions.AppendPosition = options.AccessConditions.AppendPosition;
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
    return BlobRestClient::AppendBlob::AppendBlock(
        options.Context, *m_pipeline, m_blobUrl, content, protocolLayerOptions);
  }

  Azure::Core::Response<AppendBlockFromUriResult> AppendBlobClient::AppendBlockFromUri(
      const std::string& sourceUri,
      const AppendBlockFromUriOptions& options) const
  {
    BlobRestClient::AppendBlob::AppendBlockFromUriOptions protocolLayerOptions;
    protocolLayerOptions.SourceUri = sourceUri;
    if (options.SourceOffset.HasValue() && options.SourceLength.HasValue())
    {
      protocolLayerOptions.SourceRange = std::make_pair(
          options.SourceOffset.GetValue(),
          options.SourceOffset.GetValue() + options.SourceLength.GetValue() - 1);
    }
    else if (options.SourceOffset.HasValue())
    {
      protocolLayerOptions.SourceRange = std::make_pair(
          options.SourceOffset.GetValue(),
          std::numeric_limits<
              std::remove_reference_t<decltype(options.SourceOffset.GetValue())>>::max());
    }
    protocolLayerOptions.TransactionalContentMd5 = options.TransactionalContentMd5;
    protocolLayerOptions.TransactionalContentCrc64 = options.TransactionalContentCrc64;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.MaxSize = options.AccessConditions.MaxSize;
    protocolLayerOptions.AppendPosition = options.AccessConditions.AppendPosition;
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
    return BlobRestClient::AppendBlob::AppendBlockFromUri(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<SealAppendBlobResult> AppendBlobClient::Seal(
      const SealAppendBlobOptions& options) const
  {
    BlobRestClient::AppendBlob::SealAppendBlobOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.AppendPosition = options.AccessConditions.AppendPosition;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return BlobRestClient::AppendBlob::Seal(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs
