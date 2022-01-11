// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/diagnostics/logger.hpp"
#include "azure/core/internal/diagnostics/log.hpp"

#include <mutex>

#include "private/environment_log_level_listener.hpp"

using namespace Azure::Core::Diagnostics;
using namespace Azure::Core::Diagnostics::_internal;

bool Log::Impl::IsAlive;
Log::Impl Log::Impl::Instance;

Log::Impl::Impl()
    : LogLevel(_detail::EnvironmentLogLevelListener::GetLogLevel(Logger::Level::Warning)),
      LogListener(_detail::EnvironmentLogLevelListener::GetLogListener())

{
  IsLoggingEnabled = (LogListener != nullptr);
  IsAlive = true;
}

Log::Impl::~Impl() { IsAlive = false; }

inline void Log::SetLogLevel(Logger::Level logLevel)
{
  if (Impl::IsAlive)
  {
    Impl::Instance.LogLevel = logLevel;
  }
}

void Log::Write(Logger::Level level, std::string const& message)
{
  if (ShouldWrite(level))
  {
    std::shared_lock<std::shared_timed_mutex> loggerLock(Impl::Instance.LogListenerMutex);
    if (Impl::Instance.LogListener)
    {
      Impl::Instance.LogListener(level, message);
    }
  }
}

void Log::SetListener(std::function<void(Logger::Level level, std::string const& message)> listener)
{
  if (Impl::IsAlive)
  {
    std::unique_lock<std::shared_timed_mutex> loggerLock(Impl::Instance.LogListenerMutex);
    Impl::Instance.LogListener = std::move(listener);
    Impl::Instance.IsLoggingEnabled = (Impl::Instance.LogListener != nullptr);
  }
}

void Logger::SetListener(
    std::function<void(Logger::Level level, std::string const& message)> listener)
{
  Log::SetListener(listener);
}

void Logger::SetLevel(Logger::Level level) { Log::SetLogLevel(level); }
