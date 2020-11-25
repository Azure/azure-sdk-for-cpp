// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/storage/blobs/blob_service_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  /**
   * @brief A BlobBatch allows you to batch multiple Azure Storage operations in a single request
   * via BlobBatchClient.SubmitBatch.
   */
  class BlobBatch {
  public:
    /**
     * @brief Marks the specified blob or snapshot for deletion.
     *
     * @param
     * containerName The name of the container containing the blob to delete.
     * @param blobName
     * The name of the blob to delete.
     * @param options Optional parameters to execute this
     * function.
     * @return An index of this operation result in
     * SubmitBlobBatchResult.DeleteBlobResults, after this batch is submitted via
     * BlobBatchClient.SubmitBatch.
     */
    int32_t DeleteBlob(
        const std::string& containerName,
        const std::string& blobName,
        const DeleteBlobOptions& options = DeleteBlobOptions());

    /**
     * @brief Sets the tier on a blob.
     *
     * @param containerName The name of the
     * container containing the blob to set the tier of.
     * @param blobName The name of the blob
     * to set the tier of.
     * @param tier Indicates the tier to be set on the blob.
     *
     * @param options Optional parameters to execute this function.
     * @return An index of this
     * operation result in SubmitBlobBatchResult.SetBlobAccessTierResults, after this batch is
     * submitted via BlobBatchClient.SubmitBatch.
     */
    int32_t SetBlobAccessTier(
        const std::string& containerName,
        const std::string& blobName,
        Models::AccessTier tier,
        const SetBlobAccessTierOptions& options = SetBlobAccessTierOptions());

  private:
    friend class BlobBatchClient;

    struct DeleteBlobSubRequest
    {
      std::string BlobContainerName;
      std::string BlobName;
      DeleteBlobOptions Options;
    };

    struct SetBlobAccessTierSubRequest
    {
      std::string BlobContainerName;
      std::string BlobName;
      Models::AccessTier Tier = Models::AccessTier::Unknown;
      SetBlobAccessTierOptions Options;
    };

    std::vector<DeleteBlobSubRequest> m_deleteBlobSubRequests;
    std::vector<SetBlobAccessTierSubRequest> m_setBlobAccessTierSubRequests;
  };

  struct SubmitBlobBatchResult
  {
    std::vector<Azure::Core::Response<Models::DeleteBlobResult>> DeleteBlobResults;
    std::vector<Azure::Core::Response<Models::SetBlobAccessTierResult>> SetBlobAccessTierResults;
  };

  /**
   * @brief The BlobBatchClient allows you to batch multiple Azure Storage operations in a
   * single request.
   */
  class BlobBatchClient {
  public:
    /**
     * @brief Initialize a new instance of BlobBatchClient.
     *
     * @param connectionString A connection string includes the authentication information required
     * for your application to access data in an Azure Storage account at runtime.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request and subrequest.
     * @return A new BlobBatchClient instance.
     */
    static BlobBatchClient CreateFromConnectionString(
        const std::string& connectionString,
        const BlobClientOptions& options = BlobClientOptions());

    /**
     * @brief Initialize a new instance of BlobBatchClient.
     *
     * @param serviceUrl A url referencing the blob that includes the name of the account.
     * @param credential The shared key credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request and subrequest.
     */
    explicit BlobBatchClient(
        const std::string& serviceUrl,
        std::shared_ptr<StorageSharedKeyCredential> credential,
        const BlobClientOptions& options = BlobClientOptions());

    /**
     * @brief Initialize a new instance of BlobBatchClient.
     *
     * @param serviceUrl A url referencing the blob that includes the name of the account.
     * @param credential The token credential used to sign requests.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request and subrequest.
     */
    explicit BlobBatchClient(
        const std::string& serviceUrl,
        std::shared_ptr<Core::TokenCredential> credential,
        const BlobClientOptions& options = BlobClientOptions());

    /**
     * @brief Initialize a new instance of BlobBatchClient.
     *
     * @param serviceUrl A url referencing the blob that includes the name of the account, and
     * possibly also a SAS token.
     * @param options Optional client options that define the transport pipeline policies for
     * authentication, retries, etc., that are applied to every request and subrequest.
     */
    explicit BlobBatchClient(
        const std::string& serviceUrl,
        const BlobClientOptions& options = BlobClientOptions());

    /**
     * @brief Creates a new BlobBatch to collect sub-operations that can be submitted
     * together via SubmitBatch.
     *
     * @return A new instance of BlobBatch.
     */
    static BlobBatch CreateBatch() { return BlobBatch(); }

    /**
     * @brief Submit a BlobBatch of sub-operations.
     *
     * @param batch A BlobBatch
     * of sub-operations.
     * @param options Optional parameters to execute this function.
     *
     * @return A SubmitBlobBatchResult on successful submitting.
     */
    Azure::Core::Response<SubmitBlobBatchResult> SubmitBatch(
        const BlobBatch& batch,
        const SubmitBlobBatchOptions& options = SubmitBlobBatchOptions()) const;

  protected:
    Azure::Core::Http::Url m_serviceUrl;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_pipeline;
    std::shared_ptr<Azure::Core::Http::HttpPipeline> m_subRequestPipeline;
  };

}}} // namespace Azure::Storage::Blobs
