// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief This header defines the types and functions your application uses to be notified of Azure
 * SDK client library log messages.
 */

#pragma once

#include <functional>
#include <string>

namespace Azure { namespace Core { namespace Logging {
  /**
   * @brief Log message level.
   */
  enum class LogLevel
  {
    /// Logging level for failures that the application is unlikely to recover from.
    Error,

    /// Logging level when a function fails to perform its intended task.
    Warning,

    /// Logging level when a function operates normally.
    Informational,

    /// Logging level for detailed troubleshooting scenarios.
    Verbose,
  };

  /**
   * @brief Defines the signature of the callback function that application developers must write in
   * order to receive Azure SDK log messages.
   *
   * @param level The log message level.
   * @param message The log message.
   */
  typedef std::function<void(LogLevel level, std::string const& message)> LogListener;

  /**
   * @brief Set the function that will be invoked to report an SDK log message.
   *
   * @param logListener A #Azure::Core::Logging::LogListener function that will be invoked when the
   * SDK reports a log message matching one of the log levels passed to
   * #Azure::Core::Logging::SetLogLevel(). If `nullptr`, no function will be invoked.
   */
  void SetLogListener(LogListener logListener);

  /**
   * @brief Sets the #Azure::Core::Logging::LogLevel an application is interested in receiving.
   *
   * @param level Maximum log level.
   */
  void SetLogLevel(LogLevel level);
}}} // namespace Azure::Core::Logging
