// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "logging/logging.hpp"
#include "internal/log.hpp"

#include <mutex>

using namespace Azure::Core::Logging;
using namespace Azure::Core::Logging::Details;

class Azure::Core::Logging::Details::LogClassificationConstantProvider {
public:
  static LogClassifications const LogClassificationsConstant(bool all)
  {
    return LogClassifications(all);
  }
};

namespace {
std::mutex g_loggerMutex;
LogListener g_logListener(nullptr);

LogClassifications g_logClassifications(
    LogClassificationConstantProvider::LogClassificationsConstant(true));

LogListener GetLogListener(LogClassification const& classification)
{
  // lock listener and classifications
  std::lock_guard<std::mutex> loggerLock(g_loggerMutex);

  return (!g_logListener // if no logger is set
          || g_logClassifications.IsClassificationEnabled(classification) // or if classification is enabled
      ? g_logListener // return actual listener (may be null)
      : LogListener() // return null listener
      ;
}
} // namespace

LogClassifications const Azure::Core::Logging::LogClassification::All(
    LogClassificationConstantProvider::LogClassificationsConstant(true));

LogClassifications const Azure::Core::Logging::LogClassification::None(
    LogClassificationConstantProvider::LogClassificationsConstant(false));

void Azure::Core::Logging::SetLogListener(LogListener logListener)
{
  std::lock_guard<std::mutex> loggerLock(g_loggerMutex);
  g_logListener = std::move(logListener);
}

void Azure::Core::Logging::SetLogClassifications(LogClassifications logClassifications)
{
  std::lock_guard<std::mutex> loggerLock(g_loggerMutex);
  g_logClassifications = std::move(logClassifications);
}

bool Azure::Core::Logging::Details::ShouldWrite(LogClassification const& classification)
{
  return GetLogListener(classification) != nullptr;
}

void Azure::Core::Logging::Details::Write(
    LogClassification const& classification,
    std::string const& message)
{
  if (auto logListener = GetLogListener(classification))
  {
    logListener(classification, message);
  }
}
