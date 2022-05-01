#include "azure/storage/datamovement/scheduler.hpp"

#include <condition_variable>
#include <mutex>
#include <thread>

#include <azure/core/azure_assert.hpp>

#pragma warning(disable : 26110 26117)

namespace Azure { namespace Storage { namespace DataMovement { namespace _internal {

  Scheduler::Scheduler(const SchedulerOptions& options) : m_options(options)
  {
    size_t numThreads = options.NumThreads.HasValue() ? options.NumThreads.Value()
                                                      : std::thread::hardware_concurrency() * 2;
    AZURE_ASSERT(numThreads != 0);
    size_t maxMemorySize = options.MaxMemorySize.HasValue() ? options.MaxMemorySize.Value()
                                                            : 128 * 1024 * 1024 * numThreads;
    m_options.NumThreads = numThreads;
    m_options.MaxMemorySize = maxMemorySize;

    m_memoryLeft = m_options.MaxMemorySize.Value();

    auto workerFunc = [this](TaskQueue& q, std::mutex& m, std::condition_variable& cv) {
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

        task->Execute();
        if (task->MemoryGiveBack != 0)
        {
          m_memoryLeft.fetch_add(task->MemoryGiveBack);
        }
      }
    };

    m_workerThreads.reserve(numThreads + 1);
    for (size_t i = 0; i < numThreads; ++i)
    {
      m_workerThreads.push_back(std::thread(
          workerFunc,
          std::ref(m_readyTasks),
          std::ref(m_readyTasksMutex),
          std::ref(m_readyTasksCv)));
    }
    m_workerThreads.push_back(std::thread(
        workerFunc,
        std::ref(m_readyDiskIOTasks),
        std::ref(m_readyDiskIOTasksMutex),
        std::ref(m_readyDiskIOTasksCv)));

    auto schedulerFunc = [this]() {
      std::unique_lock<std::mutex> guard(m_pendingTasksMutex);
      while (true)
      {
        if (m_stopped.load(std::memory_order_relaxed))
        {
          break;
        }

        {
          // schedule disk IO tasks
          std::unique_lock<std::mutex> readyTasksGuard(m_readyDiskIOTasksMutex, std::defer_lock);
          int numScheduledTasks = 0;
          while (!m_pendingDiskIOTasks.empty()
                 && m_pendingDiskIOTasks.front()->MemoryCost
                     < m_memoryLeft.load(std::memory_order_relaxed))
          {
            auto task = std::move(m_pendingDiskIOTasks.front());
            m_pendingDiskIOTasks.pop();
            m_memoryLeft.fetch_sub(task->MemoryCost);
            if (!readyTasksGuard.owns_lock())
            {
              readyTasksGuard.lock();
            }
            m_readyDiskIOTasks.push(std::move(task));
            ++numScheduledTasks;
          }
          if (numScheduledTasks != 0)
          {
            readyTasksGuard.unlock();
            m_readyDiskIOTasksCv.notify_all();
          }
        }
        {
          // schedule network tasks
          std::unique_lock<std::mutex> readyTasksGuard(m_readyTasksMutex, std::defer_lock);
          int numScheduledTasks = 0;
          while (true)
          {
            bool shouldBreak = true;
            if (!m_pendingNetworkUploadTasks.empty())
            {
              auto task = std::move(m_pendingNetworkUploadTasks.front());
              m_pendingNetworkUploadTasks.pop();
              if (!readyTasksGuard.owns_lock())
              {
                readyTasksGuard.lock();
              }
              m_readyTasks.push(std::move(task));
              ++numScheduledTasks;
              shouldBreak = false;
            }
            if (!m_pendingNetworkDownloadTasks.empty())
            {
              auto task = std::move(m_pendingNetworkDownloadTasks.front());
              m_pendingNetworkDownloadTasks.pop();
              if (!readyTasksGuard.owns_lock())
              {
                readyTasksGuard.lock();
              }
              m_readyTasks.push(std::move(task));
              ++numScheduledTasks;
              shouldBreak = false;
            }
            if (shouldBreak)
            {
              break;
            }
          }
          if (numScheduledTasks >= m_options.NumThreads.Value())
          {
            readyTasksGuard.unlock();
            m_readyTasksCv.notify_all();
          }
          else if (numScheduledTasks > 0)
          {
            readyTasksGuard.unlock();
            for (int i = 0; i < numScheduledTasks; ++i)
            {
              m_readyTasksCv.notify_one();
            }
          }
        }

        m_pendingTasksCv.wait(guard);
      }
    };

    m_schedulerThread = std::thread(schedulerFunc);
  }

  Scheduler::~Scheduler()
  {
    m_stopped.store(true, std::memory_order_relaxed);
    m_pendingTasksCv.notify_one();
    m_readyDiskIOTasksCv.notify_all();
    m_readyTasksCv.notify_all();
    m_schedulerThread.join();
    for (auto& th : m_workerThreads)
    {
      th.join();
    }
  }

  void Scheduler::AddTask(Task&& task)
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
  }

  void Scheduler::AddTasks(std::vector<Task>&& tasks)
  {
    std::vector<bool> validTaskBitmap(tasks.size(), false);
    {
      std::unique_lock<std::mutex> guard(m_pendingTasksMutex, std::defer_lock);
      int numTasksAdded = 0;
      for (int i = 0; i < tasks.size(); ++i)
      {
        if (tasks[i]->Type == TaskType::DiskIO)
        {
          if (!guard.owns_lock())
          {
            guard.lock();
          }
          m_pendingDiskIOTasks.push(std::move(tasks[i]));
          ++numTasksAdded;
        }
        else if (tasks[i]->Type == TaskType::NetworkUpload)
        {
          if (!guard.owns_lock())
          {
            guard.lock();
          }
          m_pendingNetworkUploadTasks.push(std::move(tasks[i]));
          ++numTasksAdded;
        }
        else if (tasks[i]->Type == TaskType::NetworkDownload)
        {
          if (!guard.owns_lock())
          {
            guard.lock();
          }
          m_pendingNetworkDownloadTasks.push(std::move(tasks[i]));
          ++numTasksAdded;
        }
        else
        {
          validTaskBitmap[i] = true;
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
      for (int i = 0; i < tasks.size(); ++i)
      {
        if (!validTaskBitmap[i])
        {
          continue;
        }
        if (tasks[i]->Type == TaskType::Other)
        {
          if (!guard.owns_lock())
          {
            guard.lock();
          }
          m_readyTasks.push(std::move(tasks[i]));
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
  }

}}}} // namespace Azure::Storage::DataMovement::_internal
