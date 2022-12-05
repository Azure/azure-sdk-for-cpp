// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>
#include <mutex>
#include <string>

#include "azure/storage/files/shares/share_client.hpp"
#include "azure/storage/files/shares/share_file_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  /**
   * @brief ShareLeaseClient allows you to manipulate Azure Storage leases on shares and files.
   */
  class ShareLeaseClient final {
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
     * @brief Get lease ID of this lease client.
     *
     * @return Lease ID of this lease client.
     */
    const std::string& GetLeaseId()
    {
      std::lock_guard<std::mutex> guard(m_mutex);
      return m_leaseId;
    }

    /**
     * @brief Acquires a lease on the file or share.
     *
     * @param duration Specifies the duration of the lease, in seconds, or InfiniteLeaseDuration for
     * a lease that never expires. A non-infinite lease can be between 15 and 60 seconds. A lease
     * duration cannot be changed using renew or change.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return An AcquireLeaseResult describing the lease.
     */
    Azure::Response<Models::AcquireLeaseResult> Acquire(
        std::chrono::seconds duration,
        const AcquireLeaseOptions& options = AcquireLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context());

    /**
     * @brief Renews the share's previously-acquired lease.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A RenewLeaseResult describing the lease.
     */
    Azure::Response<Models::RenewLeaseResult> Renew(
        const RenewLeaseOptions& options = RenewLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context());

    /**
     * @brief Releases the file or share's previously-acquired lease.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A ReleaseLeaseResult describing the updated share or file.
     */
    Azure::Response<Models::ReleaseLeaseResult> Release(
        const ReleaseLeaseOptions& options = ReleaseLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context());

    /**
     * @brief Changes the lease of an active lease.
     *
     * @param proposedLeaseId Proposed lease ID, in a GUID string format.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A ChangeLeaseResult describing the updated lease.
     * @remarks The current ShareLeaseClient becomes invalid if this operation succeeds.
     */
    Azure::Response<Models::ChangeLeaseResult> Change(
        const std::string& proposedLeaseId,
        const ChangeLeaseOptions& options = ChangeLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context());

    /**
     * @brief Breaks the previously-acquired lease.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A BreakLeaseResult describing the broken lease.
     */
    Azure::Response<Models::BreakLeaseResult> Break(
        const BreakLeaseOptions& options = BreakLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context());

  private:
    Azure::Nullable<ShareFileClient> m_fileClient;
    Azure::Nullable<ShareClient> m_shareClient;
    std::mutex m_mutex;
    std::string m_leaseId;
  };

}}}} // namespace Azure::Storage::Files::Shares