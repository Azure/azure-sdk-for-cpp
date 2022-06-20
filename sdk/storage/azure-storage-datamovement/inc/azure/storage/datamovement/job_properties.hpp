// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <future>
#include <memory>
#include <string>
#include <type_traits>

namespace Azure { namespace Storage {
  enum class TransferType
  {
    SingleUpload = 0,
    SingleDownload = 1,
    DirectoryUpload = 2,
    DirectoryDownload = 3,
  };

  enum class JobStatus
  {
    InProgress,
    Paused,
    Succeeded,
    Failed,
    PartiallySucceeded,
    Cancelled,
  };

  struct JobProperties final
  {
    std::string Id;
    std::string SourceUrl;
    std::string DestinationUrl;
    TransferType Type = static_cast<TransferType>(0);
    std::shared_future<JobStatus> WaitHandle;
  };

  namespace _internal {
    std::string JobStatusToString(JobStatus s);
  } // namespace _internal

}} // namespace Azure::Storage
