// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/credentials/credentials.hpp"
#include "azure/core/http/pipeline.hpp"
#include "azure/core/response.hpp"
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
        std::shared_ptr<Core::Credentials::ClientSecretCredential> credential,
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
     * @return Azure::Core::Response<CreateShareResult> containing the information including the
     * version and modified time of a share.
     */
    Azure::Core::Response<CreateShareResult> Create(
        const CreateShareOptions& options = CreateShareOptions()) const;

    /**
     * @brief Deletes the file share.
     * @param options Optional parameters to delete this file share.
     * @return Azure::Core::Response<ShareDeleteResult> currently empty and reserved for future
     * usage.
     */
    Azure::Core::Response<DeleteShareResult> Delete(
        const DeleteShareOptions& options = DeleteShareOptions()) const;

    /**
     * @brief Creates a snapshot for the share.
     * @param options Optional parameters to create the share snapshot.
     * @return Azure::Core::Response<CreateShareSnapshotResult> containing the information for ths
     * snapshot.
     */
    Azure::Core::Response<CreateShareSnapshotResult> CreateSnapshot(
        const CreateShareSnapshotOptions& options = CreateShareSnapshotOptions()) const;

    /**
     * @brief Gets the properties of the share.
     * @param options Optional parameters to get the share properties.
     * @return Azure::Core::Response<GetSharePropertiesResult> containing the properties for ths
     * share or one of its snapshot.
     */
    Azure::Core::Response<GetSharePropertiesResult> GetProperties(
        const GetSharePropertiesOptions& options = GetSharePropertiesOptions()) const;

    /**
     * @brief Sets the quota of the share.
     * @param quota Specifies the maximum size of the share, in gigabytes.
     * @param options Optional parameters to set the share quota.
     * @return Azure::Core::Response<SetShareQuotaResult> containing the information including the
     * version and modified time of a share.
     */
    Azure::Core::Response<SetShareQuotaResult> SetQuota(
        int32_t quota,
        const SetShareQuotaOptions& options = SetShareQuotaOptions()) const;

    /**
     * @brief Sets the metadata to the share.
     * @param metadata A name-value pair to associate with a file storage 'Share' object..
     * @param options Optional parameters to set the share metadata.
     * @return Azure::Core::Response<SetShareMetadataResult> containing the information including
     * the version and modified time of a share.
     */
    Azure::Core::Response<SetShareMetadataResult> SetMetadata(
        std::map<std::string, std::string> metadata,
        const SetShareMetadataOptions& options = SetShareMetadataOptions()) const;

    /**
     * @brief Gets the access policy of the share.
     * @param options Optional parameters to get the share's access policy.
     * @return Azure::Core::Response<GetShareAccessPolicyResult> containing the access policy of
     * the share.
     */
    Azure::Core::Response<GetShareAccessPolicyResult> GetAccessPolicy(
        const GetShareAccessPolicyOptions& options = GetShareAccessPolicyOptions()) const;

    /**
     * @brief Sets the access policy of the share.
     * @param accessPolicy Specifies the access policy to be set to the share.
     * @param options Optional parameters to Set the share's access policy.
     * @return Azure::Core::Response<SetShareAccessPolicyResult> containing the information
     * including the version and modified time of a share.
     */
    Azure::Core::Response<SetShareAccessPolicyResult> SetAccessPolicy(
        const std::vector<SignedIdentifier>& accessPolicy,
        const SetShareAccessPolicyOptions& options = SetShareAccessPolicyOptions()) const;

    /**
     * @brief Gets the stats of the share.
     * @param options Optional parameters to get share's statistics.
     * @return Azure::Core::Response<GetShareStatisticsResult> containing the information including
     * the bytes used in by the share, the version and modified time of a share.
     */
    Azure::Core::Response<GetShareStatisticsResult> GetStatistics(
        const GetShareStatsOptions& options = GetShareStatsOptions()) const;

    /**
     * @brief Creates a permission on the share.
     * @param permission Specifies the permission to be created on the share.
     * @param options Optional parameters to create the share's permission.
     * @return Azure::Core::Response<CreateSharePermissionResult> containing the information
     * including the permission key of the permission.
     */
    Azure::Core::Response<CreateSharePermissionResult> CreatePermission(
        const std::string& permission,
        const CreateSharePermissionOptions& options = CreateSharePermissionOptions()) const;

    /**
     * @brief Gets the permission of the share using the specific key.
     * @param permissionKey The permission key of a permission.
     * @param options Optional parameters to get share's permission.
     * @return Azure::Core::Response<GetSharePermissionResult> containing the permission string with
     * specified key.
     */
    Azure::Core::Response<GetSharePermissionResult> GetPermission(
        const std::string& permissionKey,
        const GetSharePermissionOptions& options = GetSharePermissionOptions()) const;

    /**
     * @brief List files and directories under the directory.
     * @param options Optional parameters to list the files and directories under this directory.
     * @return Azure::Core::Response<ListFilesAndDirectoriesSegmentResult> containing the
     * information of the operation, directory, share and the listed result.
     */
    Azure::Core::Response<ListFilesAndDirectoriesSegmentResult> ListFilesAndDirectoriesSegment(
        const ListFilesAndDirectoriesSegmentOptions& options
        = ListFilesAndDirectoriesSegmentOptions()) const;

  private:
    Azure::Core::Http::Url m_shareUri;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    explicit ShareClient(
        Azure::Core::Http::Url shareUri,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : m_shareUri(std::move(shareUri)), m_pipeline(std::move(pipeline))
    {
    }
    friend class ServiceClient;
  };
}}}} // namespace Azure::Storage::Files::Shares
