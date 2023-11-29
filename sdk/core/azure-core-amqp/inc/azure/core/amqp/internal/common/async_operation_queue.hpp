// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/context.hpp>
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <condition_variable>
#include <list>
#include <mutex>
#include <thread>
#include <tuple>

namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _internal {

  /** An AsyncOperationQueue represents a queue of "results" for an async operation.
   *
   * It expresses a relatively simple API contract. The code which produces results calls
   * "CompleteOperation" which sets the result, and a consumer calls WaitForResult which reads from
   * the AsyncOperationQueue. WaitForResult will block until a result is available.
   */
  template <typename... T> class AsyncOperationQueue final {
  public:
    AsyncOperationQueue() = default;
    ~AsyncOperationQueue() = default;

    AsyncOperationQueue(const AsyncOperationQueue&) = delete;
    AsyncOperationQueue& operator=(const AsyncOperationQueue&) = delete;

    AsyncOperationQueue(AsyncOperationQueue&&) = default;
    AsyncOperationQueue& operator=(AsyncOperationQueue&&) = default;

    void CompleteOperation(T... operationParameters)
    {
      std::unique_lock<std::mutex> lock(m_operationComplete);
      m_operationQueue.push_back(std::make_unique<std::tuple<T...>>(
          std::make_tuple<T...>(std::forward<T>(operationParameters)...)));
      lock.unlock();
      m_operationCondition.notify_one();
    }

    template <class... Poller>
    std::unique_ptr<std::tuple<T...>> WaitForPolledResult(
        Context const& context,
        Poller&... pollers)
    {
      do
      {
        {
          std::unique_lock<std::mutex> lock(m_operationComplete);
          if (!m_operationQueue.empty())
          {
            std::unique_ptr<std::tuple<T...>> rv;
            rv = std::move(m_operationQueue.front());
            m_operationQueue.pop_front();
            return rv;
          }
          if (context.IsCancelled())
          {
            return nullptr;
          }
        }
        std::this_thread::yield();

        // Note: We need to call Poll() *outside* the lock because the poller is going to call the
        // CompleteOperation function.
        Poll(pollers...);
      } while (true);
    }

    /**
     * @brief Wait for a result to be available.
     *
     * @param context The context to use for cancellation.
     * @param pollers optional set of pollers to call.
     * @return std::unique_ptr<std::tuple<T...>> The result.
     *
     * @remarks The pollers parameter is a TEST HOOK to allow test message receivers to interact
     * with the message loop. in general clients should NOT provide a poller.
     *
     */
    template <class... Poller>
    std::unique_ptr<std::tuple<T...>> WaitForResult(Context const& context, Poller&... pollers)
    {
      // If the queue is not empty, return the first element.
      do
      {
        {
          std::unique_lock<std::mutex> lock(m_operationComplete);

          if (!m_operationQueue.empty())
          {
            std::unique_ptr<std::tuple<T...>> rv;
            rv = std::move(m_operationQueue.front());
            m_operationQueue.pop_front();
            return rv;
          }

          // There's nothing in the queue, wait until something is put into the queue.
          // This will block until either something is put into the queue or the context is
          // cancelled.
          m_operationCondition.wait_for(
              lock, std::chrono::milliseconds(100), [this, &context]() -> bool {
                // If the context is cancelled, we should return immediately.
                if (context.IsCancelled())
                {
                  return true;
                }
                return !m_operationQueue.empty();
              });

          if (context.IsCancelled())
          {
            return nullptr;
          }
        }
        // Note: We need to call Poll() *outside* the lock because the poller is going to call the
        // CompleteOperation function.
        Poll(pollers...);
      } while (true);
    }

    /**
     * @brief Tries to wait for a result to be available.
     *
     * @return std::unique_ptr<std::tuple<T...>> The result. If no result is available, returns
     * nullptr.
     */
    std::unique_ptr<std::tuple<T...>> TryWaitForResult()
    {
      // If the queue is not empty, return the first element.
      std::unique_lock<std::mutex> lock(m_operationComplete);

      if (!m_operationQueue.empty())
      {
        std::unique_ptr<std::tuple<T...>> rv;
        rv = std::move(m_operationQueue.front());
        m_operationQueue.pop_front();
        return rv;
      }
      return nullptr;
    }

    // Clear any pending elements from the queue. This may be needed because some queued elements
    // may have ordering dependencies that need to be cleared before the object containing the queue
    // can be released.
    void Clear()
    {
      std::unique_lock<std::mutex> lock(m_operationComplete);
      m_operationQueue.clear();
    }

  private:
    std::mutex m_operationComplete;
    std::condition_variable m_operationCondition;
    std::list<std::unique_ptr<std::tuple<T...>>> m_operationQueue;

    void Poll() {}

    template <class PT, class... Ts> void Poll(PT& first, Ts&... rest)
    {
      first.Poll();
      Poll(rest...);
    }
  };
}}}}} // namespace Azure::Core::Amqp::Common::_internal


