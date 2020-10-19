// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/identity/client_secret_credential.hpp"
#include "azure/storage/blobs/blob_container_client.hpp"
#include "azure/storage/blobs/blob_options.hpp"
#include "azure/storage/blobs/protocol/blob_rest_client.hpp"
#include "azure/storage/common/storage_credential.hpp"

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
     * @param serviceUri A uri referencing the blob that includes the name of the account.
     * @param credential The shared key credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit BlobServiceClient(
        const std::string& serviceUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const BlobServiceClientOptions& options = BlobServiceClientOptions());

    /**
     * @brief Initialize a new instance of BlobServiceClient.
     *
     * @param serviceUri A uri referencing the blob that includes the name of the account.
     * @param credential The client secret credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit BlobServiceClient(
        const std::string& serviceUri,
        std::shared_ptr<Identity::ClientSecretCredential> credential,
        const BlobServiceClientOptions& options = BlobServiceClientOptions());

    /**
     * @brief Initialize a new instance of BlobServiceClient.
     *
     * @param serviceUri A uri referencing the blob that includes the name of the account, and
     * possibly also a SAS token.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit BlobServiceClient(
        const std::string& serviceUri,
        const BlobServiceClientOptions& options = BlobServiceClientOptions());

    /**
     * @brief Creates a new BlobContainerClient object with the same uri as this BlobServiceClient.
     * The new BlobContainerClient uses the same request policy pipeline as this BlobServiceClient.
     *
     * @return A new BlobContainerClient instance.
     */
    BlobContainerClient GetBlobContainerClient(const std::string& containerName) const;

    /**
     * @brief Gets the blob service's primary uri endpoint.
     *
     * @return the blob service's primary uri endpoint.
     */
    std::string GetUri() const { return m_serviceUrl.GetAbsoluteUrl(); }

    /**
     * @brief Returns a single segment of blob containers in the storage account, starting
     * from the specified Marker. Use an empty Marker to start enumeration from the beginning and
     * the NextMarker if it's not empty to make subsequent calls to ListBlobContainersSegment to
     * continue enumerating the containers segment by segment. Containers are ordered
     * lexicographically by name.
     *
     * @param options Optional parameters to execute this function.
     * @return A ListContainersSegmentResult describing segment of the blob containers in the
     * storage account.
     */
    Azure::Core::Response<ListContainersSegmentResult> ListBlobContainersSegment(
        const ListContainersSegmentOptions& options = ListContainersSegmentOptions()) const;

    /**
     * @brief Retrieves a key that can be used to delegate Active Directory authorization to
     * shared access signatures.
     *
     * @param startsOn Start time for the key's validity, in ISO date format. The time should be
     * specified in UTC.
     * @param expiresOn Expiration of the key's validity, in ISO date format. The time should be
     * specified in UTC.
     * @param options Optional parameters to execute this function.
     * @return A deserialized GetUserDelegationKeyResult instance.
     */
    Azure::Core::Response<GetUserDelegationKeyResult> GetUserDelegationKey(
        const std::string& startsOn,
        const std::string& expiresOn,
        const GetUserDelegationKeyOptions& options = GetUserDelegationKeyOptions()) const;

    /**
     * @brief Sets properties for a storage account's Blob service endpoint, including
     * properties for Storage Analytics, CORS (Cross-Origin Resource Sharing) rules and soft delete
     * settings. You can also use this operation to set the default request version for all incoming
     * requests to the Blob service that do not have a version specified.
     *
     * @param
     * properties The blob service properties.
     * @param options Optional parameters to execute this function.
     * @return A SetServicePropertiesResult on successfully setting the properties.
     */
    Azure::Core::Response<SetServicePropertiesResult> SetProperties(
        BlobServiceProperties properties,
        const SetServicePropertiesOptions& options = SetServicePropertiesOptions()) const;

    /**
     * @brief Gets the properties of a storage account's blob service, including properties
     * for Storage Analytics and CORS (Cross-Origin Resource Sharing) rules.
     *
     * @param options Optional parameters to execute this function.
     * @return A GetServicePropertiesResult describing the service properties.
     */
    Azure::Core::Response<GetServicePropertiesResult> GetProperties(
        const GetServicePropertiesOptions& options = GetServicePropertiesOptions()) const;

    /**
     * @brief Returns the sku name and account kind for the specified account.
     *
     * @param options Optional parameters to execute this function.
     * @return GetAccountInfoResult describing the account.
     */
    Azure::Core::Response<GetAccountInfoResult> GetAccountInfo(
        const GetAccountInfoOptions& options = GetAccountInfoOptions()) const;

    /**
     * @brief Retrieves statistics related to replication for the Blob service. It is only
     * available on the secondary location endpoint when read-access geo-redundant replication is
     * enabled for the storage account.
     *
     * @param options Optional parameters to execute this function.
     * @return A BlobServiceStatistics describing the service replication statistics.
     */
    Azure::Core::Response<GetServiceStatisticsResult> GetStatistics(
        const GetBlobServiceStatisticsOptions& options = GetBlobServiceStatisticsOptions()) const;

    /**
     * @brief The Filter Blobs operation enables callers to list blobs across all containers
     * whose tags match a given search expression. Filter blobs searches across all containers
     * within a storage account but can be scoped within the expression to a single container.
     *
     * @param tagFilterSqlExpression The where parameter enables the caller to query blobs
     * whose tags match a given expression. The given expression must evaluate to true for a blob to
     * be returned in the results. The[OData - ABNF] filter syntax rule defines the formal grammar
     * for the value of the where query parameter, however, only a subset of the OData filter syntax
     * is supported in the Blob service.
     * @param options Optional parameters to execute this
     * function.
     * @return A FilterBlobSegment describing the blobs.
     */
    Azure::Core::Response<FindBlobsByTagsResult> FindBlobsByTags(
        const std::string& tagFilterSqlExpression,
        const FindBlobsByTagsOptions& options = FindBlobsByTagsOptions()) const;

  protected:
    Azure::Core::Http::Url m_serviceUrl;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

  private:
    friend class BlobBatchClient;
  };
}}} // namespace Azure::Storage::Blobs
