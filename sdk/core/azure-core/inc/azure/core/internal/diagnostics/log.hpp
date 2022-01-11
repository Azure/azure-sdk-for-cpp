// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/diagnostics/logger.hpp"
#include "azure/core/dll_import_export.hpp"

#include <atomic>
#include <shared_mutex>
#include <type_traits>

namespace Azure { namespace Core { namespace Diagnostics { namespace _internal {
  class Log final {
  private:
    class Impl {
    private:
      Impl();
      ~Impl();

    public:
      static AZ_CORE_DLLEXPORT bool IsAlive;
      static AZ_CORE_DLLEXPORT Impl Instance;

      static_assert(
          std::is_same<int, std::underlying_type<Logger::Level>::type>::value == true,
          "Logger::Level values must be representable as lock-free");

      static_assert(ATOMIC_INT_LOCK_FREE == 2, "atomic<int> must be lock-free");

      static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "atomic<bool> must be lock-free");

      std::atomic<bool> IsLoggingEnabled;
      std::atomic<Logger::Level> LogLevel;

      std::shared_timed_mutex LogListenerMutex;
      std::function<void(Logger::Level level, std::string const& message)> LogListener;
    };

    Log() = delete;
    ~Log() = delete;

  public:
    static bool ShouldWrite(Logger::Level level)
    {
      return Impl::IsAlive && Impl::Instance.IsLoggingEnabled && level >= Impl::Instance.LogLevel;
    }

    static void Write(Logger::Level level, std::string const& message);

    static void SetLogLevel(Logger::Level logLevel);

    static void SetListener(
        std::function<void(Logger::Level level, std::string const& message)> listener);
  };
}}}} // namespace Azure::Core::Diagnostics::_internal
