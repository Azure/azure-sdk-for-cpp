// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <azure/core/azure_assert.hpp>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
#include <thread>

#if ENABLE_RUST_AMQP
#include "runtime_context.hpp"
#endif

namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _detail {

#if ENABLE_UAMQP
  /**
   * uAMQP and azure-c-shared-util require that the platform_init and platform_uninit
   * functions be called before using the various API functions.
   *
   * The GlobalState class maintains a singleton static local variable using [static local
   * variables](https://en.cppreference.com/w/cpp/language/storage_duration#Static_local_variables),
   * also known as "Magic Statics". This ensures that we initialize only once.
   *
   */

  class Pollable {
  public:
    Pollable() = default;
    Pollable(const Pollable&) = delete;
    Pollable& operator=(const Pollable&) = delete;
    Pollable(Pollable&&) = delete;
    Pollable& operator=(Pollable&&) = delete;

    virtual void Poll() = 0;
    virtual ~Pollable() = default;
  };

#endif

  class GlobalStateHolder final {
    GlobalStateHolder();
    ~GlobalStateHolder();

#if ENABLE_UAMQP
    std::list<std::shared_ptr<Pollable>> m_pollables;
    std::mutex m_pollablesMutex;
    std::condition_variable m_pollingCondition;
    uint64_t m_pollingGeneration{0};
    uint64_t m_completedGeneration{0};
    std::thread m_pollingThread;
    bool m_stopped{false};
#elif ENABLE_RUST_AMQP
    RustRuntimeContext m_runtimeContext;
#endif

  public:
    static GlobalStateHolder* GlobalStateInstance();

    GlobalStateHolder(GlobalStateHolder const&) = delete;
    GlobalStateHolder& operator=(GlobalStateHolder const&) = delete;

    GlobalStateHolder(GlobalStateHolder&&) = delete;
    GlobalStateHolder& operator=(GlobalStateHolder&&) = delete;

#if ENABLE_UAMQP
    void AddPollable(std::shared_ptr<Pollable> pollable);

    void RemovePollable(std::shared_ptr<Pollable> pollable);
#elif ENABLE_RUST_AMQP
    Azure::Core::Amqp::_detail::RustRuntimeContext* GetRuntimeContext()
    {
      return m_runtimeContext.GetRuntimeContext();
    }
#endif

    void AssertIdle()
    {
#if ENABLE_UAMQP
      std::lock_guard<std::mutex> lock(m_pollablesMutex);
      AZURE_ASSERT(m_pollables.empty());
      if (!m_pollables.empty())
      {
        Azure::Core::_internal::AzureNoReturnPath("Global state is not idle.");
      }
#endif
    }
  };
}}}}} // namespace Azure::Core::Amqp::Common::_detail
