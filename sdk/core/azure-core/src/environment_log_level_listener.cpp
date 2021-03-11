// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

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

#if (!defined(WINAPI_PARTITION_DESKTOP) || WINAPI_PARTITION_DESKTOP) // See azure/core/platform.hpp
                                                                     // for explanation.

#include "environment_log_level_listener_private.hpp"

#include "azure/core/datetime.hpp"
#include "azure/core/internal/strings.hpp"

#include <iostream>
#include <string>

using namespace Azure::Core;
using namespace Azure::Core::_detail;

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
      auto const logLevelStr = _internal::Strings::ToLower(envVar);

      // See https://github.com/Azure/azure-sdk-for-java/wiki/Logging-with-Azure-SDK
      // And
      // https://github.com/Azure/azure-sdk-for-java/blob/master/sdk/core/azure-core/src/main/java/com/azure/core/util/logging/LogLevel.java

      static Logger::Level envLogLevel = {};
      envLogLevelPtr = &envLogLevel;

      if (logLevelStr == "error" || logLevelStr == "err" || logLevelStr == "4")
      {
        envLogLevel = Logger::Level::Error;
      }
      else if (logLevelStr == "warning" || logLevelStr == "warn" || logLevelStr == "3")
      {
        envLogLevel = Logger::Level::Warning;
      }
      else if (
          logLevelStr == "informational" || logLevelStr == "info" || logLevelStr == "information"
          || logLevelStr == "2")
      {
        envLogLevel = Logger::Level::Informational;
      }
      else if (logLevelStr == "verbose" || logLevelStr == "debug" || logLevelStr == "1")
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

// Log level textual representation, including space padding, matches slf4j and log4net.
std::string const ErrorText = "ERROR";
std::string const WarningText = "WARN ";
std::string const InformationalText = "INFO ";
std::string const VerboseText = "DEBUG";
std::string const UnknownText = "?????";

inline std::string const& LogLevelToConsoleString(Logger::Level logLevel)
{
  switch (logLevel)
  {
    case Logger::Level::Error:
      return ErrorText;

    case Logger::Level::Warning:
      return WarningText;

    case Logger::Level::Informational:
      return InformationalText;

    case Logger::Level::Verbose:
      return VerboseText;

    default:
      return UnknownText;
  };
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
    std::cerr << '['
              << Azure::Core::DateTime(std::chrono::system_clock::now())
                     .ToString(
                         DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits)
              << "] " << LogLevelToConsoleString(level) << " : " << message << std::endl;
  };

  return consoleLogger;
}

#endif
