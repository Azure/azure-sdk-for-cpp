// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>

#include "azure/storage/blobs/blob_client.hpp"
#include "azure/storage/blobs/blob_container_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  /**
   * @brief BlobLeaseClient allows you to manipulate Azure Storage leases on containers and blobs.
   */
  class BlobLeaseClient {
  public:
    /**
     * @brief Initializes a new instance of the BlobLeaseClient.
     *
     * @param blobClient A BlobClient representing the blob being leased.
     * @param leaseId A lease ID. This is not required for break operation.
     */
    explicit BlobLeaseClient(BlobClient blobClient, std::string leaseId)
        : m_blobClient(std::move(blobClient)), m_leaseId(std::move(leaseId))
    {
    }

    /**
     * @brief Initializes a new instance of the BlobLeaseClient.
     *
     * @param blobContainerClient A BlobContainerClient representing the blob container being
     * leased.
     * @param leaseId A lease ID. This is not required for break operation.
     */
    explicit BlobLeaseClient(BlobContainerClient blobContainerClient, std::string leaseId)
        : m_blobContainerClient(std::move(blobContainerClient)), m_leaseId(std::move(leaseId))
    {
    }

    /**
     * @brief Gets a unique lease ID.
     *
     * @return A unique lease ID.
     */
    static std::string CreateUniqueLeaseId();

    /**
     * @brief A value representing infinite lease duration.
     */
    AZ_STORAGE_BLOBS_DLLEXPORT const static std::chrono::seconds InfiniteLeaseDuration;

    /**
     * @brief Get lease id of this lease client.
     *
     * @return Lease id of this lease client.
     */
    const std::string& GetLeaseId() const { return m_leaseId; }

    /**
     * @brief Acquires a lease on the blob or blob container.
     *
     * @param duration Specifies the duration of
     * the lease, in seconds, or InfiniteLeaseDuration for a lease that never
     * expires. A non-infinite lease can be between 15 and 60 seconds. A lease duration cannot be
     * changed using renew or change.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A AcquireBlobLeaseResult describing the lease.
     */
    Azure::Core::Response<Models::AcquireBlobLeaseResult> Acquire(
        std::chrono::seconds duration,
        const AcquireBlobLeaseOptions& options = AcquireBlobLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Renews the blob or blob container's previously-acquired lease.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A RenewBlobLeaseResult describing the lease.
     */
    Azure::Core::Response<Models::RenewBlobLeaseResult> Renew(
        const RenewBlobLeaseOptions& options = RenewBlobLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Releases the blob or blob container's previously-acquired lease.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A ReleaseBlobLeaseResult describing the updated container or blob.
     */
    Azure::Core::Response<Models::ReleaseBlobLeaseResult> Release(
        const ReleaseBlobLeaseOptions& options = ReleaseBlobLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Changes the lease of an active lease.
     *
     * @param proposedLeaseId Proposed lease ID, in a GUID string format.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A ChangeBlobLeaseResult describing the changed lease.
     * @remarks The current BlobLeaseClient becomes invalid if this operation succeeds.
     */
    Azure::Core::Response<Models::ChangeBlobLeaseResult> Change(
        const std::string& proposedLeaseId,
        const ChangeBlobLeaseOptions& options = ChangeBlobLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Breaks the previously-acquired lease.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A BreakBlobLeaseResult describing the broken lease.
     */
    Azure::Core::Response<Models::BreakBlobLeaseResult> Break(
        const BreakBlobLeaseOptions& options = BreakBlobLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

  private:
    Azure::Core::Nullable<BlobClient> m_blobClient;
    Azure::Core::Nullable<BlobContainerClient> m_blobContainerClient;
    std::string m_leaseId;
  };

}}} // namespace Azure::Storage::Blobs
