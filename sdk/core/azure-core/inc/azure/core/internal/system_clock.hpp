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
    SystemClock() = delete;
    ~SystemClock() = delete;

    static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "atomic<bool> must be lock-free");
    static AZ_CORE_DLLEXPORT std::atomic<bool> g_isOverridden;
    static std::chrono::system_clock::time_point OverriddenNow();

  public:
    static std::chrono::system_clock::time_point Now()
    {
      return g_isOverridden ? OverriddenNow() : std::chrono::system_clock::now();
    }

    static void Override(std::function<std::chrono::system_clock::time_point()> now);
  };
}}} // namespace Azure::Core::_internal
