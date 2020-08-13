// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "blobs/copy_blob_poller.hpp"
#include "blobs/blob_client.hpp"

namespace Azure { namespace Storage { namespace Blobs {

  CopyBlobPoller::CopyBlobPoller(const BlobClient& blobClient, std::string copyId)
      : m_blobClient(std::make_shared<BlobClient>(blobClient)), m_copyId(std::move(copyId))
  {
  }

  Azure::Core::Response<UpdateCopyStatusResult> CopyBlobPoller::UpdateStatus(
      const UpdateCopyBlobStatusOptions& options)
  {
    return m_blobClient->GetProperties(options);
  }

  Azure::Core::Response<UpdateCopyStatusResult> CopyBlobPoller::WaitForCompletion(
      const WaitForCopyBlobCompletionOptions& options)
  {
    UpdateCopyBlobStatusOptions updateStatusOptions;
    updateStatusOptions.Context = options.Context;
    updateStatusOptions.AccessConditions = options.AccessConditions;
    while (true)
    {
      auto ret = UpdateStatus(updateStatusOptions);
      if (!ret->CopyId.HasValue() || ret->CopyId.GetValue() != m_copyId)
      {
        throw std::runtime_error("copy destination was overwritten");
      }
      if (ret->CopyStatus.GetValue() == CopyStatus::Success)
      {
        return ret;
      }
      else if (ret->CopyStatus.GetValue() == CopyStatus::Pending)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(options.intervalMs));
      }
      else
      {
        throw std::runtime_error("unknown copy status");
      }
    }
  }

}}} // namespace Azure::Storage::Blobs
