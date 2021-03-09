// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/internal/http/pipeline.hpp>
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
     * @brief Initialize a new instance of ShareDirectoryClient using anonymous access or shared
     * access signature.
     * @param shareDirectoryUrl The URL of the directory this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareDirectoryClient(
        const std::string& shareDirectoryUrl,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Gets the directory's primary url endpoint.
     *
     * @return The directory's primary url endpoint.
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
     * @brief Initializes a new instance of the ShareDirectoryClient class with an identical url
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
     * @return Azure::Response<Models::CreateShareDirectoryResult> containing the information
     * returned when creating the directory.
     */
    Azure::Response<Models::CreateShareDirectoryResult> Create(
        const CreateShareDirectoryOptions& options = CreateShareDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Creates the directory if it does not exist.
     * @param options Optional parameters to create this directory.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::CreateShareDirectoryResult> containing the information
     * returned when creating the directory if successfully created.
     */
    Azure::Response<Models::CreateShareDirectoryResult> CreateIfNotExists(
        const CreateShareDirectoryOptions& options = CreateShareDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes the directory.
     * @param options Optional parameters to delete this directory.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::DeleteShareDirectoryResult> containing the information
     * returned when deleting the directory. Currently empty but preserved for future usage.
     */
    Azure::Response<Models::DeleteShareDirectoryResult> Delete(
        const DeleteShareDirectoryOptions& options = DeleteShareDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes the directory if it exists.
     * @param options Optional parameters to delete this directory.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::DeleteShareDirectoryResult> containing the information
     * returned when deleting the directory. Currently empty but preserved for future usage.
     * Only when the delete operation if successful, the returned information other than 'Deleted'
     * is valid.
     */
    Azure::Response<Models::DeleteShareDirectoryResult> DeleteIfExists(
        const DeleteShareDirectoryOptions& options = DeleteShareDirectoryOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Gets the properties of the directory.
     * @param options Optional parameters to get this directory's properties.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::GetShareDirectoryPropertiesResult> containing the
     * properties of the directory returned from the server.
     */
    Azure::Response<Models::GetShareDirectoryPropertiesResult> GetProperties(
        const GetShareDirectoryPropertiesOptions& options = GetShareDirectoryPropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets the properties of the directory.
     * @param smbProperties The SMB properties to be set to the directory.
     * @param options Optional parameters to set this directory's properties.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::SetShareDirectoryPropertiesResult> containing the
     * properties of the directory returned from the server.
     */
    Azure::Response<Models::SetShareDirectoryPropertiesResult> SetProperties(
        Models::FileSmbProperties smbProperties,
        const SetShareDirectoryPropertiesOptions& options = SetShareDirectoryPropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets the metadata of the directory.
     * @param metadata User-defined metadata to be stored with the directory. Note that the string
     *                 may only contain ASCII characters in the ISO-8859-1 character set.
     * @param options Optional parameters to set this directory's metadata.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::SetShareDirectoryMetadataResult> containing the
     * information of the directory returned from the server.
     */
    Azure::Response<Models::SetShareDirectoryMetadataResult> SetMetadata(
        Storage::Metadata metadata,
        const SetShareDirectoryMetadataOptions& options = SetShareDirectoryMetadataOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief List files and directories under the directory.
     * @param options Optional parameters to list the files and directories under this directory.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::ListFilesAndDirectoriesSinglePageResult> containing the
     * information of the operation, directory, share and the listed result.
     */
    Azure::Response<Models::ListFilesAndDirectoriesSinglePageResult>
    ListFilesAndDirectoriesSinglePage(
        const ListFilesAndDirectoriesSinglePageOptions& options
        = ListFilesAndDirectoriesSinglePageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief List open handles on the directory.
     * @param options Optional parameters to list this directory's open handles.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::ListShareDirectoryHandlesSinglePageResult> containing
     * the information of the operation and the open handles of this directory
     */
    Azure::Response<Models::ListShareDirectoryHandlesSinglePageResult> ListHandlesSinglePage(
        const ListShareDirectoryHandlesSinglePageOptions& options
        = ListShareDirectoryHandlesSinglePageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Closes a handle opened on a directory at the service.
     * @param handleId The ID of the handle to be closed.
     * @param options Optional parameters to close one of this directory's open handles.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::ForceCloseShareDirectoryHandleResult> containing the
     * information of the closed handle. Current empty but preserved for future usage.
     */
    Azure::Response<Models::ForceCloseShareDirectoryHandleResult> ForceCloseHandle(
        const std::string& handleId,
        const ForceCloseShareDirectoryHandleOptions& options
        = ForceCloseShareDirectoryHandleOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Closes all handles opened on a directory at the service.
     * @param options Optional parameters to close all this directory's open handles.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::ForceCloseAllShareDirectoryHandlesSinglePageResult>
     * containing the information of the closed handles
     * @remark This operation may return a marker showing that the operation can be continued.
     */
    Azure::Response<Models::ForceCloseAllShareDirectoryHandlesSinglePageResult>
    ForceCloseAllHandlesSinglePage(
        const ForceCloseAllShareDirectoryHandlesSinglePageOptions& options
        = ForceCloseAllShareDirectoryHandlesSinglePageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

  private:
    Azure::Core::Http::Url m_shareDirectoryUrl;
    std::shared_ptr<Azure::Core::Http::Internal::HttpPipeline> m_pipeline;

    explicit ShareDirectoryClient(
        Azure::Core::Http::Url shareDirectoryUrl,
        std::shared_ptr<Azure::Core::Http::Internal::HttpPipeline> pipeline)
        : m_shareDirectoryUrl(std::move(shareDirectoryUrl)), m_pipeline(std::move(pipeline))
    {
    }

    friend class ShareClient;
  };
}}}} // namespace Azure::Storage::Files::Shares
