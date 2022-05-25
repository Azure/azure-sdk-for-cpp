// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/job_properties.hpp"

#include <azure/core/azure_assert.hpp>

namespace Azure { namespace Storage { namespace _internal {

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

  JobProperties JobDetails::GetJobProperties() const
  {
    JobProperties p;
    p.Id = Id;
    p.SourceUrl = SourceUrl;
    p.DestinationUrl = DestinationUrl;
    p.Type = Type;
    p.WaitHandle = SharedStatus.lock()->WaitHandle;
    return p;
  }

  std::string JobStatusToString(JobStatus s)
  {
    switch (s)
    {
      case JobStatus::InProgress:
        return "InProgress";
      case JobStatus::Succeeded:
        return "Succeeded";
      case JobStatus::Failed:
        return "Failed";
      case JobStatus::PartiallySucceeded:
        return "PartiallySucceeded";
      case JobStatus::Cancelled:
        return "Cancelled";
      case JobStatus::Paused:
        return "Paused";
      default:
        return "Unknown";
    }
  }
}}} // namespace Azure::Storage::_internal
