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

namespace Azure { namespace Storage {

  namespace _detail {
    using TaskQueue = std::queue<_internal::Task>;

    struct TimedWaitTask
    {
      explicit TimedWaitTask(int64_t time, _internal::Task&& task)
          : time(time), task(std::move(task))
      {
      }
      int64_t time;
      _internal::Task task;
    };

    inline bool operator<(const TimedWaitTask& lhs, const TimedWaitTask& rhs)
    {
      return lhs.time > rhs.time;
    }

    struct TimedWaitTaskQueue : std::priority_queue<TimedWaitTask>
    {
      _internal::Task& front() { return this->c.front().task; }
      int64_t front_counter() const { return this->c.front().time; }
    };
  } // namespace _detail

  namespace _internal {

    struct TransferEngineOptions final
    {
      Nullable<int> NumThreads; // default: num cpus, minimum 5
      Nullable<size_t> MaxMemorySize; // default: 128MB * num threads
    };

    class TransferEngine final {
    public:
      explicit TransferEngine(const TransferEngineOptions& options);
      ~TransferEngine();

      TransferEngine(const TransferEngine&) = delete;
      TransferEngine& operator=(const TransferEngine&) = delete;

      void AddTask(Task&& task);
      void AddTimedWaitTask(int64_t delayInMs, Task&& task);
      void AddTasks(std::vector<Task>&& tasks);

      void Stop();

      static int64_t GetTimeCounter();

      // status and counters
      std::atomic<bool> m_stopped{false};
      std::atomic<size_t> m_numTasks{0};

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
      TransferEngineOptions m_options;

      // resource left
      std::atomic<int64_t> m_memoryLeft;

      // pending tasks
      _detail::TaskQueue m_pendingDiskIOTasks;
      _detail::TaskQueue m_pendingNetworkUploadTasks;
      _detail::TaskQueue m_pendingNetworkDownloadTasks;
      // timed wait tasks, should run asap if the time's passed
      _detail::TimedWaitTaskQueue m_timedWaitTasks;
      std::mutex m_pendingTasksMutex;
      std::condition_variable m_pendingTasksCv;

      std::thread m_schedulerThread;

      // ready tasks, should run asap
      _detail::TaskQueue m_readyTasks;
      std::mutex m_readyTasksMutex;
      std::condition_variable m_readyTasksCv;

      _detail::TaskQueue m_readyDiskIOTasks;
      std::mutex m_readyDiskIOTasksMutex;
      std::condition_variable m_readyDiskIOTasksCv;

      std::vector<std::thread> m_workerThreads;
    };

  } // namespace _internal
}} // namespace Azure::Storage
