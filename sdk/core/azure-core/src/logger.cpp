// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/diagnostics/logger.hpp"

#include "azure/core/internal/diagnostics/log.hpp"
#include "private/environment_log_level_listener.hpp"

#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <sstream>

using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Diagnostics::_internal;

namespace {
static std::shared_timed_mutex g_logListenerMutex;
static std::function<void(Logger::Level level, std::string const& message)> g_logListener(
    _detail::EnvironmentLogLevelListener::GetLogListener());
} // namespace

std::atomic<bool> Log::g_isLoggingEnabled(
    _detail::EnvironmentLogLevelListener::GetLogListener() != nullptr);

std::atomic<Logger::Level> Log::g_logLevel(
    _detail::EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Warning));

inline void Log::EnableLogging(bool isEnabled) { g_isLoggingEnabled = isEnabled; }

inline void Log::SetLogLevel(Logger::Level logLevel) { g_logLevel = logLevel; }

void Log::Write(Logger::Level level, std::string const& message)
{
  if (ShouldWrite(level))
  {
    std::shared_lock<std::shared_timed_mutex> loggerLock(g_logListenerMutex);
    if (g_logListener)
    {
      g_logListener(level, message);
    }
  }
}

void Logger::SetListener(
    std::function<void(Logger::Level level, std::string const& message)> listener)
{
  std::unique_lock<std::shared_timed_mutex> loggerLock(g_logListenerMutex);
  g_logListener = std::move(listener);
  Log::EnableLogging(g_logListener != nullptr);
}

void Logger::SetLevel(Logger::Level level) { Log::SetLogLevel(level); }

int Log::LoggerStringBuffer::sync()
{
  std::string val{str()};
  if (!val.empty())
  {
    Log::Write(m_level, val);
  }
  str(std::string());
  return 0;
}
namespace {
static Log::LoggerStream g_verboseLogger{Logger::Level::Verbose};
static Log::LoggerStream g_informationalLogger{Logger::Level::Informational};
static Log::LoggerStream g_warningLogger{Logger::Level::Warning};
static Log::LoggerStream g_errorLogger{Logger::Level::Error};
} // namespace

/** Returns a custom ostream implementation with a logger based stream buffer.
 *  @param level The level of the log message.
 *  @return A custom ostream implementation.
 */
Log::LoggerStream& Log::GetStream(Logger::Level level)
{
  switch (level)
  {
    case Logger::Level::Verbose:
      return g_verboseLogger;
    case Logger::Level::Informational:
      return g_informationalLogger;
    case Logger::Level::Warning:
      return g_warningLogger;
    case Logger::Level::Error:
      return g_errorLogger;
  }
  throw std::runtime_error("Unknown stream logger level.");
}
