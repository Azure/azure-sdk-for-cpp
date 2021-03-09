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
      newClient.m_blobUrl.RemoveQueryParameter(Storage::_detail::HttpQuerySnapshot);
    }
    else
    {
      newClient.m_blobUrl.AppendQueryParameter(
          Storage::_detail::HttpQuerySnapshot, Storage::_detail::UrlEncodeQueryParameter(snapshot));
    }
    return newClient;
  }

  AppendBlobClient AppendBlobClient::WithVersionId(const std::string& versionId) const
  {
    AppendBlobClient newClient(*this);
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

  Azure::Response<Models::CreateAppendBlobResult> AppendBlobClient::Create(
      const CreateAppendBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::AppendBlob::CreateAppendBlobOptions protocolLayerOptions;
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
    return _detail::BlobRestClient::AppendBlob::Create(
        *m_pipeline, m_blobUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::CreateAppendBlobResult> AppendBlobClient::CreateIfNotExists(
      const CreateAppendBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    auto optionsCopy = options;
    optionsCopy.AccessConditions.IfNoneMatch = Azure::ETag::Any();
    try
    {
      return Create(optionsCopy, context);
    }
    catch (StorageException& e)
    {
      if (e.StatusCode == Core::Http::HttpStatusCode::Conflict
          && e.ErrorCode == "BlobAlreadyExists")
      {
        Models::CreateAppendBlobResult ret;
        ret.RequestId = e.RequestId;
        ret.Created = false;
        return Azure::Response<Models::CreateAppendBlobResult>(
            std::move(ret), std::move(e.RawResponse));
      }
      throw;
    }
  }

  Azure::Response<Models::AppendBlockResult> AppendBlobClient::AppendBlock(
      Azure::IO::BodyStream* content,
      const AppendBlockOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::AppendBlob::AppendBlockOptions protocolLayerOptions;
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
    return _detail::BlobRestClient::AppendBlob::AppendBlock(
        *m_pipeline, m_blobUrl, content, protocolLayerOptions, context);
  }

  Azure::Response<Models::AppendBlockFromUriResult> AppendBlobClient::AppendBlockFromUri(
      const std::string& sourceUri,
      const AppendBlockFromUriOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::AppendBlob::AppendBlockFromUriOptions protocolLayerOptions;
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
    return _detail::BlobRestClient::AppendBlob::AppendBlockFromUri(
        *m_pipeline, m_blobUrl, protocolLayerOptions, context);
  }

  Azure::Response<Models::SealAppendBlobResult> AppendBlobClient::Seal(
      const SealAppendBlobOptions& options,
      const Azure::Core::Context& context) const
  {
    _detail::BlobRestClient::AppendBlob::SealAppendBlobOptions protocolLayerOptions;
    protocolLayerOptions.LeaseId = options.AccessConditions.LeaseId;
    protocolLayerOptions.AppendPosition = options.AccessConditions.IfAppendPositionEqual;
    protocolLayerOptions.IfModifiedSince = options.AccessConditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.AccessConditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.AccessConditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.AccessConditions.IfNoneMatch;
    protocolLayerOptions.IfTags = options.AccessConditions.TagConditions;
    return _detail::BlobRestClient::AppendBlob::Seal(
        *m_pipeline, m_blobUrl, protocolLayerOptions, context);
  }

}}} // namespace Azure::Storage::Blobs
