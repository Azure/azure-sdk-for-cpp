// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <http/policy.hpp>

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <thread>

using namespace Azure::Core::Http;

namespace {
decltype(RetryOptions::RetryDelay) ShouldRetry(
    Response const& response,
    RetryOptions const& retryOptions,
    decltype(RetryOptions::MaxRetries) attempt)
{
  // Are we out of retry attempts?
  if (attempt > retryOptions.MaxRetries)
  {
    return std::chrono::seconds(-1);
  }

  // Should we retry on the given response retry code?
  auto const& statusCodes = retryOptions.StatusCodes;
  auto const statusCodesEnd = statusCodes.end();
  if (std::find(statusCodes.begin(), statusCodesEnd, response.GetStatusCode()) == statusCodesEnd)
  {
    return std::chrono::seconds(-1);
  }

  // Try to find retry-after headers. There are several of them possible.
  decltype(response.GetHeaders()) responseHeaders = response.GetHeaders();
  auto const responseHeadersEnd = responseHeaders.end();
  auto header = responseHeadersEnd;
  if (((header = responseHeaders.find("retry-after-ms")) != responseHeadersEnd)
      || ((header = responseHeaders.find("x-ms-retry-after-ms")) != responseHeadersEnd))
  {
    // The headers above are in milliseconds.
    return std::chrono::milliseconds(std::stoi(header->second));
  }

  if ((header = responseHeaders.find("Retry-After")) != responseHeadersEnd)
  {
    // This header is in seconds.
    return std::chrono::seconds(std::stoi(header->second));

    // TODO: to be accurate, the Retry-After header is EITHER seconds, or a DateTime. So we need to
    // write a parser for that (and handle the case when parsing seconds fails).
    // More info:
    // * Retry-After header: https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Retry-After
    // * HTTP Date format: https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Date
    // * Parsing the date: https://en.cppreference.com/w/cpp/locale/time_get
    // * Get system datetime: https://en.cppreference.com/w/cpp/chrono/system_clock/now
    // * Subtract datetimes to get duration:
    // https://en.cppreference.com/w/cpp/chrono/time_point/operator_arith2
  }

  constexpr auto beforeLastBit = std::numeric_limits<decltype(attempt)>::digits
      - (std::numeric_limits<decltype(attempt)>::is_signed ? 1 : 0);

  // Scale exponentially: 1 x RetryDelay on 1st attempt, 2x on 2nd, 4x on 3rd, 8x on 4th ... all the
  // way up to std::numeric_limits<decltype(attempt)>::max() * RetryDelay.
  auto exponentialRetryAfter = retryOptions.RetryDelay
      * ((attempt <= beforeLastBit) ? (1 << attempt)
                                    : std::numeric_limits<decltype(attempt)>::max());

  // jitterFactor is a random double number in the range [0.8 .. 1.3)
  auto jitterFactor = 0.8 + (static_cast<double>(std::rand()) / RAND_MAX) * 0.5;

  // Multiply exponentialRetryAfter by jitterFactor
  exponentialRetryAfter = decltype(
      exponentialRetryAfter)(static_cast<decltype(exponentialRetryAfter)::rep>(
      (std::chrono::duration<double, decltype(exponentialRetryAfter)::period>(exponentialRetryAfter)
       * jitterFactor)
          .count()));

  return std::min(exponentialRetryAfter, retryOptions.MaxRetryDelay);
}
} // namespace

std::unique_ptr<Response> RetryPolicy::Send(
    Context& ctx,
    Request& request,
    NextHttpPolicy nextHttpPolicy) const
{
  for (decltype(m_retryOptions.MaxRetries) attempt = 1;; ++attempt)
  {
    request.StartRetry();

    auto response = nextHttpPolicy.Send(ctx, request);

    auto retryAfter = ShouldRetry(*response.get(), m_retryOptions, attempt);
    // If we are out of retry attempts, if a response is non-retriable (or simply 200 OK, i.e
    // doesn't need to be retried), then ShouldRetry returns some negative value.
    if (retryAfter.count() < 0)
    {
      return response;
    }

    response.reset();

    // Sleep(0) behavior is implementation-defined: it may yield, or may do nothing. Let's make sure
    // we proceed immediately if it is 0.
    if (retryAfter.count() > 0)
    {
      std::this_thread::sleep_for(retryAfter);
    }

    // TODO: check context for expiration; the function is not in master yet.
  }
}
