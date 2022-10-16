// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/job_properties.hpp"

namespace Azure { namespace Storage { namespace _internal {

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
