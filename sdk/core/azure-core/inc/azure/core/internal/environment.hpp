// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <atomic>
#include <cstdlib>
#include <functional>

#include "azure/core/dll_import_export.hpp"

#include "azure/core/platform.hpp"

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <windows.h>
#endif

#if !defined(WINAPI_PARTITION_DESKTOP) \
    || WINAPI_PARTITION_DESKTOP // See azure/core/platform.hpp for explanation.
namespace Azure { namespace Core { namespace _internal {
  class Environment {
#if defined(AZ_BUILD_TESTING)
  public:
    typedef std::function<char const*(char const* varName)> GetEnvCallback;

  private:
    static_assert(ATOMIC_BOOL_LOCK_FREE == 2, "atomic<bool> must be lock-free");
    static AZ_CORE_DLLEXPORT std::atomic<bool> g_isOverridden;
    static GetEnvCallback::result_type OverriddenGetEnv(char const* varName);

    static void Override(GetEnvCallback getEnv);

  public:
    Environment(GetEnvCallback getEnv) { Override(getEnv); }
    ~Environment() { Override(nullptr); }

    static char const* Get(char const* varName)
    {
#if defined(_MSC_VER)
#pragma warning(push)
// warning C4996: 'getenv': This function or variable may be unsafe. Consider using _dupenv_s
// instead.
#pragma warning(disable : 4996)
#endif
        return g_isOverridden ? OverriddenGetEnv(varName) : std::getenv(varName);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
      }
#else
  public:
    static char const* Get(char const* varName)
    {
#if defined(_MSC_VER)
#pragma warning(push)
// warning C4996: 'getenv': This function or variable may be unsafe. Consider using _dupenv_s
// instead.
#pragma warning(disable : 4996)
#endif
        return std::getenv(varName);
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
      }
#endif
    };
}}} // namespace Azure::Core::_internal
#endif
