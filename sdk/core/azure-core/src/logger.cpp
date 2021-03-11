// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/logger.hpp"
#include "azure/core/internal/log.hpp"

#include <mutex>
#include <shared_mutex>

#include "environment_log_level_listener_private.hpp"

using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Diagnostics::_internal;

namespace {
static std::shared_timed_mutex g_logListenerMutex;
static Logger::Listener g_logListener(_detail::EnvironmentLogLevelListener::GetLogListener());
} // namespace

std::atomic<bool> Log::g_isLoggingEnabled(
    _detail::EnvironmentLogLevelListener::GetLogListener() != nullptr);

std::atomic<Log::LogLevelInt> Log::g_logLevel(static_cast<LogLevelInt>(
    _detail::EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Warning)));

inline void Log::EnableLogging(bool isEnabled) { g_isLoggingEnabled = isEnabled; }

inline void Log::SetLogLevel(Logger::Level logLevel)
{
  g_logLevel = static_cast<LogLevelInt>(logLevel);
}

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

void Logger::SetListener(Logger::Listener listener)
{
  std::unique_lock<std::shared_timed_mutex> loggerLock(g_logListenerMutex);
  g_logListener = std::move(listener);
  Log::EnableLogging(g_logListener != nullptr);
}

void Logger::SetLevel(Logger::Level level) { Log::SetLogLevel(level); }
