// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-Licence-Identifier: MIT

#include "azure/core/amqp/common/global_state.hpp"
#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/xlogging.h>
#include <cassert>
#include <stdarg.h>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <stdexcept>

using namespace Azure::Core::Diagnostics::_internal;
using namespace Azure::Core::Diagnostics;

namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _detail {

  void AmqpLogFunction(
      LOG_CATEGORY logCategory,
      const char* file,
      const char* func,
      int line,
      unsigned int, // Either LOG_NONE or LOG_LINE
      const char* format,
      ...)
  {
    Logger::Level logLevel;
    switch (logCategory)
    {
      case AZ_LOG_ERROR:
        logLevel = Logger::Level::Error;
        break;
      case AZ_LOG_INFO:
        logLevel = Logger::Level::Informational;
        break;
      case AZ_LOG_TRACE:
        logLevel = Logger::Level::Verbose;
        break;
      default:
        logLevel = Logger::Level::Verbose;
    }
    std::stringstream ss;
    ss << "File: " << file << "Func: " << func << ":" << line;

    char outputBuffer[512];
    va_list args;
    va_start(args, format);
    vsprintf_s(outputBuffer, format, args);
    ss << "Msg: " << outputBuffer;
    Log::Write(logLevel, ss.str());
    va_end(args);
  }

  GlobalStateHolder::GlobalStateHolder()
  {
    if (platform_init())
    {
      throw std::runtime_error("Could not initialize platform."); // LCOV_EXCL_LINE
    }

    xlogging_set_log_function(AmqpLogFunction);
  }

  GlobalStateHolder::~GlobalStateHolder() { platform_deinit(); }

  GlobalStateHolder* GlobalStateHolder::GlobalStateInstance()
  {
    static GlobalStateHolder globalState;
    return &globalState;
  }

}}}}} // namespace Azure::Core::Amqp::Common::_detail
