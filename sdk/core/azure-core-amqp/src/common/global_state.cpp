// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

// cspell: words xlogging

#include "azure/core/amqp/common/global_state.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/xlogging.h>

#include <cassert>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <stdarg.h>
#include <stdexcept>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _detail {

  // Logging callback for uAMQP and azure-c-shared-utility.
  void AmqpLogFunction(
      LOG_CATEGORY logCategory,
      const char* file,
      const char* func,
      int line,
      unsigned int options, // Either LOG_NONE or LOG_LINE
      const char* format,
      ...)
  {
    Logger::Level logLevel;
    // We accumulate the log message in a thread_local string, and only write it to the logger when
    // LOG_LINE is set. This allows the caller to accumulate traces to be logged on a single line.
    thread_local std::string accumulatedString;

    switch (logCategory)
    {
      case AZ_LOG_ERROR:
        logLevel = Logger::Level::Error;
        break;
      case AZ_LOG_INFO: // LCOV_EXCL_LINE
        logLevel = Logger::Level::Informational; // LCOV_EXCL_LINE
        break; // LCOV_EXCL_LINE
      case AZ_LOG_TRACE:
        logLevel = Logger::Level::Verbose;
        break;
      default: // LCOV_EXCL_LINE
        logLevel = Logger::Level::Verbose; // LCOV_EXCL_LINE
    }
    std::stringstream ss;
    // We don't want to log header information for outgoing and incoming frames, the header
    // information gets in the way of the message.
    if (logCategory == AZ_LOG_TRACE
        && (strcmp(func, "log_outgoing_frame") == 0 || strcmp(func, "log_incoming_frame") == 0
            || strcmp(func, "log_message_chunk") == 0))
    {
    }
    else
    {
      ss << "File: " << file << ":" << line << " Func: " << func << ": ";
    }
    char outputBuffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(outputBuffer, sizeof(outputBuffer), format, args);
    ss << outputBuffer;
    if (options == LOG_NONE)
    {
      accumulatedString += ss.str();
    }
    else
    {
      accumulatedString += ss.str() + "\n";
      Log::Write(logLevel, accumulatedString);
      accumulatedString.clear();
    }
    va_end(args);
  }

  GlobalStateHolder::GlobalStateHolder()
  {
    if (platform_init())
    {
      throw std::runtime_error("Could not initialize platform."); // LCOV_EXCL_LINE
    }

    // Integrate AMQP logging with Azure Core logging.
    xlogging_set_log_function(AmqpLogFunction);
  }

  GlobalStateHolder::~GlobalStateHolder() { platform_deinit(); }

  GlobalStateHolder* GlobalStateHolder::GlobalStateInstance()
  {
    static GlobalStateHolder globalState;
    return &globalState;
  }

}}}}} // namespace Azure::Core::Amqp::Common::_detail
