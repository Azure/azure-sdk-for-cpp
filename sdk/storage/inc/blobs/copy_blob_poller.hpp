// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "blobs/blob_options.hpp"
#include "protocol/blob_rest_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  using UpdateCopyStatusResult = GetBlobPropertiesResult;

  using UpdateCopyBlobStatusOptions = GetBlobPropertiesOptions;

  /**
   * @brief Optional parameters for CopyBlobPoller::WaitForCompletion.
   */
  struct WaitForCopyBlobCompletionOptions
  {
    /**
     * @brief Context for cancelling long running operations.
     */
    Azure::Core::Context Context;

    /**
     * @brief Inteval of calling UpdateStatus function, in milliseconds.
     */
    int32_t intervalMs = 1000;

    /**
     * @brief Optional conditions that must be met to perform this operation.
     */
    BlobAccessConditions AccessConditions;
  };

  class BlobClient;

  class CopyBlobPoller {
  public:
    CopyBlobPoller() = default;

    explicit CopyBlobPoller(const BlobClient& blobClient, std::string copyId);

    /**
     * @brief Check for the latest status of the copy operation.
     *
     * @param
     * options Optional parameters to execute this function.
     * @return
     */
    Azure::Core::Response<UpdateCopyStatusResult> UpdateStatus(
        const UpdateCopyBlobStatusOptions& options = UpdateCopyBlobStatusOptions());

    /**
     * @brief Periodically check the status of the copy operation till the operation
     * completes.
     *
     * @param options Optional parameters to execute this function.
     * @return UpdateCopyStatusResult descri
     */
    Azure::Core::Response<UpdateCopyStatusResult> WaitForCompletion(
        const WaitForCopyBlobCompletionOptions& options = WaitForCopyBlobCompletionOptions());

  private:
    std::shared_ptr<BlobClient> m_blobClient;
    std::string m_copyId;
  };

}}} // namespace Azure::Storage::Blobs
