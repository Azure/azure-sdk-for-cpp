// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/storage/datamovement/transfer_engine.hpp"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <azure/core/azure_assert.hpp>

#include "azure/storage/datamovement/job_properties.hpp"
#include "azure/storage/datamovement/task_shared_status.hpp"

#if defined(_MSC_VER)
#pragma warning(disable : 26110 26117)
#endif

namespace Azure { namespace Storage { namespace _internal {

  TransferEngine::TransferEngine(const TransferEngineOptions& options) : m_options(options)
  {
    int numThreads = options.NumThreads.HasValue()
        ? options.NumThreads.Value()
        : std::max<int>(5, std::thread::hardware_concurrency());
    AZURE_ASSERT(numThreads != 0);
    size_t maxMemorySize = options.MaxMemorySize.HasValue() ? options.MaxMemorySize.Value()
                                                            : 128ULL * 1024 * 1024 * numThreads;
    m_options.NumThreads = numThreads;
    m_options.MaxMemorySize = maxMemorySize;

    m_memoryLeft = m_options.MaxMemorySize.Value();

    auto workerFunc = [this](TaskQueue& q, std::mutex& m, std::condition_variable& cv) {
      // Deadlock prevention: readyQueueLock
      while (true)
      {
        std::unique_lock<std::mutex> guard(m);
        cv.wait(
            guard, [this, &q] { return m_stopped.load(std::memory_order_relaxed) || !q.empty(); });
        if (m_stopped.load(std::memory_order_relaxed))
        {
          break;
        }
        auto task = std::move(q.front());
        q.pop();
        guard.unlock();

        auto jobStatus = task->SharedStatus->Status.load(std::memory_order_relaxed);
        if (jobStatus == JobStatus::Paused || jobStatus == JobStatus::Cancelled
            || jobStatus == JobStatus::Failed)
        {
          ReclaimProvisionedResource(task);
        }
        else if (jobStatus == JobStatus::InProgress)
        {
          task->MemoryGiveBack += task->MemoryCost;
          task->Execute();
        }
        else
        {
          AZURE_UNREACHABLE_CODE();
        }

        ReclaimAllocatedResource(task);
        m_numTasks.fetch_sub(1, std::memory_order_relaxed);
      }
    };

    for (int i = 0; i < numThreads; ++i)
    {
      m_workerThreads.push_back(std::thread(
          workerFunc,
          std::ref(m_readyTasks),
          std::ref(m_readyTasksMutex),
          std::ref(m_readyTasksCv)));
    }
    for (size_t i = 0; i < 2; ++i)
    {
      m_workerThreads.push_back(std::thread(
          workerFunc,
          std::ref(m_readyDiskIOTasks),
          std::ref(m_readyDiskIOTasksMutex),
          std::ref(m_readyDiskIOTasksCv)));
    }

    auto schedulerFunc = [this]() {
      // Deadlock prevention: pendingQueueLock, readyQueueLock
      std::unique_lock<std::mutex> guard(m_pendingTasksMutex);
      while (true)
      {
        if (m_stopped.load(std::memory_order_relaxed))
        {
          break;
        }
        std::vector<Task> readyTasks;
        auto scheduleTasksInPendingQueue
            = [this,
               &readyTasks](TaskQueue& pendingQueue, std::function<bool(const Task&)> predicate) {
                while (!pendingQueue.empty())
                {
                  Task& task = pendingQueue.front();
                  auto jobStatus = task->SharedStatus->Status.load(std::memory_order_relaxed);
                  if (jobStatus == JobStatus::Paused || jobStatus == JobStatus::Cancelled
                      || jobStatus == JobStatus::Failed)
                  {
                    ReclaimAllocatedResource(task);
                    pendingQueue.pop();
                  }
                  else if (jobStatus == JobStatus::InProgress)
                  {
                    if (!predicate(task))
                    {
                      break;
                    }
                    if (task->MemoryCost != 0)
                    {
                      m_memoryLeft.fetch_sub(task->MemoryCost);
                    }

                    readyTasks.push_back(std::move(task));
                    pendingQueue.pop();
                  }
                  else
                  {
                    AZURE_UNREACHABLE_CODE();
                  }
                }
              };

        {
          // schedule disk IO tasks
          scheduleTasksInPendingQueue(m_pendingDiskIOTasks, [this](const Task& t) {
            return t->MemoryCost <= m_memoryLeft.load(std::memory_order_relaxed);
          });
          if (!readyTasks.empty())
          {
            {
              std::lock_guard<std::mutex> readyTasksGuard(m_readyDiskIOTasksMutex);
              for (auto& t : readyTasks)
              {
                m_readyDiskIOTasks.push(std::move(t));
              }
            }
            m_readyDiskIOTasksCv.notify_all();
            readyTasks.clear();
          }
        }

        {
          // schedule network tasks
          scheduleTasksInPendingQueue(
              m_pendingNetworkUploadTasks, [](const Task&) { return true; });
          size_t n1 = readyTasks.size();
          scheduleTasksInPendingQueue(
              m_pendingNetworkDownloadTasks, [](const Task&) { return true; });
          size_t n2 = readyTasks.size();

          if (!readyTasks.empty())
          {
            std::lock_guard<std::mutex> readyTasksGuard(m_readyTasksMutex);
            for (size_t i = 0; i < std::max(n1, n2 - n1); ++i)
            {
              if (i < n1)
              {
                m_readyTasks.push(std::move(readyTasks[i]));
              }
              if (n1 + i < n2)
              {
                m_readyTasks.push(std::move(readyTasks[n1 + i]));
              }
            }
          }
          if (static_cast<int>(readyTasks.size()) >= m_options.NumThreads.Value())
          {
            m_readyTasksCv.notify_all();
          }
          else if (readyTasks.size() > 0)
          {
            for (size_t i = 0; i < readyTasks.size(); ++i)
            {
              m_readyTasksCv.notify_one();
            }
          }
          readyTasks.clear();
        }

        m_pendingTasksCv.wait_for(guard, std::chrono::milliseconds(100));
      }
    };

    m_schedulerThread = std::thread(schedulerFunc);
  }

