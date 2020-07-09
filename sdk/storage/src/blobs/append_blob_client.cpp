// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/append_blob_client.hpp"

#include "common/constants.hpp"
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
      newClient.m_blobUrl.RemoveQuery(Details::c_HttpQuerySnapshot);
    }
    else
    {
      newClient.m_blobUrl.AppendQuery(Details::c_HttpQuerySnapshot, snapshot);
    }
    return newClient;
  }

  BlobContentInfo AppendBlobClient::Create(const CreateAppendBlobOptions& options)
  {
    BlobRestClient::AppendBlob::CreateOptions protocolLayerOptions;
    protocolLayerOptions.HttpHeaders = options.HttpHeaders;
    protocolLayerOptions.Metadata = options.Metadata;
    protocolLayerOptions.LeaseId = options.Conditions.LeaseId;
    protocolLayerOptions.IfModifiedSince = options.Conditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.Conditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.Conditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.Conditions.IfNoneMatch;
    return BlobRestClient::AppendBlob::Create(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

  BlobAppendInfo AppendBlobClient::AppendBlock(
      Azure::Core::Http::BodyStream& content,
      const AppendBlockOptions& options)
  {
    BlobRestClient::AppendBlob::AppendBlockOptions protocolLayerOptions;
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.ContentCRC64 = options.ContentCRC64;
    protocolLayerOptions.LeaseId = options.Conditions.LeaseId;
    protocolLayerOptions.MaxSize = options.Conditions.MaxSize;
    protocolLayerOptions.AppendPosition = options.Conditions.AppendPosition;
    protocolLayerOptions.IfModifiedSince = options.Conditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.Conditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.Conditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.Conditions.IfNoneMatch;
    return BlobRestClient::AppendBlob::AppendBlock(
        options.Context, *m_pipeline, m_blobUrl.ToString(), content, protocolLayerOptions);
  }

  BlobAppendInfo AppendBlobClient::AppendBlockFromUri(
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
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.ContentCRC64 = options.ContentCRC64;
    protocolLayerOptions.LeaseId = options.Conditions.LeaseId;
    protocolLayerOptions.MaxSize = options.Conditions.MaxSize;
    protocolLayerOptions.AppendPosition = options.Conditions.AppendPosition;
    protocolLayerOptions.IfModifiedSince = options.Conditions.IfModifiedSince;
    protocolLayerOptions.IfUnmodifiedSince = options.Conditions.IfUnmodifiedSince;
    protocolLayerOptions.IfMatch = options.Conditions.IfMatch;
    protocolLayerOptions.IfNoneMatch = options.Conditions.IfNoneMatch;
    return BlobRestClient::AppendBlob::AppendBlockFromUri(
        options.Context, *m_pipeline, m_blobUrl.ToString(), protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs
