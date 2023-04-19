// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/context.hpp>
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
  template <typename... T> class AsyncOperationQueue {

  public:
    AsyncOperationQueue() = default;
    ~AsyncOperationQueue() = default;

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
        Azure::Core::Context context,
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

    template <class PT, class... Ts> void Poll(PT const& first, Ts const&... rest)
    {
      first.Poll();
      Poll(rest...);
    }
  };
}}}}} // namespace Azure::Core::Amqp::Common::_internal
