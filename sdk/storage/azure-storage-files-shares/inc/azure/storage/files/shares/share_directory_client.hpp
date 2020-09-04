// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/credentials/credentials.hpp"
#include "azure/core/http/pipeline.hpp"
#include "azure/core/response.hpp"
#include "azure/storage/common/storage_credential.hpp"
#include "azure/storage/files/shares/protocol/share_rest_client.hpp"
#include "azure/storage/files/shares/share_client.hpp"
#include "azure/storage/files/shares/share_options.hpp"
#include "azure/storage/files/shares/share_responses.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  class FileClient;

  class DirectoryClient {
  public:
    /**
     * @brief Create A DirectoryClient from connection string to manage a File Share Directory
     * resource.
     * @param connectionString Azure Storage connection string.
     * @param shareName The name of a file share.
     * @param directoryPath The path of a directory.
     * @param options Optional parameters used to initialize the client.
     * @return ShareClient The client that can be used to manage a share resource.
     */
    static DirectoryClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& shareName,
        const std::string& directoryPath,
        const DirectoryClientOptions& options = DirectoryClientOptions());

    /**
     * @brief Initialize a new instance of DirectoryClient using shared key authentication.
     * @param shareDirectoryUri The URI of the directory this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DirectoryClient(
        const std::string& shareDirectoryUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const DirectoryClientOptions& options = DirectoryClientOptions());

    /**
     * @brief Initialize a new instance of DirectoryClient using token authentication.
     * @param shareDirectoryUri The URI of the directory this client's request targets.
     * @param credential The client secret credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DirectoryClient(
        const std::string& shareDirectoryUri,
        std::shared_ptr<Core::Credentials::ClientSecretCredential> credential,
        const DirectoryClientOptions& options = DirectoryClientOptions());

    /**
     * @brief Initialize a new instance of DirectoryClient using anonymous access or shared access
     * signature.
     * @param shareDirectoryUri The URI of the directory this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DirectoryClient(
        const std::string& shareDirectoryUri,
        const DirectoryClientOptions& options = DirectoryClientOptions());

    /**
     * @brief Gets the directory's primary uri endpoint.
     *
     * @return The directory's primary uri endpoint.
     */
    std::string GetUri() const { return m_shareDirectoryUri.GetAbsoluteUrl(); }

    /**
     * @brief Create a DirectoryClient that's a sub directory of the current DirectoryClient
     * @param subDirectoryName The name of the subdirectory.
     * @return DirectoryClient A directory client that can be used to manage a share directory
     * resource.
     */
    DirectoryClient GetSubDirectoryClient(const std::string& subDirectoryName) const;

    /**
     * @brief Create a FileClient from current DirectoryClient
     * @param filePath The path of the file.
     * @return FileClient A file client that can be used to manage a share file
     * resource.
     */
    FileClient GetFileClient(const std::string& filePath) const;

    /**
     * @brief Initializes a new instance of the DirectoryClient class with an identical uri
     * source but the specified share snapshot timestamp.
     *
     * @param shareSnapshot The snapshot identifier for a share snapshot.
     * @return A new DirectoryClient instance.
     * @remarks Pass empty string to remove the snapshot returning the directory client without
     * specifying the share snapshot.
     */
    DirectoryClient WithShareSnapshot(const std::string& shareSnapshot) const;

    /**
     * @brief Creates the directory.
     * @param options Optional parameters to create this directory.
     * @return Azure::Core::Response<CreateDirectoryResult> containing the information returned when
     * creating the directory.
     */
    Azure::Core::Response<CreateDirectoryResult> Create(
        const CreateDirectoryOptions& options = CreateDirectoryOptions()) const;

    /**
     * @brief Deletes the directory.
     * @param options Optional parameters to delete this directory.
     * @return Azure::Core::Response<DeleteDirectoryResult> containing the information returned when
     * deleting the directory. Currently empty but preserved for future usage.
     */
    Azure::Core::Response<DeleteDirectoryResult> Delete(
        const DeleteDirectoryOptions& options = DeleteDirectoryOptions()) const;

    /**
     * @brief Gets the properties of the directory.
     * @param options Optional parameters to get this directory's properties.
     * @return Azure::Core::Response<GetDirectoryPropertiesResult> containing the properties of the
     * directory returned from the server.
     */
    Azure::Core::Response<GetDirectoryPropertiesResult> GetProperties(
        const GetDirectoryPropertiesOptions& options = GetDirectoryPropertiesOptions()) const;

    /**
     * @brief Sets the properties of the directory.
     * @param smbProperties The SMB properties to be set to the directory.
     * @param options Optional parameters to set this directory's properties.
     * @return Azure::Core::Response<SetDirectoryPropertiesResult> containing the properties of the
     * directory returned from the server.
     */
    Azure::Core::Response<SetDirectoryPropertiesResult> SetProperties(
        FileShareSmbProperties smbProperties,
        const SetDirectoryPropertiesOptions& options = SetDirectoryPropertiesOptions()) const;

    /**
     * @brief Sets the metadata of the directory.
     * @param metadata User-defined metadata to be stored with the directory. Note that the string
     *                 may only contain ASCII characters in the ISO-8859-1 character set.
     * @param options Optional parameters to set this directory's metadata.
     * @return Azure::Core::Response<SetDirectoryMetadataResult> containing the information of the
     * directory returned from the server.
     */
    Azure::Core::Response<SetDirectoryMetadataResult> SetMetadata(
        const std::map<std::string, std::string>& metadata,
        const SetDirectoryMetadataOptions& options = SetDirectoryMetadataOptions()) const;

    /**
     * @brief List files and directories under the directory.
     * @param options Optional parameters to list the files and directories under this directory.
     * @return Azure::Core::Response<ListFilesAndDirectoriesSegmentResult> containing the
     * information of the operation, directory, share and the listed result.
     */
    Azure::Core::Response<ListFilesAndDirectoriesSegmentResult> ListFilesAndDirectoriesSegment(
        const ListFilesAndDirectoriesSegmentOptions& options
        = ListFilesAndDirectoriesSegmentOptions()) const;

    /**
     * @brief List open handles on the directory.
     * @param options Optional parameters to list this directory's open handles.
     * @return Azure::Core::Response<ListDirectoryHandlesSegmentResult> containing the information
     * of the operation and the open handles of this directory
     */
    Azure::Core::Response<ListDirectoryHandlesSegmentResult> ListHandlesSegment(
        const ListDirectoryHandlesSegmentOptions& options
        = ListDirectoryHandlesSegmentOptions()) const;

    /**
     * @brief Closes a handle opened on a directory at the service.
     * @param handleId The ID of the handle to be closed.
     * @param options Optional parameters to close one of this directory's open handles.
     * @return Azure::Core::Response<ForceCloseDirectoryHandleResult> containing the information
     * of the closed handle. Current empty but preserved for future usage.
     */
    Azure::Core::Response<ForceCloseDirectoryHandleResult> ForceCloseHandle(
        const std::string& handleId,
        const ForceCloseDirectoryHandleOptions& options = ForceCloseDirectoryHandleOptions()) const;

    /**
     * @brief Closes all handles opened on a directory at the service.
     * @param options Optional parameters to close all this directory's open handles.
     * @return Azure::Core::Response<ForceCloseAllDirectoryHandlesResult> containing the information
     * of the closed handles
     * @remark This operation may return a marker showing that the operation can be continued.
     */
    Azure::Core::Response<ForceCloseAllDirectoryHandlesResult> ForceCloseAllHandles(
        const ForceCloseAllDirectoryHandlesOptions& options
        = ForceCloseAllDirectoryHandlesOptions()) const;

  private:
    Azure::Core::Http::Url m_shareDirectoryUri;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    explicit DirectoryClient(
        Azure::Core::Http::Url shareDirectoryUri,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : m_shareDirectoryUri(std::move(shareDirectoryUri)), m_pipeline(std::move(pipeline))
    {
    }

    friend class ShareClient;
  };
}}}} // namespace Azure::Storage::Files::Shares
