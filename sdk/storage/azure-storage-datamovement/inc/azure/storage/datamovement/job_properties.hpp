// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <type_traits>

#include "azure/storage/datamovement/datamovement_options.hpp"
#include "azure/storage/datamovement/scheduler.hpp"

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
    TransferType Type = static_cast<TransferType>(std::underlying_type_t<TransferType>(0));
    std::shared_future<JobStatus> WaitHandle;
  };

  namespace _internal {
    std::string JobStatusToString(JobStatus s);

    class TaskSharedStatus final {
    private:
      std::promise<JobStatus> m_notificationHandle;

    public:
      TaskSharedStatus() = default;
      TaskSharedStatus(const TaskSharedStatus&) = delete;
      TaskSharedStatus& operator=(const TaskSharedStatus&) = delete;
      ~TaskSharedStatus();

      std::atomic<JobStatus> Status{JobStatus::InProgress};
      std::function<void(const TransferProgress&)> ProgressHandler{};
      std::function<void(TransferError&)> ErrorHandler{};
      std::shared_future<JobStatus> WaitHandle = m_notificationHandle.get_future().share();
      _internal::Scheduler* Scheduler = nullptr;

      std::string JobId;
      std::atomic<int64_t> NumFilesTransferred{0};
      std::atomic<int64_t> NumFilesSkipped{0};
      std::atomic<int64_t> NumFilesFailed{0};
      std::atomic<int64_t> TotalBytesTransferred{0};

      void TaskTransferedCallback(int64_t numFiles, int64_t bytes) noexcept
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

      void TaskSkippedCallback(int64_t numFiles) noexcept
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

      void TaskFailedCallback(
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
    };

    struct JobDetails final
    {
      std::string Id;
      std::string SourceUrl;
      std::string DestinationUrl;
      TransferType Type = static_cast<TransferType>(std::underlying_type_t<TransferType>(0));
      std::weak_ptr<TaskSharedStatus> SharedStatus;

      JobProperties GetJobProperties() const;
    };

  } // namespace _internal

}} // namespace Azure::Storage