  void TransferEngine::Stop()
  {
    bool oldValue = m_stopped.exchange(true, std::memory_order_relaxed);
    if (!oldValue)
    {
      m_pendingTasksCv.notify_one();
      m_readyDiskIOTasksCv.notify_all();
      m_readyTasksCv.notify_all();
      m_schedulerThread.join();
      for (auto& th : m_workerThreads)
      {
        th.join();
      }
    }
  }

  TransferEngine::~TransferEngine()
  {
    Stop();
    m_numTasks.fetch_sub(
        m_readyTasks.size() + m_readyDiskIOTasks.size() + +m_pendingDiskIOTasks.size()
            + m_pendingNetworkUploadTasks.size() + m_pendingNetworkDownloadTasks.size(),
        std::memory_order_relaxed);
    {
      std::lock_guard<std::mutex> guard(m_readyTasksMutex);
      while (!m_readyTasks.empty())
      {
        ReclaimProvisionedResource(m_readyTasks.front());
        ReclaimAllocatedResource(m_readyTasks.front());
        m_readyTasks.pop();
      }
    }
    {
      std::lock_guard<std::mutex> guard(m_readyDiskIOTasksMutex);
      while (!m_readyDiskIOTasks.empty())
      {
        ReclaimProvisionedResource(m_readyDiskIOTasks.front());
        ReclaimAllocatedResource(m_readyTasks.front());
        m_readyDiskIOTasks.pop();
      }
    }
    {
      std::lock_guard<std::mutex> guard(m_pendingTasksMutex);
      while (!m_pendingDiskIOTasks.empty())
      {
        ReclaimAllocatedResource(m_pendingDiskIOTasks.front());
        m_pendingDiskIOTasks.pop();
      }
      while (!m_pendingNetworkUploadTasks.empty())
      {
        ReclaimAllocatedResource(m_pendingNetworkUploadTasks.front());
        m_pendingNetworkUploadTasks.pop();
      }
      while (!m_pendingNetworkDownloadTasks.empty())
      {
        ReclaimAllocatedResource(m_pendingNetworkDownloadTasks.front());
        m_pendingNetworkDownloadTasks.pop();
      }
    }
    AZURE_ASSERT(m_memoryLeft == m_options.MaxMemorySize.Value());
    AZURE_ASSERT(m_numTasks == 0);
  }

