// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words xlogging

#include "azure/core/amqp/common/global_state.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>

#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/xlogging.h>

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <list>
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
    // We don't want to log header information for outgoing and incoming frames, the header
    // information gets in the way of the message.
    if (logCategory == AZ_LOG_TRACE
        && (strcmp(func, "log_outgoing_frame") == 0 || strcmp(func, "log_incoming_frame") == 0
            || strcmp(func, "log_message_chunk") == 0 || strcmp(func, "_log_outgoing_frame") == 0
            || strcmp(func, "_log_incoming_frame") == 0))
    {
    }
    else
    {
      ss << "File: " << file << ":" << line << " Func: " << func << ": ";
    }
    char outputBuffer[2048];
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
      throw std::runtime_error("Could not initialize platform."); 
    }

    // Integrate AMQP logging with Azure Core logging.
    xlogging_set_log_function(AmqpLogFunction);

    m_pollingThread = std::thread([this]() {
      do
      {
        std::list<std::shared_ptr<Pollable>> capturedList;
        {
          std::unique_lock<std::mutex> lock{m_pollablesMutex};
          // If there are no pollables, there's no point in doing any work.
          if (m_pollables.empty())
          {
            continue;
          }
          capturedList = m_pollables;
        }

        for (auto const& pollable : capturedList)
        {
          pollable->Poll();
        }
        //        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      } while (!m_stopped);
    });
  }

  GlobalStateHolder::~GlobalStateHolder()
  {
    m_stopped = true;
    if (m_pollingThread.joinable())
    {
      m_pollingThread.join();
    }
    platform_deinit();
  }

  void GlobalStateHolder::AddPollable(std::shared_ptr<Pollable> pollable)
  {
    std::lock_guard<std::mutex> lock(m_pollablesMutex);
    if (std::find(m_pollables.begin(), m_pollables.end(), pollable) == m_pollables.end())
    {
      m_pollables.push_back(pollable);
    }
  }

  GlobalStateHolder* GlobalStateHolder::GlobalStateInstance()
  {
    static GlobalStateHolder globalState;
    return &globalState;
  }

}}}}} // namespace Azure::Core::Amqp::Common::_detail
