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
#include "azure/storage/files/shares/share_directory_client.hpp"
#include "azure/storage/files/shares/share_options.hpp"
#include "azure/storage/files/shares/share_responses.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  class ShareFileClient {
  public:
    /**
     * @brief Create A ShareFileClient from connection string to manage a File Share File
     * resource.
     * @param connectionString Azure Storage connection string.
     * @param shareName The name of a file share.
     * @param fileName The name of a file.
     * @param options Optional parameters used to initialize the client.
     * @return ShareClient The client that can be used to manage a share resource.
     */
    static ShareFileClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& shareName,
        const std::string& fileName,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Initialize a new instance of ShareFileClient using shared key authentication.
     * @param shareFileUrl The URL of the file this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareFileClient(
        const std::string& shareFileUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Initialize a new instance of ShareFileClient using anonymous access or shared access
     * signature.
     * @param shareFileUrl The URL of the file this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareFileClient(
        const std::string& shareFileUrl,
        const ShareClientOptions& options = ShareClientOptions());

    /**
     * @brief Gets the file's primary url endpoint.
     *
     * @return The file's primary url endpoint.
     */
    std::string GetUrl() const { return m_shareFileUrl.GetAbsoluteUrl(); }

    /**
     * @brief Initializes a new instance of the ShareFileClient class with an identical url
     * source but the specified share snapshot timestamp.
     *
     * @param shareSnapshot The snapshot identifier for the share snapshot.
     * @return A new ShareFileClient instance.
     * @remarks Pass empty string to remove the snapshot returning the file client without
     * specifying the share snapshot.
     */
    ShareFileClient WithShareSnapshot(const std::string& shareSnapshot) const;

    /**
     * @brief Creates the file.
     * @param fileSize Size of the file in bytes.
     * @param options Optional parameters to create this file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<CreateShareFileResult> containing the information returned when
     * creating the file.
     */
    Azure::Core::Response<Models::CreateShareFileResult> Create(
        int64_t fileSize,
        const CreateShareFileOptions& options = CreateShareFileOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes the file.
     * @param options Optional parameters to delete this file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<DeleteShareFileResult> containing the information returned when
     * deleting the file.
     */
    Azure::Core::Response<Models::DeleteShareFileResult> Delete(
        const DeleteShareFileOptions& options = DeleteShareFileOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes the file if it exists.
     * @param options Optional parameters to delete this file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<DeleteShareFileResult> containing the information returned when
     * deleting the file. Only valid when successfully deleted.
     */
    Azure::Core::Response<Models::DeleteShareFileResult> DeleteIfExists(
        const DeleteShareFileOptions& options = DeleteShareFileOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Open a stream for the file's content, or a range of the file's content that can be
     * used to download the server end data.
     * @param options Optional parameters to get the content of this file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::DownloadShareFileResult> containing the range or full
     * content and the information of the file.
     */
    Azure::Core::Response<Models::DownloadShareFileResult> Download(
        const DownloadShareFileOptions& options = DownloadShareFileOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Downloads a file or a file range from the service to a memory buffer using parallel
     * requests.
     *
     * @param buffer A memory buffer to write the file content to.
     * @param bufferSize Size of the memory buffer. Size must be larger or equal to size of the file
     * or file range.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::DownloadShareFileToResult> containing the information
     * of the downloaded file/file range.
     */
    Azure::Core::Response<Models::DownloadShareFileToResult> DownloadTo(
        uint8_t* buffer,
        std::size_t bufferSize,
        const DownloadShareFileToOptions& options = DownloadShareFileToOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Downloads a file or a file range from the service to a memory buffer using parallel
     * requests.
     *
     * @param fileName A file path to write the downloaded content to.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::DownloadShareFileToResult> containing the information
     * of the downloaded file/file range.
     */
    Azure::Core::Response<Models::DownloadShareFileToResult> DownloadTo(
        const std::string& fileName,
        const DownloadShareFileToOptions& options = DownloadShareFileToOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Creates a new file, or updates the content of an existing file. Updating
     * an existing file overwrites any existing metadata on the file.
     *
     * @param buffer A memory buffer containing the content to upload.
     * @param bufferSize Size of the memory buffer.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::UploadShareFileFromResult> describing the state of the
     * updated file.
     */
    Azure::Core::Response<Models::UploadShareFileFromResult> UploadFrom(
        const uint8_t* buffer,
        std::size_t bufferSize,
        const UploadShareFileFromOptions& options = UploadShareFileFromOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Creates a new file, or updates the content of an existing file. Updating
     * an existing file overwrites any existing metadata on the file.
     *
     * @param fileName A file containing the content to upload.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::UploadShareFileFromResult> describing the state of the
     * updated file.
     */
    Azure::Core::Response<Models::UploadShareFileFromResult> UploadFrom(
        const std::string& fileName,
        const UploadShareFileFromOptions& options = UploadShareFileFromOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Starts copy the file specified from source URI to the file this client points to.
     * @param copySource Specifies the URL of the source file or file, up to 2 KB in length. To copy
     * a file to another file within the same storage account, you may use Shared Key to
     * authenticate the source file. If you are copying a file from another storage account, or if
     * you are copying a file from the same storage account or another storage account, then you
     * must authenticate the source file or file using a shared access signature. If the source is a
     * public file, no authentication is required to perform the copy operation. A file in a share
     * snapshot can also be specified as a copy source.
     * @param options Optional parameters to copy the content of this file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::StartCopyShareFileResult> containing the copy related
     * information.
     */
    Azure::Core::Response<Models::StartCopyShareFileResult> StartCopy(
        std::string copySource,
        const StartCopyShareFileOptions& options = StartCopyShareFileOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Aborts copying the file specified with the copy ID.
     * @param copyId The copy identifier provided in the StartCopyShareFileResult of the original
     * StartCopy operation.
     * @param options Optional parameters to abort copying the content of this file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::AbortCopyShareFileResult> containing the abort copy
     * related information, current empty but preserved for future usage.
     */
    Azure::Core::Response<Models::AbortCopyShareFileResult> AbortCopy(
        std::string copyId,
        const AbortCopyShareFileOptions& options = AbortCopyShareFileOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Gets the properties of a file.
     * @param options Optional parameters to get the properties of this file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::GetShareFilePropertiesResult> containing the file
     * properties.
     */
    Azure::Core::Response<Models::GetShareFilePropertiesResult> GetProperties(
        const GetShareFilePropertiesOptions& options = GetShareFilePropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets the properties of the file, or resize a file specifying NewSize in options.
     * @param httpHeaders The Http headers to be set to the file.
     * @param smbProperties The SMB properties to be set to the file.
     * @param options Optional parameters to set this file's properties.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::SetShareFilePropertiesResult> containing the properties
     * of the file returned from the server.
     */
    Azure::Core::Response<Models::SetShareFilePropertiesResult> SetProperties(
        const Models::FileHttpHeaders& httpHeaders,
        const Models::FileSmbProperties& smbProperties,
        const SetShareFilePropertiesOptions& options = SetShareFilePropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets the metadata of the file.
     * @param metadata User-defined metadata to be stored with the file. Note that the string
     *                 may only contain ASCII characters in the ISO-8859-1 character set.
     * @param options Optional parameters to set this file's metadata.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::SetShareFileMetadataResult> containing the information
     * of the file returned from the server.
     */
    Azure::Core::Response<Models::SetShareFileMetadataResult> SetMetadata(
        Storage::Metadata metadata,
        const SetShareFileMetadataOptions& options = SetShareFileMetadataOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Uploads some data to a range of the file.
     * @param offset Specifies the starting offset for the content to be written as a range.
     * @param content A BodyStream containing the content of the range to upload.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::UploadFileRange> containing the information of the
     * uploaded range and the file returned from the server.
     */
    Azure::Core::Response<Models::UploadShareFileRangeResult> UploadRange(
        int64_t offset,
        Azure::Core::Http::BodyStream* content,
        const UploadShareFileRangeOptions& options = UploadShareFileRangeOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Clears some range of data within the file.
     * @param offset Specifies the starting offset for the content to be cleared within the file.
     * @param length Specifies the length for the content to be cleared within the file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::ClearShareFileRangeResult> containing the information
     * of the cleared range returned from the server.
     */
    Azure::Core::Response<Models::ClearShareFileRangeResult> ClearRange(
        int64_t offset,
        int64_t length,
        const ClearShareFileRangeOptions& options = ClearShareFileRangeOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Gets the list of valid range from the file within specified range.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::GetShareFileRangeListResult> containing the valid
     * ranges within the file for the specified range.
     */
    Azure::Core::Response<Models::GetShareFileRangeListResult> GetRangeList(
        const GetShareFileRangeListOptions& options = GetShareFileRangeListOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief List open handles on the file.
     * @param options Optional parameters to list this file's open handles.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::ListShareFileHandlesSinglePageResult> containing the
     * information of the operation and the open handles of this file
     */
    Azure::Core::Response<Models::ListShareFileHandlesSinglePageResult> ListHandlesSinglePage(
        const ListShareFileHandlesSinglePageOptions& options
        = ListShareFileHandlesSinglePageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Closes a handle opened on a file at the service.
     * @param handleId The ID of the handle to be closed.
     * @param options Optional parameters to close one of this file's open handles.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::ForceCloseShareFileHandleResult> containing the
     * information of the closed handle. Current empty but preserved for future usage.
     */
    Azure::Core::Response<Models::ForceCloseShareFileHandleResult> ForceCloseHandle(
        const std::string& handleId,
        const ForceCloseShareFileHandleOptions& options = ForceCloseShareFileHandleOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Closes all handles opened on a file at the service.
     * @param options Optional parameters to close all this file's open handles.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::ForceCloseAllShareFileHandlesSinglePageResult>
     * containing the information of the closed handles
     * @remark This operation may return a marker showing that the operation can be continued.
     */
    Azure::Core::Response<Models::ForceCloseAllShareFileHandlesSinglePageResult>
    ForceCloseAllHandlesSinglePage(
        const ForceCloseAllShareFileHandlesSinglePageOptions& options
        = ForceCloseAllShareFileHandlesSinglePageOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Upload a range from the source URI to this file's specific range.
     * @param destinationOffset Specifies the starting offset for the content to be written.
     * @param sourceUri The source URI of the content to be uploaded.
     * @param sourceRange The source URI's range to be uploaded to file.
     * @param options Optional parameters to upload a range to file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Core::Response<Models::UploadFileRangeFromUriResult> containing the returned
     * information.
     */
    Azure::Core::Response<Models::UploadFileRangeFromUriResult> UploadRangeFromUri(
        int64_t destinationOffset,
        const std::string& sourceUri,
        const Azure::Core::Http::Range& sourceRange,
        const UploadFileRangeFromUriOptions& options = UploadFileRangeFromUriOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

  private:
    Azure::Core::Http::Url m_shareFileUrl;
    std::shared_ptr<Azure::Core::Internal::Http::HttpPipeline> m_pipeline;

    explicit ShareFileClient(
        Azure::Core::Http::Url shareFileUrl,
        std::shared_ptr<Azure::Core::Internal::Http::HttpPipeline> pipeline)
        : m_shareFileUrl(std::move(shareFileUrl)), m_pipeline(std::move(pipeline))
    {
    }

    friend class ShareClient;
    friend class ShareDirectoryClient;
    friend class ShareLeaseClient;
  };
}}}} // namespace Azure::Storage::Files::Shares