  void TransferEngine::AddTask(Task&& task)
  {
    if (task->Type == TaskType::DiskIO)
    {
      std::lock_guard<std::mutex> guard(m_pendingTasksMutex);
      m_pendingDiskIOTasks.push(std::move(task));
      m_pendingTasksCv.notify_one();
    }
    else if (task->Type == TaskType::NetworkUpload)
    {
      std::lock_guard<std::mutex> guard(m_pendingTasksMutex);
      m_pendingNetworkUploadTasks.push(std::move(task));
      m_pendingTasksCv.notify_one();
    }
    else if (task->Type == TaskType::NetworkDownload)
    {
      std::lock_guard<std::mutex> guard(m_pendingTasksMutex);
      m_pendingNetworkDownloadTasks.push(std::move(task));
      m_pendingTasksCv.notify_one();
    }
    else if (task->Type == TaskType::Other)
    {
      std::lock_guard<std::mutex> guard(m_readyTasksMutex);
      m_memoryLeft.fetch_sub(task->MemoryCost);
      m_readyTasks.push(std::move(task));
      m_readyTasksCv.notify_one();
    }
    else
    {
      AZURE_UNREACHABLE_CODE();
    }
    m_numTasks.fetch_add(1, std::memory_order_relaxed);
  }

  void TransferEngine::AddTasks(std::vector<Task>&& tasks)
  {
    {
      std::unique_lock<std::mutex> guard(m_pendingTasksMutex, std::defer_lock);
      int numTasksAdded = 0;
      for (auto& task : tasks)
      {
        if (task->Type == TaskType::DiskIO)
        {
          if (!guard.owns_lock())
          {
            guard.lock();
          }
          m_pendingDiskIOTasks.push(std::move(task));
          ++numTasksAdded;
        }
        else if (task->Type == TaskType::NetworkUpload)
        {
          if (!guard.owns_lock())
          {
            guard.lock();
          }
          m_pendingNetworkUploadTasks.push(std::move(task));
          ++numTasksAdded;
        }
        else if (task->Type == TaskType::NetworkDownload)
        {
          if (!guard.owns_lock())
          {
            guard.lock();
          }
          m_pendingNetworkDownloadTasks.push(std::move(task));
          ++numTasksAdded;
        }
      }
      if (numTasksAdded > 0)
      {
        guard.unlock();
        m_pendingTasksCv.notify_one();
      }
    }
    {
      std::unique_lock<std::mutex> guard(m_readyTasksMutex, std::defer_lock);
      int numTasksAdded = 0;
      for (auto& task : tasks)
      {
        if (!task)
        {
          continue;
        }
        if (task->Type == TaskType::Other)
        {
          if (!guard.owns_lock())
          {
            guard.lock();
          }
          m_readyTasks.push(std::move(task));
          ++numTasksAdded;
        }
      }
      if (numTasksAdded >= m_options.NumThreads.Value())
      {
        guard.unlock();
        m_readyTasksCv.notify_all();
      }
      else if (numTasksAdded > 0)
      {
        guard.unlock();
        for (int i = 0; i < numTasksAdded; ++i)
        {
          m_readyTasksCv.notify_one();
        }
      }
    }
    m_numTasks.fetch_add(tasks.size(), std::memory_order_relaxed);
  }

}}} // namespace Azure::Storage::_internal
