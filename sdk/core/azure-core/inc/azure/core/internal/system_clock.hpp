// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <chrono>
#include <functional>

#include "azure/core/dll_import_export.hpp"
#include "azure/core/http/transport.hpp"

namespace Azure { namespace Core { namespace _internal {
  class SystemClock {
#if defined(AZ_BUILD_TESTING)
  public:
    typedef std::function<std::chrono::system_clock::time_point()> NowCallback;

  private:
    static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "atomic<bool> must be lock-free");
    static AZ_CORE_DLLEXPORT std::atomic<bool> g_isOverridden;
    static NowCallback::result_type OverriddenNow();

    static void Override(NowCallback now);

  public:
    SystemClock(NowCallback now) { Override(now); }
    ~SystemClock() { Override(nullptr); }

    static std::chrono::system_clock::time_point Now()
    {
      return g_isOverridden ? OverriddenNow() : std::chrono::system_clock::now();
    }
#else
  public:
    static std::chrono::system_clock::time_point Now() { return std::chrono::system_clock::now(); }
#endif
  };
}}} // namespace Azure::Core::_internal
