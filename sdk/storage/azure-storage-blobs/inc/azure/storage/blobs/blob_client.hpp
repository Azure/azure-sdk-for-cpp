// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include <azure/core/credentials.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/blobs/blob_options.hpp"
#include "azure/storage/blobs/blob_responses.hpp"
#include "azure/storage/blobs/protocol/blob_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {
  class DataLakeDirectoryClient;
  class DataLakeFileClient;
}}}} // namespace Azure::Storage::Files::DataLake

namespace Azure { namespace Storage { namespace Blobs {

  class BlockBlobClient;
  class AppendBlobClient;
  class PageBlobClient;

  /**
   * @brief The BlobClient allows you to manipulate Azure Storage blobs.
   */
  class BlobClient {
  public:
    /**
     * @brief Initialize a new instance of BlobClient.
     *
     * @param connectionString A connection string includes the authentication information required
     * for your application to access data in an Azure Storage account at runtime.
     * @param blobContainerName The name of the container containing this blob.
     * @param blobName The name of this blob.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     * @return A new BlobClient instance.
     */
    static BlobClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& blobContainerName,
        const std::string& blobName,
        const BlobClientOptions& options = BlobClientOptions());

    /**
     * @brief Initialize a new instance of BlobClient.
     *
     * @param blobUrl A url referencing the blob that includes the name of the account, the name of
     * the container, and the name of the blob.
     * @param credential The shared key credential used to sign requests.
     * @param options Optional client options that define the transport pipeline
     * policies for authentication, retries, etc., that are applied to every request.
     */
    explicit BlobClient(
        const std::string& blobUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const BlobClientOptions& options = BlobClientOptions());

    /**
     * @brief Initialize a new instance of BlobClient.
     *
     * @param blobUrl A url referencing the blob that includes the name of the account, the name of
     * the container, and the name of the blob.
     * @param credential The token credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit BlobClient(
        const std::string& blobUrl,
        std::shared_ptr<Core::TokenCredential> credential,
        const BlobClientOptions& options = BlobClientOptions());

    /**
     * @brief Initialize a new instance of BlobClient.
     *
     * @param blobUrl A url referencing the blob that includes the name of the account, the name of
     * the container, and the name of the blob, and possibly also a SAS token.
     * @param options Optional client
     * options that define the transport pipeline policies for authentication, retries, etc., that
     * are applied to every request.
     */
    explicit BlobClient(
        const std::string& blobUrl,
        const BlobClientOptions& options = BlobClientOptions());

    /**
     * @brief Creates a new BlockBlobClient object with the same url as this BlobClient. The
     * new BlockBlobClient uses the same request policy pipeline as this BlobClient.
     *
     *
     * @return A new BlockBlobClient instance.
     */
    BlockBlobClient AsBlockBlobClient() const;

    /**
     * @brief Creates a new AppendBlobClient object with the same url as this BlobClient.
     * The new AppendBlobClient uses the same request policy pipeline as this BlobClient.
     *
     * @return A new AppendBlobClient instance.
     */
    AppendBlobClient AsAppendBlobClient() const;

    /**
     * @brief Creates a new PageBlobClient object with the same url as this BlobClient.
     * The new PageBlobClient uses the same request policy pipeline as this BlobClient.
     *
     * @return A new PageBlobClient instance.
     */
    PageBlobClient AsPageBlobClient() const;

    /**
     * @brief Gets the blob's primary url endpoint.
     *
     * @return The blob's primary url endpoint.
     */
    std::string GetUrl() const { return m_blobUrl.GetAbsoluteUrl(); }

    /**
     * @brief Initializes a new instance of the BlobClient class with an identical url
     * source but the specified snapshot timestamp.
     *
     * @param snapshot The snapshot identifier.
     * @return A new BlobClient instance.
     * @remarks Pass empty string to remove the snapshot returning the base blob.
     */
    BlobClient WithSnapshot(const std::string& snapshot) const;

    /**
     * @brief Creates a clone of this instance that references a version ID rather than the base
     * blob.
     *
     * @param versionId The version ID returning a URL to the base blob.
     * @return A new BlobClient instance.
     * @remarks Pass empty string to remove the version ID returning the base blob.
     */
    BlobClient WithVersionId(const std::string& versionId) const;

    /**
     * @brief Returns all user-defined metadata, standard HTTP properties, and system
     * properties for the blob. It does not return the content of the blob.
     *
     * @param options Optional parameters to execute this function.
     * @return A GetBlobPropertiesResult describing the blob's properties.
     */
    Azure::Core::Response<Models::GetBlobPropertiesResult> GetProperties(
        const GetBlobPropertiesOptions& options = GetBlobPropertiesOptions()) const;

    /**
     * @brief Sets system properties on the blob.
     *
     * @param httpHeaders The standard HTTP header system properties to set.
     * @param options Optional parameters to execute this function.
     * @return A SetBlobHttpHeadersResult describing the updated blob.
     */
    Azure::Core::Response<Models::SetBlobHttpHeadersResult> SetHttpHeaders(
        Models::BlobHttpHeaders httpHeaders,
        const SetBlobHttpHeadersOptions& options = SetBlobHttpHeadersOptions()) const;

