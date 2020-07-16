// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "blob_options.hpp"
#include "blobs/blob_client.hpp"
#include "common/storage_credential.hpp"
#include "common/storage_uri_builder.hpp"
#include "internal/protocol/blob_rest_client.hpp"

#include <map>
#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Blobs {

  /**
   * The BlobContainerClient allows you to manipulate Azure Storage containers and their
   * blobs.
   */
  class BlobContainerClient {
  public:
    /**
     * @brief Initialize a new instance of BlobContainerClient.
     *
     * @param connectionString A connection string includes the authentication information required
     * for your application to access data in an Azure Storage account at runtime.
     * @param containerName The name of the container containing this blob.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     * @return A new BlobContainerClient instance.
     */
    static BlobContainerClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& containerName,
        const BlobContainerClientOptions& options = BlobContainerClientOptions());

    /**
     * @brief Initialize a new instance of BlobContainerClient.
     *
     * @param containerUri A uri
     * referencing the blob container that includes the name of the account and the name of the
     * container.
     * @param credential The shared key credential used to sign
     * requests.
     * @param options Optional client options that define the transport pipeline
     * policies for authentication, retries, etc., that are applied to every request.
     */
    explicit BlobContainerClient(
        const std::string& containerUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const BlobContainerClientOptions& options = BlobContainerClientOptions());

    /**
     * @brief Initialize a new instance of BlobContainerClient.
     *
     * @param containerUri A uri
     * referencing the blob container that includes the name of the account and the name of the
     * container.
     * @param credential The token credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit BlobContainerClient(
        const std::string& containerUri,
        std::shared_ptr<TokenCredential> credential,
        const BlobContainerClientOptions& options = BlobContainerClientOptions());

    /**
     * @brief Initialize a new instance of BlobContainerClient.
     *
     * @param containerUri A uri
     * referencing the blob that includes the name of the account and the name of the container, and
     * possibly also a SAS token.
     * @param options Optional client
     * options that define the transport pipeline policies for authentication, retries, etc., that
     * are applied to every request.
     */
    explicit BlobContainerClient(
        const std::string& containerUri,
        const BlobContainerClientOptions& options = BlobContainerClientOptions());

    /**
     * @brief Create a new BlobClient object by appending blobName to the end of uri. The
     * new BlobClient uses the same request policy pipeline as this BlobContainerClient.
     *
     * @param blobName The name of the blob.
     * @return A new BlobClient instance.
     */
    BlobClient GetBlobClient(const std::string& blobName) const;

    /**
     * @brief Create a new BlockBlobClient object by appending blobName to the end of uri.
     * The new BlockBlobClient uses the same request policy pipeline as this BlobContainerClient.
     *
     * @param blobName The name of the blob.
     * @return A new BlockBlobClient instance.
     */
    BlockBlobClient GetBlockBlobClient(const std::string& blobName) const;

    /**
     * @brief Create a new AppendBlobClient object by appending blobName to the end of uri.
     * The new AppendBlobClient uses the same request policy pipeline as this BlobContainerClient.
     *
     * @param blobName The name of the blob.
     * @return A new AppendBlobClient instance.
     */
    AppendBlobClient GetAppendBlobClient(const std::string& blobName) const;

    /**
     * @brief Create a new PageBlobClient object by appending blobName to the end of uri.
     * The new PageBlobClient uses the same request policy pipeline as this BlobContainerClient.
     *
     * @param blobName The name of the blob.
     * @return A new PageBlobClient instance.
     */
    PageBlobClient GetPageBlobClient(const std::string& blobName) const;

    /**
     * @brief Gets the container's primary uri endpoint.
     *
     * @return The
     * container's primary uri endpoint.
     */
    std::string GetUri() const { return m_containerUrl.ToString(); }

    /**
     * @brief Creates a new container under the specified account. If the container with the
     * same name already exists, the operation fails.
     *
     * @param options Optional
     * parameters to execute this function.
     * @return A BlobContainerInfo describing the newly
     * created blob container.
     */
    BlobContainerInfo Create(
        const CreateBlobContainerOptions& options = CreateBlobContainerOptions()) const;

    /**
     * @brief Marks the specified container for deletion. The container and any blobs
     * contained within it are later deleted during garbage collection.
     *
     * @param
     * options Optional parameters to execute this function.
     * @return A DeleteContainerResponse if successful.
     */
    DeleteContainerResponse Delete(
        const DeleteBlobContainerOptions& options = DeleteBlobContainerOptions()) const;

    /**
     * @brief Returns all user-defined metadata and system properties for the specified
     * container. The data returned does not include the container's list of blobs.
     *
     * @param options Optional parameters to execute this function.
     * @return A
     * BlobContainerProperties describing the container and its properties.
     */
    BlobContainerProperties GetProperties(
        const GetBlobContainerPropertiesOptions& options
        = GetBlobContainerPropertiesOptions()) const;

    /**
     * @brief Sets one or more user-defined name-value pairs for the specified container.
     *
     * @param metadata Custom metadata to set for this container.
     * @param options
     * Optional parameters to execute this function.
     * @return A SetContainerMetadataResponse if successful.
     */
    SetContainerMetadataResponse SetMetadata(
        std::map<std::string, std::string> metadata,
        SetBlobContainerMetadataOptions options = SetBlobContainerMetadataOptions()) const;

    /**
     * @brief Returns a single segment of blobs in this container, starting from the
     * specified Marker, Use an empty Marker to start enumeration from the beginning and the
     * NextMarker if it's not empty to make subsequent calls to ListBlobsFlat to continue
     * enumerating the blobs segment by segment. Blobs are ordered lexicographically by name.
     *
     * @param options Optional parameters to execute this function.
     * @return A
     * BlobsFlatSegment describing a segment of the blobs in the container.
     */
    BlobsFlatSegment ListBlobsFlat(const ListBlobsOptions& options = ListBlobsOptions()) const;

    /**
     * @brief Returns a single segment of blobs in this container, starting from the
     * specified Marker, Use an empty Marker to start enumeration from the beginning and the
     * NextMarker if it's not empty to make subsequent calls to ListBlobsByHierarchy to continue
     * enumerating the blobs segment by segment. Blobs are ordered lexicographically by name. A
     * Delimiter can be used to traverse a virtual hierarchy of blobs as though it were a file
     * system.
     *
     * @param delimiter This can be used to to traverse a virtual hierarchy of blobs as though it
     * were a file system. The delimiter may be a single character or a string.
     * @param options Optional parameters to execute this function.
     * @return A BlobsFlatSegment describing a segment of the blobs in the container.
     */
    BlobsHierarchySegment ListBlobsByHierarchy(
        const std::string& delimiter,
        const ListBlobsOptions& options = ListBlobsOptions()) const;

  private:
    UriBuilder m_containerUrl;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    explicit BlobContainerClient(
        UriBuilder containerUri,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : m_containerUrl(std::move(containerUri)), m_pipeline(std::move(pipeline))
    {
    }

    friend class BlobServiceClient;
  };

}}} // namespace Azure::Storage::Blobs
