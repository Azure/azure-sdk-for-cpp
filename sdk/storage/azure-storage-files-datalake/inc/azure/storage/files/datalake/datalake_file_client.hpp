// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <azure/core/credentials.hpp>
#include <azure/core/http/pipeline.hpp>
#include <azure/core/response.hpp>
#include <azure/storage/blobs/block_blob_client.hpp>
#include <azure/storage/common/storage_credential.hpp>

#include "azure/storage/files/datalake/datalake_options.hpp"
#include "azure/storage/files/datalake/datalake_path_client.hpp"
#include "azure/storage/files/datalake/datalake_responses.hpp"
#include "azure/storage/files/datalake/protocol/datalake_rest_client.hpp"

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  class DataLakeFileClient : public DataLakePathClient {
  public:
    /**
     * @brief Create from connection string
     * @param connectionString Azure Storage connection string.
     * @param fileSystemName The name of a file system.
     * @param filePath The path of a file within the file system.
     * @param options Optional parameters used to initialize the client.
     * @return DataLakeFileClient
     */
    static DataLakeFileClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& fileSystemName,
        const std::string& filePath,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Shared key authentication client.
     * @param fileUri The URI of the file this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeFileClient(
        const std::string& fileUri,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param fileUri The URI of the file this client's request targets.
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeFileClient(
        const std::string& fileUri,
        std::shared_ptr<Core::TokenCredential> credential,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param fileUri The URI of the file this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit DataLakeFileClient(
        const std::string& fileUri,
        const DataLakeClientOptions& options = DataLakeClientOptions());

    /**
     * @brief Gets the file's primary uri endpoint. This is the endpoint used for blob
     * storage available features in DataLake.
     *
     * @return The file's primary uri endpoint.
     */
    std::string GetUri() const { return m_blockBlobClient.GetUrl(); }

    /**
     * @brief Gets the file's primary uri endpoint. This is the endpoint used for dfs
     * endpoint only operations
     *
     * @return The file's primary uri endpoint.
     */
    std::string GetDfsUri() const { return m_dfsUri.GetAbsoluteUrl(); }

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
     * @return Azure::Core::Response<Models::AppendDataLakeFileDataResult> containing the
     * information returned when appending some data to the path.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::AppendDataLakeFileDataResult> AppendData(
        Azure::Core::Http::BodyStream* content,
        int64_t offset,
        const AppendDataLakeFileDataOptions& options = AppendDataLakeFileDataOptions()) const;

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
     * @return Azure::Core::Response<Models::FlushDataLakeFileDataResult> containing the information
     * returned when flushing the data appended to the path.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::FlushDataLakeFileDataResult> FlushData(
        int64_t endingOffset,
        const FlushDataLakeFileDataOptions& options = FlushDataLakeFileDataOptions()) const;

    /**
     * @brief Create a file. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param options Optional parameters to create the resource the path points to.
     * @return Azure::Core::Response<Models::CreateDataLakeFileResult> containing the information
     * returned when creating the file.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::CreateDataLakeFileResult> Create(
        const CreateDataLakeFileOptions& options = CreateDataLakeFileOptions()) const
    {
      return DataLakePathClient::Create(Models::PathResourceType::File, options);
    }

    /**
     * @brief Create a file. If it already exists, it will remain unchanged.
     * @param options Optional parameters to create the resource the path points to.
     * @return Azure::Core::Response<Models::CreateDataLakeFileResult> containing the information
     * returned when creating the file.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::CreateDataLakeFileResult> CreateIfNotExists(
        const CreateDataLakeFileOptions& options = CreateDataLakeFileOptions()) const
    {
      return DataLakePathClient::CreateIfNotExists(Models::PathResourceType::File, options);
    }

    /**
     * @brief Renames a file. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param destinationFilePath The path of the file this file is renaming to.
     * @param options Optional parameters to rename a resource to the resource the destination path
     * points to.
     * @return Azure::Core::Response<Models::RenameDataLakeDataLakeFileResult> containing the
     * information returned when renaming the file.
     * @remark This operation will not change the URL this file client points too, to use the
     *         new name, customer needs to initialize a new file client with the new name/path.
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::RenameDataLakeFileResult> Rename(
        const std::string& destinationFilePath,
        const RenameDataLakeFileOptions& options = RenameDataLakeFileOptions()) const;

    /**
     * @brief Deletes the file.
     * @param options Optional parameters to delete the file the path points to.
     * @return Azure::Core::Response<Models::DeleteDataLakeFileResult>
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::DeleteDataLakeFileResult> Delete(
        const DeleteDataLakeFileOptions& options = DeleteDataLakeFileOptions()) const;

    /**
     * @brief Deletes the file if it already exists.
     * @param options Optional parameters to delete the file the path points to.
     * @return Azure::Core::Response<Models::DeleteDataLakeFileResult>
     * @remark This request is sent to dfs endpoint.
     */
    Azure::Core::Response<Models::DeleteDataLakeFileResult> DeleteIfExists(
        const DeleteDataLakeFileOptions& options = DeleteDataLakeFileOptions()) const;

    /**
     * @brief Read the contents of a file. For read operations, range requests are supported.
     * @param options Optional parameters to read the content from the resource the path points to.
     * @return Azure::Core::Response<Models::ReadDataLakeFileResult> containing the information and
     * content returned when reading from a file.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::ReadDataLakeFileResult> Read(
        const ReadDataLakeFileOptions& options = ReadDataLakeFileOptions()) const;

    /**
     * @brief Creates a new file, or updates the content of an existing file. Updating
     * an existing file overwrites any existing metadata on the file.
     * @param buffer A memory buffer containing the content to upload.
     * @param bufferSize Size of the memory buffer.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<UploadDataLakeFileFromResult> containing the information
     * returned when uploading a file from a buffer.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::UploadDataLakeFileFromResult> UploadFrom(
        const uint8_t* buffer,
        std::size_t bufferSize,
        const UploadDataLakeFileFromOptions& options = UploadDataLakeFileFromOptions()) const;

    /**
     * @brief Creates a new file, or updates the content of an existing file. Updating
     * an existing file overwrites any existing metadata on the file.
     * @param fileName A file containing the content to upload.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<Models::UploadDataLakeFileFromResult> containing the
     * information returned when uploading a file from a local file.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::UploadDataLakeFileFromResult> UploadFrom(
        const std::string& fileName,
        const UploadDataLakeFileFromOptions& options = UploadDataLakeFileFromOptions()) const;

    /**
     * @brief Downloads a file or a file range from the service to a memory buffer using parallel
     * requests.
     * @param buffer A memory buffer to write the file content to.
     * @param bufferSize Size of the memory buffer. Size must be larger or equal to size of the file
     * or file range.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<Models::DownloadDataLakeFileToResult> containing the
     * information returned when downloading a file to a local buffer.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::DownloadDataLakeFileToResult> DownloadTo(
        uint8_t* buffer,
        std::size_t bufferSize,
        const DownloadDataLakeFileToOptions& options = DownloadDataLakeFileToOptions()) const;

    /**
     * @brief Downloads a file or a file range from the service to a file using parallel
     * requests.
     * @param fileName A file path to write the downloaded content to.
     * @param options Optional parameters to execute this function.
     * @return Azure::Core::Response<Models::DownloadDataLakeFileToResult> containing the
     * information returned when downloading a file to a local file.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::DownloadDataLakeFileToResult> DownloadTo(
        const std::string& fileName,
        const DownloadDataLakeFileToOptions& options = DownloadDataLakeFileToOptions()) const;

    /**
     * @brief Schedules the file for deletion.
     * @param expiryOrigin Specify the origin of expiry.
     * @param options Optional parameters to schedule the file for deletion.
     * @return Azure::Core::Response<Models::ScheduleDataLakeFileDeletionResult> containing the
     * information and content returned when schedule the file for deletion.
     * @remark This request is sent to blob endpoint.
     */
    Azure::Core::Response<Models::ScheduleDataLakeFileDeletionResult> ScheduleDeletion(
        ScheduleDataLakeFileExpiryOriginType expiryOrigin,
        const ScheduleDataLakeFileDeletionOptions& options
        = ScheduleDataLakeFileDeletionOptions()) const;

  private:
    Blobs::BlockBlobClient m_blockBlobClient;

    explicit DataLakeFileClient(
        Azure::Core::Http::Url dfsUri,
        Blobs::BlobClient blobClient,
        Blobs::BlockBlobClient blockBlobClient,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : DataLakePathClient(std::move(dfsUri), std::move(blobClient), pipeline),
          m_blockBlobClient(std::move(blockBlobClient))
    {
    }
    friend class DataLakeFileSystemClient;
    friend class DataLakeDirectoryClient;
  };
}}}} // namespace Azure::Storage::Files::DataLake
