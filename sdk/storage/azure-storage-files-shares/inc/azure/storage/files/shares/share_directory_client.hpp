// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/files/shares/share_client.hpp"
#include "azure/storage/files/shares/share_options.hpp"
#include "azure/storage/files/shares/share_responses.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  class ShareFileClient;

  class ShareDirectoryClient final {
  public:
    /**
     * @brief Create A ShareDirectoryClient from connection string to manage a File Share Directory
     * resource.
     * @param connectionString Azure Storage connection string.
     * @param shareName The name of a file share.
     * @param directoryName The name of a directory.
     * @param options Optional parameters used to initialize the client.
     * @return ShareClient The client that can be used to manage a share resource.
     */
    static ShareDirectoryClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& shareName,
        const std::string& directoryName,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Initialize a new instance of ShareDirectoryClient using shared key authentication.
     * @param shareDirectoryUrl The URL of the directory this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareDirectoryClient(
        const std::string& shareDirectoryUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Initialize a new instance of ShareDirectoryClient using shared key authentication.
     * @param shareDirectoryUrl The URL of the directory this client's request targets.
     * @param credential The token credential used to sign requests.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareDirectoryClient(
        const std::string& shareDirectoryUrl,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Initialize a new instance of ShareDirectoryClient using anonymous access or shared
     * access signature.
     * @param shareDirectoryUrl The URL of the directory this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareDirectoryClient(
        const std::string& shareDirectoryUrl,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Gets the directory's primary URL endpoint.
     *
     * @return The directory's primary URL endpoint.
     */
    std::string GetUrl() const { return m_shareDirectoryUrl.GetAbsoluteUrl(); }

    /**
     * @brief Create a ShareDirectoryClient that's a sub directory of the current
     * ShareDirectoryClient
     * @param subdirectoryName The name of the subdirectory.
     * @return ShareDirectoryClient A directory client that can be used to manage a share directory
     * resource.
     */
    ShareDirectoryClient GetSubdirectoryClient(const std::string& subdirectoryName) const;

    /**
     * @brief Create a ShareFileClient from current ShareDirectoryClient
     * @param fileName The name of the file.
     * @return ShareFileClient A file client that can be used to manage a share file
     * resource.
     */
    ShareFileClient GetFileClient(const std::string& fileName) const;

    /**
     * @brief Initializes a new instance of the ShareDirectoryClient class with an identical URL
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
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::CreateDirectoryResult> containing the information
     * returned when creating the directory.
     */
    Azure::Response<Models::CreateDirectoryResult> Create(
        const CreateDirectoryOptions& options = CreateDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Creates the directory if it does not exist.
     * @param options Optional parameters to create this directory.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::CreateDirectoryResult> containing the information
     * returned when creating the directory if successfully created.
     */
    Azure::Response<Models::CreateDirectoryResult> CreateIfNotExists(
        const CreateDirectoryOptions& options = CreateDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Renames a file. This API does not support renaming a file
     * from one share to another, or between storage accounts.
     * @param fileName The file that gets renamed.
     * @param destinationFilePath The path of the file the source file is renaming to. The
     * destination is an absolute path under the root of the share, without leading slash.
     * @param options Optional parameters to rename a file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<ShareFileClient> The client targets the renamed file.
     */
    Azure::Response<ShareFileClient> RenameFile(
        const std::string& fileName,
        const std::string& destinationFilePath,
        const RenameFileOptions& options = RenameFileOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Renames a directory. This API does not support renaming a directory
     * from one share to another, or between storage accounts.
     * @param subdirectoryName The subdirectory that gets renamed.
     * @param destinationDirectoryPath The destinationPath the source subdirectory is renaming to.
     * The destination is an absolute path under the root of the share, without leading slash.
     * @param options Optional parameters to rename a directory.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<ShareDirectoryClient> The client targets the renamed
     * directory.
     */
    Azure::Response<ShareDirectoryClient> RenameSubdirectory(
        const std::string& subdirectoryName,
        const std::string& destinationDirectoryPath,
        const RenameDirectoryOptions& options = RenameDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes the directory.
     * @param options Optional parameters to delete this directory.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::DeleteDirectoryResult> containing the information
     * returned when deleting the directory. Currently empty but preserved for future usage.
     */
    Azure::Response<Models::DeleteDirectoryResult> Delete(
        const DeleteDirectoryOptions& options = DeleteDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes the directory if it exists.
     * @param options Optional parameters to delete this directory.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::DeleteDirectoryResult> containing the information
     * returned when deleting the directory. Currently empty but preserved for future usage.
     * Only when the delete operation if successful, the returned information other than 'Deleted'
     * is valid.
     */
    Azure::Response<Models::DeleteDirectoryResult> DeleteIfExists(
        const DeleteDirectoryOptions& options = DeleteDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Gets the properties of the directory.
     * @param options Optional parameters to get this directory's properties.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::DirectoryProperties> containing the
     * properties of the directory returned from the server.
     */
    Azure::Response<Models::DirectoryProperties> GetProperties(
        const GetDirectoryPropertiesOptions& options = GetDirectoryPropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets the properties of the directory.
     * @param smbProperties The SMB properties to be set to the directory.
     * @param options Optional parameters to set this directory's properties.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::SetDirectoryPropertiesResult> containing the
     * properties of the directory returned from the server.
     */
    Azure::Response<Models::SetDirectoryPropertiesResult> SetProperties(
        Models::FileSmbProperties smbProperties,
        const SetDirectoryPropertiesOptions& options = SetDirectoryPropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets the metadata of the directory.
     * @param metadata User-defined metadata to be stored with the directory. Note that the string
     *                 may only contain ASCII characters in the ISO-8859-1 character set.
     * @param options Optional parameters to set this directory's metadata.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::SetDirectoryMetadataResult> containing the
     * information of the directory returned from the server.
     */
    Azure::Response<Models::SetDirectoryMetadataResult> SetMetadata(
        Storage::Metadata metadata,
        const SetDirectoryMetadataOptions& options = SetDirectoryMetadataOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Returns a sequence of files and subdirectories in this directory. Enumerating the
     * files and directories may make multiple requests to the service while fetching all the
     * values.
     * @param options Optional parameters to list the files and directories under this directory.
     * @param context Context for cancelling long running operations.
     * @return ListFilesAndDirectoriesPagedResponse describing the items in the directory.
     */
    ListFilesAndDirectoriesPagedResponse ListFilesAndDirectories(
        const ListFilesAndDirectoriesOptions& options = ListFilesAndDirectoriesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Returns a sequence of the open handles on a directory or a file. Enumerating the
     * handles may make multiple requests to the service while fetching all the values.
     * @param options Optional parameters to list this directory's open handles.
     * @param context Context for cancelling long running operations.
     * @return ListDirectoryHandlesPagedResponse describing the handles in the directory.
     */
    ListDirectoryHandlesPagedResponse ListHandles(
        const ListDirectoryHandlesOptions& options = ListDirectoryHandlesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Closes a handle opened on a directory at the service.
     * @param handleId The ID of the handle to be closed.
     * @param options Optional parameters to close one of this directory's open handles.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::ForceCloseDirectoryHandleResult> containing the
     * information of the closed handle. Current empty but preserved for future usage.
     */
    Azure::Response<Models::ForceCloseDirectoryHandleResult> ForceCloseHandle(
        const std::string& handleId,
        const ForceCloseDirectoryHandleOptions& options = ForceCloseDirectoryHandleOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Closes all handles opened on a directory at the service. Optionally supports
     * recursively closing handles on subresources.
     * @param options Optional parameters to close all this directory's open handles.
     * @param context Context for cancelling long running operations.
     * @return ForceCloseAllDirectoryHandlesPagedResponse containing the information of the closed
     * handles.
     */
    ForceCloseAllDirectoryHandlesPagedResponse ForceCloseAllHandles(
        const ForceCloseAllDirectoryHandlesOptions& options
        = ForceCloseAllDirectoryHandlesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

  private:
    Azure::Core::Url m_shareDirectoryUrl;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
    Nullable<bool> m_allowTrailingDot;
    Nullable<bool> m_allowSourceTrailingDot;
    Nullable<Models::ShareTokenIntent> m_shareTokenIntent;

    explicit ShareDirectoryClient(
        Azure::Core::Url shareDirectoryUrl,
        std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> pipeline)
        : m_shareDirectoryUrl(std::move(shareDirectoryUrl)), m_pipeline(std::move(pipeline))
    {
    }

    friend class ShareClient;
  };
}}}} // namespace Azure::Storage::Files::Shares
