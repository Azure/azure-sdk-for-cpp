// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/append_blob_client.hpp"

#include "common/storage_common.hpp"

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
      std::shared_ptr<TokenCredential> credential,
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
      newClient.m_blobUrl.RemoveQuery("snapshot");
    }
    else
    {
      newClient.m_blobUrl.AppendQuery("snapshot", snapshot);
    }
    return newClient;
  }

  BlobContentInfo AppendBlobClient::Create(const CreateAppendBlobOptions& options)
  {
    BlobRestClient::AppendBlob::CreateOptions protocolLayerOptions;
    protocolLayerOptions.Properties = options.Properties;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::AppendBlob::Create(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  BlobAppendInfo AppendBlobClient::AppendBlock(
      std::unique_ptr<Azure::Core::Http::BodyStream> content,
      const AppendBlockOptions& options)
  {
    BlobRestClient::AppendBlob::AppendBlockOptions protocolLayerOptions;
    protocolLayerOptions.BodyStream = std::move(content);
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.ContentCRC64 = options.ContentCRC64;
    protocolLayerOptions.LeaseId = options.LeaseId;
    protocolLayerOptions.MaxSize = options.MaxSize;
    protocolLayerOptions.AppendPosition = options.AppendPosition;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::AppendBlob::AppendBlock(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  BlobAppendInfo AppendBlobClient::AppendBlockFromUri(
      const std::string& sourceUri,
      const AppendBlockFromUriOptions& options) const
  {
    BlobRestClient::AppendBlob::AppendBlockFromUriOptions protocolLayerOptions;
    protocolLayerOptions.SourceUri = sourceUri;
    protocolLayerOptions.SourceRange
        = std::make_pair(options.SourceOffset, options.SourceOffset + options.SourceLength - 1);
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.ContentCRC64 = options.ContentCRC64;
    protocolLayerOptions.LeaseId = options.LeaseId;
    protocolLayerOptions.MaxSize = options.MaxSize;
    protocolLayerOptions.AppendPosition = options.AppendPosition;
    protocolLayerOptions.IfModifiedSince = options.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.IfNoneMatch;
    return BlobRestClient::AppendBlob::AppendBlockFromUri(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs
