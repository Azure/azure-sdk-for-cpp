// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "azure/storage/blobs/blob_client.hpp"

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
     * @param blobContainerName The name of the container containing this blob.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     * @return A new BlobContainerClient instance.
     */
    static BlobContainerClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& blobContainerName,
        const BlobClientOptions& options = BlobClientOptions());

    /**
     * @brief Initialize a new instance of BlobContainerClient.
     *
     * @param blobContainerUrl A url
     * referencing the blob container that includes the name of the account and the name of the
     * container.
     * @param credential The shared key credential used to sign
     * requests.
     * @param options Optional client options that define the transport pipeline
     * policies for authentication, retries, etc., that are applied to every request.
     */
    explicit BlobContainerClient(
        const std::string& blobContainerUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const BlobClientOptions& options = BlobClientOptions());

    /**
     * @brief Initialize a new instance of BlobContainerClient.
     *
     * @param blobContainerUrl A url
     * referencing the blob container that includes the name of the account and the name of the
     * container.
     * @param credential The token credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit BlobContainerClient(
        const std::string& blobContainerUrl,
        std::shared_ptr<Core::TokenCredential> credential,
        const BlobClientOptions& options = BlobClientOptions());

    /**
     * @brief Initialize a new instance of BlobContainerClient.
     *
     * @param blobContainerUrl A url
     * referencing the blob that includes the name of the account and the name of the container, and
     * possibly also a SAS token.
     * @param options Optional client
     * options that define the transport pipeline policies for authentication, retries, etc., that
     * are applied to every request.
     */
    explicit BlobContainerClient(
        const std::string& blobContainerUrl,
        const BlobClientOptions& options = BlobClientOptions());

    /**
     * @brief Create a new BlobClient object by appending blobName to the end of url. The
     * new BlobClient uses the same request policy pipeline as this BlobContainerClient.
     *
     * @param blobName The name of the blob.
     * @return A new BlobClient instance.
     */
    BlobClient GetBlobClient(const std::string& blobName) const;

    /**
     * @brief Create a new BlockBlobClient object by appending blobName to the end of url.
     * The new BlockBlobClient uses the same request policy pipeline as this BlobContainerClient.
     *
     * @param blobName The name of the blob.
     * @return A new BlockBlobClient instance.
     */
    BlockBlobClient GetBlockBlobClient(const std::string& blobName) const;

    /**
     * @brief Create a new AppendBlobClient object by appending blobName to the end of url.
     * The new AppendBlobClient uses the same request policy pipeline as this BlobContainerClient.
     *
     * @param blobName The name of the blob.
     * @return A new AppendBlobClient instance.
     */
    AppendBlobClient GetAppendBlobClient(const std::string& blobName) const;

    /**
     * @brief Create a new PageBlobClient object by appending blobName to the end of url.
     * The new PageBlobClient uses the same request policy pipeline as this BlobContainerClient.
     *
     * @param blobName The name of the blob.
     * @return A new PageBlobClient instance.
     */
    PageBlobClient GetPageBlobClient(const std::string& blobName) const;

    /**
     * @brief Gets the container's primary url endpoint.
     *
     * @return The
     * container's primary url endpoint.
     */
    std::string GetUrl() const { return m_blobContainerUrl.GetAbsoluteUrl(); }

    /**
     * @brief Creates a new container under the specified account. If the container with the
     * same name already exists, the operation fails.
     *
     * @param options Optional parameters to execute this function.
     * @return A CreateBlobContainerResult describing the newly created blob container.
     */
    Azure::Core::Response<Models::CreateBlobContainerResult> Create(
        const CreateBlobContainerOptions& options = CreateBlobContainerOptions()) const;

    /**
     * @brief Creates a new container under the specified account. If the container with the
     * same name already exists, it is not changed.
     *
     * @param options Optional parameters to execute this function.
     * @return A CreateBlobContainerResult describing the newly created blob container if the
     * container doesn't exist. CreateBlobContainerResult.Created is false if the container already
     * exists.
     */
    Azure::Core::Response<Models::CreateBlobContainerResult> CreateIfNotExists(
        const CreateBlobContainerOptions& options = CreateBlobContainerOptions()) const;

    /**
     * @brief Marks the specified container for deletion. The container and any blobs
     * contained within it are later deleted during garbage collection.
     *
     * @param options Optional parameters to execute this function.
     * @return A DeleteBlobContainerResult if successful.
     */
    Azure::Core::Response<Models::DeleteBlobContainerResult> Delete(
        const DeleteBlobContainerOptions& options = DeleteBlobContainerOptions()) const;

    /**
     * @brief Marks the specified container for deletion if it exists. The container and any blobs
     * contained within it are later deleted during garbage collection.
     *
     * @param options Optional parameters to execute this function.
     * @return A DeleteBlobContainerResult if the container exists.
     * DeleteBlobContainerResult.Deleted is false if the container doesn't exist.
     */
    Azure::Core::Response<Models::DeleteBlobContainerResult> DeleteIfExists(
        const DeleteBlobContainerOptions& options = DeleteBlobContainerOptions()) const;

    /**
     * @brief Returns all user-defined metadata and system properties for the specified
     * container. The data returned does not include the container's list of blobs.
     *
     * @param options Optional parameters to execute this function.
     * @return A GetBlobContainerPropertiesResult describing the container and its properties.
     */
    Azure::Core::Response<Models::GetBlobContainerPropertiesResult> GetProperties(
        const GetBlobContainerPropertiesOptions& options
        = GetBlobContainerPropertiesOptions()) const;

    /**
     * @brief Sets one or more user-defined name-value pairs for the specified container.
     *
     * @param metadata Custom metadata to set for this container.
     * @param options
     * Optional parameters to execute this function.
     * @return A SetBlobContainerMetadataResult if successful.
     */
    Azure::Core::Response<Models::SetBlobContainerMetadataResult> SetMetadata(
        Metadata metadata,
        SetBlobContainerMetadataOptions options = SetBlobContainerMetadataOptions()) const;

    /**
     * @brief Returns a single segment of blobs in this container, starting from the
     * specified Marker, Use an empty Marker to start enumeration from the beginning and the
     * NextMarker if it's not empty to make subsequent calls to ListBlobsFlatSegment to continue
     * enumerating the blobs segment by segment. Blobs are ordered lexicographically by name.
     *
     * @param options Optional parameters to execute this function.
     * @return A ListBlobsSinglePageResult describing a segment of the blobs in the container.
     */
    Azure::Core::Response<Models::ListBlobsSinglePageResult> ListBlobsSinglePage(
        const ListBlobsSinglePageOptions& options = ListBlobsSinglePageOptions()) const;

    /**
     * @brief Returns a single segment of blobs in this container, starting from the
     * specified Marker, Use an empty Marker to start enumeration from the beginning and the
     * NextMarker if it's not empty to make subsequent calls to ListBlobsByHierarchySegment to
     * continue enumerating the blobs segment by segment. Blobs are ordered lexicographically by
     * name. A Delimiter can be used to traverse a virtual hierarchy of blobs as though it were a
     * file system.
     *
     * @param delimiter This can be used to to traverse a virtual hierarchy of blobs as though it
     * were a file system. The delimiter may be a single character or a string.
     * @param options Optional parameters to execute this function.
     * @return A ListBlobsByHierarchySinglePageResult describing a segment of the blobs in the
     * container.
     */
    Azure::Core::Response<Models::ListBlobsByHierarchySinglePageResult>
    ListBlobsByHierarchySinglePage(
        const std::string& delimiter,
        const ListBlobsSinglePageOptions& options = ListBlobsSinglePageOptions()) const;

    /**
     * @brief Gets the permissions for this container. The permissions indicate whether
     * container data may be accessed publicly.
     *
     * @param options Optional parameters to
     * execute this function.
     * @return A GetBlobContainerAccessPolicyResult describing the container's access policy.
     */
    Azure::Core::Response<Models::GetBlobContainerAccessPolicyResult> GetAccessPolicy(
        const GetBlobContainerAccessPolicyOptions& options
        = GetBlobContainerAccessPolicyOptions()) const;

    /**
     * @brief Sets the permissions for the specified container. The permissions indicate
     * whether blob container data may be accessed publicly.
     *
     * @param options Optional
     * parameters to execute this function.
     * @return A SetBlobContainerAccessPolicyResult describing the updated container.
     */
    Azure::Core::Response<Models::SetBlobContainerAccessPolicyResult> SetAccessPolicy(
        const SetBlobContainerAccessPolicyOptions& options
        = SetBlobContainerAccessPolicyOptions()) const;

    /**
     * @brief Acquires a lease on the container.
     *
     * @param proposedLeaseId
     * Proposed lease ID, in a GUID string format.
     * @param duration Specifies the duration of
     * the lease, in seconds, or Azure::Storage::InfiniteLeaseDuration for a lease that never
     * expires. A non-infinite lease can be between 15 and 60 seconds. A lease duration cannot be
     * changed using renew or change.
     * @param options Optional parameters to execute this
     * function.
     * @return A AcquireBlobContainerLeaseResult describing the lease.
     */
    Azure::Core::Response<Models::AcquireBlobContainerLeaseResult> AcquireLease(
        const std::string& proposedLeaseId,
        int32_t duration,
        const AcquireBlobContainerLeaseOptions& options = AcquireBlobContainerLeaseOptions()) const;

    /**
     * @brief Renews the container's previously-acquired lease.
     *
     * @param
     * leaseId ID of the previously-acquired lease.
     * @param options Optional parameters to
     * execute this function.
     * @return A RenewBlobContainerLeaseResult describing the lease.
     */
    Azure::Core::Response<Models::RenewBlobContainerLeaseResult> RenewLease(
        const std::string& leaseId,
        const RenewBlobContainerLeaseOptions& options = RenewBlobContainerLeaseOptions()) const;

    /**
     * @brief Releases the container's previously-acquired lease.
     *
     * @param
     * leaseId ID of the previously-acquired lease.
     * @param options Optional parameters to
     * execute this function.
     * @return A ReleaseBlobContainerLeaseResult describing the updated container.
     */
    Azure::Core::Response<Models::ReleaseBlobContainerLeaseResult> ReleaseLease(
        const std::string& leaseId,
        const ReleaseBlobContainerLeaseOptions& options = ReleaseBlobContainerLeaseOptions()) const;

    /**
     * @brief Changes the lease of an active lease.
     *
     * @param leaseId ID of the
     * previously-acquired lease.
     * @param proposedLeaseId Proposed lease ID, in a GUID string
     * format.
     * @param options Optional parameters to execute this function.
     * @return A ChangeBlobContainerLeaseResult describing the lease.
     */
    Azure::Core::Response<Models::ChangeBlobContainerLeaseResult> ChangeLease(
        const std::string& leaseId,
        const std::string& proposedLeaseId,
        const ChangeBlobContainerLeaseOptions& options = ChangeBlobContainerLeaseOptions()) const;

    /**
     * @brief Breaks the previously-acquired lease.
     *
     * @param options Optional
     * parameters to execute this function.
     * @return A BreakBlobContainerLeaseResult describing the broken lease.
     */
    Azure::Core::Response<Models::BreakBlobContainerLeaseResult> BreakLease(
        const BreakBlobContainerLeaseOptions& options = BreakBlobContainerLeaseOptions()) const;

    /**
     * @brief Marks the specified blob or snapshot for deletion. The blob is later deleted
     * during garbage collection. Note that in order to delete a blob, you must delete all of its
     * snapshots. You can delete both at the same time using DeleteBlobOptions.DeleteSnapshots.
     *
     * @param blobName The name of the blob to delete.
     * @param options Optional parameters to execute this function.
     * @return Nothing.
     */
    Azure::Core::Response<void> DeleteBlob(
        const std::string& blobName,
        const DeleteBlobOptions& options = DeleteBlobOptions()) const;

  private:
    Azure::Core::Http::Url m_blobContainerUrl;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;
    Azure::Core::Nullable<EncryptionKey> m_customerProvidedKey;
    Azure::Core::Nullable<std::string> m_encryptionScope;

    explicit BlobContainerClient(
        Azure::Core::Http::Url blobContainerUrl,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline,
        Azure::Core::Nullable<EncryptionKey> customerProvidedKey,
        Azure::Core::Nullable<std::string> encryptionScope)
        : m_blobContainerUrl(std::move(blobContainerUrl)), m_pipeline(std::move(pipeline)),
          m_customerProvidedKey(std::move(customerProvidedKey)),
          m_encryptionScope(std::move(encryptionScope))
    {
    }

    friend class BlobServiceClient;
  };

}}} // namespace Azure::Storage::Blobs
