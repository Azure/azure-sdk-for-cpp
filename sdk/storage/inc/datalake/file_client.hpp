// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "blobs/block_blob_client.hpp"
#include "common/storage_credential.hpp"
#include "common/storage_uri_builder.hpp"
#include "credentials/credentials.hpp"
#include "datalake/path_client.hpp"
#include "datalake_options.hpp"
#include "datalake_responses.hpp"
#include "http/pipeline.hpp"
#include "protocol/datalake_rest_client.hpp"
#include "response.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  class FileClient : public PathClient {
  public:
    /**
     * @brief Create from connection string
     * @param connectionString Azure Storage connection string.
     * @param fileSystemName The name of a file system.
     * @param filePath The path of a file within the file system.
     * @param options Optional parameters used to initialize the client.
     * @return FileClient
     */
    static FileClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& fileSystemName,
        const std::string& filePath,
        const FileClientOptions& options = FileClientOptions());

    /**
     * @brief Shared key authentication client.
     * @param fileUri The URI of the file this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit FileClient(
        const std::string& fileUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const FileClientOptions& options = FileClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param fileUri The URI of the file this client's request targets.
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit FileClient(
        const std::string& fileUri,
        std::shared_ptr<Core::Credentials::TokenCredential> credential,
        const FileClientOptions& options = FileClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param fileUri The URI of the file this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit FileClient(
        const std::string& fileUri,
        const FileClientOptions& options = FileClientOptions());

    /**
     * @brief Gets the file's primary uri endpoint. This is the endpoint used for blob
     * storage available features in DataLake.
     *
     * @return The file's primary uri endpoint.
     */
    std::string GetUri() const { return m_blockBlobClient.GetUri(); }

    /**
     * @brief Gets the file's primary uri endpoint. This is the endpoint used for dfs
     * endpoint only operations
     *
     * @return The file's primary uri endpoint.
     */
    std::string GetDfsUri() const { return m_dfsUri.ToString(); }

    /**
     * @brief Uploads data to be appended to a file. Data can only be appended to a file.
     * @param content The data to be appended.
     * @param offset This parameter allows the caller to upload data in parallel and control
     *                 the order in which it is appended to the file.
     *                 The value must be the offset where the data is to be appended.
     *                 Uploaded data is not immediately flushed, or written, to the file. To flush,
     *                 the previously uploaded data must be contiguous, the offset parameter must
     *                 be specified and equal to the length of the file after all data has been
     *                 written, and there must not be a request entity body included with the
     *                 request.
     * @param options Optional parameters to append data to the resource the path points to.
     * @return Azure::Core::Response<PathAppendDataInfo>
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<PathAppendDataInfo> AppendData(
        Azure::Core::Http::BodyStream* content,
        int64_t offset,
        const PathAppendDataOptions& options = PathAppendDataOptions()) const;

    /**
     * @brief Flushes previous uploaded data to a file.
     * @param endingOffset This parameter allows the caller to upload data in parallel and control
     *                 the order in which it is appended to the file.
     *                 The value must be the offset where the data is to be appended.
     *                 Uploaded data is not immediately flushed, or written, to the file. To flush,
     *                 the previously uploaded data must be contiguous, the offset parameter must
     *                 be specified and equal to the length of the file after all data has been
     *                 written, and there must not be a request entity body included with the
     *                 request.
     * @param options Optional parameters to flush data to the resource the path points to.
     * @return Azure::Core::Response<PathFlushDataInfo>
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<PathFlushDataInfo> FlushData(
        int64_t endingOffset,
        const PathFlushDataOptions& options = PathFlushDataOptions()) const;

    /**
     * @brief Create a file. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param options Optional parameters to create the resource the path points to.
     * @return Azure::Core::Response<FileInfo>
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<FileInfo> Create(
        const FileCreateOptions& options = FileCreateOptions()) const
    {
      return PathClient::Create(PathResourceType::File, options);
    }

    /**
     * @brief Renames a file. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param destinationFilePath The path of the file this file is renaming to.
     * @param options Optional parameters to rename a resource to the resource the destination path
     * points to.
     * @return Azure::Core::Response<FileRenameInfo>
     * @remark This operation will not change the URL this file client points too, to use the
     *         new name, customer needs to initialize a new file client with the new name/path.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<FileRenameInfo> Rename(
        const std::string& destinationFilePath,
        const FileRenameOptions& options = FileRenameOptions()) const;

    /**
     * @brief Deletes the file.
     * @param options Optional parameters to delete the file the path points to.
     * @return Azure::Core::Response<FileDeleteInfo>
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<FileDeleteInfo> Delete(
        const FileDeleteOptions& options = FileDeleteOptions()) const;

    /**
     * @brief Read the contents of a file. For read operations, range requests are supported.
     * @param options Optional parameters to read the content from the resource the path points to.
     * @return Azure::Core::Response<FileReadInfo>
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<FileReadInfo> Read(
        const FileReadOptions& options = FileReadOptions()) const;

    /**
     * @brief Creates a new file, or updates the content of an existing file. Updating
     * an existing file overwrites any existing metadata on the file.
     * @param buffer A memory buffer containing the content to upload.
     * @param bufferSize Size of the memory buffer.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<FileContentInfo>
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<FileContentInfo> UploadFromBuffer(
        const uint8_t* buffer,
        std::size_t bufferSize,
        const UploadFileOptions& options = UploadFileOptions()) const;

    /**
     * @brief Creates a new file, or updates the content of an existing file. Updating
     * an existing file overwrites any existing metadata on the file.
     * @param file A file containing the content to upload.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<FileContentInfo>
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<FileContentInfo> UploadFromFile(
        const std::string& file,
        const UploadFileOptions& options = UploadFileOptions()) const;

    /**
     * @brief Downloads a file or a file range from the service to a memory buffer using parallel
     * requests.
     * @param buffer A memory buffer to write the file content to.
     * @param bufferSize Size of the memory buffer. Size must be larger or equal to size of the file
     * or file range.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<FileDownloadInfo>
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<FileDownloadInfo> DownloadToBuffer(
        uint8_t* buffer,
        std::size_t bufferSize,
        const DownloadFileOptions& options = DownloadFileOptions()) const;

    /**
     * @brief Downloads a file or a file range from the service to a file using parallel
     * requests.
     * @param file A file path to write the downloaded content to.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<FileDownloadInfo>
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<FileDownloadInfo> DownloadToFile(
        const std::string& file,
        const DownloadFileOptions& options = DownloadFileOptions()) const;

  private:
    Blobs::BlockBlobClient m_blockBlobClient;

    explicit FileClient(
        UriBuilder dfsUri,
        Blobs::BlobClient blobClient,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : PathClient(std::move(dfsUri), std::move(blobClient), pipeline),
          m_blockBlobClient(blobClient.GetBlockBlobClient())
    {
    }
    friend class FileSystemClient;
  };
}}}} // namespace Azure::Storage::Files::DataLake
