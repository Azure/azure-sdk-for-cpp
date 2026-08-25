// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <gtest/gtest.h>

#if ENABLE_UAMQP

#include "azure/core/amqp/internal/common/global_state.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <thread>

namespace Azure { namespace Core { namespace Amqp { namespace Tests {
  namespace {
    using GlobalStateHolder = Common::_detail::GlobalStateHolder;
    using Pollable = Common::_detail::Pollable;

    class Latch final {
    public:
      void Signal()
      {
        {
          std::lock_guard<std::mutex> lock(m_mutex);
          m_signaled = true;
        }
        m_condition.notify_all();
      }

      void Wait()
      {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this]() { return m_signaled; });
      }

    private:
      std::condition_variable m_condition;
      std::mutex m_mutex;
      bool m_signaled{false};
    };

    class GatedPollable final : public Pollable {
    public:
      explicit GatedPollable(bool blockPoll) : m_blockPoll{blockPoll} {}

      void Poll() override
      {
        bool expected = false;
        if (m_pollStarted.compare_exchange_strong(expected, true))
        {
          m_started.Signal();
        }
        if (m_blockPoll)
        {
          m_release.Wait();
        }
        m_pollCount++;
      }

      void WaitForPollStart() { m_started.Wait(); }
      void Release() { m_release.Signal(); }
      int PollCount() const { return m_pollCount.load(); }

    private:
      bool m_blockPoll;
      std::atomic<bool> m_pollStarted{false};
      std::atomic<int> m_pollCount{0};
      Latch m_started;
      Latch m_release;
    };

    template <class Predicate>
    bool WaitUntil(Predicate predicate, std::chrono::milliseconds timeout)
    {
      auto const deadline = std::chrono::steady_clock::now() + timeout;
      while (!predicate() && std::chrono::steady_clock::now() < deadline)
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
      return predicate();
    }
  } // namespace

  TEST(GlobalStateHolderTest, ConcurrentRegistryOperationsWhilePollBlocked)
  {
    auto const globalState = GlobalStateHolder::GlobalStateInstance();
    globalState->AssertIdle();

    auto blockedPollable = std::make_shared<GatedPollable>(true);
    auto removedPollableOne = std::make_shared<GatedPollable>(false);
    auto removedPollableTwo = std::make_shared<GatedPollable>(false);
    globalState->AddPollable(blockedPollable);
    globalState->AddPollable(removedPollableOne);
    globalState->AddPollable(removedPollableTwo);
    blockedPollable->WaitForPollStart();

    auto removeBlocked = std::async(
        std::launch::async, [globalState, blockedPollable]() {
          globalState->RemovePollable(blockedPollable);
        });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto removeOne = std::async(
        std::launch::async, [globalState, removedPollableOne]() {
          globalState->RemovePollable(removedPollableOne);
        });
    auto removeTwo = std::async(
        std::launch::async, [globalState, removedPollableTwo]() {
          globalState->RemovePollable(removedPollableTwo);
        });

    auto addedPollableOne = std::make_shared<GatedPollable>(false);
    auto addedPollableTwo = std::make_shared<GatedPollable>(false);
    auto addOne = std::async(
        std::launch::async, [globalState, addedPollableOne]() {
          globalState->AddPollable(addedPollableOne);
        });
    auto addTwo = std::async(
        std::launch::async, [globalState, addedPollableTwo]() {
          globalState->AddPollable(addedPollableTwo);
        });

    bool const addsCompletedBeforePollRelease
        = addOne.wait_for(std::chrono::milliseconds(200)) == std::future_status::ready
        && addTwo.wait_for(std::chrono::milliseconds(200)) == std::future_status::ready;

    blockedPollable->Release();
    removeOne.get();
    removeTwo.get();
    removeBlocked.get();
    addOne.get();
    addTwo.get();

    globalState->RemovePollable(addedPollableOne);
    globalState->RemovePollable(addedPollableTwo);
    globalState->AssertIdle();

    EXPECT_TRUE(addsCompletedBeforePollRelease);
  }

  TEST(GlobalStateHolderTest, RemovedPollableIsACompletionBarrier)
  {
    auto const globalState = GlobalStateHolder::GlobalStateInstance();
    globalState->AssertIdle();

    auto pollable = std::make_shared<GatedPollable>(true);
    globalState->AddPollable(pollable);
    pollable->WaitForPollStart();

    auto remove = std::async(
        std::launch::async, [globalState, pollable]() {
          globalState->RemovePollable(pollable);
        });
    bool const removeCompletedBeforePollRelease
        = remove.wait_for(std::chrono::milliseconds(200)) == std::future_status::ready;

    pollable->Release();
    remove.get();
    auto const pollCountAfterRemove = pollable->PollCount();
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    bool const noPollAfterRemove = pollable->PollCount() == pollCountAfterRemove;

    globalState->AddPollable(pollable);
    bool const pollRanAfterAdd = WaitUntil(
        [pollable, pollCountAfterRemove]() {
          return pollable->PollCount() > pollCountAfterRemove;
        },
        std::chrono::milliseconds(500));
    globalState->RemovePollable(pollable);
    globalState->AssertIdle();

    EXPECT_FALSE(removeCompletedBeforePollRelease);
    EXPECT_TRUE(noPollAfterRemove);
    EXPECT_TRUE(pollRanAfterAdd);
  }
}}}} // namespace Azure::Core::Amqp::Tests

#endif // ENABLE_UAMQP
