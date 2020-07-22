// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "logging/logging.hpp"
#include "internal/log.hpp"

#include <mutex>
#include <utility>

using namespace Azure::Core::Logging;
using namespace Azure::Core::Logging::Details;

namespace {
std::mutex g_loggerMutex;

LogListener g_logListener;

std::set<LogClassification> g_logClassifications;
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

namespace Azure { namespace Core { namespace Logging {
  void SetLogListener(LogListener logListener)
  {
    std::lock_guard<std::mutex> loggerLock(g_loggerMutex);
    g_logListener = std::move(logListener);
  }

  void ResetLogListener()
  {
    std::lock_guard<std::mutex> loggerLock(g_loggerMutex);
    g_logListener = {};
  }

  void SetLogClassifications(std::set<LogClassification> logClassifications)
  {
    std::lock_guard<std::mutex> loggerLock(g_loggerMutex);

    g_logClassifications = std::move(logClassifications);
    g_allClassificationsEnabled = false;
  }

  void ResetLogClassifications()
  {
    std::lock_guard<std::mutex> loggerLock(g_loggerMutex);

    g_allClassificationsEnabled = true;
    g_logClassifications = {};
  }

  namespace Details {
    bool ShouldWrite(LogClassification classification) noexcept
    {
      return GetLogListener(classification) != nullptr;
    }

    void Write(LogClassification classification, std::string const& message) noexcept
    {
      if (auto logListener = GetLogListener(classification))
      {
        logListener(classification, message);
      }
    }
  } // namespace Details
}}} // namespace Azure::Core::Logging
