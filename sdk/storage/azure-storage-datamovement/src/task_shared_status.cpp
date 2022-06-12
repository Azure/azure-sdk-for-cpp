// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/task_shared_status.hpp"

namespace Azure { namespace Storage { namespace _internal {
  void TaskSharedStatus::TaskTransferedCallback(int64_t numFiles, int64_t bytes) noexcept
  {
    NumFilesTransferred.fetch_add(numFiles, std::memory_order_relaxed);
    TotalBytesTransferred.fetch_add(bytes, std::memory_order_relaxed);
    if (ProgressHandler)
    {
      TransferProgress progress;
      progress.NumFilesTransferred = NumFilesTransferred.load(std::memory_order_relaxed);
      progress.NumFilesSkipped = NumFilesSkipped.load(std::memory_order_relaxed);
      progress.NumFilesFailed = NumFilesFailed.load(std::memory_order_relaxed);
      progress.TotalBytesTransferred = TotalBytesTransferred.load(std::memory_order_relaxed);
      ProgressHandler(progress);
    }
  }

  void TaskSharedStatus::TaskSkippedCallback(int64_t numFiles) noexcept
  {
    NumFilesSkipped.fetch_add(numFiles, std::memory_order_relaxed);
    if (ProgressHandler)
    {
      TransferProgress progress;
      progress.NumFilesTransferred = NumFilesTransferred.load(std::memory_order_relaxed);
      progress.NumFilesSkipped = NumFilesSkipped.load(std::memory_order_relaxed);
      progress.NumFilesFailed = NumFilesFailed.load(std::memory_order_relaxed);
      progress.TotalBytesTransferred = TotalBytesTransferred.load(std::memory_order_relaxed);
      ProgressHandler(progress);
    }
  }

  void TaskSharedStatus::TaskFailedCallback(
      int64_t numFiles,
      std::string sourceUrl,
      std::string destinationUrl) noexcept
  {
    NumFilesFailed.fetch_add(numFiles, std::memory_order_relaxed);
    if (ProgressHandler)
    {
      TransferProgress progress;
      progress.NumFilesTransferred = NumFilesTransferred.load(std::memory_order_relaxed);
      progress.NumFilesSkipped = NumFilesSkipped.load(std::memory_order_relaxed);
      progress.NumFilesFailed = NumFilesFailed.load(std::memory_order_relaxed);
      progress.TotalBytesTransferred = TotalBytesTransferred.load(std::memory_order_relaxed);
      ProgressHandler(progress);
    }
    if (ErrorHandler)
    {
      TransferError e;
      e.JobId = JobId;
      e.SourceUrl = std::move(sourceUrl);
      e.DestinationUrl = std::move(destinationUrl);
      ErrorHandler(e);
    }
  }

  TaskSharedStatus::~TaskSharedStatus()
  {
    if (Status == JobStatus::Cancelled || Status == JobStatus::Failed)
    {
    }
    else if (Status == JobStatus::Paused)
    {
    }
    else if (Status == JobStatus::InProgress)
    {
      if (NumFilesFailed.load(std::memory_order_relaxed) == 0)
      {
        Status = JobStatus::Succeeded;
      }
      else if (
          NumFilesSkipped.load(std::memory_order_relaxed)
              + NumFilesTransferred.load(std::memory_order_relaxed)
          != 0)
      {
        Status = JobStatus::PartiallySucceeded;
      }
      else
      {
        Status = JobStatus::Failed;
      }
    }
    else
    {
      AZURE_UNREACHABLE_CODE();
    }
    m_notificationHandle.set_value(Status);
  }
}}} // namespace Azure::Storage::_internal
