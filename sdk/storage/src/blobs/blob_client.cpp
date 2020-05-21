// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/blob_client.hpp"
#include "common/storage_common.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobClient::BlobClient(const std::string& blobUri, BlobClientOptions options) : m_blobUrl(blobUri)
  {
    unused(options);
  }

  BlobClient BlobClient::WithSnapshot(const std::string& snapshot)
  {
    BlobClient newClient(*this);
    m_blobUrl += "?snapshot=" + snapshot;
    return newClient;
  }

  FlattenedDownloadProperties BlobClient::Download(const DownloadBlobOptions& options)
  {
    BlobRestClient::Blob::DownloadOptions protocolLayerOptions;
    if (options.Offset != std::numeric_limits<decltype(options.Offset)>::max())
    {
      protocolLayerOptions.Range
          = std::make_pair(options.Offset, options.Offset + options.Length - 1);
    }
    else
    {
      protocolLayerOptions.Range
          = std::make_pair(std::numeric_limits<uint64_t>::max(), uint64_t(0));
    }

    return BlobRestClient::Blob::Download(m_blobUrl, protocolLayerOptions);
  }

  BlobProperties BlobClient::GetProperties(const GetBlobPropertiesOptions& options)
  {
    unused(options);

    BlobRestClient::Blob::GetPropertiesOptions protocolLayerOptions;
    return BlobRestClient::Blob::GetProperties(m_blobUrl, protocolLayerOptions);
  }

  BlobInfo BlobClient::SetHttpHeaders(const SetBlobHttpHeadersOptions& options)
  {
    BlobRestClient::Blob::SetHttpHeadersOptions protocolLayerOptions;
    protocolLayerOptions.ContentType = options.ContentType;
    protocolLayerOptions.ContentEncoding = options.ContentEncoding;
    protocolLayerOptions.ContentLanguage = options.ContentLanguage;
    protocolLayerOptions.ContentMD5 = options.ContentMD5;
    protocolLayerOptions.CacheControl = options.CacheControl;
    protocolLayerOptions.ContentDisposition = options.ContentDisposition;
    return BlobRestClient::Blob::SetHttpHeaders(m_blobUrl, protocolLayerOptions);
  }

  BlobInfo BlobClient::SetMetadata(
      std::map<std::string, std::string> metadata,
      const SetBlobMetadataOptions& options)
  {
    unused(options);
    BlobRestClient::Blob::SetMetadataOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = std::move(metadata);
    return BlobRestClient::Blob::SetMetadata(m_blobUrl, protocolLayerOptions);
  }

  BasicResponse BlobClient::Delete(const DeleteBlobOptions& options)
  {
    BlobRestClient::Blob::DeleteOptions protocolLayerOptions;
    protocolLayerOptions.DeleteSnapshots = options.DeleteSnapshots;
    return BlobRestClient::Blob::Delete(m_blobUrl, protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs
