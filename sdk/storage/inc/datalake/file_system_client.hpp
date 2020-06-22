// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "common/storage_credential.hpp"
#include "common/storage_url_builder.hpp"
#include "datalake/service_client.hpp"
#include "datalake_options.hpp"
#include "http/pipeline.hpp"
#include "protocol/datalake_rest_client.hpp"

#include <memory>
#include <string>

namespace Azure { namespace Storage { namespace DataLake {

  class PathClient;

  struct FileSystemProperties
  {
    std::string Date;
    std::string ETag;
    std::string LastModified;
    std::string RequestId;
    std::string Version;
    std::map<std::string, std::string> Metadata;
    bool NamespaceEnabled;
  };

  typedef FileSystemSetPropertiesResponse FileSystemSetMetadataResponse;

  class FileSystemClient {
  public:
    /**
     * @brief Create from connection string
     * @param connectionString Azure Storage connection string.
     * @param fileSystemName The name of a file system.
     * @param options Optional parameters used to initialize the client.
     * @return FileSystemClient
     */
    static FileSystemClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& fileSystemName,
        const FileSystemClientOptions& options = FileSystemClientOptions());

    /**
     * @brief Shared key authentication client.
     * @param fileSystemUri The URI of the file system this client's request targets.
     * @param credential The shared key credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit FileSystemClient(
        const std::string& fileSystemUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const FileSystemClientOptions& options = FileSystemClientOptions());

    /**
     * @brief Bearer token authentication client.
     * @param fileSystemUri The URI of the file system this client's request targets.
     * @param credential The token credential used to initialize the client.
     * @param options Optional parameters used to initialize the client.
     */
    explicit FileSystemClient(
        const std::string& fileSystemUri,
        std::shared_ptr<TokenCredential> credential,
        const FileSystemClientOptions& options = FileSystemClientOptions());

    /**
     * @brief Anonymous/SAS/customized pipeline auth.
     * @param fileSystemUri The URI of the file system this client's request targets.
     * @param options Optional parameters used to initialize the client.
     */
    explicit FileSystemClient(
        const std::string& fileSystemUri,
        const FileSystemClientOptions& options = FileSystemClientOptions());

    /**
     * @brief Create a PathClient from current FileSystemClient
     * @param path Path of the resource within the file system.
     * @return PathClient
     */
    PathClient GetPathClient(const std::string& path) const;

    /**
     * @brief Creates the file system.
     * @param options Optional parameters to create this file system.
     * @return FileSystemCreateResponse
     */
    FileSystemCreateResponse Create(
        const FileSystemCreateOptions& options = FileSystemCreateOptions()) const;

    /**
     * @brief Deletes the file system.
     * @param options Optional parameters to delete this file system.
     * @return FileSystemDeleteResponse
     */
    FileSystemDeleteResponse Delete(
        const FileSystemDeleteOptions& options = FileSystemDeleteOptions()) const;

    /**
     * @brief Sets the metadata of file system.
     * @param metadata User-defined metadata to be stored with the filesystem. Note that the string
     *                 may only contain ASCII characters in the ISO-8859-1 character set.
     * @param options Optional parameters to set the metadata to this file system.
     * @return FileSystemSetMetadataResponse
     */
    FileSystemSetMetadataResponse SetMetadata(
        const std::map<std::string, std::string>& metadata,
        const FileSystemSetMetadataOptions& options = FileSystemSetMetadataOptions()) const;

    /**
     * @brief Gets the metadata of file system.
     * @param options Optional parameters to get the metadata of this file system.
     * @return std::map<std::string, std::string>
     */
    std::map<std::string, std::string> GetMetadata(
        const FileSystemGetMetadataOptions& options = FileSystemGetMetadataOptions()) const;

    /**
     * @brief Gets the properties of file system.
     * @param options Optional parameters to get the metadata of this file system.
     * @return FileSystemGetMetadataResponse
     */
    FileSystemProperties GetProperties(
        const FileSystemPropertiesOptions& options = FileSystemPropertiesOptions()) const;

    /**
     * @brief List the paths in this file system.
     * @param recursive If "true", all paths are listed; otherwise, only paths at the root of the
     *                  filesystem are listed. If "directory" is specified, the list will only
     *                  include paths that share the same root.
     * @param options Optional parameters to list the paths in file system.
     * @return FileSystemListPathsResponse
     */
    FileSystemListPathsResponse ListPaths(
        bool recursive,
        const ListPathsOptions& options = ListPathsOptions()) const;

  private:
    UrlBuilder m_dfsUri;
    UrlBuilder m_blobUri;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;

    explicit FileSystemClient(
        UrlBuilder dfsUri,
        UrlBuilder blobUri,
        std::shared_ptr<Azure::Core::Http::HttpPipeline> pipeline)
        : m_dfsUri(std::move(dfsUri)), m_blobUri(std::move(blobUri)),
          m_pipeline(std::move(pipeline))
    {
    }
    friend class ServiceClient;
  };
}}} // namespace Azure::Storage::DataLake
