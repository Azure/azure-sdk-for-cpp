// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_credential.hpp"
#include "common/storage_url_builder.hpp"
#include "datalake/path_client.hpp"
#include "datalake_options.hpp"
#include "http/pipeline.hpp"
#include "protocol/datalake_rest_client.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace DataLake {

  struct ReadPathResponse
  {
    std::unique_ptr<Azure::Core::Http::BodyStream> Body;
    std::string AcceptRanges;
    DataLakeHttpHeaders HttpHeaders;
    int64_t ContentLength = int64_t();
    Azure::Core::Nullable<int64_t> RangeOffset;
    Azure::Core::Nullable<int64_t> RangeLength;
    Azure::Core::Nullable<std::string> TransactionalMD5;
    std::string Date;
    std::string ETag;
    std::string LastModified;
    std::string RequestId;
    std::string Version;
    std::string ResourceType;
    Azure::Core::Nullable<std::string> LeaseDuration;
    LeaseStateType LeaseState;
    LeaseStatusType LeaseStatus;
    Azure::Core::Nullable<std::string> ContentMD5;
    std::map<std::string, std::string> Metadata;
  };

  struct FileRenameResponse
  {
    std::string Date;
    Azure::Core::Nullable<std::string> ETag;
    Azure::Core::Nullable<std::string> LastModified;
    std::string RequestId;
    std::string Version;
  };

  struct FileDeleteResponse
  {
    std::string Date;
    std::string RequestId;
    std::string Version;
  };

  using FileInfo = PathInfo;
  using FileCreateOptions = PathCreateOptions;

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
     * @param pathUri The URI of the file this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit FileClient(
        const std::string& filePathUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const FileClientOptions& options = FileClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param pathUri The URI of the file this client's request targets.
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit FileClient(
        const std::string& filePathUri,
        std::shared_ptr<TokenCredential> credential,
        const FileClientOptions& options = FileClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param pathUri The URI of the file this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit FileClient(
        const std::string& filePathUri,
        const FileClientOptions& options = FileClientOptions());

    /**
     * @brief Gets the file's primary uri endpoint. This is the endpoint used for blob
     * service interop.
     *
     * @return The file's primary uri endpoint.
     */
    std::string GetUri() const { return m_blobUri.ToString(); }

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
     * @return PathAppendDataResponse
     */
    PathAppendDataResponse AppendData(
        Azure::Core::Http::BodyStream* content,
        int64_t offset,
        const PathAppendDataOptions& options = PathAppendDataOptions()) const;

    /**
     * @brief Flushes previous uploaded data to a file.
     * @param endOffset This parameter allows the caller to upload data in parallel and control
     *                 the order in which it is appended to the file.
     *                 The value must be the offset where the data is to be appended.
     *                 Uploaded data is not immediately flushed, or written, to the file. To flush,
     *                 the previously uploaded data must be contiguous, the offset parameter must
     *                 be specified and equal to the length of the file after all data has been
     *                 written, and there must not be a request entity body included with the
     *                 request.
     * @param options Optional parameters to flush data to the resource the path points to.
     * @return PathFlushDataResponse
     */
    PathFlushDataResponse FlushData(
        int64_t endOffset,
        const PathFlushDataOptions& options = PathFlushDataOptions()) const;

    /**
     * @brief Create a file. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param options Optional parameters to create the resource the path points to.
     * @return PathInfo
     */
    FileInfo Create(const FileCreateOptions& options = FileCreateOptions()) const
    {
      return PathClient::Create(PathResourceType::File, options);
    }

    /**
     * @brief Renames a file. By default, the destination is overwritten and
     *        if the destination already exists and has a lease the lease is broken.
     * @param destinationFilePath The path of the file this file is renaming to.
     * @param options Optional parameters to rename a resource to the resource the destination path
     * points to.
     * @return FileRenameResponse
     * @remark This will change the URL the client is pointing to.
     */
    FileRenameResponse Rename(
        const std::string& destinationFilePath,
        const FileRenameOptions& options = FileRenameOptions());

    /**
     * @brief Deletes the file.
     * @param options Optional parameters to delete the file the path points to.
     * @return PathDeleteResponse
     */
    FileDeleteResponse Delete(const FileDeleteOptions& options = FileDeleteOptions()) const;

    /**
     * @brief Read the contents of a file. For read operations, range requests are supported.
     * @param options Optional parameters to read the content from the resource the path points to.
     * @return ReadPathResponse
     */
    ReadPathResponse Read(const FileReadOptions& options = FileReadOptions()) const;

  private:
    explicit FileClient(
        UrlBuilder dfsUri,
        UrlBuilder blobUri,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : PathClient(dfsUri, blobUri, pipeline)
    {
    }
    friend class FileSystemClient;
  };
}}}} // namespace Azure::Storage::Files::DataLake
