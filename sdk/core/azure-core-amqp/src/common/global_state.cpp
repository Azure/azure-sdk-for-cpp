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
      do
      {
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
            m_activelyPolling = true;
          }

          for (auto const& pollable : capturedList)
          {
            pollable->Poll();
          }
        }
        m_activelyPolling = false;
        //        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      } while (!m_stopped);
    });
#endif
  }

  GlobalStateHolder::~GlobalStateHolder()
  {
#if ENABLE_UAMQP
    m_stopped = true;
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
   * @note Note that you *cannot* hold any connection or link locks when calling AddPollable. This
   * is because the the AddPollable function attempts to lock the pollable and the RemovePollable
   * function blocks until any pollables have completed while holding the pollable lock.
   *
   * This can result in a deadlock because the polling thread is also going to acquire the
   * connection lock resulting in a deadlock.
   *
   */
  void GlobalStateHolder::AddPollable(std::shared_ptr<Pollable> pollable)
  {
    std::lock_guard<std::mutex> lock(m_pollablesMutex);
    if (std::find(m_pollables.begin(), m_pollables.end(), pollable) == m_pollables.end())
    {
      m_pollables.push_back(pollable);
    }
  }

  void GlobalStateHolder::RemovePollable(std::shared_ptr<Pollable> pollable)
  {
    // There is a bit of a complicated lock-free dance happening here.
    // The m_pollables list is accessed by the polling thread, and the list is modified by the user
    // thread. To ensure integrity of the list, the polling thread takes the lock, copies the
    // pollable from the list, releases the lock and then iterates over the pollables at the
    // snapshot.
    //
    // Because the pollable is a shared_ptr, the user thread can remove a pollable while the
    // background thread is polling.
    //
    // But we want to make sure that the thread has finished polling (and thus has removed the copy
    // of the pollables list). For that, we have the m_activelyPolling variable. It is set under the
    // pollables lock, and cleared after the polling thread has finished polling (outside the lock).
    //
    // This means that we can spin on the m_activelyPolling variable *under* the pollables lock safe
    // in the knowledge that IF the variable is set to true, it means that we acquired the
    // pollablesMutex during the interval when the captured list is being interated over. And that
    // the m_activelyPolling variable will only be cleared AFTER the captured list is freed.
    //

    std::lock_guard<std::mutex> lock(m_pollablesMutex);
    m_pollables.remove(pollable);
    // Spin until m_activelyPolling is false, this ensures that the polling thread is not using the
    // capturedList copy of m_pollables.
    while (m_activelyPolling.load())
      ;
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
  void UniqueHandleHelper<RustRuntimeContext>::FreeRuntimeContext(
      RustRuntimeContext* obj)
  {
    Azure::Core::Amqp::RustInterop::_detail::runtime_context_delete(obj);
  }

  void UniqueHandleHelper<RustCallContext>::FreeCallContext(
      RustCallContext* obj)
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
