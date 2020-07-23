// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "blob_options.hpp"
#include "blobs/blob_container_client.hpp"
#include "common/storage_credential.hpp"
#include "common/storage_uri_builder.hpp"
#include "credentials/credentials.hpp"
#include "protocol/blob_rest_client.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Blobs {

  /**
   * The BlobServiceClient allows you to manipulate Azure Storage service resources and blob
   * containers. The storage account provides the top-level namespace for the Blob service.
   */
  class BlobServiceClient {
  public:
    /**
     * @brief Initialize a new instance of BlobServiceClient.
     *
     * @param connectionString A connection string includes the authentication information required
     * for your application to access data in an Azure Storage account at runtime.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     * @return A new BlobServiceClient instance.
     */
    static BlobServiceClient CreateFromConnectionString(
        const std::string& connectionString,
        const BlobServiceClientOptions& options = BlobServiceClientOptions());

    /**
     * @brief Initialize a new instance of BlobServiceClient.
     *
     * @param serviceUri A uri
     * referencing the blob that includes the name of the account.
     * @param credential The shared key credential used to sign
     * requests.
     * @param options Optional client options that define the transport pipeline
     * policies for authentication, retries, etc., that are applied to every request.
     */
    explicit BlobServiceClient(
        const std::string& serviceUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const BlobServiceClientOptions& options = BlobServiceClientOptions());

    /**
     * @brief Initialize a new instance of BlobServiceClient.
     *
     * @param serviceUri A uri
     * referencing the blob that includes the name of the account.
     * @param credential The token credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit BlobServiceClient(
        const std::string& serviceUri,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const BlobServiceClientOptions& options = BlobServiceClientOptions());

    /**
     * @brief Initialize a new instance of BlobServiceClient.
     *
     * @param serviceUri A uri
     * referencing the blob that includes the name of the account, and possibly also a SAS token.
     * @param options Optional client
     * options that define the transport pipeline policies for authentication, retries, etc., that
     * are applied to every request.
     */
    explicit BlobServiceClient(
        const std::string& serviceUri,
        const BlobServiceClientOptions& options = BlobServiceClientOptions());

    /**
     * @brief Creates a new BlobContainerClient object with the same uri as this BlobServiceClient.
     * The new BlobContainerClient uses the same request policy pipeline as this BlobServiceClient.
     *
     *
     * @return A new BlobContainerClient instance.
     */
    BlobContainerClient GetBlobContainerClient(const std::string& containerName) const;

    /**
     * @brief Gets the blob service's primary uri endpoint.
     *
     * @return the blob
     * service's primary uri endpoint.
     */
    std::string GetUri() const { return m_serviceUrl.ToString(); }

    /**
     * @brief Returns a single segment of blob containers in the storage account, starting
     * from the specified Marker. Use an empty Marker to start enumeration from the beginning and
     * the NextMarker if it's not empty to make subsequent calls to ListBlobContainersSegment to
     * continue enumerating the containers segment by segment. Containers are ordered
     * lexicographically by name.
     *
     * @param options Optional parameters to execute this function.
     * @return A
     * ListContainersSegment describing segment of the blob containers in the storage account.
     */
    Azure::Core::Response<ListContainersSegment> ListBlobContainersSegment(
        const ListBlobContainersOptions& options = ListBlobContainersOptions()) const;

    /**
     * @brief Retrieves a key that can be used to delegate Active Directory authorization to
     * shared access signatures.
     *
     * @param startsOn Start time for the key's validity, in ISO date format. The time should be
     * specified in UTC.
     * @param expiresOn Expiration of the key's validity, in ISO date format. The time should be
     * specified in UTC.
     * @param options Optional parameters to execute
     * this function.
     * @return A deserialized UserDelegationKey instance.
     */
    Azure::Core::Response<UserDelegationKey> GetUserDelegationKey(
        const std::string& startsOn,
        const std::string& expiresOn,
        const GetUserDelegationKeyOptions& options = GetUserDelegationKeyOptions()) const;

  protected:
    UriBuilder m_serviceUrl;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;
  };
}}} // namespace Azure::Storage::Blobs
