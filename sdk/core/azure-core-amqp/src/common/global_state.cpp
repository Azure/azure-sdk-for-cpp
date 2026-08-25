// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words xlogging

#include "azure/core/amqp/internal/common/global_state.hpp"

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/internal/diagnostics/log.hpp>
#include <azure/core/internal/unique_handle.hpp>

#if ENABLE_UAMQP
#include <azure_c_shared_utility/gballoc.h>
#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/xlogging.h>
#endif

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

// cspell: words gballoc
namespace Azure { namespace Core { namespace Amqp { namespace Common { namespace _detail {

#if ENABLE_UAMQP
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
        logLevel = Logger::Level::Warning;
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
#endif

  GlobalStateHolder::GlobalStateHolder()
  {
#if ENABLE_UAMQP
#if defined(GB_DEBUG_ALLOC)
    gballoc_init();
#endif
    if (platform_init())
    {
      throw std::runtime_error("Could not initialize platform.");
    }

    // Integrate AMQP logging with Azure Core logging.
    xlogging_set_log_function(AmqpLogFunction);

    m_pollingThread = std::thread([this]() {
      while (true)
      {
        std::list<std::shared_ptr<Pollable>> capturedList;
        uint64_t pollingGeneration;
        {
          std::unique_lock<std::mutex> lock{m_pollablesMutex};
          m_pollingCondition.wait(lock, [this]() { return m_stopped || !m_pollables.empty(); });
          if (m_stopped)
          {
            break;
          }
          capturedList = m_pollables;
          pollingGeneration = ++m_pollingGeneration;
        }

        for (auto const& pollable : capturedList)
        {
          pollable->Poll();
        }
        capturedList.clear();

        {
          std::lock_guard<std::mutex> lock{m_pollablesMutex};
          m_completedGeneration = pollingGeneration;
        }
        m_pollingCondition.notify_all();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    });
#endif
  }

  GlobalStateHolder::~GlobalStateHolder()
  {
#if ENABLE_UAMQP
    {
      std::lock_guard<std::mutex> lock{m_pollablesMutex};
      m_stopped = true;
    }
    m_pollingCondition.notify_all();
    if (m_pollingThread.joinable())
    {
      m_pollingThread.join();
    }
    platform_deinit();
#if defined(GB_DEBUG_ALLOC)
    gballoc_deinit();
#endif
#endif
  }

#if ENABLE_UAMQP
  /**
   * @brief Adds a pollable object to the list of objects to be polled.
   *
   * @param pollable The pollable object to add.
   *
   * @note Do not hold a connection or link lock when calling AddPollable or RemovePollable. The
   * polling thread acquires those locks while it polls an item, and RemovePollable waits for the
   * active polling generation to complete.
   *
   */
  void GlobalStateHolder::AddPollable(std::shared_ptr<Pollable> pollable)
  {
    std::lock_guard<std::mutex> lock{m_pollablesMutex};
    if (std::find(m_pollables.begin(), m_pollables.end(), pollable) == m_pollables.end())
    {
      m_pollables.push_back(pollable);
    }
    m_pollingCondition.notify_one();
  }

  void GlobalStateHolder::RemovePollable(std::shared_ptr<Pollable> pollable)
  {
    std::unique_lock<std::mutex> lock{m_pollablesMutex};
    m_pollables.remove(pollable);
    auto const pollingGeneration = m_pollingGeneration;
    m_pollingCondition.wait(
        lock, [this, pollingGeneration]() { return m_completedGeneration >= pollingGeneration; });
  }
#endif

  GlobalStateHolder* GlobalStateHolder::GlobalStateInstance()
  {
    static GlobalStateHolder globalState;
    return &globalState;
  }

}}}}} // namespace Azure::Core::Amqp::Common::_detail

#if ENABLE_RUST_AMQP
namespace Azure { namespace Core { namespace Amqp { namespace _detail {
  void UniqueHandleHelper<RustRuntimeContext>::FreeRuntimeContext(RustRuntimeContext* obj)
  {
    Azure::Core::Amqp::RustInterop::_detail::runtime_context_delete(obj);
  }

  void UniqueHandleHelper<RustCallContext>::FreeCallContext(RustCallContext* obj)
  {
    Azure::Core::Amqp::RustInterop::_detail::call_context_delete(obj);
  }

  //  void UniqueHandleHelper<Azure::Core::Amqp::_detail::RustAmqpError>::FreeRustError(
  //      RustAmqpError* obj)
  //  {
  //    Azure::Core::Amqp::_detail::RustInterop::rust_error_delete(obj);
  //  }

}}}} // namespace Azure::Core::Amqp::_detail
#endif
