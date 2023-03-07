// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <string>

#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/files/shares/share_client.hpp"
#include "azure/storage/files/shares/share_directory_client.hpp"
#include "azure/storage/files/shares/share_options.hpp"
#include "azure/storage/files/shares/share_responses.hpp"

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  class ShareFileClient final {
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
     * @brief Initialize a new instance of ShareDirectoryClient using shared key authentication.
     * @param shareUrl The URL of the file this client's request targets.
     * @param credential The token credential used to sign requests.
     * @param options Optional parameters used to initialize the client.
     */
    explicit ShareFileClient(
        const std::string& shareFileUrl,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
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
     * @brief Gets the file's primary URL endpoint.
     *
     * @return The file's primary URL endpoint.
     */
    std::string GetUrl() const { return m_shareFileUrl.GetAbsoluteUrl(); }

    /**
     * @brief Initializes a new instance of the ShareFileClient class with an identical URL
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
     * @return Azure::Response<CreateFileResult> containing the information returned when
     * creating the file.
     */
    Azure::Response<Models::CreateFileResult> Create(
        int64_t fileSize,
        const CreateFileOptions& options = CreateFileOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes the file.
     * @param options Optional parameters to delete this file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<DeleteFileResult> containing the information returned when
     * deleting the file.
     */
    Azure::Response<Models::DeleteFileResult> Delete(
        const DeleteFileOptions& options = DeleteFileOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Deletes the file if it exists.
     * @param options Optional parameters to delete this file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<DeleteFileResult> containing the information returned when
     * deleting the file. Only valid when successfully deleted.
     */
    Azure::Response<Models::DeleteFileResult> DeleteIfExists(
        const DeleteFileOptions& options = DeleteFileOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Open a stream for the file's content, or a range of the file's content that can be
     * used to download the server end data.
     * @param options Optional parameters to get the content of this file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::DownloadFileResult> containing the range or full
     * content and the information of the file.
     */
    Azure::Response<Models::DownloadFileResult> Download(
        const DownloadFileOptions& options = DownloadFileOptions(),
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
     * @return Azure::Response<Models::DownloadFileToResult> containing the information
     * of the downloaded file/file range.
     */
    Azure::Response<Models::DownloadFileToResult> DownloadTo(
        uint8_t* buffer,
        size_t bufferSize,
        const DownloadFileToOptions& options = DownloadFileToOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Downloads a file or a file range from the service to a memory buffer using parallel
     * requests.
     *
     * @param fileName A file path to write the downloaded content to.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::DownloadFileToResult> containing the information
     * of the downloaded file/file range.
     */
    Azure::Response<Models::DownloadFileToResult> DownloadTo(
        const std::string& fileName,
        const DownloadFileToOptions& options = DownloadFileToOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Creates a new file, or updates the content of an existing file. Updating
     * an existing file overwrites any existing metadata on the file.
     *
     * @param buffer A memory buffer containing the content to upload.
     * @param bufferSize Size of the memory buffer.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::UploadFileFromResult> describing the state of the
     * updated file.
     */
    Azure::Response<Models::UploadFileFromResult> UploadFrom(
        const uint8_t* buffer,
        size_t bufferSize,
        const UploadFileFromOptions& options = UploadFileFromOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Creates a new file, or updates the content of an existing file. Updating
     * an existing file overwrites any existing metadata on the file.
     *
     * @param fileName A file containing the content to upload.
     * @param options Optional parameters to execute this function.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::UploadFileFromResult> describing the state of the
     * updated file.
     */
    Azure::Response<Models::UploadFileFromResult> UploadFrom(
        const std::string& fileName,
        const UploadFileFromOptions& options = UploadFileFromOptions(),
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
     * @return StartFileCopyOperation containing the copy related information.
     */
    StartFileCopyOperation StartCopy(
        std::string copySource,
        const StartFileCopyOptions& options = StartFileCopyOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Aborts copying the file specified with the copy ID.
     * @param copyId The copy identifier provided in the StartCopyShareFileResult of the original
     * StartCopy operation.
     * @param options Optional parameters to abort copying the content of this file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::AbortFileCopyResult> containing the abort copy
     * related information, current empty but preserved for future usage.
     */
    Azure::Response<Models::AbortFileCopyResult> AbortCopy(
        std::string copyId,
        const AbortFileCopyOptions& options = AbortFileCopyOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Gets the properties of a file.
     * @param options Optional parameters to get the properties of this file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::FileProperties> containing the file
     * properties.
     */
    Azure::Response<Models::FileProperties> GetProperties(
        const GetFilePropertiesOptions& options = GetFilePropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets the properties of the file, or resize a file specifying NewSize in options.
     * @param httpHeaders The HTTP headers to be set to the file.
     * @param smbProperties The SMB properties to be set to the file.
     * @param options Optional parameters to set this file's properties.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::SetFilePropertiesResult> containing the properties
     * of the file returned from the server.
     */
    Azure::Response<Models::SetFilePropertiesResult> SetProperties(
        const Models::FileHttpHeaders& httpHeaders,
        const Models::FileSmbProperties& smbProperties,
        const SetFilePropertiesOptions& options = SetFilePropertiesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Sets the metadata of the file.
     * @param metadata User-defined metadata to be stored with the file. Note that the string
     *                 may only contain ASCII characters in the ISO-8859-1 character set.
     * @param options Optional parameters to set this file's metadata.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::SetFileMetadataResult> containing the information
     * of the file returned from the server.
     */
    Azure::Response<Models::SetFileMetadataResult> SetMetadata(
        Storage::Metadata metadata,
        const SetFileMetadataOptions& options = SetFileMetadataOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Uploads some data to a range of the file.
     * @param offset Specifies the starting offset for the content to be written as a range.
     * @param content A BodyStream containing the content of the range to upload.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::UploadFileRange> containing the information of the
     * uploaded range and the file returned from the server.
     */
    Azure::Response<Models::UploadFileRangeResult> UploadRange(
        int64_t offset,
        Azure::Core::IO::BodyStream& content,
        const UploadFileRangeOptions& options = UploadFileRangeOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Clears some range of data within the file.
     * @param offset Specifies the starting offset for the content to be cleared within the file.
     * @param length Specifies the length for the content to be cleared within the file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::ClearFileRangeResult> containing the information
     * of the cleared range returned from the server.
     */
    Azure::Response<Models::ClearFileRangeResult> ClearRange(
        int64_t offset,
        int64_t length,
        const ClearFileRangeOptions& options = ClearFileRangeOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Gets the list of valid range from the file within specified range.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::GetFileRangeListResult> containing the valid
     * ranges within the file for the specified range.
     */
    Azure::Response<Models::GetFileRangeListResult> GetRangeList(
        const GetFileRangeListOptions& options = GetFileRangeListOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Gets the list of valid range from the file within specified range that have changed
     * since previousShareSnapshot was taken.
     * @param previousShareSnapshot Specifies the previous snapshot.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::GetFileRangeListResult> containing the valid
     * ranges within the file for the specified range.
     */
    Azure::Response<Models::GetFileRangeListResult> GetRangeListDiff(
        std::string previousShareSnapshot,
        const GetFileRangeListOptions& options = GetFileRangeListOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Returns a sequence of the open handles on a directory or a file. Enumerating the
     * handles may make multiple requests to the service while fetching all the values.
     * @param options Optional parameters to list this file's open handles.
     * @param context Context for cancelling long running operations.
     * @return ListFileHandlesPagedResponse describing the handles of the file.
     */
    ListFileHandlesPagedResponse ListHandles(
        const ListFileHandlesOptions& options = ListFileHandlesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Closes a handle opened on a file at the service.
     * @param handleId The ID of the handle to be closed.
     * @param options Optional parameters to close one of this file's open handles.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::ForceCloseFileHandleResult> containing the
     * information of the closed handle. Current empty but preserved for future usage.
     */
    Azure::Response<Models::ForceCloseFileHandleResult> ForceCloseHandle(
        const std::string& handleId,
        const ForceCloseFileHandleOptions& options = ForceCloseFileHandleOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Closes all handles opened on a file at the service.
     * @param options Optional parameters to close all this file's open handles.
     * @param context Context for cancelling long running operations.
     * @return ForceCloseAllFileHandlesPagedResponse containing the information of the closed
     * handles.
     */
    ForceCloseAllFileHandlesPagedResponse ForceCloseAllHandles(
        const ForceCloseAllFileHandlesOptions& options = ForceCloseAllFileHandlesOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

    /**
     * @brief Upload a range from the source URI to this file's specific range.
     * @param destinationOffset Specifies the starting offset for the content to be written.
     * @param sourceUri The source URI of the content to be uploaded.
     * @param sourceRange The source URI's range to be uploaded to file.
     * @param options Optional parameters to upload a range to file.
     * @param context Context for cancelling long running operations.
     * @return Azure::Response<Models::UploadFileRangeFromUriResult> containing the returned
     * information.
     */
    Azure::Response<Models::UploadFileRangeFromUriResult> UploadRangeFromUri(
        int64_t destinationOffset,
        const std::string& sourceUri,
        const Azure::Core::Http::HttpRange& sourceRange,
        const UploadFileRangeFromUriOptions& options = UploadFileRangeFromUriOptions(),
        const Azure::Core::Context& context = Azure::Core::Context()) const;

  private:
    Azure::Core::Url m_shareFileUrl;
    std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;
    Nullable<bool> m_allowTrailingDot;
    Nullable<bool> m_allowSourceTrailingDot;
    Nullable<Models::ShareTokenIntent> m_shareTokenIntent;

    explicit ShareFileClient(
        Azure::Core::Url shareFileUrl,
        std::shared_ptr<Azure::Core::Http::_internal::HttpPipeline> pipeline)
        : m_shareFileUrl(std::move(shareFileUrl)), m_pipeline(std::move(pipeline))
    {
    }

    friend class ShareClient;
    friend class ShareDirectoryClient;
    friend class ShareLeaseClient;
  };
}}}} // namespace Azure::Storage::Files::Shares
