// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <string>

#include "azure/storage/files/shares/share_client.hpp"
#include "azure/storage/files/shares/share_file_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  /**
   * @brief ShareLeaseClient allows you to manipulate Azure Storage leases on shares and files.
   */
  class ShareLeaseClient {
  public:
    /**
     * @brief Initializes a new instance of the ShareLeaseClient.
     *
     * @param fileClient A ShareFileClient representing the file being leased.
     * @param leaseId A lease ID. This is not required for break operation.
     */
    explicit ShareLeaseClient(ShareFileClient fileClient, std::string leaseId)
        : m_fileClient(std::move(fileClient)), m_leaseId(std::move(leaseId))
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
    AZ_STORAGE_FILES_SHARES_DLLEXPORT static const std::chrono::seconds InfiniteLeaseDuration;

    /**
     * @brief Get lease id of this lease client.
     *
     * @return Lease id of this lease client.
     */
    const std::string& GetLeaseId() const { return m_leaseId; }

    /**
     * @brief Acquires a lease on the file or share.
     *
     * @param duration Specifies the duration of the lease, in seconds, or InfiniteLeaseDuration for
     * a lease that never expires. A non-infinite lease can be between 15 and 60 seconds. A lease
     * duration cannot be changed using renew or change.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A AcquireShareLeaseResult describing the lease.
     */
    Azure::Core::Response<Models::AcquireShareLeaseResult> Acquire(
        std::chrono::seconds duration,
        const AcquireShareLeaseOptions& options = AcquireShareLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Releases the file or share's previously-acquired lease.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A ReleaseShareLeaseResult describing the updated share or file.
     */
    Azure::Core::Response<Models::ReleaseShareLeaseResult> Release(
        const ReleaseShareLeaseOptions& options = ReleaseShareLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Changes the lease of an active lease.
     *
     * @param proposedLeaseId Proposed lease ID, in a GUID string format.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A ChangeShareLeaseResult describing the updated lease.
     * @remarks The current ShareLeaseClient becomes invalid if this operation succeeds.
     */
    Azure::Core::Response<Models::ChangeShareLeaseResult> Change(
        const std::string& proposedLeaseId,
        const ChangeShareLeaseOptions& options = ChangeShareLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Breaks the previously-acquired lease.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A BreakShareLeaseResult describing the broken lease.
     */
    Azure::Core::Response<Models::BreakShareLeaseResult> Break(
        const BreakShareLeaseOptions& options = BreakShareLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

  private:
    /**
     * @brief Initializes a new instance of the ShareLeaseClient.
     *
     * @param shareClient A ShareClient representing the share being leased.
     * @param leaseId A lease ID. This is not required for break operation.
     */
    explicit ShareLeaseClient(ShareClient shareClient, std::string leaseId)
        : m_shareClient(std::move(shareClient)), m_leaseId(std::move(leaseId))
    {
    }

    /**
     * @brief Renews the file or share's previously-acquired lease.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A RenewShareLeaseResult describing the lease.
     */
    Azure::Core::Response<Models::RenewShareLeaseResult> Renew(
        const RenewShareLeaseOptions& options = RenewShareLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    Azure::Core::Nullable<ShareFileClient> m_fileClient;
    Azure::Core::Nullable<ShareClient> m_shareClient;
    std::string m_leaseId;
  };

}}}} // namespace Azure::Storage::Files::Shares
