// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/diagnostics/logger.hpp"

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

// This use of windows.h within the header is OK because the header is private and in source only.
#include <windows.h>
#endif

namespace Azure { namespace Core { namespace Diagnostics { namespace _detail {

  class EnvironmentLogLevelListener final {
    EnvironmentLogLevelListener() = delete;
    ~EnvironmentLogLevelListener() = delete;

  public:
    static Logger::Level GetLogLevel(Logger::Level defaultValue);
    static std::function<void(Logger::Level level, std::string const& message)> GetLogListener();
    static bool IsInitialized();
    static void SetInitialized(bool value);
  };

#if (defined(WINAPI_PARTITION_DESKTOP) && !WINAPI_PARTITION_DESKTOP) // See azure/core/platform.hpp
                                                                     // for explanation.
  inline Logger::Level EnvironmentLogLevelListener::GetLogLevel(Logger::Level defaultValue)
  {
    return defaultValue;
  }

  inline std::function<void(Logger::Level level, std::string const& message)>
  EnvironmentLogLevelListener::GetLogListener()
  {
    return nullptr;
  }
#endif
}}}} // namespace Azure::Core::Diagnostics::_detail
