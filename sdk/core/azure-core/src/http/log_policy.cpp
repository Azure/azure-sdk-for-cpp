// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/diagnostics/log.hpp"

#include <algorithm>
#include <chrono>
#include <iterator>
#include <sstream>
#include <type_traits>

using Azure::Core::Context;
using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

namespace {
std::string RedactedPlaceholder = "REDACTED";

inline void AppendHeaders(
    std::ostringstream& log,
    Azure::Core::_internal::InputSanitizer const& inputSanitizer,
    Azure::Core::CaseInsensitiveMap const& headers)
{
  for (auto const& header : headers)
  {
    log << std::endl << header.first << " : ";

    if (!header.second.empty())
    {
      log << inputSanitizer.SanitizeHeader(header.first, header.second);
    }
  }
}

inline std::string GetRequestLogMessage(
    Azure::Core::_internal::InputSanitizer const& inputSanitizer,
    Request const& request)
{
  std::ostringstream log;
  log << "HTTP Request : " << request.GetMethod().ToString() << " ";

  Azure::Core::Url urlToLog(inputSanitizer.SanitizeUrl(request.GetUrl()));
  log << urlToLog.GetAbsoluteUrl();

  AppendHeaders(log, inputSanitizer, request.GetHeaders());
  return log.str();
}

inline std::string GetResponseLogMessage(
    Azure::Core::_internal::InputSanitizer const& inputSanitizer,
    RawResponse const& response,
    std::chrono::system_clock::duration const& duration)
{
  std::ostringstream log;

  log << "HTTP Response ("
      << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()
      << "ms) : " << static_cast<int>(response.GetStatusCode()) << " "
      << response.GetReasonPhrase();

  AppendHeaders(log, inputSanitizer, response.GetHeaders());
  return log.str();
}
} // namespace

Azure::Core::CaseInsensitiveSet const
    Azure::Core::Http::Policies::_detail::g_defaultAllowedHttpHeaders
    = {
        "Accept",
        "Cache-Control",
        "Connection",
        "Content-Length",
        "Content-Type",
        "Date",
        "ETag",
        "Expires",
        "If-Match",
        "If-Modified-Since",
        "If-None-Match",
        "If-Unmodified-Since",
        "Last-Modified",
        "Pragma",
        "Request-Id",
        "Retry-After",
        "Server",
        "traceparent",
        "tracestate",
        "Transfer-Encoding",
        "User-Agent"
        "x-ms-client-request-id",
        "x-ms-request-id",
        "x-ms-return-client-request-id",
};

std::unique_ptr<RawResponse> LogPolicy::Send(
    Request& request,
    NextHttpPolicy nextPolicy,
    Context const& context) const
{
  using Azure::Core::Diagnostics::Logger;
  using Azure::Core::Diagnostics::_internal::Log;

  if (Log::ShouldWrite(Logger::Level::Verbose))
  {
    Log::Write(Logger::Level::Informational, GetRequestLogMessage(m_inputSanitizer, request));
  }
  else
  {
    return nextPolicy.Send(request, context);
  }

  auto const start = std::chrono::system_clock::now();
  auto response = nextPolicy.Send(request, context);
  auto const end = std::chrono::system_clock::now();

  Log::Write(
      Logger::Level::Informational,
      GetResponseLogMessage(m_inputSanitizer, *response, end - start));

  return response;
}
