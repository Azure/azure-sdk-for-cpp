// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/logger.hpp"

#include <mutex>

using namespace Azure::Core;

std::shared_timed_mutex Logger::g_mutex;
Logger::Listener Logger::g_listener(nullptr);
LogLevel Logger::g_level(LogLevel::Warning);

Logger::Listener Logger::GetListener(LogLevel level)
{
  std::unique_lock<std::shared_timed_mutex> loggerLock(g_mutex);
  return level <= g_level ? g_listener : Listener(nullptr);
}

void Logger::SetListener(Logger::Listener listener)
{
  std::unique_lock<std::shared_timed_mutex> loggerLock(g_mutex);
  g_listener = std::move(listener);
}

void Logger::SetLevel(LogLevel level)
{
  std::unique_lock<std::shared_timed_mutex> loggerLock(g_mutex);
  g_level = level;
}
