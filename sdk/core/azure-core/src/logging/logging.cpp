// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "logging/logging.hpp"
#include "internal/log.hpp"

#include <mutex>

using namespace Azure::Core::Logging;
using namespace Azure::Core::Logging::Details;

class Azure::Core::Logging::Details::LogClassificationsPrivate {
public:
  static LogClassifications const LogClassificationsConstant(bool all)
  {
    return LogClassifications(all);
  }

  static bool IsClassificationEnabled(LogClassifications const& cls, LogClassification c)
  {
    return cls.m_all || (cls.m_classifications.find(c) != cls.m_classifications.end());
  }
};

namespace {
std::mutex g_loggerMutex;
LogListener g_logListener(nullptr);

LogClassifications g_logClassifications(
    LogClassificationsPrivate::LogClassificationsConstant(true));

LogListener GetLogListener(LogClassification classification)
{
  // lock listener and classifications
  std::lock_guard<std::mutex> loggerLock(g_loggerMutex);

  return (!g_logListener // if no logger is set
          || LogClassificationsPrivate::IsClassificationEnabled(
              g_logClassifications, classification))
      // or if classification is enabled
      ? g_logListener // return actual listener (may be null)
      : LogListener(nullptr) // return null listener
      ;
}
} // namespace

LogClassifications const Azure::Core::Logging::LogClassification::All(
    LogClassificationsPrivate::LogClassificationsConstant(true));

LogClassifications const Azure::Core::Logging::LogClassification::None(
    LogClassificationsPrivate::LogClassificationsConstant(false));

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

bool Azure::Core::Logging::Details::ShouldWrite(LogClassification classification)
{
  return GetLogListener(classification) != nullptr;
}

void Azure::Core::Logging::Details::Write(
    LogClassification classification,
    std::string const& message)
{
  if (auto logListener = GetLogListener(classification))
  {
    logListener(classification, message);
  }
}
