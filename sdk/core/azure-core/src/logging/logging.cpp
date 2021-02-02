// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/logging/logging.hpp"
#include "azure/core/internal/log.hpp"

#include <mutex>

using namespace Azure::Core::Logging;
using namespace Azure::Core::Logging::Internal;

namespace {
std::mutex g_loggerMutex;
LogListener g_logListener(nullptr);
LogLevel g_logLevel = LogLevel::Verbose;

LogListener GetLogListener(LogLevel level)
{
  std::lock_guard<std::mutex> loggerLock(g_loggerMutex);
  return (g_logListener && g_logLevel <= level) ? g_logListener : LogListener(nullptr);
}
} // namespace

void Azure::Core::Logging::SetLogListener(LogListener logListener)
{
  std::lock_guard<std::mutex> loggerLock(g_loggerMutex);
  g_logListener = std::move(logListener);
}

void Azure::Core::Logging::SetLogLevel(LogLevel level)
{
  std::lock_guard<std::mutex> loggerLock(g_loggerMutex);
  g_logLevel = level;
}

bool Azure::Core::Logging::Internal::ShouldLog(LogLevel level)
{
  return GetLogListener(level) != nullptr;
}

void Azure::Core::Logging::Internal::Log(LogLevel level, std::string const& message)
{
  if (auto logListener = GetLogListener(level))
  {
    logListener(level, message);
  }
}
