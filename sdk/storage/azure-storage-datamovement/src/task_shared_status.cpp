// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/task_shared_status.hpp"

#include "azure/storage/datamovement/task.hpp"

namespace Azure { namespace Storage { namespace _internal {

  void TaskBase::TransferSucceeded(int64_t bytesTransferred, int64_t numFiles) const
  {
    SharedStatus->WriteJournal(JournalContext, numFiles, 0, 0, bytesTransferred);
  }

  void TaskBase::TransferFailed(std::string sourceUrl, std::string destinationUrl, int64_t numFiles)
      const
  {
    TransferError e;
    e.SourceUrl = std::move(sourceUrl);
    e.DestinationUrl = std::move(destinationUrl);
    SharedStatus->ErrorHandler(e);
    SharedStatus->WriteJournal(JournalContext, 0, 0, numFiles, 0);
  }

  void TaskBase::Transferkipped(int64_t numFiles) const
  {
    SharedStatus->WriteJournal(JournalContext, 0, numFiles, 0, 0);
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
      if (!HasFailure.load(std::memory_order_relaxed))
      {
        Status = JobStatus::Succeeded;
      }
      else if (HasSuccess.load(std::memory_order_relaxed))
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
