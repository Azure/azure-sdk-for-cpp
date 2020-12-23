// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/http/pipeline.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/files/shares/protocol/share_rest_client.hpp"
#include "azure/storage/files/shares/share_client.hpp"
#include "azure/storage/files/shares/share_options.hpp"
#include "azure/storage/files/shares/share_responses.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  class ShareFileClient;

  class ShareDirectoryClient {
  public:
    /**
     * @brief Create A ShareDirectoryClient from connection string to manage a File Share Directory
     * resource.
     * @param connectionString Azure Storage connection string.
     * @param shareName The name of a file share.
     * @param directoryPath The path of a directory.
     * @param options Optional parameters used to initialize the client.
     * @return ShareClient The client that can be used to manage a share resource.
     */
    static ShareDirectoryClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& shareName,
        const std::string& directoryPath,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Initialize a new instance of ShareDirectoryClient using shared key authentication.
     * @param shareDirectoryUri The URI of the directory this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareDirectoryClient(
        const std::string& shareDirectoryUri,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Initialize a new instance of ShareDirectoryClient using anonymous access or shared
     * access signature.
     * @param shareDirectoryUri The URI of the directory this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareDirectoryClient(
        const std::string& shareDirectoryUri,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Gets the directory's primary uri endpoint.
     *
     * @return The directory's primary uri endpoint.
     */
    std::string GetUri() const { return m_shareDirectoryUri.GetAbsoluteUrl(); }

    /**
     * @brief Create a ShareDirectoryClient that's a sub directory of the current
     * ShareDirectoryClient
     * @param subDirectoryName The name of the subdirectory.
     * @return ShareDirectoryClient A directory client that can be used to manage a share directory
     * resource.
     */
    ShareDirectoryClient GetSubShareDirectoryClient(const std::string& subDirectoryName) const;

    /**
     * @brief Create a ShareFileClient from current ShareDirectoryClient
     * @param filePath The path of the file.
     * @return ShareFileClient A file client that can be used to manage a share file
     * resource.
     */
    ShareFileClient GetShareFileClient(const std::string& filePath) const;

    /**
     * @brief Initializes a new instance of the ShareDirectoryClient class with an identical uri
     * source but the specified share snapshot timestamp.
     *
     * @param shareSnapshot The snapshot identifier for a share snapshot.
     * @return A new ShareDirectoryClient instance.
     * @remarks Pass empty string to remove the snapshot returning the directory client without
     * specifying the share snapshot.
     */
    ShareDirectoryClient WithShareSnapshot(const std::string& shareSnapshot) const;

    /**
     * @brief Creates the directory.
     * @param options Optional parameters to create this directory.
     * @return Azure::Core::Response<Models::CreateDirectoryResult> containing the information
     * returned when creating the directory.
     */
    Azure::Core::Response<Models::CreateDirectoryResult> Create(
        const CreateDirectoryOptions& options = CreateDirectoryOptions()) const;

    /**
     * @brief Deletes the directory.
     * @param options Optional parameters to delete this directory.
     * @return Azure::Core::Response<Models::DeleteDirectoryResult> containing the information
     * returned when deleting the directory. Currently empty but preserved for future usage.
     */
    Azure::Core::Response<Models::DeleteDirectoryResult> Delete(
        const DeleteDirectoryOptions& options = DeleteDirectoryOptions()) const;

    /**
     * @brief Gets the properties of the directory.
     * @param options Optional parameters to get this directory's properties.
     * @return Azure::Core::Response<Models::GetDirectoryPropertiesResult> containing the properties
     * of the directory returned from the server.
     */
    Azure::Core::Response<Models::GetDirectoryPropertiesResult> GetProperties(
        const GetDirectoryPropertiesOptions& options = GetDirectoryPropertiesOptions()) const;

    /**
     * @brief Sets the properties of the directory.
     * @param smbProperties The SMB properties to be set to the directory.
     * @param options Optional parameters to set this directory's properties.
     * @return Azure::Core::Response<Models::SetDirectoryPropertiesResult> containing the properties
     * of the directory returned from the server.
     */
    Azure::Core::Response<Models::SetDirectoryPropertiesResult> SetProperties(
        Models::FileShareSmbProperties smbProperties,
        const SetDirectoryPropertiesOptions& options = SetDirectoryPropertiesOptions()) const;

    /**
     * @brief Sets the metadata of the directory.
     * @param metadata User-defined metadata to be stored with the directory. Note that the string
     *                 may only contain ASCII characters in the ISO-8859-1 character set.
     * @param options Optional parameters to set this directory's metadata.
     * @return Azure::Core::Response<Models::SetDirectoryMetadataResult> containing the information
     * of the directory returned from the server.
     */
    Azure::Core::Response<Models::SetDirectoryMetadataResult> SetMetadata(
        Storage::Metadata metadata,
        const SetDirectoryMetadataOptions& options = SetDirectoryMetadataOptions()) const;

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
     * @brief List open handles on the directory.
     * @param options Optional parameters to list this directory's open handles.
     * @return Azure::Core::Response<Models::ListDirectoryHandlesSegmentResult> containing the
     * information of the operation and the open handles of this directory
     */
    Azure::Core::Response<Models::ListDirectoryHandlesSegmentResult> ListHandlesSegment(
        const ListDirectoryHandlesSegmentOptions& options
        = ListDirectoryHandlesSegmentOptions()) const;

    /**
     * @brief Closes a handle opened on a directory at the service.
     * @param handleId The ID of the handle to be closed.
     * @param options Optional parameters to close one of this directory's open handles.
     * @return Azure::Core::Response<Models::ForceCloseDirectoryHandleResult> containing the
     * information of the closed handle. Current empty but preserved for future usage.
     */
    Azure::Core::Response<Models::ForceCloseDirectoryHandleResult> ForceCloseHandle(
        const std::string& handleId,
        const ForceCloseDirectoryHandleOptions& options = ForceCloseDirectoryHandleOptions()) const;

    /**
     * @brief Closes all handles opened on a directory at the service.
     * @param options Optional parameters to close all this directory's open handles.
     * @return Azure::Core::Response<Models::ForceCloseAllDirectoryHandlesResult> containing the
     * information of the closed handles
     * @remark This operation may return a marker showing that the operation can be continued.
     */
    Azure::Core::Response<Models::ForceCloseAllDirectoryHandlesResult> ForceCloseAllHandles(
        const ForceCloseAllDirectoryHandlesOptions& options
        = ForceCloseAllDirectoryHandlesOptions()) const;

  private:
    Azure::Core::Http::Url m_shareDirectoryUri;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    explicit ShareDirectoryClient(
        Azure::Core::Http::Url shareDirectoryUri,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : m_shareDirectoryUri(std::move(shareDirectoryUri)), m_pipeline(std::move(pipeline))
    {
    }

    friend class ShareClient;
  };
}}}} // namespace Azure::Storage::Files::Shares
