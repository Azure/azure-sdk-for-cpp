// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <chrono>

#include <azure/storage/blobs/blob_lease_client.hpp>

#include "azure/storage/files/datalake/datalake_file_system_client.hpp"
#include "azure/storage/files/datalake/datalake_path_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  /**
   * @brief DataLakeLeaseClient allows you to manipulate Azure Storage leases on filesystems and
   * paths.
   */
  class DataLakeLeaseClient {
  public:
    /**
     * @brief Initializes a new instance of the DataLakeLeaseClient.
     *
     * @param pathClient A DataLakePathClient representing the datalake path being leased.
     * @param leaseId A lease ID. This is not required for break operation.
     */
    explicit DataLakeLeaseClient(DataLakePathClient pathClient, std::string leaseId)
        : m_blobLeaseClient(std::move(pathClient.m_blobClient), std::move(leaseId))
    {
    }

    /**
     * @brief Initializes a new instance of the DataLakeLeaseClient.
     *
     * @param fileSystemClient A DataLakeFileSystemClient representing the filesystem being leased.
     * @param leaseId A lease ID. This is not required for break operation.
     */
    explicit DataLakeLeaseClient(DataLakeFileSystemClient fileSystemClient, std::string leaseId)
        : m_blobLeaseClient(std::move(fileSystemClient.m_blobContainerClient), std::move(leaseId))
    {
    }

    /**
     * @brief Gets a unique lease ID.
     *
     * @return A unique lease ID.
     */
    static std::string CreateUniqueLeaseId()
    {
      return Blobs::BlobLeaseClient::CreateUniqueLeaseId();
    };

    /**
     * @brief A value representing infinite lease duration.
     */
    AZ_STORAGE_FILES_DATALAKE_DLLEXPORT const static std::chrono::seconds InfiniteLeaseDuration;

    /**
     * @brief Get lease id of this lease client.
     *
     * @return Lease id of this lease client.
     */
    const std::string& GetLeaseId() const { return m_blobLeaseClient.GetLeaseId(); }

    /**
     * @brief Acquires a lease on the datalake path or datalake path container.
     *
     * @param duration Specifies the duration of
     * the lease, in seconds, or InfiniteLeaseDuration for a lease that never
     * expires. A non-infinite lease can be between 15 and 60 seconds. A lease duration cannot be
     * changed using renew or change.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A AcquireDataLakeLeaseResult describing the lease.
     */
    Azure::Core::Response<Models::AcquireDataLakeLeaseResult> Acquire(
        std::chrono::seconds duration,
        const AcquireDataLakeLeaseOptions& options = AcquireDataLakeLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const
    {
      return m_blobLeaseClient.Acquire(duration, options, context);
    }

    /**
     * @brief Renews the datalake path or datalake path container's previously-acquired lease.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A RenewDataLakeLeaseResult describing the lease.
     */
    Azure::Core::Response<Models::RenewDataLakeLeaseResult> Renew(
        const RenewDataLakeLeaseOptions& options = RenewDataLakeLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const
    {
      return m_blobLeaseClient.Renew(options, context);
    }

    /**
     * @brief Releases the datalake path or datalake path container's previously-acquired lease.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A ReleaseDataLakeLeaseResult describing the updated container or blob.
     */
    Azure::Core::Response<Models::ReleaseDataLakeLeaseResult> Release(
        const ReleaseDataLakeLeaseOptions& options = ReleaseDataLakeLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const
    {
      return m_blobLeaseClient.Release(options, context);
    }

    /**
     * @brief Changes the lease of an active lease.
     *
     * @param proposedLeaseId Proposed lease ID, in a GUID string format.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A ChangeDataLakeLeaseResult describing the changed lease.
     * @remarks The current DataLakeLeaseClient becomes invalid if this operation succeeds.
     */
    Azure::Core::Response<Models::ChangeDataLakeLeaseResult> Change(
        const std::string& proposedLeaseId,
        const ChangeDataLakeLeaseOptions& options = ChangeDataLakeLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const
    {
      return m_blobLeaseClient.Change(proposedLeaseId, options, context);
    }

    /**
     * @brief Breaks the previously-acquired lease.
     *
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return A BreakDataLakeLeaseResult describing the broken lease.
     */
    Azure::Core::Response<Models::BreakDataLakeLeaseResult> Break(
        const BreakDataLakeLeaseOptions& options = BreakDataLakeLeaseOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const
    {
      return m_blobLeaseClient.Break(options, context);
    }

  private:
    Blobs::BlobLeaseClient m_blobLeaseClient;
  };

}}}} // namespace Azure::Storage::Files::DataLake
