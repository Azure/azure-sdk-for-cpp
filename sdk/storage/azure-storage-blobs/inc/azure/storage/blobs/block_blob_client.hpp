// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/identity/client_secret_credential.hpp"
#include "azure/storage/blobs/blob_client.hpp"
#include "azure/storage/blobs/blob_options.hpp"
#include "azure/storage/blobs/protocol/blob_rest_client.hpp"
#include "azure/storage/common/storage_credential.hpp"

#include <map>
#include <string>

namespace Azure { namespace Storage { namespace Files { namespace DataLake {
  class FileClient;
}}}} // namespace Azure::Storage::Files::DataLake

namespace Azure { namespace Storage { namespace Blobs {

  /**
   * @brief The BlockBlobClient allows you to manipulate Azure Storage block blobs.
   *
   * Block blobs let you upload large blobs efficiently. Block blobs are comprised of blocks, each
   * of which is identified by a block ID. You create or modify a block blob by writing a set of
   * blocks and committing them by their block IDs. Each block can be a different size.
   *
   * When you upload a block to a blob in your storage account, it is associated with the specified
   * block blob, but it does not become part of the blob until you commit a list of blocks that
   * includes the new block's ID. New blocks remain in an uncommitted state until they are
   * specifically committed or discarded. Writing a block does not update the last modified time of
   * an existing blob.
   */
  class BlockBlobClient : public BlobClient {
  public:
    /**
     * @brief Initialize a new instance of BlockBlobClient.
     *
     * @param connectionString A connection string includes the authentication information required
     * for your application to access data in an Azure Storage account at runtime.
     * @param containerName The name of the container containing this blob.
     * @param blobName The name of this blob.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     * @return A new BlockBlobClient instance.
     */
    static BlockBlobClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& containerName,
        const std::string& blobName,
        const BlockBlobClientOptions& options = BlockBlobClientOptions());

    /**
     * @brief Initialize a new instance of BlockBlobClient.
     *
     * @param blobUri A uri
     * referencing the blob that includes the name of the account, the name of the container, and
     * the name of the blob.
     * @param credential The shared key credential used to sign
     * requests.
     * @param options Optional client options that define the transport pipeline
     * policies for authentication, retries, etc., that are applied to every request.
     */
    explicit BlockBlobClient(
        const std::string& blobUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const BlockBlobClientOptions& options = BlockBlobClientOptions());

    /**
     * @brief Initialize a new instance of BlockBlobClient.
     *
     * @param blobUri A uri
     * referencing the blob that includes the name of the account, the name of the container, and
     * the name of the blob.
     * @param credential The client secret credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit BlockBlobClient(
        const std::string& blobUri,
        std::shared_ptr<Identity::ClientSecretCredential> credential,
        const BlockBlobClientOptions& options = BlockBlobClientOptions());

    /**
     * @brief Initialize a new instance of BlockBlobClient.
     *
     * @param blobUri A uri
     * referencing the blob that includes the name of the account, the name of the container, and
     * the name of the blob, and possibly also a SAS token.
     * @param options Optional client
     * options that define the transport pipeline policies for authentication, retries, etc., that
     * are applied to every request.
     */
    explicit BlockBlobClient(
        const std::string& blobUri,
        const BlockBlobClientOptions& options = BlockBlobClientOptions());

    /**
     * @brief Initializes a new instance of the BlockBlobClient class with an identical uri
     * source but the specified snapshot timestamp.
     *
     * @param snapshot The snapshot
     * identifier.
     * @return A new BlockBlobClient instance.
     * @remarks Pass empty string to remove the snapshot returning the base blob.
     */
    BlockBlobClient WithSnapshot(const std::string& snapshot) const;

    /**
     * @brief Creates a clone of this instance that references a version ID rather than the base
     * blob.
     *
     * @param versionId The version ID returning a URL to the base blob.
     * @return A new BlockBlobClient instance.
     * @remarks Pass empty string to remove the version ID returning the base blob.
     */
    BlockBlobClient WithVersionId(const std::string& versionId) const;

    /**
     * @brief Creates a new block blob, or updates the content of an existing block blob. Updating
     * an existing block blob overwrites any existing metadata on the blob.
     *
     * @param content A BodyStream containing the content to upload.
     * @param options Optional parameters to execute this function.
     * @return A UploadBlockBlobResult describing the state of the updated block blob.
     */
    Azure::Core::Response<UploadBlockBlobResult> Upload(
        Azure::Core::Http::BodyStream* content,
        const UploadBlockBlobOptions& options = UploadBlockBlobOptions()) const;

    /**
     * @brief Creates a new block blob, or updates the content of an existing block blob. Updating
     * an existing block blob overwrites any existing metadata on the blob.
     *
     * @param buffer A memory buffer containing the content to upload.
     * @param bufferSize Size of the memory buffer.
     * @param options Optional parameters to execute this function.
     * @return A UploadBlockBlobFromResult describing the state of the updated block blob.
     */
    Azure::Core::Response<UploadBlockBlobFromResult> UploadFrom(
        const uint8_t* buffer,
        std::size_t bufferSize,
        const UploadBlockBlobFromOptions& options = UploadBlockBlobFromOptions()) const;

    /**
     * @brief Creates a new block blob, or updates the content of an existing block blob. Updating
     * an existing block blob overwrites any existing metadata on the blob.
     *
     * @param file A file containing the content to upload.
     * @param options Optional parameters to execute this function.
     * @return A UploadBlockBlobFromResult describing the state of the updated block blob.
     */
    Azure::Core::Response<UploadBlockBlobFromResult> UploadFrom(
        const std::string& file,
        const UploadBlockBlobFromOptions& options = UploadBlockBlobFromOptions()) const;

    /**
     * @brief Creates a new block as part of a block blob's staging area to be eventually
     * committed via the CommitBlockList operation.
     *
     * @param blockId A valid Base64 string value that identifies the block. Prior to encoding, the
     * string must be less than or equal to 64 bytes in size.
     * @param content A BodyStream containing the content to upload.
     * @param options Optional parameters to execute this function.
     * @return A StageBlockResult describing the state of the updated block.
     */
    Azure::Core::Response<StageBlockResult> StageBlock(
        const std::string& blockId,
        Azure::Core::Http::BodyStream* content,
        const StageBlockOptions& options = StageBlockOptions()) const;

    /**
     * @brief Creates a new block to be committed as part of a blob where the contents are read from
     * the sourceUri.
     *
     * @param blockId A valid Base64 string value that identifies the block. Prior to encoding, the
     * string must be less than or equal to 64 bytes in size.
     * @param sourceUri Specifies the uri of the source
     * blob. The value may be a uri of up to 2 KB in length that specifies a blob. The source blob
     * must either be public or must be authenticated via a shared access signature. If the source
     * blob is public, no authentication is required to perform the operation.
     * @param options Optional parameters to execute this function.
     * @return A StageBlockFromUriResult describing the state of the updated block blob.
     */
    Azure::Core::Response<StageBlockFromUriResult> StageBlockFromUri(
        const std::string& blockId,
        const std::string& sourceUri,
        const StageBlockFromUriOptions& options = StageBlockFromUriOptions()) const;

    /**
     * @brief Writes a blob by specifying the list of block IDs that make up the blob. In order to
     * be written as part of a blob, a block must have been successfully written to the server in a
     * prior StageBlock operation. You can call CommitBlockList to update a blob by uploading only
     * those blocks that have changed, then committing the new and existing blocks together. You can
     * do this by specifying whether to commit a block from the committed block list or from the
     * uncommitted block list, or to commit the most recently uploaded version of the block,
     * whichever list it may belong to.
     *
     * @param blockIds Base64 encoded block IDs to indicate that make up the blob.
     * @param options Optional parameters to execute this function.
     * @return A CommitBlobBlockListResult describing the state of the updated block blob.
     */
    Azure::Core::Response<CommitBlockListResult> CommitBlockList(
        const std::vector<std::pair<BlockType, std::string>>& blockIds,
        const CommitBlockListOptions& options = CommitBlockListOptions()) const;

    /**
     * @brief Retrieves the list of blocks that have been uploaded as part of a block blob. There
     * are two block lists maintained for a blob. The Committed Block list has blocks that have been
     * successfully committed to a given blob with CommitBlockList. The Uncommitted Block list has
     * blocks that have been uploaded for a blob using StageBlock, but that have not yet been
     * committed.
     *
     * @param options Optional parameters to execute this function.
     * @return A GetBlobBlockListResult describing requested block list.
     */
    Azure::Core::Response<GetBlockListResult> GetBlockList(
        const GetBlockListOptions& options = GetBlockListOptions()) const;

  private:
    explicit BlockBlobClient(BlobClient blobClient);
    friend class BlobClient;
    friend class Files::DataLake::FileClient;
  };

}}} // namespace Azure::Storage::Blobs
