// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "private/environment_log_level_listener.hpp"

#include "azure/core/datetime.hpp"
#include "azure/core/internal/environment.hpp"
#include "azure/core/internal/strings.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <thread>

using Azure::Core::_internal::Environment;
using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Diagnostics::_detail;
using Azure::Core::Diagnostics::_detail::EnvironmentLogLevelListener;

namespace {
Logger::Level const* GetEnvironmentLogLevel()
{
  static Logger::Level* envLogLevelPtr = nullptr;

  if (!EnvironmentLogLevelListener::IsInitialized())
  {
    EnvironmentLogLevelListener::SetInitialized(true);

    auto const envVar = Environment::GetVariable("AZURE_LOG_LEVEL");
    if (!envVar.empty())
    {
      auto const logLevelStr = Azure::Core::_internal::StringExtensions::ToLower(envVar);

      // See https://github.com/Azure/azure-sdk-for-java/wiki/Logging-with-Azure-SDK
      // And
      // https://github.com/Azure/azure-sdk-for-java/blob/main/sdk/core/azure-core/src/main/java/com/azure/core/util/logging/LogLevel.java

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
  }
}
} // namespace

Logger::Level EnvironmentLogLevelListener::GetLogLevel(Logger::Level defaultValue)
{
  auto const envLogLevelPtr = GetEnvironmentLogLevel();
  return envLogLevelPtr ? *envLogLevelPtr : defaultValue;
}

std::function<void(Logger::Level level, std::string const& message)>
EnvironmentLogLevelListener::GetLogListener()
{
  bool const isEnvLogLevelSet = GetEnvironmentLogLevel() != nullptr;
  if (!isEnvLogLevelSet)
  {
    return nullptr;
  }

  static std::function<void(Logger::Level level, std::string const& message)> const consoleLogger =
      [](auto level, auto message) {
        std::stringstream ss;
        ss << '['
           << Azure::DateTime(std::chrono::system_clock::now())
                  .ToString(DateTime::DateFormat::Rfc3339, DateTime::TimeFractionFormat::AllDigits)
           << " T: " << std::this_thread::get_id() << "] " << LogLevelToConsoleString(level)
           << " : " << message << std::endl;
        std::cerr << ss.str();
      };

  return consoleLogger;
}

namespace {
static bool g_initialized;
} // namespace

bool EnvironmentLogLevelListener::IsInitialized() { return g_initialized; }

void EnvironmentLogLevelListener::SetInitialized(bool value) { g_initialized = value; }
