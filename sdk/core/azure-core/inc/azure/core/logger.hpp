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

namespace Azure { namespace Core {
  /**
   * @brief Log message handler.
   */
  class Logger {
  public:
    /**
     * @brief Log message level.
     */
    enum class Level
    {
      /// Logging level for failures that the application is unlikely to recover from.
      Error = 10,

      /// Logging level when a function fails to perform its intended task.
      Warning = 20,

      /// Logging level when a function operates normally.
      Informational = 30,

      /// Logging level for detailed troubleshooting scenarios.
      Verbose = 40,
    };

    /**
     * @brief Defines the signature of the callback function that application developers must write
     * in order to receive Azure SDK log messages.
     *
     * @param level The log message level.
     * @param message The log message.
     */
    typedef std::function<void(Level level, std::string const& message)> Listener;

  public:
    /**
     * @brief Set the function that will be invoked to report an SDK log message.
     *
     * @param listener An #Azure::Core::Logger::Listener function that will be invoked when
     * the SDK reports a log message. If `nullptr`, no function will be invoked.
     */
    static void SetListener(Listener listener);

    /**
     * @brief Sets the #Azure::Core::Logger::Level an application is interested in receiving.
     *
     * @param level Maximum log level.
     */
    static void SetLevel(Level level);

  private:
    Logger() = delete;
    ~Logger() = delete;
  };
}} // namespace Azure::Core
