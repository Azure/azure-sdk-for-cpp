// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <azure/core/credentials.hpp>
#include <azure/core/http/pipeline.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/blobs/blob_client.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/files/datalake/datalake_file_system_client.hpp"
#include "azure/storage/files/datalake/datalake_options.hpp"
#include "azure/storage/files/datalake/datalake_responses.hpp"
#include "azure/storage/files/datalake/protocol/datalake_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  class DataLakePathClient {
  public:
    /**
     * @brief Create from connection string
     * @param connectionString Azure Storage connection string.
     * @param fileSystemName The name of a file system.
     * @param path The path of a resource within the file system.
     * @param options Optional parameters used to initialize the client.
     * @return DataLakePathClient
     */
    static DataLakePathClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& fileSystemName,
        const std::string& path,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Shared key authentication client.
     * @param pathUri The URI of the path this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakePathClient(
        const std::string& pathUri,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param pathUri The URI of the path this client's request targets.
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakePathClient(
        const std::string& pathUri,
        std::shared_ptr<Core::TokenCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param pathUri The URI of the path this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakePathClient(
        const std::string& pathUri,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Gets the path's primary uri endpoint. This is the endpoint used for blob
     * storage available features in DataLake.
     *
     * @return The path's primary uri endpoint.
     */
    std::string GetUri() const { return m_blobClient.GetUrl(); }

    /**
     * @brief Gets the path's primary uri endpoint. This is the endpoint used for dfs
     * endpoint only operations
     *
     * @return The path's primary uri endpoint.
     */
    std::string GetDfsUri() const { return m_dfsUri.GetAbsoluteUrl(); }

    /**
     * @brief Creates a file or directory. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param options Optional parameters to create the resource the path points to.
     * @return Azure::Core::Response<Models::CreateDataLakePathResult> containing the information
     * returned when creating a path.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::CreateDataLakePathResult> Create(
        Models::PathResourceType type,
        const CreateDataLakePathOptions& options = CreateDataLakePathOptions()) const;

    /**
     * @brief Creates a file or directory. By default, the destination is not changed if it already
     * exists.
     * @param options Optional parameters to create the resource the path points to.
     * @return Azure::Core::Response<Models::CreateDataLakePathResult> containing the information
     * returned when creating a path, the information will only be valid when the create operation
     * is successful.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::CreateDataLakePathResult> CreateIfNotExists(
        Models::PathResourceType type,
        const CreateDataLakePathOptions& options = CreateDataLakePathOptions()) const;

    /**
     * @brief Deletes the resource the path points to.
     * @param options Optional parameters to delete the reource the path points to.
     * @return Azure::Core::Response<Models::DeleteDataLakePathResult> which is current empty but
     * preserved for future usage.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::DeleteDataLakePathResult> Delete(
        const DeleteDataLakePathOptions& options = DeleteDataLakePathOptions()) const;

    /**
     * @brief Deletes the resource the path points to if it exists.
     * @param options Optional parameters to delete the reource the path points to.
     * @return Azure::Core::Response<Models::DeleteDataLakePathResult> which is current empty but
     * preserved for future usage. The result will only valid if the delete operation is successful.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::DeleteDataLakePathResult> DeleteIfExists(
        const DeleteDataLakePathOptions& options = DeleteDataLakePathOptions()) const;

    /**
     * @brief Sets the owner, group, permissions, or access control list for a file or directory.
     *        Note that Hierarchical Namespace must be enabled for the account in order to use
     *        access control. Also note that the Access Control List (ACL) includes permissions for
     *        the owner, owning group, and others, so the x-ms-permissions and x-ms-acl request
     *        headers are mutually exclusive.
     * @param acls Sets POSIX access control rights on files and directories. Each access control
     *             entry (ACE) consists of a scope, a type, a user or group identifier, and
     *             permissions.
     * @param options Optional parameters to set an access control to the resource the path points
     *                to.
     * @return Azure::Core::Response<Models::SetDataLakePathAccessControlResult> containing the
     * information returned when setting path's access control.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::SetDataLakePathAccessControlResult> SetAccessControl(
        std::vector<Models::Acl> acls,
        const SetDataLakePathAccessControlOptions& options
        = SetDataLakePathAccessControlOptions()) const;

    /**
     * @brief Sets the properties of a resource the path points to.
     * @param options Optional parameters to set the http headers to the resource the path points
     * to.
     * @return Azure::Core::Response<SetDataLakePathHttpHeadersResult> containing the information
     * returned when setting the path's Http headers.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::SetDataLakePathHttpHeadersResult> SetHttpHeaders(
        Models::PathHttpHeaders httpHeaders,
        const SetDataLakePathHttpHeadersOptions& options
        = SetDataLakePathHttpHeadersOptions()) const;

    /**
     * @brief Get Properties returns all system and user defined properties for a path. Get Status
     *        returns all system defined properties for a path. Get Access Control List returns the
     *        access control list for a path.
     * @param options Optional parameters to get the properties from the resource the path points
     *                to.
     * @return Azure::Core::Response<Models::GetDataLakePathPropertiesResult> containing the
     * properties of the path.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::GetDataLakePathPropertiesResult> GetProperties(
        const GetDataLakePathPropertiesOptions& options = GetDataLakePathPropertiesOptions()) const;

    /**
     * @brief Returns all access control list stored for the given path.
     * @param options Optional parameters to get the ACLs from the resource the path points to.
     * @return Azure::Core::Response<Models::GetDataLakePathAccessControlResult> containing the
     * access control list of the path.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::GetDataLakePathAccessControlResult> GetAccessControls(
        const GetDataLakePathAccessControlOptions& options
        = GetDataLakePathAccessControlOptions()) const;

    /**
     * @brief Sets the metadata of a resource the path points to.
     * @param metadata User-defined metadata to be stored with the filesystem. Note that the string
     *                 may only contain ASCII characters in the ISO-8859-1 character set.
     * @param options Optional parameters to set the metadata to the resource the path points to.
     * @return Azure::Core::Response<Models::SetDataLakePathMetadataResult> containing the
     * information returned when setting the metadata.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::SetDataLakePathMetadataResult> SetMetadata(
        Storage::Metadata metadata,
        const SetDataLakePathMetadataOptions& options = SetDataLakePathMetadataOptions()) const;

    /**
     * @brief Acquires a lease on the path.
     * @param proposedLeaseId Proposed lease ID, in a GUID string format.
     * @param duration Specifies the duration of the lease, in seconds, or
     * Azure::Storage::InfiniteLeaseDuration for a lease that never expires. A non-infinite lease
     * can be between 15 and 60 seconds. A lease duration cannot be changed using renew or change.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<Models::AcquireDataLakePathLeaseResult> describing the lease.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::AcquireDataLakePathLeaseResult> AcquireLease(
        const std::string& proposedLeaseId,
        int32_t duration,
        const AcquireDataLakePathLeaseOptions& options = AcquireDataLakePathLeaseOptions()) const
    {
      return m_blobClient.AcquireLease(proposedLeaseId, duration, options);
    }

    /**
     * @brief Renews the path's previously-acquired lease.
     * @param leaseId ID of the previously-acquired lease.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<Models::RenewDataLakePathLeaseResult> describing the lease.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::RenewDataLakePathLeaseResult> RenewLease(
        const std::string& leaseId,
        const RenewDataLakePathLeaseOptions& options = RenewDataLakePathLeaseOptions()) const
    {
      return m_blobClient.RenewLease(leaseId, options);
    }

    /**
     * @brief Releases the path's previously-acquired lease.
     * @param leaseId ID of the previously-acquired lease.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<Models::ReleaseDataLakePathLeaseResult> describing the updated
     * path.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::ReleaseDataLakePathLeaseResult> ReleaseLease(
        const std::string& leaseId,
        const ReleaseDataLakePathLeaseOptions& options = ReleaseDataLakePathLeaseOptions()) const
    {
      return m_blobClient.ReleaseLease(leaseId, options);
    }

    /**
     * @brief Changes the lease of an active lease.
     * @param leaseId ID of the previously-acquired lease.
     * @param proposedLeaseId Proposed lease ID, in a GUID string format.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<Models::ChangeDataLakePathLeaseResult> describing the lease.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::ChangeDataLakePathLeaseResult> ChangeLease(
        const std::string& leaseId,
        const std::string& proposedLeaseId,
        const ChangeDataLakePathLeaseOptions& options = ChangeDataLakePathLeaseOptions()) const
    {
      return m_blobClient.ChangeLease(leaseId, proposedLeaseId, options);
    }

    /**
     * @brief Breaks the previously-acquired lease.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<Models::BreakDataLakePathLeaseResult> describing the broken
     * lease.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::BreakDataLakePathLeaseResult> BreakLease(
        const BreakDataLakePathLeaseOptions& options = BreakDataLakePathLeaseOptions()) const
    {
      return m_blobClient.BreakLease(options);
    }

  protected:
    Azure::Core::Http::Url m_dfsUri;
    Blobs::BlobClient m_blobClient;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    explicit DataLakePathClient(
        Azure::Core::Http::Url dfsUri,
        Blobs::BlobClient blobClient,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : m_dfsUri(std::move(dfsUri)), m_blobClient(std::move(blobClient)),
          m_pipeline(std::move(pipeline))
    {
    }

    friend class DataLakeFileSystemClient;
  };
}}}} // namespace Azure::Storage::Files::DataLake