    /**
     * @brief Sets user-defined metadata for the specified blob as one or more name-value
     * pairs.
     *
     * @param metadata Custom metadata to set for this blob.
     * @param
     * options Optional parameters to execute this function.
     * @return A SetBlobMetadataResult describing the updated blob.
     */
    Azure::Core::Response<Models::SetBlobMetadataResult> SetMetadata(
        Metadata metadata,
        const SetBlobMetadataOptions& options = SetBlobMetadataOptions()) const;

    /**
     * @brief Sets the tier on a blob. The operation is allowed on a page blob in a premium
     * storage account and on a block blob in a blob storage or general purpose v2 account.
     *
     * @param tier Indicates the tier to be set on the blob.
     * @param options Optional
     * parameters to execute this function.
     * @return A SetBlobAccessTierResult on successfully setting the tier.
     */
    Azure::Core::Response<Models::SetBlobAccessTierResult> SetAccessTier(
        Models::AccessTier tier,
        const SetBlobAccessTierOptions& options = SetBlobAccessTierOptions()) const;

    /**
     * @brief Copies data at from the source to this blob.
     *
     * @param sourceUri
     * Specifies the uri of the source blob. The value may a uri of up to 2 KB in length that
     * specifies a blob. A source blob in the same storage account can be authenticated via Shared
     * Key. However, if the source is a blob in another account, the source blob must either be
     * public or must be authenticated via a shared access signature. If the source blob is public,
     * no authentication is required to perform the copy operation.
     * @param options Optional parameters to execute this function.
     * @return A StartCopyBlobFromUriResult describing the state of the copy operation.
     */
    Azure::Core::Response<Models::StartCopyBlobFromUriResult> StartCopyFromUri(
        const std::string& sourceUri,
        const StartCopyBlobFromUriOptions& options = StartCopyBlobFromUriOptions()) const;

    /**
     * @brief Aborts a pending StartCopyFromUri operation, and leaves this blob with zero
     * length and full metadata.
     *
     * @param copyId ID of the copy operation to abort.
     * @param options Optional parameters to execute this function.
     * @return A AbortCopyBlobFromUriResult on successfully aborting.
     */
    Azure::Core::Response<Models::AbortCopyBlobFromUriResult> AbortCopyFromUri(
        const std::string& copyId,
        const AbortCopyBlobFromUriOptions& options = AbortCopyBlobFromUriOptions()) const;

    /**
     * @brief Downloads a blob or a blob range from the service, including its metadata and
     * properties.
     *
     * @param options Optional parameters to execute this function.
     * @return A DownloadBlobResult describing the downloaded blob.
     * BlobDownloadResponse.BodyStream contains the blob's data.
     */
    Azure::Core::Response<Models::DownloadBlobResult> Download(
        const DownloadBlobOptions& options = DownloadBlobOptions()) const;

    /**
     * @brief Downloads a blob or a blob range from the service to a memory buffer using parallel
     * requests.
     *
     * @param buffer A memory buffer to write the blob content to.
     * @param bufferSize Size of the memory buffer. Size must be larger or equal to size of the blob
     * or blob range.
     * @param options Optional parameters to execute this function.
     * @return A DownloadBlobToResult describing the downloaded blob.
     */
    Azure::Core::Response<Models::DownloadBlobToResult> DownloadTo(
        uint8_t* buffer,
        std::size_t bufferSize,
        const DownloadBlobToOptions& options = DownloadBlobToOptions()) const;

    /**
     * @brief Downloads a blob or a blob range from the service to a file using parallel
     * requests.
     *
     * @param fileName A file path to write the downloaded content to.
     * @param options Optional parameters to execute this function.
     * @return A DownloadBlobToResult describing the downloaded blob.
     */
    Azure::Core::Response<Models::DownloadBlobToResult> DownloadTo(
        const std::string& fileName,
        const DownloadBlobToOptions& options = DownloadBlobToOptions()) const;

    /**
     * @brief Creates a read-only snapshot of a blob.
     *
     * @param options Optional parameters to execute this function.
     * @return A CreateBlobSnapshotResult describing the new blob snapshot.
     */
    Azure::Core::Response<Models::CreateBlobSnapshotResult> CreateSnapshot(
        const CreateBlobSnapshotOptions& options = CreateBlobSnapshotOptions()) const;

    /**
     * @brief Marks the specified blob or snapshot for deletion. The blob is later deleted
     * during garbage collection. Note that in order to delete a blob, you must delete all of its
     * snapshots. You can delete both at the same time using DeleteBlobOptions.DeleteSnapshots.
     *
     * @param options Optional parameters to execute this function.
     * @return A DeleteBlobResult on successfully deleting.
     */
    Azure::Core::Response<Models::DeleteBlobResult> Delete(
        const DeleteBlobOptions& options = DeleteBlobOptions()) const;

