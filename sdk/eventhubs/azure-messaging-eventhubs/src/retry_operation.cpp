// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "private/retry_operation.hpp"

#include "azure/messaging/eventhubs/eventhubs_exception.hpp"

#include <azure/core/internal/diagnostics/log.hpp>

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <system_error>
#include <thread>

namespace {
constexpr std::chrono::milliseconds CancellationCheckInterval{100};

bool IsRetryableSystemError(std::error_code const& errorCode)
{
  return errorCode == std::errc::broken_pipe || errorCode == std::errc::connection_aborted
      || errorCode == std::errc::connection_refused || errorCode == std::errc::connection_reset
      || errorCode == std::errc::interrupted || errorCode == std::errc::io_error
      || errorCode == std::errc::network_down || errorCode == std::errc::network_reset
      || errorCode == std::errc::network_unreachable || errorCode == std::errc::no_buffer_space
      || errorCode == std::errc::not_connected || errorCode == std::errc::operation_would_block
      || errorCode == std::errc::resource_unavailable_try_again
      || errorCode == std::errc::timed_out;
}

void WaitForRetryDelay(std::chrono::milliseconds retryAfter, Azure::Core::Context const& context)
{
  auto const deadline = std::chrono::steady_clock::now() + retryAfter;
  while (true)
  {
    context.ThrowIfCancelled();

    auto const now = std::chrono::steady_clock::now();
    if (now >= deadline)
    {
      return;
    }

    std::this_thread::sleep_until((std::min)(deadline, now + CancellationCheckInterval));
  }
}
} // namespace

bool Azure::Messaging::EventHubs::_detail::RetryOperation::Execute(
    std::function<bool()> operation,
    Azure::Core::Context const& context)
{
  using Azure::Core::Diagnostics::Logger;
  using Azure::Core::Diagnostics::_internal::Log;

  int32_t retryCount = 0;
  while (true)
  {
    context.ThrowIfCancelled();
    std::chrono::milliseconds retryAfter{};

    try
    {
      bool result = operation();
      if (!result)
      {
        context.ThrowIfCancelled();
      }

      if (!ShouldRetry(result, retryCount, retryAfter))
      {
        return result;
      }
    }
    catch (EventHubsException const& e)
    {
      context.ThrowIfCancelled();
      if (Log::ShouldWrite(Logger::Level::Warning))
      {
        Log::Stream(Logger::Level::Warning)
            << "Exception thrown. " << e.ErrorCondition << " - " << e.ErrorDescription << std::endl;
      }
      if (!ShouldRetry(e, retryCount, retryAfter))
      {
        throw;
      }
    }
    catch (Azure::Core::OperationCancelledException const&)
    {
      throw;
    }
    catch (std::system_error const& e)
    {
      context.ThrowIfCancelled();
      if (Log::ShouldWrite(Logger::Level::Warning))
      {
        Log::Write(Logger::Level::Warning, std::string("System error while trying: ") + e.what());
      }
      if (!IsRetryableSystemError(e.code()) || !ShouldRetry(false, retryCount, retryAfter))
      {
        throw;
      }
    }
#if ENABLE_RUST_AMQP
    catch (std::runtime_error const& e)
    {
      context.ThrowIfCancelled();
      if (Log::ShouldWrite(Logger::Level::Warning))
      {
        Log::Write(Logger::Level::Warning, std::string("Runtime error while trying: ") + e.what());
      }
      if (!ShouldRetry(false, retryCount, retryAfter))
      {
        throw;
      }
    }
#endif

    ++retryCount;
    WaitForRetryDelay(retryAfter, context);
  }
}

bool Azure::Messaging::EventHubs::_detail::RetryOperation::ShouldRetry(
    bool response,
    int32_t attempt,
    std::chrono::milliseconds& retryAfter,
    double jitterFactor)
{
  using Azure::Core::Diagnostics::Logger;
  using Azure::Core::Diagnostics::_internal::Log;

  if (response)
  {
    Log::Write(
        Logger::Level::Informational,
        std::string("Operation completed successfully and will not be retried."));
    return false;
  }

  if (WasLastAttempt(attempt))
  {
    Log::Write(
        Logger::Level::Informational,
        std::string("Retry attempts exhausted. Operation will not be retried."));
    return false;
  }

  retryAfter = CalculateExponentialDelay(attempt + 1, jitterFactor);
  Log::Write(Logger::Level::Informational, std::string("Operation failed and will be retried."));

  return true;
}

bool Azure::Messaging::EventHubs::_detail::RetryOperation::ShouldRetry(
    Azure::Messaging::EventHubs::EventHubsException const& exception,
    int32_t attempt,
    std::chrono::milliseconds& retryAfter,
    double jitterFactor)
{
  using Azure::Core::Diagnostics::Logger;
  using Azure::Core::Diagnostics::_internal::Log;

  if (!exception.IsTransient)
  {
    Log::Write(
        Logger::Level::Informational,
        std::string("Event Hubs exception is not transient. Operation will not be retried."));
    return false;
  }

  return ShouldRetry(false, attempt, retryAfter, jitterFactor);
}

std::chrono::milliseconds Azure::Messaging::EventHubs::_detail::RetryOperation::
    CalculateExponentialDelay(int32_t attempt, double jitterFactor)
{
  if (jitterFactor < 0.8 || jitterFactor > 1.3)
  {
    // jitterFactor is a random double number in the range [0.8 .. 1.3]
    jitterFactor
        = 0.8 + ((static_cast<double>(static_cast<int32_t>(std::rand())) / RAND_MAX) * 0.5);
  }

  constexpr auto beforeLastBit
      = std::numeric_limits<int32_t>::digits - (std::numeric_limits<int32_t>::is_signed ? 1 : 0);

  // Scale exponentially: 1 x RetryDelay on 1st attempt, 2x on 2nd, 4x on 3rd, 8x on 4th ... all
  // the way up to (std::numeric_limits<int32_t>::max()) * RetryDelay.
  auto exponentialRetryAfter = m_retryOptions.RetryDelay
      * (((attempt - 1) <= beforeLastBit) ? (1 << (attempt - 1))
                                          : (std::numeric_limits<int32_t>::max)());

  // Multiply exponentialRetryAfter by jitterFactor
  exponentialRetryAfter = std::chrono::milliseconds(
      static_cast<std::chrono::milliseconds::rep>(
          (std::chrono::duration<double, std::chrono::milliseconds::period>(exponentialRetryAfter)
           * jitterFactor)
              .count()));

  return (std::min)(exponentialRetryAfter, m_retryOptions.MaxRetryDelay);
}
