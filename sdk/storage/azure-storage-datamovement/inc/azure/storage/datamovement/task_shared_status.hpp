// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <functional>
#include <future>
#include <string>

#include "azure/storage/datamovement/datamovement_options.hpp"
#include "azure/storage/datamovement/job_properties.hpp"

namespace Azure { namespace Storage {
  namespace _detail {
    struct JournalContext;
  } // namespace _detail
  namespace _internal {
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
      std::function<void(const _detail::JournalContext&, int64_t, int64_t, int64_t, int64_t)>
          WriteJournal;
      std::function<void(TransferError&)> ErrorHandler;
      std::shared_future<JobStatus> WaitHandle = m_notificationHandle.get_future().share();
      _internal::TransferEngine* TransferEngine = nullptr;

      std::atomic<bool> HasFailure{false};
      std::atomic<bool> HasSuccess{false};
    };

  } // namespace _internal
}} // namespace Azure::Storage
