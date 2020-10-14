// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/identity/client_secret_credential.hpp"
#include "azure/storage/blobs/blob_client.hpp"
#include "azure/storage/blobs/blob_options.hpp"
#include "azure/storage/blobs/protocol/blob_rest_client.hpp"
#include "azure/storage/common/storage_credential.hpp"

#include <string>

namespace Azure { namespace Storage { namespace Blobs {

  /**
   * @brief The AppendBlobClient allows you to manipulate Azure Storage append blobs.
   *
   * An append blob is comprised of blocks and is optimized for append operations. When you modify
   * an append blob, blocks are added to the end of the blob only, via the AppendBlock operation.
   * Updating or deleting of existing blocks is not supported. Unlike a block blob, an append blob
   * does not expose its block IDs.
   */
  class AppendBlobClient : public BlobClient {
  public:
    /**
     * @brief Initialize a new instance of AppendBlobClient.
     *
     * @param connectionString A connection string includes the authentication information required
     * for your application to access data in an Azure Storage account at runtime.
     * @param containerName The name of the container containing this blob.
     * @param blobName The name of this blob.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     * @return A new AppendBlobClient instance.
     */
    static AppendBlobClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& containerName,
        const std::string& blobName,
        const AppendBlobClientOptions& options = AppendBlobClientOptions());

    /**
     * @brief Initialize a new instance of AppendBlobClient.
     *
     * @param blobUri A uri
     * referencing the blob that includes the name of the account, the name of the container, and
     * the name of the blob.
     * @param credential The shared key credential used to sign
     * requests.
     * @param options Optional client options that define the transport pipeline
     * policies for authentication, retries, etc., that are applied to every request.
     */
    explicit AppendBlobClient(
        const std::string& blobUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const AppendBlobClientOptions& options = AppendBlobClientOptions());

    /**
     * @brief Initialize a new instance of AppendBlobClient.
     *
     * @param blobUri A uri
     * referencing the blob that includes the name of the account, the name of the container, and
     * the name of the blob.
     * @param credential The client secret credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit AppendBlobClient(
        const std::string& blobUri,
        std::shared_ptr<Identity::ClientSecretCredential> credential,
        const AppendBlobClientOptions& options = AppendBlobClientOptions());

    /**
     * @brief Initialize a new instance of AppendBlobClient.
     *
     * @param blobUri A uri
     * referencing the blob that includes the name of the account, the name of the container, and
     * the name of the blob, and possibly also a SAS token.
     * @param options Optional client
     * options that define the transport pipeline policies for authentication, retries, etc., that
     * are applied to every request.
     */
    explicit AppendBlobClient(
        const std::string& blobUri,
        const AppendBlobClientOptions& options = AppendBlobClientOptions());

    /**
     * @brief Initializes a new instance of the AppendBlobClient class with an identical uri
     * source but the specified snapshot timestamp.
     *
     * @param snapshot The snapshot
     * identifier.
     * @return A new AppendBlobClient instance.
     * @remarks Pass empty string to remove the snapshot returning the base blob.
     */
    AppendBlobClient WithSnapshot(const std::string& snapshot) const;

    /**
     * @brief Creates a clone of this instance that references a version ID rather than the base
     * blob.
     *
     * @param versionId The version ID returning a URL to the base blob.
     * @return A new AppendBlobClient instance.
     * @remarks Pass empty string to remove the version ID returning the base blob.
     */
    AppendBlobClient WithVersionId(const std::string& versionId) const;

    /**
     * @brief Creates a new 0-length append blob. The content of any existing blob is
     * overwritten with the newly initialized append blob.
     *
     * @param options Optional
     * parameters to execute this function.
     * @return A CreateAppendBlobResult describing the newly created append blob.
     */
    Azure::Core::Response<CreateAppendBlobResult> Create(
        const CreateAppendBlobOptions& options = CreateAppendBlobOptions()) const;

    /**
     * @brief Commits a new block of data, represented by the content BodyStream to the end
     * of the existing append blob.
     *
     * @param content A BodyStream containing the
     * content of the block to append.
     * @param options Optional parameters to execute this
     * function.
     * @return A AppendBlockResult describing the state of the updated append blob.
     */
    Azure::Core::Response<AppendBlockResult> AppendBlock(
        Azure::Core::Http::BodyStream* content,
        const AppendBlockOptions& options = AppendBlockOptions()) const;

    /**
     * @brief Commits a new block of data, represented by the content BodyStream to the end
     * of the existing append blob.
     *
     * @param sourceUri Specifies the uri of the source
     * blob. The value may be a uri of up to 2 KB in length that specifies a blob. The source blob
     * must either be public or must be authenticated via a shared access signature. If the source
     * blob is public, no authentication is required to perform the operation.
     * @param options
     * Optional parameters to execute this function.
     * @return A AppendBlockFromUriResult describing the state of the updated append blob.
     */
    Azure::Core::Response<AppendBlockFromUriResult> AppendBlockFromUri(
        const std::string& sourceUri,
        const AppendBlockFromUriOptions& options = AppendBlockFromUriOptions()) const;

    /**
     * @brief Seals the append blob, making it read only. Any subsequent appends will fail.
     *
     * @param options Optional parameters to execute this function.
     * @return A SealAppendBlobResult describing the state of the sealed append blob.
     */
    Azure::Core::Response<SealAppendBlobResult> Seal(
        const SealAppendBlobOptions& options = SealAppendBlobOptions()) const;

  private:
    explicit AppendBlobClient(BlobClient blobClient);
    friend class BlobClient;
  };

}}} // namespace Azure::Storage::Blobs
