// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/credentials/credentials.hpp"
#include "azure/core/http/pipeline.hpp"
#include "azure/core/response.hpp"
#include "azure/storage/common/storage_credential.hpp"
#include "azure/storage/files/shares/protocol/share_rest_client.hpp"
#include "azure/storage/files/shares/share_client.hpp"
#include "azure/storage/files/shares/share_directory_client.hpp"
#include "azure/storage/files/shares/share_options.hpp"
#include "azure/storage/files/shares/share_responses.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace Shares {

  class FileClient {
  public:
    /**
     * @brief Create A FileClient from connection string to manage a File Share File
     * resource.
     * @param connectionString Azure Storage connection string.
     * @param shareName The name of a file share.
     * @param filePath The path of a file.
     * @param options Optional parameters used to initialize the client.
     * @return ShareClient The client that can be used to manage a share resource.
     */
    static FileClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& shareName,
        const std::string& filePath,
        const FileClientOptions& options = FileClientOptions());

    /**
     * @brief Initialize a new instance of FileClient using shared key authentication.
     * @param shareFileUri The URI of the file this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit FileClient(
        const std::string& shareFileUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const FileClientOptions& options = FileClientOptions());

    /**
     * @brief Initialize a new instance of FileClient using token authentication.
     * @param shareFileUri The URI of the file this client's request targets.
     * @param credential The client secret credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit FileClient(
        const std::string& shareFileUri,
        std::shared_ptr<Core::Credentials::ClientSecretCredential> credential,
        const FileClientOptions& options = FileClientOptions());

    /**
     * @brief Initialize a new instance of FileClient using anonymous access or shared access
     * signature.
     * @param shareFileUri The URI of the file this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit FileClient(
        const std::string& shareFileUri,
        const FileClientOptions& options = FileClientOptions());

    /**
     * @brief Gets the file's primary uri endpoint.
     *
     * @return The file's primary uri endpoint.
     */
    std::string GetUri() const { return m_shareFileUri.GetAbsoluteUrl(); }

    /**
     * @brief Initializes a new instance of the FileClient class with an identical uri
     * source but the specified share snapshot timestamp.
     *
     * @param snapshot The snapshot identifier for the share snapshot.
     * @return A new FileClient instance.
     * @remarks Pass empty string to remove the snapshot returning the file client without
     * specifying the share snapshot.
     */
    FileClient WithShareSnapshot(const std::string& shareSnapshot) const;

    /**
     * @brief Creates the file.
     * @param fileSize Size of the file in bytes.
     * @param options Optional parameters to create this file.
     * @return Azure::Core::Response<CreateFileResult> containing the information returned when
     * creating the file.
     */
    Azure::Core::Response<CreateFileResult> Create(
        int64_t fileSize,
        const CreateFileOptions& options = CreateFileOptions()) const;

    /**
     * @brief Deletes the file.
     * @param options Optional parameters to delete this file.
     * @return Azure::Core::Response<DeleteFileResult> containing the information returned when
     * deleting the file.
     */
    Azure::Core::Response<DeleteFileResult> Delete(
        const DeleteFileOptions& options = DeleteFileOptions()) const;

    /**
     * @brief Open a stream for the file's content, or a range of the file's content that can be
     * used to download the server end data.
     * @param options Optional parameters to get the content of this file.
     * @return Azure::Core::Response<DownloadFileResult> containing the range or full content and
     * the information of the file.
     */
    Azure::Core::Response<DownloadFileResult> Download(
        const DownloadFileOptions& options = DownloadFileOptions()) const;

    /**
     * @brief Downloads a file or a file range from the service to a memory buffer using parallel
     * requests.
     *
     * @param buffer A memory buffer to write the file content to.
     * @param bufferSize Size of the memory buffer. Size must be larger or equal to size of the file
     * or file range.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<DownloadFileToResult> containing the information of the
     * downloaded file/file range.
     */
    Azure::Core::Response<DownloadFileToResult> DownloadTo(
        uint8_t* buffer,
        std::size_t bufferSize,
        const DownloadFileToOptions& options = DownloadFileToOptions()) const;

    /**
     * @brief Downloads a file or a file range from the service to a memory buffer using parallel
     * requests.
     *
     * @param file A file path to write the downloaded content to.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<DownloadFileToResult> containing the information of the
     * downloaded file/file range.
     */
    Azure::Core::Response<DownloadFileToResult> DownloadTo(
        const std::string& file,
        const DownloadFileToOptions& options = DownloadFileToOptions()) const;

    /**
     * @brief Creates a new file, or updates the content of an existing file. Updating
     * an existing file overwrites any existing metadata on the file.
     *
     * @param buffer A memory buffer containing the content to upload.
     * @param bufferSize Size of the memory buffer.
     * @param options Optional parameters to execute this function.
     * @return A UploadFilePagesResult describing the state of the updated file.
     */
    Azure::Core::Response<UploadFileFromResult> UploadFrom(
        const uint8_t* buffer,
        std::size_t bufferSize,
        const UploadFileFromOptions& options = UploadFileFromOptions()) const;

    /**
     * @brief Creates a new file, or updates the content of an existing file. Updating
     * an existing file overwrites any existing metadata on the file.
     *
     * @param file A file containing the content to upload.
     * @param options Optional parameters to execute this function.
     * @return A UploadFileFromResult describing the state of the updated file.
     */
    Azure::Core::Response<UploadFileFromResult> UploadFrom(
        const std::string& file,
        const UploadFileFromOptions& options = UploadFileFromOptions()) const;

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
     * @return Azure::Core::Response<StartCopyFileResult> containing the copy related information.
     */
    Azure::Core::Response<StartCopyFileResult> StartCopy(
        std::string copySource,
        const StartCopyFileOptions& options = StartCopyFileOptions()) const;

    /**
     * @brief Aborts copying the file specified with the copy ID.
     * @param copyId The copy identifier provided in the StartCopyFileResult of the original
     * StartCopy operation.
     * @param options Optional parameters to abort copying the content of this file.
     * @return Azure::Core::Response<AbortCopyFileResult> containing the abort copy related
     * information, current empty but preserved for future usage.
     */
    Azure::Core::Response<AbortCopyFileResult> AbortCopy(
        std::string copyId,
        const AbortCopyFileOptions& options = AbortCopyFileOptions()) const;

    /**
     * @brief Gets the properties of a file.
     * @param options Optional parameters to get the properties of this file.
     * @return Azure::Core::Response<GetFilePropertiesResult> containing the file properties.
     */
    Azure::Core::Response<GetFilePropertiesResult> GetProperties(
        const GetFilePropertiesOptions& options = GetFilePropertiesOptions()) const;

    /**
     * @brief Sets the properties of the file, or resize a file specifying NewSize in options.
     * @param httpHeaders The Http headers to be set to the file.
     * @param smbProperties The SMB properties to be set to the file.
     * @param options Optional parameters to set this file's properties.
     * @return Azure::Core::Response<SetFilePropertiesResult> containing the properties of the
     * file returned from the server.
     */
    Azure::Core::Response<SetFilePropertiesResult> SetProperties(
        FileShareHttpHeaders httpHeaders,
        FileShareSmbProperties smbProperties,
        const SetFilePropertiesOptions& options = SetFilePropertiesOptions()) const;

    /**
     * @brief Sets the metadata of the file.
     * @param metadata User-defined metadata to be stored with the file. Note that the string
     *                 may only contain ASCII characters in the ISO-8859-1 character set.
     * @param options Optional parameters to set this file's metadata.
     * @return Azure::Core::Response<SetFileMetadataResult> containing the information of the
     * file returned from the server.
     */
    Azure::Core::Response<SetFileMetadataResult> SetMetadata(
        const std::map<std::string, std::string>& metadata,
        const SetFileMetadataOptions& options = SetFileMetadataOptions()) const;

    /**
     * @brief Uploads some data to a range of the file.
     * @param offset Specifies the starting offset for the content to be written as a range.
     * @param content A BodyStream containing the content of the range to upload.
     * @return Azure::Core::Response<UploadFileRange> containing the information of the uploaded
     * range and the file returned from the server.
     */
    Azure::Core::Response<UploadFileRangeResult> UploadRange(
        int64_t offset,
        Azure::Core::Http::BodyStream* content,
        const UploadFileRangeOptions& options = UploadFileRangeOptions()) const;

    /**
     * @brief Clears some range of data within the file.
     * @param offset Specifies the starting offset for the content to be cleared within the file.
     * @param length Specifies the length for the content to be cleared within the file.
     * @return Azure::Core::Response<ClearFileRangeResult> containing the information of the cleared
     * range returned from the server.
     */
    Azure::Core::Response<ClearFileRangeResult> ClearRange(
        int64_t offset,
        int64_t length,
        const ClearFileRangeOptions& options = ClearFileRangeOptions()) const;

    /**
     * @brief Gets the list of valid range from the file within specified range.
     * @return Azure::Core::Response<GetFileRangeListResult> containing the valid ranges within the
     * file for the specified range.
     */
    Azure::Core::Response<GetFileRangeListResult> GetRangeList(
        const GetFileRangeListOptions& options = GetFileRangeListOptions()) const;

    /**
     * @brief List open handles on the file.
     * @param options Optional parameters to list this file's open handles.
     * @return Azure::Core::Response<ListFileHandlesSegmentResult> containing the information
     * of the operation and the open handles of this file
     */
    Azure::Core::Response<ListFileHandlesSegmentResult> ListHandlesSegment(
        const ListFileHandlesSegmentOptions& options = ListFileHandlesSegmentOptions()) const;

    /**
     * @brief Closes a handle opened on a file at the service.
     * @param handleId The ID of the handle to be closed.
     * @param options Optional parameters to close one of this file's open handles.
     * @return Azure::Core::Response<ForceCloseFileHandleResult> containing the information
     * of the closed handle. Current empty but preserved for future usage.
     */
    Azure::Core::Response<ForceCloseFileHandleResult> ForceCloseHandle(
        const std::string& handleId,
        const ForceCloseFileHandleOptions& options = ForceCloseFileHandleOptions()) const;

    /**
     * @brief Closes all handles opened on a file at the service.
     * @param options Optional parameters to close all this file's open handles.
     * @return Azure::Core::Response<ForceCloseAllFileHandlesResult> containing the information
     * of the closed handles
     * @remark This operation may return a marker showing that the operation can be continued.
     */
    Azure::Core::Response<ForceCloseAllFileHandlesResult> ForceCloseAllHandles(
        const ForceCloseAllFileHandlesOptions& options = ForceCloseAllFileHandlesOptions()) const;

    /**
     * @brief Acquires an infinite lease on the file.
     *
     * @param proposedLeaseId Proposed lease ID, in a GUID string format.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<AcquireFileLeaseResult> describing the lease.
     */
    Azure::Core::Response<AcquireFileLeaseResult> AcquireLease(
        const std::string& proposedLeaseId,
        const AcquireFileLeaseOptions& options = AcquireFileLeaseOptions()) const;

    /**
     * @brief Releases the file's previously-acquired lease.
     *
     * @param leaseId ID of the previously-acquired lease.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<ReleaseFileLeaseResult> describing the updated container.
     */
    Azure::Core::Response<ReleaseFileLeaseResult> ReleaseLease(
        const std::string& leaseId,
        const ReleaseFileLeaseOptions& options = ReleaseFileLeaseOptions()) const;

    /**
     * @brief Changes the lease of an active lease.
     *
     * @param leaseId ID of the previously-acquired lease.
     * @param proposedLeaseId Proposed lease ID, in a GUID string format.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<ChangeFileLeaseResult> describing the changed lease.
     */
    Azure::Core::Response<ChangeFileLeaseResult> ChangeLease(
        const std::string& leaseId,
        const std::string& proposedLeaseId,
        const ChangeFileLeaseOptions& options = ChangeFileLeaseOptions()) const;

    /**
     * @brief Breaks the previously-acquired lease.
     *
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<BreakFileLeaseResult> describing the broken lease.
     */
    Azure::Core::Response<BreakFileLeaseResult> BreakLease(
        const BreakFileLeaseOptions& options = BreakFileLeaseOptions()) const;

  private:
    Azure::Core::Http::Url m_shareFileUri;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    explicit FileClient(
        Azure::Core::Http::Url shareFileUri,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : m_shareFileUri(std::move(shareFileUri)), m_pipeline(std::move(pipeline))
    {
    }

    friend class ShareClient;
    friend class DirectoryClient;
  };
}}}} // namespace Azure::Storage::Files::Shares
