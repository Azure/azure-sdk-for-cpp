// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/http/pipeline.hpp"
#include "azure/core/response.hpp"
#include "azure/identity/client_secret_credential.hpp"
#include "azure/storage/common/storage_credential.hpp"
#include "azure/storage/files/shares/protocol/share_rest_client.hpp"
#include "azure/storage/files/shares/share_options.hpp"
#include "azure/storage/files/shares/share_responses.hpp"
#include "azure/storage/files/shares/share_service_client.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  class DirectoryClient;
  class FileClient;

  class ShareClient {
  public:
    /**
     * @brief Create A ShareClient from connection string to manage a File Share resource.
     * @param connectionString Azure Storage connection string.
     * @param shareName The name of a file share.
     * @param options Optional parameters used to initialize the client.
     * @return ShareClient The client that can be used to manage a share resource.
     */
    static ShareClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& shareName,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Initialize a new instance of ShareClient using shared key authentication.
     * @param shareUri The URI of the file share this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareClient(
        const std::string& shareUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Initialize a new instance of ShareClient using token authentication.
     * @param shareUri The URI of the file share this client's request targets.
     * @param credential The client secret credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareClient(
        const std::string& shareUri,
        std::shared_ptr<Core::TokenCredential> credential,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Initialize a new instance of ShareClient using anonymous access or shared access
     * signature.
     * @param shareUri The URI of the file share this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareClient(
        const std::string& shareUri,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Gets the share's primary uri endpoint.
     *
     * @return The share's primary uri endpoint.
     */
    std::string GetUri() const { return m_shareUri.GetAbsoluteUrl(); }

    /**
     * @brief Initializes a new instance of the ShareClient class with an identical uri
     * source but the specified share snapshot timestamp.
     *
     * @param snapshot The snapshot identifier.
     * @return A new ShareClient instance.
     * @remarks Pass empty string to remove the snapshot returning the base share.
     */
    ShareClient WithSnapshot(const std::string& snapshot) const;

    /**
     * @brief Create a DirectoryClient from current ShareClient
     * @param directoryPath The path of the directory.
     * @return DirectoryClient A directory client that can be used to manage a share directory
     * resource.
     */
    DirectoryClient GetDirectoryClient(const std::string& directoryPath) const;

    /**
     * @brief Create a FileClient from current ShareClient
     * @param filePath The path of the file.
     * @return FileClient A file client that can be used to manage a share file
     * resource.
     */
    FileClient GetFileClient(const std::string& filePath) const;

    /**
     * @brief Creates the file share.
     * @param options Optional parameters to create this file share.
     * @return Azure::Core::Response<Models::CreateShareResult> containing the information including
     * the version and modified time of a share.
     */
    Azure::Core::Response<Models::CreateShareResult> Create(
        const CreateShareOptions& options = CreateShareOptions()) const;

    /**
     * @brief Deletes the file share.
     * @param options Optional parameters to delete this file share.
     * @return Azure::Core::Response<Models::ShareDeleteResult> currently empty and reserved for
     * future usage.
     */
    Azure::Core::Response<Models::DeleteShareResult> Delete(
        const DeleteShareOptions& options = DeleteShareOptions()) const;

    /**
     * @brief Creates a snapshot for the share.
     * @param options Optional parameters to create the share snapshot.
     * @return Azure::Core::Response<Models::CreateShareSnapshotResult> containing the information
     * for ths snapshot.
     */
    Azure::Core::Response<Models::CreateShareSnapshotResult> CreateSnapshot(
        const CreateShareSnapshotOptions& options = CreateShareSnapshotOptions()) const;

    /**
     * @brief Gets the properties of the share.
     * @param options Optional parameters to get the share properties.
     * @return Azure::Core::Response<Models::GetSharePropertiesResult> containing the properties for
     * ths share or one of its snapshot.
     */
    Azure::Core::Response<Models::GetSharePropertiesResult> GetProperties(
        const GetSharePropertiesOptions& options = GetSharePropertiesOptions()) const;

    /**
     * @brief Sets the quota of the share.
     * @param quota Specifies the maximum size of the share, in gigabytes.
     * @param options Optional parameters to set the share quota.
     * @return Azure::Core::Response<Models::SetShareQuotaResult> containing the information
     * including the version and modified time of a share.
     */
    Azure::Core::Response<Models::SetShareQuotaResult> SetQuota(
        int32_t quotaInGiB,
        const SetShareQuotaOptions& options = SetShareQuotaOptions()) const;

    /**
     * @brief Sets the metadata to the share.
     * @param metadata A name-value pair to associate with a file storage 'Share' object..
     * @param options Optional parameters to set the share metadata.
     * @return Azure::Core::Response<Models::SetShareMetadataResult> containing the information
     * including the version and modified time of a share.
     */
    Azure::Core::Response<Models::SetShareMetadataResult> SetMetadata(
        std::map<std::string, std::string> metadata,
        const SetShareMetadataOptions& options = SetShareMetadataOptions()) const;

    /**
     * @brief Gets the access policy of the share.
     * @param options Optional parameters to get the share's access policy.
     * @return Azure::Core::Response<Models::GetShareAccessPolicyResult> containing the access
     * policy of the share.
     */
    Azure::Core::Response<Models::GetShareAccessPolicyResult> GetAccessPolicy(
        const GetShareAccessPolicyOptions& options = GetShareAccessPolicyOptions()) const;

    /**
     * @brief Sets the access policy of the share.
     * @param accessPolicy Specifies the access policy to be set to the share.
     * @param options Optional parameters to Set the share's access policy.
     * @return Azure::Core::Response<Models::SetShareAccessPolicyResult> containing the information
     * including the version and modified time of a share.
     */
    Azure::Core::Response<Models::SetShareAccessPolicyResult> SetAccessPolicy(
        const std::vector<Models::SignedIdentifier>& accessPolicy,
        const SetShareAccessPolicyOptions& options = SetShareAccessPolicyOptions()) const;

    /**
     * @brief Gets the stats of the share.
     * @param options Optional parameters to get share's statistics.
     * @return Azure::Core::Response<Models::GetShareStatisticsResult> containing the information
     * including the bytes used in by the share, the version and modified time of a share.
     */
    Azure::Core::Response<Models::GetShareStatisticsResult> GetStatistics(
        const GetShareStatsOptions& options = GetShareStatsOptions()) const;

    /**
     * @brief Creates a permission on the share.
     * @param permission Specifies the permission to be created on the share.
     * @param options Optional parameters to create the share's permission.
     * @return Azure::Core::Response<Models::CreateSharePermissionResult> containing the information
     * including the permission key of the permission.
     */
    Azure::Core::Response<Models::CreateSharePermissionResult> CreatePermission(
        const std::string& permission,
        const CreateSharePermissionOptions& options = CreateSharePermissionOptions()) const;

    /**
     * @brief Gets the permission of the share using the specific key.
     * @param permissionKey The permission key of a permission.
     * @param options Optional parameters to get share's permission.
     * @return Azure::Core::Response<Models::GetSharePermissionResult> containing the permission
     * string with specified key.
     */
    Azure::Core::Response<Models::GetSharePermissionResult> GetPermission(
        const std::string& permissionKey,
        const GetSharePermissionOptions& options = GetSharePermissionOptions()) const;

    /**
     * @brief List files and directories under the directory.
     * @param options Optional parameters to list the files and directories under this directory.
     * @return Azure::Core::Response<Models::ListFilesAndDirectoriesSegmentResult> containing the
     * information of the operation, directory, share and the listed result.
     */
    Azure::Core::Response<Models::ListFilesAndDirectoriesSegmentResult>
    ListFilesAndDirectoriesSegment(
        const ListFilesAndDirectoriesSegmentOptions& options
        = ListFilesAndDirectoriesSegmentOptions()) const;

    /**
     * @brief Acquires a lease on the share.
     *
     * @param proposedLeaseId Proposed lease ID, in a GUID string format.
     * @param duration Specifies the duration of the lease, in seconds, or
     * Azure::Storage::c_InfiniteLeaseDuration for a lease that never expires. A non-infinite lease
     * can be between 15 and 60 seconds. A lease duration cannot be changed using renew or change.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<Models::AcquireShareLeaseResult> describing the lease.
     */
    Azure::Core::Response<Models::AcquireShareLeaseResult> AcquireLease(
        const std::string& proposedLeaseId,
        int32_t duration,
        const AcquireShareLeaseOptions& options = AcquireShareLeaseOptions()) const;

    /**
     * @brief Releases the share's previously-acquired lease.
     *
     * @param leaseId ID of the previously-acquired lease.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<Models::ReleaseShareLeaseResult> describing the updated lease
     * status.
     */
    Azure::Core::Response<Models::ReleaseShareLeaseResult> ReleaseLease(
        const std::string& leaseId,
        const ReleaseShareLeaseOptions& options = ReleaseShareLeaseOptions()) const;

    /**
     * @brief Changes the lease of an active lease.
     *
     * @param leaseId ID of the previously-acquired lease.
     * @param proposedLeaseId Proposed lease ID, in a GUID string format.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<Models::ChangeShareLeaseResult> describing the changed lease.
     */
    Azure::Core::Response<Models::ChangeShareLeaseResult> ChangeLease(
        const std::string& leaseId,
        const std::string& proposedLeaseId,
        const ChangeShareLeaseOptions& options = ChangeShareLeaseOptions()) const;

    /**
     * @brief Breaks the previously-acquired lease.
     *
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<Models::BreakShareLeaseResult> describing the broken lease.
     */
    Azure::Core::Response<Models::BreakShareLeaseResult> BreakLease(
        const BreakShareLeaseOptions& options = BreakShareLeaseOptions()) const;

    /**
     * @brief Renew the previously-acquired lease.
     *
     * @param leaseId ID of the previously-acquired lease.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<Models::BreakShareLeaseResult> describing the renewed lease.
     */
    Azure::Core::Response<Models::RenewShareLeaseResult> RenewLease(
        const std::string& leaseId,
        const RenewShareLeaseOptions& options = RenewShareLeaseOptions()) const;

  private:
    Azure::Core::Http::Url m_shareUri;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    explicit ShareClient(
        Azure::Core::Http::Url shareUri,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : m_shareUri(std::move(shareUri)), m_pipeline(std::move(pipeline))
    {
    }
    friend class ShareServiceClient;
  };
}}}} // namespace Azure::Storage::Files::Shares
