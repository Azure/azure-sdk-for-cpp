// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policy.hpp"
#include "azure/core/internal/log.hpp"

#include <algorithm>
#include <chrono>
#include <iterator>
#include <sstream>
#include <type_traits>

using Azure::Core::Context;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;

namespace {
std::string RedactedPlaceholder = "REDACTED";

inline void AppendHeaders(
    std::ostringstream& log,
    Azure::Core::CaseInsensitiveMap const& headers,
    Azure::Core::CaseInsensitiveSet const& allowedHaders)
{
  for (auto const& header : headers)
  {
    log << std::endl << header.first << " : ";

    if (!header.second.empty())
    {
      log
          << ((allowedHaders.find(header.first) != allowedHaders.end()) ? header.second
                                                                        : RedactedPlaceholder);
    }
  }
}

inline std::string GetRequestLogMessage(LogOptions const& options, Request const& request)
{
  auto const& requestUrl = request.GetUrl();

  std::ostringstream log;
  log << "HTTP Request : " << HttpMethodToString(request.GetMethod()) << " "
      << requestUrl.GetUrlWithoutQuery();
  {
    auto encodedRequestQueryParams = requestUrl.GetQueryParameters();
    auto const& unencodedAllowedQueryParams = options.AllowedHttpQueryParameters;
    if (!encodedRequestQueryParams.empty() && !unencodedAllowedQueryParams.empty())
    {
      std::remove_const<std::remove_reference<decltype(unencodedAllowedQueryParams)>::type>::type
          encodedAllowedQueryParams;
      std::transform(
          unencodedAllowedQueryParams.begin(),
          unencodedAllowedQueryParams.end(),
          std::inserter(encodedAllowedQueryParams, encodedAllowedQueryParams.begin()),
          [](std::string const& s) { return Url::Encode(s); });

      std::remove_const<std::remove_reference<decltype(encodedRequestQueryParams)>::type>::type
          encodedAllowedRequestQueryParams;
      for (auto const& encodedRequestQueryParam : encodedRequestQueryParams)
      {
        if (encodedRequestQueryParam.second.empty()
            || (encodedAllowedQueryParams.find(encodedRequestQueryParam.first)
                != encodedAllowedQueryParams.end()))
        {
          encodedAllowedRequestQueryParams.insert(encodedRequestQueryParam);
        }
        else
        {
          encodedAllowedRequestQueryParams.insert(
              std::make_pair(encodedRequestQueryParam.first, RedactedPlaceholder));
        }
      }

      log << Azure::Core::Http::_detail::FormatEncodedUrlQueryParameters(
          encodedAllowedRequestQueryParams);
    }
  }
  AppendHeaders(log, request.GetHeaders(), options.AllowedHttpHeaders);
  return log.str();
}

inline std::string GetResponseLogMessage(
    LogOptions const& options,
    RawResponse const& response,
    std::chrono::system_clock::duration const& duration)
{
  std::ostringstream log;

  log << "HTTP Response ("
      << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()
      << "ms) : " << static_cast<int>(response.GetStatusCode()) << " "
      << response.GetReasonPhrase();

  AppendHeaders(log, response.GetHeaders(), options.AllowedHttpHeaders);
  return log.str();
}
} // namespace

Azure::Core::CaseInsensitiveSet const
    Azure::Core::Http::Policies::_detail::g_defaultAllowedHttpHeaders
    = {"x-ms-client-request-id",
       "x-ms-return-client-request-id",
       "traceparent",
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
       "Transfer-Encoding",
       "User-Agent"};

std::unique_ptr<RawResponse> LogPolicy::Send(
    Request& request,
    NextHttpPolicy nextHttpPolicy,
    Context const& ctx) const
{
  using Azure::Core::Diagnostics::Logger;
  using Azure::Core::Diagnostics::_internal::Log;

  if (Log::ShouldWrite(Logger::Level::Verbose))
  {
    Log::Write(Logger::Level::Informational, GetRequestLogMessage(m_options, request));
  }
  else
  {
    return nextHttpPolicy.Send(request, ctx);
  }

  auto const start = std::chrono::system_clock::now();
  auto response = nextHttpPolicy.Send(request, ctx);
  auto const end = std::chrono::system_clock::now();

  Log::Write(
      Logger::Level::Informational, GetResponseLogMessage(m_options, *response, end - start));

  return response;
}
