// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/logger.hpp"
#include "azure/core/internal/log.hpp"

#include <mutex>
#include <shared_mutex>

using namespace Azure::Core;

namespace {
static std::shared_timed_mutex g_logMutex;
static Logger::Listener g_logListener(nullptr);
static LogLevel g_logLevel(LogLevel::Warning);
} // namespace

void Logger::SetListener(Logger::Listener listener)
{
  std::unique_lock<std::shared_timed_mutex> loggerLock(g_logMutex);
  g_logListener = std::move(listener);
}

void Logger::SetLevel(LogLevel level)
{
  std::unique_lock<std::shared_timed_mutex> loggerLock(g_logMutex);
  g_logLevel = level;
}

Logger::Listener Azure::Core::Details::GetLogListener(LogLevel level)
{
  std::unique_lock<std::shared_timed_mutex> loggerLock(g_logMutex);
  return level <= g_logLevel ? g_logListener : Logger::Listener(nullptr);
}
