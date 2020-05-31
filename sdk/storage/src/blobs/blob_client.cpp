// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/blob_client.hpp"

#include "common/storage_common.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobClient BlobClient::FromConnectionString(
      const std::string& connectionString,
      const std::string& containerName,
      const std::string& blobName,
      const BlobClientOptions& options)
  {
    auto parsedConnectionString = ParseConnectionString(connectionString);

    std::string accountName;
    std::string accountKey;
    std::string blobEndpoint;
    std::string EndpointSuffix;
    std::string defaultEndpointsProtocol = ".core.windows.net";

    auto ite = parsedConnectionString.find("AccountName");
    if (ite != parsedConnectionString.end())
    {
      accountName = ite->second;
    }
    ite = parsedConnectionString.find("AccountKey");
    if (ite != parsedConnectionString.end())
    {
      accountKey = ite->second;
    }
    ite = parsedConnectionString.find("BlobEndpoint");
    if (ite != parsedConnectionString.end())
    {
      blobEndpoint = ite->second;
    }
    ite = parsedConnectionString.find("EndpointSuffix");
    if (ite != parsedConnectionString.end())
    {
      EndpointSuffix = ite->second;
    }
    ite = parsedConnectionString.find("DefaultEndpointsProtocol");
    if (ite != parsedConnectionString.end())
    {
      defaultEndpointsProtocol = ite->second;
    }

    UrlBuilder builder;
    builder.SetScheme(defaultEndpointsProtocol);
    if (!blobEndpoint.empty())
    {
      builder = UrlBuilder(blobEndpoint);
    }
    else if (!accountName.empty())
    {
      builder.SetHost(accountName + ".blob" + defaultEndpointsProtocol);
    }
    else
    {
      throw std::runtime_error("invalid connection string");
    }
    builder.AppendPath(containerName, true);
    builder.AppendPath(blobName, true);

    auto credential = std::make_shared<SharedKeyCredential>(accountName, accountKey);

    return BlobClient(builder.to_string(), credential, options);
  }

  BlobClient::BlobClient(
      const std::string& blobUri,
      std::shared_ptr<SharedKeyCredential> credential,
      const BlobClientOptions& options)
      : BlobClient(blobUri, options)
  {
    // not implemented yet
    unused(credential, options);
  }

  BlobClient::BlobClient(
      const std::string& blobUri,
      std::shared_ptr<TokenCredential> credential,
      const BlobClientOptions& options)
      : BlobClient(blobUri, options)
  {
    // not implemented yet
    unused(credential);
  }

  BlobClient::BlobClient(const std::string& blobUri, const BlobClientOptions& options)
      : m_blobUrl(blobUri)
  {
    unused(options);
  }

  BlobClient BlobClient::WithSnapshot(const std::string& snapshot)
  {
    BlobClient newClient(*this);
    if (snapshot.empty())
    {
      m_blobUrl.RemoveQuery("snapshot");
    }
    else
    {
      m_blobUrl.AppendQuery("snapshot", snapshot);
    }
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

    return BlobRestClient::Blob::Download(m_blobUrl.to_string(), protocolLayerOptions);
  }

  BlobProperties BlobClient::GetProperties(const GetBlobPropertiesOptions& options)
  {
    unused(options);

    BlobRestClient::Blob::GetPropertiesOptions protocolLayerOptions;
    return BlobRestClient::Blob::GetProperties(m_blobUrl.to_string(), protocolLayerOptions);
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
    return BlobRestClient::Blob::SetHttpHeaders(m_blobUrl.to_string(), protocolLayerOptions);
  }

  BlobInfo BlobClient::SetMetadata(
      std::map<std::string, std::string> metadata,
      const SetBlobMetadataOptions& options)
  {
    unused(options);
    BlobRestClient::Blob::SetMetadataOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = std::move(metadata);
    return BlobRestClient::Blob::SetMetadata(m_blobUrl.to_string(), protocolLayerOptions);
  }

  BasicResponse BlobClient::Delete(const DeleteBlobOptions& options)
  {
    BlobRestClient::Blob::DeleteOptions protocolLayerOptions;
    protocolLayerOptions.DeleteSnapshots = options.DeleteSnapshots;
    return BlobRestClient::Blob::Delete(m_blobUrl.to_string(), protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs
