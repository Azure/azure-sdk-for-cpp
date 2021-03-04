// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/logger.hpp"
#include "azure/core/internal/log.hpp"

#include <mutex>
#include <shared_mutex>

using namespace Azure::Core;
using namespace Azure::Core::Internal;

namespace {
static std::shared_timed_mutex g_logListenerMutex;
static Logger::Listener g_logListener(nullptr);
} // namespace

std::atomic<bool> Log::g_isLoggingEnabled(false);
std::atomic<Log::LogLevelInt> Log::g_logLevel(static_cast<LogLevelInt>(Logger::Level::Warning));

inline void Log::EnableLogging(bool isEnabled) { g_isLoggingEnabled = isEnabled; }

void Logger::SetListener(Logger::Listener listener)
{
  std::unique_lock<std::shared_timed_mutex> loggerLock(g_logListenerMutex);
  g_logListener = std::move(listener);
  Log::EnableLogging(g_logListener != nullptr);
}

Logger::Listener Log::GetLogListener()
{
  std::unique_lock<std::shared_timed_mutex> loggerLock(g_logListenerMutex);
  return g_logListener;
}

inline void Log::SetLogLevel(Logger::Level logLevel)
{
  g_logLevel = static_cast<LogLevelInt>(logLevel);
}

void Logger::SetLevel(Logger::Level level) { Log::SetLogLevel(level); }
