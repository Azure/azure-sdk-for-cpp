// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/** @file global_state.hpp
 *
 * @note This file has no customer visible types, it should never be included in a customer
 * facing project.
 */

#pragma once

#include <azure/core/azure_assert.hpp>

#include <list>
#include <memory>
#include <mutex>
#include <thread>

namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _detail {

  /**
   * uAMQP and azure-c-shared-util require that the platform_init and platform_uninit functions be
   * called before using the various API functions.
   *
   * The GlobalState class maintains a singleton static local variable using [static local
   * variables](https://en.cppreference.com/w/cpp/language/storage_duration#Static_local_variables),
   * also known as "Magic Statics". This ensures that we initialize only once.
   */
  class Pollable {
  public:
    virtual void Poll() = 0;
    virtual ~Pollable() = default;
  };
  class GlobalStateHolder final {
    GlobalStateHolder();
    ~GlobalStateHolder();

    std::list<std::shared_ptr<Pollable>> m_pollables;
    std::mutex m_pollablesMutex;
    std::thread m_pollingThread;
    bool m_stopped{false};

  public:
    static GlobalStateHolder* GlobalStateInstance();

    GlobalStateHolder(GlobalStateHolder const&) = delete;
    GlobalStateHolder& operator=(GlobalStateHolder const&) = delete;

    GlobalStateHolder(GlobalStateHolder&&) = delete;
    GlobalStateHolder& operator=(GlobalStateHolder&&) = delete;

    void AddPollable(std::shared_ptr<Pollable> pollable);

    void RemovePollable(std::shared_ptr<Pollable> pollable)
    {
      std::lock_guard<std::mutex> lock(m_pollablesMutex);
      m_pollables.remove(pollable);
    }

    // Assert that the global state is idle. Used at the end of test collateral to ensure that all
    // pollables have been disposed.
    void AssertIdle()
    {
      std::lock_guard<std::mutex> lock(m_pollablesMutex);
      AZURE_ASSERT(m_pollables.empty());
      if (!m_pollables.empty())
      {
        Azure::Core::_internal::AzureNoReturnPath("Global state is not idle.");
      }
    }
  };
}}}}} // namespace Azure::Core::Amqp::Common::_detail