    /**
     * @brief Marks the specified blob or snapshot for deletion if it exists.
     *
     * @param options Optional parameters to execute this function.
     * @return A DeleteBlobResult on successfully deleting. DeleteBlobResult.Deleted is false if the
     * blob doesn't exist.
     */
    Azure::Core::Response<Models::DeleteBlobResult> DeleteIfExists(
        const DeleteBlobOptions& options = DeleteBlobOptions()) const;

    /**
     * @brief Restores the contents and metadata of a soft deleted blob and any associated
     * soft deleted snapshots.
     *
     * @param options Optional parameters to execute this function.
     * @return A UndeleteBlobResult on successfully deleting.
     */
    Azure::Core::Response<Models::UndeleteBlobResult> Undelete(
        const UndeleteBlobOptions& options = UndeleteBlobOptions()) const;

    /**
     * @brief Acquires a lease on the blob.
     *
     * @param proposedLeaseId Proposed lease ID, in a GUID string format.
     * @param duration Specifies the duration of
     * the lease, in seconds, or Azure::Storage::InfiniteLeaseDuration for a lease that never
     * expires. A non-infinite lease can be between 15 and 60 seconds. A lease duration cannot be
     * changed using renew or change.
     * @param options Optional parameters to execute this function.
     * @return A AcquireBlobLeaseResult describing the lease.
     */
    Azure::Core::Response<Models::AcquireBlobLeaseResult> AcquireLease(
        const std::string& proposedLeaseId,
        int32_t duration,
        const AcquireBlobLeaseOptions& options = AcquireBlobLeaseOptions()) const;

    /**
     * @brief Renews the blob's previously-acquired lease.
     *
     * @param leaseId ID of the previously-acquired lease.
     * @param options Optional parameters to execute this function.
     * @return A RenewBlobLeaseResult describing the lease.
     */
    Azure::Core::Response<Models::RenewBlobLeaseResult> RenewLease(
        const std::string& leaseId,
        const RenewBlobLeaseOptions& options = RenewBlobLeaseOptions()) const;

    /**
     * @brief Releases the blob's previously-acquired lease.
     *
     * @param leaseId ID of the previously-acquired lease.
     * @param options Optional parameters to execute this function.
     * @return A ReleaseBlobLeaseResult describing the updated container.
     */
    Azure::Core::Response<Models::ReleaseBlobLeaseResult> ReleaseLease(
        const std::string& leaseId,
        const ReleaseBlobLeaseOptions& options = ReleaseBlobLeaseOptions()) const;

    /**
     * @brief Changes the lease of an active lease.
     *
     * @param leaseId ID of the
     * previously-acquired lease.
     * @param proposedLeaseId Proposed lease ID, in a GUID string format.
     * @param options Optional parameters to execute this function.
     * @return A ChangeBlobLeaseResult describing the lease.
     */
    Azure::Core::Response<Models::ChangeBlobLeaseResult> ChangeLease(
        const std::string& leaseId,
        const std::string& proposedLeaseId,
        const ChangeBlobLeaseOptions& options = ChangeBlobLeaseOptions()) const;

    /**
     * @brief Breaks the previously-acquired lease.
     *
     * @param options Optional parameters to execute this function.
     * @return A BreakBlobLeaseResult describing the broken lease.
     */
    Azure::Core::Response<Models::BreakBlobLeaseResult> BreakLease(
        const BreakBlobLeaseOptions& options = BreakBlobLeaseOptions()) const;

    /**
     * @brief Sets tags on the underlying blob.
     *
     * @param tags The tags to set on the blob.
     * @param options Optional parameters to execute this function.
     * @return A SetBlobTagsInfo on successfully setting tags.
     */
    Azure::Core::Response<Models::SetBlobTagsResult> SetTags(
        std::map<std::string, std::string> tags,
        const SetBlobTagsOptions& options = SetBlobTagsOptions()) const;

    /**
     * @brief Gets the tags associated with the underlying blob.
     *
     * @param options Optional parameters to execute this function.
     * @return Tags on successfully getting tags.
     */
    Azure::Core::Response<Models::GetBlobTagsResult> GetTags(
        const GetBlobTagsOptions& options = GetBlobTagsOptions()) const;

  protected:
    Azure::Core::Http::Url m_blobUrl;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;
    Azure::Core::Nullable<EncryptionKey> m_customerProvidedKey;
    Azure::Core::Nullable<std::string> m_encryptionScope;

  private:
    explicit BlobClient(
        Azure::Core::Http::Url blobUrl,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline,
        Azure::Core::Nullable<EncryptionKey> customerProvidedKey,
        Azure::Core::Nullable<std::string> encryptionScope)
        : m_blobUrl(std::move(blobUrl)), m_pipeline(std::move(pipeline)),
          m_customerProvidedKey(std::move(customerProvidedKey)),
          m_encryptionScope(std::move(encryptionScope))
    {
    }

    friend class BlobContainerClient;
    friend class Files::DataLake::DataLakeDirectoryClient;
    friend class Files::DataLake::DataLakeFileClient;
  };
}}} // namespace Azure::Storage::Blobs
