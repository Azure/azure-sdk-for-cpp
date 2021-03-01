// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policy.hpp"
#include "azure/core/internal/log.hpp"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <sstream>
#include <thread>

using Azure::Core::Context;
using namespace Azure::Core::Http;

namespace {
typedef decltype(RetryOptions::RetryDelay) Delay;
typedef decltype(RetryOptions::MaxRetries) RetryNumber;

bool GetResponseHeaderBasedDelay(RawResponse const& response, Delay& retryAfter)
{
  // Try to find retry-after headers. There are several of them possible.
  auto const& responseHeaders = response.GetHeaders();
  auto const responseHeadersEnd = responseHeaders.end();
  auto header = responseHeadersEnd;
  if (((header = responseHeaders.find("retry-after-ms")) != responseHeadersEnd)
      || ((header = responseHeaders.find("x-ms-retry-after-ms")) != responseHeadersEnd))
  {
    // The headers above are in milliseconds.
    retryAfter = std::chrono::milliseconds(std::stoi(header->second));
    return true;
  }

  if ((header = responseHeaders.find("retry-after")) != responseHeadersEnd)
  {
    // This header is in seconds.
    retryAfter = std::chrono::seconds(std::stoi(header->second));
    return true;

    // Tracked by https://github.com/Azure/azure-sdk-for-cpp/issues/262
    // ----------------------------------------------------------------
    //
    // To be accurate, the Retry-After header is EITHER seconds, or a DateTime. So we need to
    // write a parser for that (and handle the case when parsing seconds fails).
    // More info:
    // * Retry-After header: https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Retry-After
    // * HTTP Date format: https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Date
    // * Parsing the date: https://en.cppreference.com/w/cpp/locale/time_get
    // * Get system datetime: https://en.cppreference.com/w/cpp/chrono/system_clock/now
    // * Subtract datetimes to get duration:
    // https://en.cppreference.com/w/cpp/chrono/time_point/operator_arith2
  }

  return false;
}

Delay CalculateExponentialDelay(RetryOptions const& retryOptions, RetryNumber attempt)
{
  constexpr auto beforeLastBit = std::numeric_limits<RetryNumber>::digits
      - (std::numeric_limits<RetryNumber>::is_signed ? 1 : 0);

  // Scale exponentially: 1 x RetryDelay on 1st attempt, 2x on 2nd, 4x on 3rd, 8x on 4th ... all the
  // way up to std::numeric_limits<RetryNumber>::max() * RetryDelay.
  auto exponentialRetryAfter = retryOptions.RetryDelay
      * ((attempt <= beforeLastBit) ? (1 << attempt) : std::numeric_limits<RetryNumber>::max());

  // jitterFactor is a random double number in the range [0.8 .. 1.3)
  auto jitterFactor = 0.8 + (static_cast<double>(std::rand()) / RAND_MAX) * 0.5;

  // Multiply exponentialRetryAfter by jitterFactor
  exponentialRetryAfter = Delay(static_cast<Delay::rep>(
      (std::chrono::duration<double, Delay::period>(exponentialRetryAfter) * jitterFactor)
          .count()));

  return std::min(exponentialRetryAfter, retryOptions.MaxRetryDelay);
}

bool WasLastAttempt(RetryOptions const& retryOptions, RetryNumber attempt)
{
  return attempt > retryOptions.MaxRetries;
}

bool ShouldRetryOnTransportFailure(
    RetryOptions const& retryOptions,
    RetryNumber attempt,
    Delay& retryAfter)
{
  // Are we out of retry attempts?
  if (WasLastAttempt(retryOptions, attempt))
  {
    return false;
  }

  retryAfter = CalculateExponentialDelay(retryOptions, attempt);
  return true;
}

bool ShouldRetryOnResponse(
    RawResponse const& response,
    RetryOptions const& retryOptions,
    RetryNumber attempt,
    Delay& retryAfter)
{
  // Are we out of retry attempts?
  if (WasLastAttempt(retryOptions, attempt))
  {
    return false;
  }

  // Should we retry on the given response retry code?
  auto const& statusCodes = retryOptions.StatusCodes;
  auto const statusCodesEnd = statusCodes.end();
  if (std::find(statusCodes.begin(), statusCodesEnd, response.GetStatusCode()) == statusCodesEnd)
  {
    return false;
  }

  if (!GetResponseHeaderBasedDelay(response, retryAfter))
  {
    retryAfter = CalculateExponentialDelay(retryOptions, attempt);
  }

  return true;
}

static constexpr char RetryKey[] = "azureSdkRetryPolicyCounter";

/**
 * @brief Creates a new #Context node from \p parent with the information about the retrying while
 * sending an Http request.
 *
 * @param parent The parent context for the new created.
 * @return Context with information about retry counter.
 */
Context inline GetRetryContext(Context const& parent)
{
  // First try as default
  int retryCount = 1;
  if (parent.HasKey(RetryKey))
  {
    retryCount = parent[RetryKey].Get<int>() + 1;
  }
  return parent.WithValue(RetryKey, retryCount);
}
} // namespace

int Azure::Core::Http::RetryPolicy::GetRetryNumber(Context const& context)
{
  if (!context.HasKey(RetryKey))
  {
    // Not in retry mode.
    return 0;
  }
  return context[RetryKey].Get<int>();
}

std::unique_ptr<RawResponse> Azure::Core::Http::RetryPolicy::Send(
    Context const& ctx,
    Request& request,
    NextHttpPolicy nextHttpPolicy) const
{
  auto const shouldLog = Logging::Internal::ShouldLog(Logging::LogLevel::Informational);
  auto retryContext = GetRetryContext(ctx);
  for (RetryNumber attempt = 1;; ++attempt)
  {
    Delay retryAfter{};
    request.StartTry();
    // creates a copy of original query parameters from request
    auto originalQueryParameters = request.GetUrl().GetQueryParameters();

    try
    {
      auto response = nextHttpPolicy.Send(retryContext, request);

      // If we are out of retry attempts, if a response is non-retriable (or simply 200 OK, i.e
      // doesn't need to be retried), then ShouldRetry returns false.
      if (!ShouldRetryOnResponse(*response.get(), m_retryOptions, attempt, retryAfter))
      {
        // If this is the second attempt and StartTry was called, we need to stop it. Otherwise
        // trying to perform same request would use last retry query/headers
        return response;
      }
    }
    catch (const TransportException&)
    {
      if (!ShouldRetryOnTransportFailure(m_retryOptions, attempt, retryAfter))
      {
        throw;
      }
    }

    if (shouldLog)
    {
      std::ostringstream log;

      log << "HTTP Retry attempt #" << attempt << " will be made in "
          << std::chrono::duration_cast<std::chrono::milliseconds>(retryAfter).count() << "ms.";

      Logging::Internal::Log(Logging::LogLevel::Informational, log.str());
    }

    // Sleep(0) behavior is implementation-defined: it may yield, or may do nothing. Let's make sure
    // we proceed immediately if it is 0.
    if (retryAfter.count() > 0)
    {
      ctx.ThrowIfCancelled();
      std::this_thread::sleep_for(retryAfter);
    }

    // Restore the original query parameters before next retry
    request.GetUrl().SetQueryParameters(std::move(originalQueryParameters));

    // Update retry number
    retryContext = GetRetryContext(retryContext);
  }
}
