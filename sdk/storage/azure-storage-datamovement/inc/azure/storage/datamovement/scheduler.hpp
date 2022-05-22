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

namespace Azure { namespace Storage { namespace _internal {

  using TaskQueue = std::queue<Task>;

  struct SchedulerOptions final
  {
    Nullable<int> NumThreads; // default: num cpus, minimum 5
    Nullable<size_t> MaxMemorySize; // default: 128MB * num threads
  };

  class Scheduler final {
  public:
    explicit Scheduler(const SchedulerOptions& options);
    ~Scheduler();

    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;

    void AddTask(Task&& task);
    void AddTasks(std::vector<Task>&& tasks);

    void ResumePausedTasks();

  private:
    void ReclaimProvisionedResource(const Task& t)
    {
      if (!t)
      {
        return;
      }
      if (t->MemoryCost != 0)
      {
        m_memoryLeft.fetch_add(t->MemoryCost);
        m_pendingTasksCv.notify_one();
      }
    }

    void ReclaimAllocatedResource(const Task& t)
    {
      if (!t)
      {
        return;
      }
      if (t->MemoryGiveBack != 0)
      {
        m_memoryLeft.fetch_add(t->MemoryGiveBack);
        m_pendingTasksCv.notify_one();
      }
    }

  private:
    SchedulerOptions m_options;

    std::atomic<bool> m_stopped{false};

    // resource left
    std::atomic<size_t> m_memoryLeft;

    // tasks for paused jobs
    TaskQueue m_pausedTasks;
    std::mutex m_pausedTasksMutex;

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

}}} // namespace Azure::Storage::_internal
