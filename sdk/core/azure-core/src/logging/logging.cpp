// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "logging/logging.hpp"
#include "internal/log.hpp"

#include <mutex>
#include <set>
#include <utility>

using namespace Azure::Core::Logging;
using namespace Azure::Core::Logging::Details;

namespace {
typedef std::set<LogClassification, LogClassificationsCompare> LogClassificationsSet;

std::mutex g_loggerMutex;
LogListener g_logListener = nullptr;

LogClassificationsSet g_logClassifications;
auto g_allClassificationsEnabled = true;

LogListener GetLogListener(LogClassification classification) noexcept
{
  // lock listener and classifications
  std::lock_guard<std::mutex> loggerLock(g_loggerMutex);

  return (!g_logListener // if no logger is set
          || g_allClassificationsEnabled // or if no classifications filter is specified
          || (g_logClassifications.find(classification)
              != g_logClassifications.end())) // or if this classification is enabled in the list
      ? g_logListener // return actual listener (may be null)
      : LogListener() // return null listener
      ;
}
} // namespace

void Azure::Core::Logging::SetLogListener(LogListener logListener)
{
  std::lock_guard<std::mutex> loggerLock(g_loggerMutex);
  g_logListener = std::move(logListener);
}

void Azure::Core::Logging::ResetLogListener()
{
  std::lock_guard<std::mutex> loggerLock(g_loggerMutex);
  g_logListener = nullptr;
}

void Azure::Core::Logging::SetLogClassifications(std::vector<LogClassification> const& logClassifications)
{
  LogClassificationsSet set(logClassifications.begin(), logClassifications.end());

  std::lock_guard<std::mutex> loggerLock(g_loggerMutex);
  g_logClassifications = std::move(set);
  g_allClassificationsEnabled = false;
}

void Azure::Core::Logging::ResetLogClassifications()
{
  std::lock_guard<std::mutex> loggerLock(g_loggerMutex);

  g_allClassificationsEnabled = true;
  g_logClassifications = {};
}

bool Azure::Core::Logging::Details::ShouldWrite(LogClassification classification) noexcept
{
  return GetLogListener(classification) != nullptr;
}

void Azure::Core::Logging::Details::Write(
    LogClassification classification,
    std::string const& message) noexcept
{
  if (auto logListener = GetLogListener(classification))
  {
    logListener(classification, message);
  }
}
