// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/credentials/credentials.hpp"
#include "azure/storage/blobs/blob_client.hpp"
#include "azure/storage/blobs/blob_options.hpp"
#include "azure/storage/blobs/protocol/blob_rest_client.hpp"
#include "azure/storage/common/storage_credential.hpp"

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
     * @param credential The client secret credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit BlobContainerClient(
        const std::string& containerUri,
        std::shared_ptr<Core::Credentials::ClientSecretCredential> credential,
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
    std::string GetUri() const { return m_containerUrl.GetAbsoluteUrl(); }

    /**
     * @brief Creates a new container under the specified account. If the container with the
     * same name already exists, the operation fails.
     *
     * @param options Optional
     * parameters to execute this function.
     * @return A CreateContainerResult describing the newly created blob container.
     */
    Azure::Core::Response<CreateContainerResult> Create(
        const CreateContainerOptions& options = CreateContainerOptions()) const;

    /**
     * @brief Marks the specified container for deletion. The container and any blobs
     * contained within it are later deleted during garbage collection.
     *
     * @param
     * options Optional parameters to execute this function.
     * @return A DeleteContainerResult if successful.
     */
    Azure::Core::Response<DeleteContainerResult> Delete(
        const DeleteContainerOptions& options = DeleteContainerOptions()) const;

    /**
     * @brief Restores a previously deleted container. The destionation is referenced by current
     * BlobContainerClient.
     *
     * @param deletedContainerName The name of the previously deleted container.
     * @param deletedContainerVersion The version of the previously deleted container.
     * @param options Optional parameters to execute this function.
     * @return An UndeleteContainerResult if successful.
     */
    Azure::Core::Response<UndeleteContainerResult> Undelete(
        const std::string& deletedContainerName,
        const std::string& deletedContainerVersion,
        const UndeleteContainerOptions& options = UndeleteContainerOptions()) const;

    /**
     * @brief Returns all user-defined metadata and system properties for the specified
     * container. The data returned does not include the container's list of blobs.
     *
     * @param options Optional parameters to execute this function.
     * @return A GetContainerPropertiesResult describing the container and its properties.
     */
    Azure::Core::Response<GetContainerPropertiesResult> GetProperties(
        const GetContainerPropertiesOptions& options = GetContainerPropertiesOptions()) const;

    /**
     * @brief Sets one or more user-defined name-value pairs for the specified container.
     *
     * @param metadata Custom metadata to set for this container.
     * @param options
     * Optional parameters to execute this function.
     * @return A SetContainerMetadataResult if successful.
     */
    Azure::Core::Response<SetContainerMetadataResult> SetMetadata(
        std::map<std::string, std::string> metadata,
        SetContainerMetadataOptions options = SetContainerMetadataOptions()) const;

    /**
     * @brief Returns a single segment of blobs in this container, starting from the
     * specified Marker, Use an empty Marker to start enumeration from the beginning and the
     * NextMarker if it's not empty to make subsequent calls to ListBlobsFlatSegment to continue
     * enumerating the blobs segment by segment. Blobs are ordered lexicographically by name.
     *
     * @param options Optional parameters to execute this function.
     * @return A ListBlobsFlatSegmentResult describing a segment of the blobs in the container.
     */
    Azure::Core::Response<ListBlobsFlatSegmentResult> ListBlobsFlatSegment(
        const ListBlobsSegmentOptions& options = ListBlobsSegmentOptions()) const;

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
     * @return A ListBlobsByHierarchySegmentResult describing a segment of the blobs in the
     * container.
     */
    Azure::Core::Response<ListBlobsByHierarchySegmentResult> ListBlobsByHierarchySegment(
        const std::string& delimiter,
        const ListBlobsSegmentOptions& options = ListBlobsSegmentOptions()) const;

    /**
     * @brief Gets the permissions for this container. The permissions indicate whether
     * container data may be accessed publicly.
     *
     * @param options Optional parameters to
     * execute this function.
     * @return A GetContainerAccessPolicyResult describing the container's access policy.
     */
    Azure::Core::Response<GetContainerAccessPolicyResult> GetAccessPolicy(
        const GetContainerAccessPolicyOptions& options = GetContainerAccessPolicyOptions()) const;

    /**
     * @brief Sets the permissions for the specified container. The permissions indicate
     * whether blob container data may be accessed publicly.
     *
     * @param options Optional
     * parameters to execute this function.
     * @return A SetContainerAccessPolicyResult describing the updated container.
     */
    Azure::Core::Response<SetContainerAccessPolicyResult> SetAccessPolicy(
        const SetContainerAccessPolicyOptions& options = SetContainerAccessPolicyOptions()) const;

    /**
     * @brief Acquires a lease on the container.
     *
     * @param proposedLeaseId
     * Proposed lease ID, in a GUID string format.
     * @param duration Specifies the duration of
     * the lease, in seconds, or Azure::Storage::c_InfiniteLeaseDuration for a lease that never
     * expires. A non-infinite lease can be between 15 and 60 seconds. A lease duration cannot be
     * changed using renew or change.
     * @param options Optional parameters to execute this
     * function.
     * @return A AcquireContainerLeaseResult describing the lease.
     */
    Azure::Core::Response<AcquireContainerLeaseResult> AcquireLease(
        const std::string& proposedLeaseId,
        int32_t duration,
        const AcquireContainerLeaseOptions& options = AcquireContainerLeaseOptions()) const;

    /**
     * @brief Renews the container's previously-acquired lease.
     *
     * @param
     * leaseId ID of the previously-acquired lease.
     * @param options Optional parameters to
     * execute this function.
     * @return A RenewContainerLeaseResult describing the lease.
     */
    Azure::Core::Response<RenewContainerLeaseResult> RenewLease(
        const std::string& leaseId,
        const RenewContainerLeaseOptions& options = RenewContainerLeaseOptions()) const;

    /**
     * @brief Releases the container's previously-acquired lease.
     *
     * @param
     * leaseId ID of the previously-acquired lease.
     * @param options Optional parameters to
     * execute this function.
     * @return A ReleaseContainerLeaseResult describing the updated container.
     */
    Azure::Core::Response<ReleaseContainerLeaseResult> ReleaseLease(
        const std::string& leaseId,
        const ReleaseContainerLeaseOptions& options = ReleaseContainerLeaseOptions()) const;

    /**
     * @brief Changes the lease of an active lease.
     *
     * @param leaseId ID of the
     * previously-acquired lease.
     * @param proposedLeaseId Proposed lease ID, in a GUID string
     * format.
     * @param options Optional parameters to execute this function.
     * @return A ChangeContainerLeaseResult describing the lease.
     */
    Azure::Core::Response<ChangeContainerLeaseResult> ChangeLease(
        const std::string& leaseId,
        const std::string& proposedLeaseId,
        const ChangeContainerLeaseOptions& options = ChangeContainerLeaseOptions()) const;

    /**
     * @brief Breaks the previously-acquired lease.
     *
     * @param options Optional
     * parameters to execute this function.
     * @return A BreakContainerLeaseResult describing the broken lease.
     */
    Azure::Core::Response<BreakContainerLeaseResult> BreakLease(
        const BreakContainerLeaseOptions& options = BreakContainerLeaseOptions()) const;

  protected:
    Azure::Core::Http::Url m_containerUrl;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;
    Azure::Core::Nullable<EncryptionKey> m_customerProvidedKey;
    Azure::Core::Nullable<std::string> m_encryptionScope;

  private:
    explicit BlobContainerClient(
        Azure::Core::Http::Url containerUri,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : m_containerUrl(std::move(containerUri)), m_pipeline(std::move(pipeline))
    {
    }

    friend class BlobServiceClient;
  };

}}} // namespace Azure::Storage::Blobs
