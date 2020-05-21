// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/blob_container_client.hpp"
#include "common/storage_common.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  BlobContainerClient::BlobContainerClient(
      const std::string& containerUri,
      BlobContainerClientOptions options)
      : m_ContainerUri(containerUri)
  {
    unused(options);
  }

  BlobContainerInfo BlobContainerClient::Create(const CreateBlobContainerOptions& options)
  {
    BlobRestClient::Container::CreateOptions protocolLayerOptions;
    protocolLayerOptions.AccessType = options.AccessType;
    protocolLayerOptions.Metadata = options.Metadata;
    return BlobRestClient::Container::Create(m_ContainerUri, protocolLayerOptions);
  }

  BasicResponse BlobContainerClient::Delete(const DeleteBlobContainerOptions& options)
  {
    unused(options);
    BlobRestClient::Container::DeleteOptions protocolLayerOptions;
    return BlobRestClient::Container::Delete(m_ContainerUri, protocolLayerOptions);
  }

  BlobContainerProperties BlobContainerClient::GetProperties(
      const GetBlobContainerPropertiesOptions& options)
  {
    unused(options);
    BlobRestClient::Container::GetPropertiesOptions protocolLayerOptions;
    return BlobRestClient::Container::GetProperties(m_ContainerUri, protocolLayerOptions);
  }

  BlobContainerInfo BlobContainerClient::SetMetadata(
      std::map<std::string, std::string> metadata,
      SetBlobContainerMetadataOptions options)
  {
    unused(options);
    BlobRestClient::Container::SetMetadataOptions protocolLayerOptions;
    protocolLayerOptions.Metadata = metadata;
    return BlobRestClient::Container::SetMetadata(m_ContainerUri, protocolLayerOptions);
  }

  BlobsFlatSegment BlobContainerClient::ListBlobs(const ListBlobsOptions& options)
  {
    BlobRestClient::Container::ListBlobsOptions protocolLayerOptions;
    protocolLayerOptions.Prefix = options.Prefix;
    protocolLayerOptions.Delimiter = options.Delimiter;
    protocolLayerOptions.Marker = options.Marker;
    protocolLayerOptions.MaxResults = options.MaxResults;
    protocolLayerOptions.Include = options.Include;
    return BlobRestClient::Container::ListBlobs(m_ContainerUri, protocolLayerOptions);
  }

}}} // namespace Azure::Storage::Blobs
