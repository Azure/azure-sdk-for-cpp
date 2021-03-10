// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include "azure/core/logger.hpp"

#if defined(AZ_PLATFORM_WINDOWS)
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif
#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <windows.h>
#endif

namespace Azure { namespace Core { namespace Details {

  class EnvironmentLogLevelListener {
    EnvironmentLogLevelListener() = delete;
    ~EnvironmentLogLevelListener() = delete;

  public:
    static Logger::Level GetLogLevel(Logger::Level defaultValue);
    static Logger::Listener GetLogListener();
  };

#if (defined(WINAPI_PARTITION_DESKTOP) && !WINAPI_PARTITION_DESKTOP) // See azure/core/platform.hpp
                                                                     // for explanation.
  inline Logger::Level EnvironmentLogLevelListener::GetLogLevel(Logger::Level defaultValue)
  {
    return defaultValue;
  }

  inline Logger::Listener EnvironmentLogLevelListener::GetLogListener() { return nullptr; }
#endif
}}} // namespace Azure::Core::Details
