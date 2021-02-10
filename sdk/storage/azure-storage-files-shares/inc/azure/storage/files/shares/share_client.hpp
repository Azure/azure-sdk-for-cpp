// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/response.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/files/shares/protocol/share_rest_client.hpp"
#include "azure/storage/files/shares/share_options.hpp"
#include "azure/storage/files/shares/share_responses.hpp"
#include "azure/storage/files/shares/share_service_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  class ShareDirectoryClient;

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
     * @param shareUrl The URL of the file share this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareClient(
        const std::string& shareUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Initialize a new instance of ShareClient using anonymous access or shared access
     * signature.
     * @param shareUrl The URL of the file share this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareClient(
        const std::string& shareUrl,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Gets the share's primary url endpoint.
     *
     * @return The share's primary url endpoint.
     */
    std::string GetUrl() const { return m_shareUrl.GetAbsoluteUrl(); }

    /**
     * @brief Initializes a new instance of the ShareClient class with an identical url
     * source but the specified share snapshot timestamp.
     *
     * @param snapshot The snapshot identifier.
     * @return A new ShareClient instance.
     * @remarks Pass empty string to remove the snapshot returning the base share.
     */
    ShareClient WithSnapshot(const std::string& snapshot) const;

    /**
     * @brief Gets the ShareDirectoryClient that's pointing to the root directory of current
     * ShareClient
     * @return ShareDirectoryClient The root directory of the share.
     */
    ShareDirectoryClient GetRootDirectoryClient() const;

    /**
     * @brief Creates the file share.
     * @param options Optional parameters to create this file share.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::CreateShareResult> containing the information including
     * the version and modified time of a share.
     */
    Azure::Core::Response<Models::CreateShareResult> Create(
        const CreateShareOptions& options = CreateShareOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Creates the file share if it does not exist, nothing will happen if the file share
     * already exists.
     * @param options Optional parameters to create this file share.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::CreateShareResult> containing the information including
     * the version and modified time of a share if it is successfully created.
     */
    Azure::Core::Response<Models::CreateShareResult> CreateIfNotExists(
        const CreateShareOptions& options = CreateShareOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes the file share.
     * @param options Optional parameters to delete this file share.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::ShareDeleteResult> currently empty and reserved for
     * future usage.
     */
    Azure::Core::Response<Models::DeleteShareResult> Delete(
        const DeleteShareOptions& options = DeleteShareOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes the file share if it exists.
     * @param options Optional parameters to delete this file share.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::ShareDeleteResult> currently empty and reserved for
     * future usage.
     */
    Azure::Core::Response<Models::DeleteShareResult> DeleteIfExists(
        const DeleteShareOptions& options = DeleteShareOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Creates a snapshot for the share.
     * @param options Optional parameters to create the share snapshot.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::CreateShareSnapshotResult> containing the information
     * for ths snapshot.
     */
    Azure::Core::Response<Models::CreateShareSnapshotResult> CreateSnapshot(
        const CreateShareSnapshotOptions& options = CreateShareSnapshotOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets the properties of the share.
     * @param options Optional parameters to set the share properties.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::SetSharePropertiesResult> containing the information
     * including the version and modified time of a share.
     */
    Azure::Core::Response<Models::SetSharePropertiesResult> SetProperties(
        const SetSharePropertiesOptions& options = SetSharePropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Gets the properties of the share.
     * @param options Optional parameters to get the share properties.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::GetSharePropertiesResult> containing the properties for
     * ths share or one of its snapshot.
     */
    Azure::Core::Response<Models::GetSharePropertiesResult> GetProperties(
        const GetSharePropertiesOptions& options = GetSharePropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets the metadata to the share.
     * @param metadata A name-value pair to associate with a file storage 'Share' object..
     * @param options Optional parameters to set the share metadata.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::SetShareMetadataResult> containing the information
     * including the version and modified time of a share.
     */
    Azure::Core::Response<Models::SetShareMetadataResult> SetMetadata(
        Storage::Metadata metadata,
        const SetShareMetadataOptions& options = SetShareMetadataOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Gets the access policy of the share.
     * @param options Optional parameters to get the share's access policy.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::GetShareAccessPolicyResult> containing the access
     * policy of the share.
     */
    Azure::Core::Response<Models::GetShareAccessPolicyResult> GetAccessPolicy(
        const GetShareAccessPolicyOptions& options = GetShareAccessPolicyOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets the access policy of the share.
     * @param accessPolicy Specifies the access policy to be set to the share.
     * @param options Optional parameters to Set the share's access policy.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::SetShareAccessPolicyResult> containing the information
     * including the version and modified time of a share.
     */
    Azure::Core::Response<Models::SetShareAccessPolicyResult> SetAccessPolicy(
        const std::vector<Models::SignedIdentifier>& accessPolicy,
        const SetShareAccessPolicyOptions& options = SetShareAccessPolicyOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Gets the stats of the share.
     * @param options Optional parameters to get share's statistics.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::GetShareStatisticsResult> containing the information
     * including the bytes used in by the share, the version and modified time of a share.
     */
    Azure::Core::Response<Models::GetShareStatisticsResult> GetStatistics(
        const GetShareStatsOptions& options = GetShareStatsOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Creates a permission on the share.
     * @param permission Specifies the permission to be created on the share.
     * @param options Optional parameters to create the share's permission.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::CreateSharePermissionResult> containing the information
     * including the permission key of the permission.
     */
    Azure::Core::Response<Models::CreateSharePermissionResult> CreatePermission(
        const std::string& permission,
        const CreateSharePermissionOptions& options = CreateSharePermissionOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Gets the permission of the share using the specific key.
     * @param permissionKey The permission key of a permission.
     * @param options Optional parameters to get share's permission.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::GetSharePermissionResult> containing the permission
     * string with specified key.
     */
    Azure::Core::Response<Models::GetSharePermissionResult> GetPermission(
        const std::string& permissionKey,
        const GetSharePermissionOptions& options = GetSharePermissionOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief List files and directories under the directory.
     * @param options Optional parameters to list the files and directories under this directory.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::ListFilesAndDirectoriesSinglePageResult> containing the
     * information of the operation, directory, share and the listed result.
     */
    Azure::Core::Response<Models::ListFilesAndDirectoriesSinglePageResult>
    ListFilesAndDirectoriesSinglePage(
        const ListFilesAndDirectoriesSinglePageOptions& options
        = ListFilesAndDirectoriesSinglePageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

  private:
    Azure::Core::Http::Url m_shareUrl;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    explicit ShareClient(
        Azure::Core::Http::Url shareUrl,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : m_shareUrl(std::move(shareUrl)), m_pipeline(std::move(pipeline))
    {
    }
    friend class ShareLeaseClient;
    friend class ShareServiceClient;
  };
}}}} // namespace Azure::Storage::Files::Shares
