// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <string>

#include "azure/storage/datamovement/datamovement_options.hpp"
#include "azure/storage/datamovement/job_properties.hpp"

namespace Azure { namespace Storage { namespace _internal {
  class TransferEngine;

  class TaskSharedStatus final {
  private:
    std::promise<JobStatus> m_notificationHandle;

  public:
    TaskSharedStatus() = default;
    TaskSharedStatus(const TaskSharedStatus&) = delete;
    TaskSharedStatus& operator=(const TaskSharedStatus&) = delete;
    ~TaskSharedStatus();

    std::string JobId;
    std::atomic<JobStatus> Status{JobStatus::InProgress};
    std::function<void(const TransferProgress&)> ProgressHandler{};
    std::function<void(TransferError&)> ErrorHandler{};
    std::shared_future<JobStatus> WaitHandle = m_notificationHandle.get_future().share();
    TransferEngine* TransferEngine = nullptr;

    std::atomic<int64_t> NumFilesTransferred{0};
    std::atomic<int64_t> NumFilesSkipped{0};
    std::atomic<int64_t> NumFilesFailed{0};
    std::atomic<int64_t> TotalBytesTransferred{0};

    void TaskTransferedCallback(int64_t numFiles, int64_t bytes) noexcept;

    void TaskSkippedCallback(int64_t numFiles) noexcept;

    void TaskFailedCallback(
        int64_t numFiles,
        std::string sourceUrl,
        std::string destinationUrl) noexcept;
  };

}}} // namespace Azure::Storage::_internal
