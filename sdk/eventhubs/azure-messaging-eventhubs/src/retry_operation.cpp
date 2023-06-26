// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include "azure/core/internal/diagnostics/log.hpp"

#include <azure/messaging/eventhubs/retry_operation.hpp>

bool Azure::Messaging::EventHubs::_internal::RetryOperation::Execute(
    std::function<bool()> operation)
{
  using Azure::Core::Diagnostics::Logger;
  using Azure::Core::Diagnostics::_internal::Log;
  int retryCount = 0;
  while (retryCount < m_retryOptions.MaxRetries)
  {
    std::chrono::milliseconds retryAfter{};

    try
    {
      bool result = operation();

      if (ShouldRetry(result, retryCount, retryAfter))
      {
        retryCount++;
        std::this_thread::sleep_for(retryAfter);
      }
      else
      {
        return result;
      }
    }
    catch (std::exception const& e)
    {
      if (Log::ShouldWrite(Logger::Level::Warning))
      {
        Log::Write(Logger::Level::Warning, std::string("Exception while trying ") + e.what());
      }
      if (ShouldRetry(false, retryCount, retryAfter))
      {
        retryCount++;
        std::this_thread::sleep_for(retryAfter);
      }
    }
  }
  return false;
}

bool Azure::Messaging::EventHubs::_internal::RetryOperation::ShouldRetry(
    bool response,
    int32_t attempt,
    std::chrono::milliseconds& retryAfter,
    double jitterFactor)
{
  using Azure::Core::Diagnostics::Logger;
  using Azure::Core::Diagnostics::_internal::Log;

  // Are we out of retry attempts?
  if (response || WasLastAttempt(attempt))
  {
    Log::Write(
        Logger::Level::Informational,
        std::string("Response was true or lase attempt.Operation will not be retried."));
    return false;
  }
  if (response == false)
  {
    Log::Write(
        Logger::Level::Informational, std::string("Response was false.Operation will be retried."));
  }

  retryAfter = CalculateExponentialDelay(attempt, jitterFactor);

  return true;
}

std::chrono::milliseconds Azure::Messaging::EventHubs::_internal::RetryOperation::
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
  // the way up to std::numeric_limits<int32_t>::max() * RetryDelay.
  auto exponentialRetryAfter = m_retryOptions.RetryDelay
      * (((attempt - 1) <= beforeLastBit) ? (1 << (attempt - 1))
                                          : std::numeric_limits<int32_t>::max());

  // Multiply exponentialRetryAfter by jitterFactor
  exponentialRetryAfter = std::chrono::milliseconds(static_cast<std::chrono::milliseconds::rep>(
      (std::chrono::duration<double, std::chrono::milliseconds::period>(exponentialRetryAfter)
       * jitterFactor)
          .count()));

  return std::min(exponentialRetryAfter, m_retryOptions.MaxRetryDelay);
}
