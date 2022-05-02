// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <azure/core/nullable.hpp>

#include "azure/storage/datamovement/task.hpp"

namespace Azure { namespace Storage { namespace DataMovement { namespace _internal {

  using TaskQueue = std::queue<Task>;

  struct SchedulerOptions
  {
    Nullable<int> NumThreads; // default: 2 * num cpus
    Nullable<size_t> MaxMemorySize; // default: 128MB * num threads
  };

  class Scheduler {
  public:
    explicit Scheduler(const SchedulerOptions& options);
    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    void AddTask(Task&& task);
    void AddTasks(std::vector<Task>&& tasks);

    // TODO: Pause/Suspend and Resume

  private:
    SchedulerOptions m_options;

    std::atomic<bool> m_stopped{false};

    // resource left
    std::atomic<size_t> m_memoryLeft;
    // TODO: other resource

    // pending tasks
    TaskQueue m_pendingDiskIOTasks;
    TaskQueue m_pendingNetworkUploadTasks;
    TaskQueue m_pendingNetworkDownloadTasks;
    std::mutex m_pendingTasksMutex;
    std::condition_variable m_pendingTasksCv;

    std::thread m_schedulerThread;

    // ready tasks, should run asap
    TaskQueue m_readyTasks;
    std::mutex m_readyTasksMutex;
    std::condition_variable m_readyTasksCv;

    TaskQueue m_readyDiskIOTasks;
    std::mutex m_readyDiskIOTasksMutex;
    std::condition_variable m_readyDiskIOTasksCv;

    std::vector<std::thread> m_workerThreads;
  };

}}}} // namespace Azure::Storage::DataMovement::_internal
