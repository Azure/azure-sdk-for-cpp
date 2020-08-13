// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_credential.hpp"
#include "common/storage_uri_builder.hpp"
#include "credentials/credentials.hpp"
#include "http/pipeline.hpp"
#include "protocol/share_rest_client.hpp"
#include "response.hpp"
#include "share_options.hpp"
#include "share_responses.hpp"
#include "shares/share_service_client.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  class ShareClient {
  public:
    /**
     * @brief Create from connection string
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
     * @brief Shared key authentication client.
     * @param shareUri The URI of the file share this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareClient(
        const std::string& shareUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param shareUri The URI of the file share this client's request targets.
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareClient(
        const std::string& shareUri,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
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
    std::string GetUri() const { return m_shareUri.ToString(); }

    /**
     * @brief Creates the file share.
     * @param options Optional parameters to create this file share.
     * @return Azure::Core::Response<ShareInfo> The information containing the version and modified
     * time of a share.
     */
    Azure::Core::Response<ShareInfo> Create(
        const CreateShareOptions& options = CreateShareOptions()) const;

    /**
     * @brief Deletes the file share.
     * @param options Optional parameters to delete this file share.
     * @return Azure::Core::Response<ShareDeleteInfo> Currently empty and reserved for future usage.
     */
    Azure::Core::Response<ShareDeleteInfo> Delete(
        const DeleteShareOptions& options = DeleteShareOptions()) const;

    /**
     * @brief Creates a snapshot for the share.
     * @param options Optional parameters to create the share snapshot.
     * @return Azure::Core::Response<ShareSnapshotInfo> Containing the information for ths snapshot.
     */
    Azure::Core::Response<ShareSnapshotInfo> CreateSnapshot(
        const CreateShareSnapshotOptions& options = CreateShareSnapshotOptions()) const;

    /**
     * @brief Gets the properties of the share.
     * @param options Optional parameters to get the share properties.
     * @return Azure::Core::Response<FileShareProperties> Containing the information for ths share
     * or one of its snapshot.
     */
    Azure::Core::Response<FileShareProperties> GetProperties(
        const GetSharePropertiesOptions& options = GetSharePropertiesOptions()) const;

    /**
     * @brief Sets the quota of the share.
     * @param quota Specifies the maximum size of the share, in gigabytes.
     * @param options Optional parameters to set the share quota.
     * @return Azure::Core::Response<SetShareQuotaInfo> The information containing the version
     * and modified time of a share.
     */
    Azure::Core::Response<SetShareQuotaInfo> SetQuota(
        int32_t quota,
        const SetShareQuotaOptions& options = SetShareQuotaOptions()) const;

    /**
     * @brief Sets the metadata to the share.
     * @param metadata A name-value pair to associate with a file storage 'Share' object..
     * @param options Optional parameters to set the share metadata.
     * @return Azure::Core::Response<SetShareMetadataInfo> The information containing the version
     * and modified time of a share.
     */
    Azure::Core::Response<SetShareMetadataInfo> SetMetadata(
        std::map<std::string, std::string> metadata,
        const SetShareMetadataOptions& options = SetShareMetadataOptions()) const;

    /**
     * @brief Gets the access policy of the share.
     * @param options Optional parameters to get the share's access policy.
     * @return Azure::Core::Response<std::vector<SignedIdentifier>> The access policy of the
     * share.
     */
    Azure::Core::Response<GetShareAccessPolicyResult> GetAccessPolicy(
        const GetShareAccessPolicyOptions& options = GetShareAccessPolicyOptions()) const;

    /**
     * @brief Sets the access policy of the share.
     * @param accessPolicy Specifies the access policy to be set to the share.
     * @param options Optional parameters to Set the share's access policy.
     * @return Azure::Core::Response<SetAccessPolicyInfo> The information containing the version
     * and modified time of a share.
     */
    Azure::Core::Response<SetAccessPolicyInfo> SetAccessPolicy(
        const std::vector<SignedIdentifier>& accessPolicy,
        const SetShareAccessPolicyOptions& options = SetShareAccessPolicyOptions()) const;

    /**
     * @brief Gets the stats of the share.
     * @param options Optional parameters to get share's statistics.
     * @return Azure::Core::Response<ShareStatistics> The information containing the bytes used in
     * by the share, the version and modified time of a share.
     */
    Azure::Core::Response<ShareStatistics> GetStatistics(
        const GetShareStatsOptions& options = GetShareStatsOptions()) const;

    /**
     * @brief Creates a permission on the share.
     * @param permission Specifies the permission to be created on the share.
     * @param options Optional parameters to create the share's permission.
     * @return Azure::Core::Response<SharePermissionInfo> The information containing the permission
     * key of the permission.
     */
    Azure::Core::Response<SharePermissionInfo> CreatePermission(
        const std::string& permission,
        const CreateSharePermissionOptions& options = CreateSharePermissionOptions()) const;

    /**
     * @brief Gets the permission of the share using the specific key.
     * @param permissionKey The permission key of a permission.
     * @param options Optional parameters to get share's permission.
     * @return Azure::Core::Response<std::string> The permission sting with specified key.
     */
    Azure::Core::Response<std::string> GetPermission(
        const std::string& permissionKey,
        const GetSharePermissionOptions& options = GetSharePermissionOptions()) const;

  private:
    UriBuilder m_shareUri;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    explicit ShareClient(
        UriBuilder shareUri,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : m_shareUri(std::move(shareUri)), m_pipeline(std::move(pipeline))
    {
    }
    friend class ServiceClient;
  };
}}}} // namespace Azure::Storage::Files::Shares
