// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#if defined(AZ_NO_ENV_LOGGER)
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
#endif

#if !defined(AZ_NO_ENV_LOGGER) \
    && (!defined(WINAPI_PARTITION_DESKTOP) \
        || WINAPI_PARTITION_DESKTOP) // See azure/core/platform.hpp for explanation.

#include "environment_log_level_listener_private.hpp"

#include "azure/core/datetime.hpp"
#include "azure/core/internal/strings.hpp"

#include <iostream>
#include <string>

using namespace Azure::Core;
using namespace Azure::Core::Details;

namespace {
Logger::Level const* GetEnvironmentLogLevel()
{
  static Logger::Level* envLogLevelPtr = nullptr;

  static bool initialized = false;
  if (!initialized)
  {
    initialized = true;

#if defined(_MSC_VER)
#pragma warning(push)
// warning C4996: 'getenv': This function or variable may be unsafe. Consider using _dupenv_s
// instead.
#pragma warning(disable : 4996)
#endif
    auto envVar = std::getenv("AZURE_LOG_LEVEL");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

    if (envVar)
    {
      auto const logLevelStr = Internal::Strings::ToLower(envVar);

      // See https://github.com/Azure/azure-sdk-for-java/wiki/Logging-with-Azure-SDK
      static Logger::Level envLogLevel = {};
      envLogLevelPtr = &envLogLevel;

      if (logLevelStr == "error" || logLevelStr == "err")
      {
        envLogLevel = Logger::Level::Error;
      }
      else if (logLevelStr == "warning" || logLevelStr == "warn")
      {
        envLogLevel = Logger::Level::Warning;
      }
      else if (
          logLevelStr == "informational" || logLevelStr == "info" || logLevelStr == "information")
      {
        envLogLevel = Logger::Level::Informational;
      }
      else if (logLevelStr == "verbose" || logLevelStr == "debug")
      {
        envLogLevel = Logger::Level::Verbose;
      }
      else
      {
        envLogLevelPtr = nullptr;
      }
    }
  }

  return envLogLevelPtr;
}
} // namespace

Logger::Level EnvironmentLogLevelListener::GetLogLevel(Logger::Level defaultValue)
{
  auto const envLogLevelPtr = GetEnvironmentLogLevel();
  return envLogLevelPtr ? *envLogLevelPtr : defaultValue;
}

Logger::Listener EnvironmentLogLevelListener::GetLogListener()
{
  bool const isEnvLogLevelSet = GetEnvironmentLogLevel() != nullptr;
  if (!isEnvLogLevelSet)
  {
    return nullptr;
  }

  static Logger::Listener const consoleLogger = [](auto level, auto message) {
    std::cerr << '[' << Azure::Core::DateTime(std::chrono::system_clock::now()) << "] " << level
              << ": " << message << std::endl;
  };

  return consoleLogger;
}

#endif
