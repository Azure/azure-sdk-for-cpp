// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/blobs/append_blob_client.hpp"

#include <azure/storage/common/constants.hpp>
#include <azure/storage/common/storage_common.hpp>

namespace Azure { namespace Storage { namespace Blobs {

  AppendBlobClient AppendBlobClient::CreateFromConnectionString(
      const std::string& connectionString,
      const std::string& blobContainerName,
      const std::string& blobName,
      const BlobClientOptions& options)
  {
    AppendBlobClient newClient(BlobClient::CreateFromConnectionString(
        connectionString, blobContainerName, blobName, options));
    return newClient;
  }

  AppendBlobClient::AppendBlobClient(
      const std::string& blobUrl,
      std::shared_ptr<StorageSharedKeyCredential> credential,
      const BlobClientOptions& options)
      : BlobClient(blobUrl, std::move(credential), options)
  {
  }

  AppendBlobClient::AppendBlobClient(
      const std::string& blobUrl,
      std::shared_ptr<Core::TokenCredential> credential,
      const BlobClientOptions& options)
      : BlobClient(blobUrl, std::move(credential), options)
  {
  }

  AppendBlobClient::AppendBlobClient(const std::string& blobUrl, const BlobClientOptions& options)
      : BlobClient(blobUrl, options)
  {
  }

  AppendBlobClient::AppendBlobClient(BlobClient blobClient) : BlobClient(std::move(blobClient)) {}

  AppendBlobClient AppendBlobClient::WithSnapshot(const std::string& snapshot) const
  {
    AppendBlobClient newClient(*this);
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

  AppendBlobClient AppendBlobClient::WithVersionId(const std::string& versionId) const
  {
    AppendBlobClient newClient(*this);
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

  Azure::Core::Response<Models::CreateAppendBlobResult> AppendBlobClient::Create(
      const CreateAppendBlobOptions& options) const
  {
    Details::BlobRestClient::AppendBlob::CreateAppendBlobOptions protocolLayerOptions;
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
    return Details::BlobRestClient::AppendBlob::Create(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::CreateAppendBlobResult> AppendBlobClient::CreateIfNotExists(
      const CreateAppendBlobOptions& options) const
  {
    auto optionsCopy = options;
    optionsCopy.AccessConditions.IfNoneMatch = ETagWildcard;
    try
    {
      return Create(optionsCopy);
    }
    catch (StorageException& e)
    {
      if (e.StatusCode == Core::Http::HttpStatusCode::Conflict
          && e.ErrorCode == "BlobAlreadyExists")
      {
        Models::CreateAppendBlobResult ret;
        ret.Created = false;
        return Azure::Core::Response<Models::CreateAppendBlobResult>(std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Core::Response<Models::AppendBlockResult> AppendBlobClient::AppendBlock(
      Azure::Core::Http::BodyStream* content,
      const AppendBlockOptions& options) const
  {
    Details::BlobRestClient::AppendBlob::AppendBlockOptions protocolLayerOptions;
    protocolLayerOptions.TransactionalContentHash = options.TransactionalContentHash;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.MaxSize = options.AccessConditions.IfMaxSizeLessThanOrEqual;
    protocolLayerOptions.AppendPosition = options.AccessConditions.IfAppendPositionEqual;
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
    return Details::BlobRestClient::AppendBlob::AppendBlock(
        options.Context, *m_pipeline, m_blobUrl, content, protocolLayerOptions);
  }

  Azure::Core::Response<Models::AppendBlockFromUriResult> AppendBlobClient::AppendBlockFromUri(
      const std::string& sourceUri,
      const AppendBlockFromUriOptions& options) const
  {
    Details::BlobRestClient::AppendBlob::AppendBlockFromUriOptions protocolLayerOptions;
    protocolLayerOptions.SourceUri = sourceUri;
    protocolLayerOptions.SourceRange = options.SourceRange;
    protocolLayerOptions.TransactionalContentHash = options.TransactionalContentHash;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.MaxSize = options.AccessConditions.IfMaxSizeLessThanOrEqual;
    protocolLayerOptions.AppendPosition = options.AccessConditions.IfAppendPositionEqual;
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
    return Details::BlobRestClient::AppendBlob::AppendBlockFromUri(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

  Azure::Core::Response<Models::SealAppendBlobResult> AppendBlobClient::Seal(
      const SealAppendBlobOptions& options) const
  {
    Details::BlobRestClient::AppendBlob::SealAppendBlobOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.AppendPosition = options.AccessConditions.IfAppendPositionEqual;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return Details::BlobRestClient::AppendBlob::Seal(
        options.Context, *m_pipeline, m_blobUrl, protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs
