// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/identity/client_secret_credential.hpp"
#include "azure/storage/blobs/blob_client.hpp"
#include "azure/storage/blobs/blob_options.hpp"
#include "azure/storage/blobs/blob_responses.hpp"
#include "azure/storage/blobs/protocol/blob_rest_client.hpp"
#include "azure/storage/common/storage_credential.hpp"

#include <string>

namespace Azure { namespace Storage { namespace Blobs {

  /**
   * The PageBlobClient allows you to manipulate Azure Storage page blobs.
   *
   * Page blobs are a collection of 512-byte pages optimized for random read and write operations.
   * To create a page blob, you initialize the page blob and specify the maximum size the page blob
   * will grow. To add or update the contents of a page blob, you write a page or pages by
   * specifying an offset and a range that align to 512-byte page boundaries. Writes to page blobs
   * happen in-place and are immediately committed to the blob.
   */
  class PageBlobClient : public BlobClient {
  public:
    /**
     * @brief Initialize a new instance of PageBlobClient.
     *
     * @param connectionString A connection string includes the authentication information required
     * for your application to access data in an Azure Storage account at runtime.
     * @param containerName The name of the container containing this blob.
     * @param blobName The name of this blob.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     * @return A new PageBlobClient instance.
     */
    static PageBlobClient CreateFromConnectionString(
        const std::string& connectionString,
        const std::string& containerName,
        const std::string& blobName,
        const PageBlobClientOptions& options = PageBlobClientOptions());

    /**
     * @brief Initialize a new instance of PageBlobClient.
     *
     * @param blobUri A uri
     * referencing the blob that includes the name of the account, the name of the container, and
     * the name of the blob.
     * @param credential The shared key credential used to sign
     * requests.
     * @param options Optional client options that define the transport pipeline
     * policies for authentication, retries, etc., that are applied to every request.
     */
    explicit PageBlobClient(
        const std::string& blobUri,
        std::shared_ptr<SharedKeyCredential> credential,
        const PageBlobClientOptions& options = PageBlobClientOptions());

    /**
     * @brief Initialize a new instance of PageBlobClient.
     *
     * @param blobUri A uri
     * referencing the blob that includes the name of the account, the name of the container, and
     * the name of the blob.
     * @param credential The client secret credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request.
     */
    explicit PageBlobClient(
        const std::string& blobUri,
        std::shared_ptr<Identity::ClientSecretCredential> credential,
        const PageBlobClientOptions& options = PageBlobClientOptions());

    /**
     * @brief Initialize a new instance of PageBlobClient.
     *
     * @param blobUri A uri
     * referencing the blob that includes the name of the account, the name of the container, and
     * the name of the blob, and possibly also a SAS token.
     * @param options Optional client
     * options that define the transport pipeline policies for authentication, retries, etc., that
     * are applied to every request.
     */
    explicit PageBlobClient(
        const std::string& blobUri,
        const PageBlobClientOptions& options = PageBlobClientOptions());

    /**
     * @brief Initializes a new instance of the PageBlobClient class with an identical uri
     * source but the specified snapshot timestamp.
     *
     * @param snapshot The snapshot
     * identifier.
     * @return A new PageBlobClient instance.
     * @remarks Pass empty string to remove the snapshot returning the base blob.
     */
    PageBlobClient WithSnapshot(const std::string& snapshot) const;

    /**
     * @brief Creates a clone of this instance that references a version ID rather than the base
     * blob.
     *
     * @param versionId The version ID returning a URL to the base blob.
     * @return A new PageBlobClient instance.
     * @remarks Pass empty string to remove the version ID returning the base blob.
     */
    PageBlobClient WithVersionId(const std::string& versionId) const;

    /**
     * @brief Creates a new page blob of the specified size. The content of any  existing
     * blob is overwritten with the newly initialized page blob.
     *
     * @param
     * blobContentLength Specifies the maximum size for the page blob. The size must be aligned to a
     * 512-byte boundary.
     * @param options Optional parameters to execute this function.
     * @return A CreatePageBlobResult describing the newly created page blob.
     */
    Azure::Core::Response<CreatePageBlobResult> Create(
        int64_t blobContentLength,
        const CreatePageBlobOptions& options = CreatePageBlobOptions()) const;

    /**
     * @brief Writes content to a range of pages in a page blob, starting at offset.
     *
     * @param offset Specifies the starting offset for the content to be written as a page. Given
     * that pages must be aligned with 512-byte boundaries, the start offset must be a modulus of
     * 512.
     * @param content A BodyStream containing the content of the pages to upload.
     * @param options Optional parameters to execute this function.
     * @return A UploadPageBlobPagesResult describing the state of the updated pages.
     */
    Azure::Core::Response<UploadPageBlobPagesResult> UploadPages(
        int64_t offset,
        Azure::Core::Http::BodyStream* content,
        const UploadPageBlobPagesOptions& options = UploadPageBlobPagesOptions()) const;

    /**
     * @brief Writes a range of pages to a page blob where the contents are read from a
     * uri.
     *
     * @param destinationOffset Specifies the starting offset for the content to be written. Given
     * that pages must be aligned with 512-byte boundaries, the start offset must be a modulus of
     * 512.
     * @param sourceUri Specifies the uri of the source blob. The value may be a
     * uri of up to 2 KB in length that specifies a blob. The source blob must either be public or
     * must be authenticated via a shared access signature. If the source blob is public, no
     * authentication is required to perform the operation.
     * @param sourceOffset Only upload a part of the blob in the sourceUri from the specified
     * offset. This offset doesn't need to be a modulus of 512.
     * @param sourceLength Only upload specified length of the blob in the sourceUri. This length
     * must be a modulus of 512.
     * @param options Optional parameters to execute this function.
     * @return A UploadPageBlobPagesFromUriResult describing the state of the updated pages.
     */
    Azure::Core::Response<UploadPageBlobPagesFromUriResult> UploadPagesFromUri(
        int64_t destinationoffset,
        std::string sourceUri,
        int64_t sourceOffset,
        int64_t sourceLength,
        const UploadPageBlobPagesFromUriOptions& options
        = UploadPageBlobPagesFromUriOptions()) const;

    /**
     * @brief Clears one or more pages from the page blob, as specificed by offset and length.
     *
     * @param offset Specifies the starting offset for the content to be cleared. Given that pages
     * must be aligned with 512-byte boundaries, the start offset must be a modulus of 512.
     * @param length Specifies the length of the content to be cleared. The length must be a modulus
     * of 512.
     * @param options Optional parameters to execute this function.
     * @return A ClearPageBlobPagesResult describing the state of the updated pages.
     */
    Azure::Core::Response<ClearPageBlobPagesResult> ClearPages(
        int64_t offset,
        int64_t length,
        const ClearPageBlobPagesOptions& options = ClearPageBlobPagesOptions()) const;

    /**
     * @brief Resizes the page blob to the specified size (which must be a multiple of 512). If the
     * specified value is less than the current size of the blob, then all pages above the specified
     * value are cleared.
     *
     * @param blobContentLength Specifies the maximum size for the page blob. The size must be
     * aligned to a 512-byte boundary.
     * @param options Optional parameters to execute this function.
     * @return A ResizePageBlobResult describing the resized page blob.
     */
    Azure::Core::Response<ResizePageBlobResult> Resize(
        int64_t blobContentLength,
        const ResizePageBlobOptions& options = ResizePageBlobOptions()) const;

    /**
     * @brief Returns the list of valid page ranges for a page blob or snapshot of a page blob.
     *
     * @param options Optional parameters to execute this function.
     * @return A GetPageBlobPageRangesResult describing the valid page ranges for this blob.
     */
    Azure::Core::Response<GetPageBlobPageRangesResult> GetPageRanges(
        const GetPageBlobPageRangesOptions& options = GetPageBlobPageRangesOptions()) const;

    /**
     * @brief Starts copying a snapshot of the sourceUri page blob to this page blob. The snapshot
     * is copied such that only the differential changes between the previously copied snapshot
     * are transferred to the destination. The copied snapshots are complete copies of the original
     * snapshot and can be read or copied from as usual.
     *
     * @param sourceUri Specifies the to the source page blob as a uri up to 2 KB in length. The
     * source blob must either be public or must be authenticated via a shared access signature.
     * @param options Optional parameters to execute this function.
     * @return A StartCopyPageBlobIncrementalResult describing the state of the copy operation.
     */
    Azure::Core::Response<StartCopyPageBlobIncrementalResult> StartCopyIncremental(
        const std::string& sourceUri,
        const StartCopyPageBlobIncrementalOptions& options
        = StartCopyPageBlobIncrementalOptions()) const;

  private:
    explicit PageBlobClient(BlobClient blobClient);
    friend class BlobClient;
  };

}}} // namespace Azure::Storage::Blobs
