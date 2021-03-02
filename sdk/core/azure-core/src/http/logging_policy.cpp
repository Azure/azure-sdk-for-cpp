// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policy.hpp"
#include "azure/core/internal/log.hpp"

#include <chrono>
#include <sstream>

using Azure::Core::Context;
using namespace Azure::Core::Http;

namespace {
std::string TruncateIfLengthy(std::string const& s)
{
  static constexpr auto const MaxLength = 50;

  auto const length = s.length();
  if (length <= MaxLength)
  {
    return s;
  }

  static constexpr char const Ellipsis[] = " ... ";
  static constexpr auto const EllipsisLength = sizeof(Ellipsis) - 1;

  auto const BeginLength = (MaxLength / 2) - ((EllipsisLength / 2) + (EllipsisLength % 2));
  auto const EndLength = ((MaxLength / 2) + (MaxLength % 2)) - (EllipsisLength / 2);

  return s.substr(0, BeginLength) + Ellipsis + s.substr(length - EndLength, EndLength);
}

std::string GetRequestLogMessage(
    Azure::Core::Http::LoggingPolicyOptions const& options,
    Request const& request)
{
  std::ostringstream log;

  std::string url = request.GetUrl().GetAbsoluteUrl();
  if (!url.empty())
  {
    auto qm = url.find('?');
    if (qm != std::string::npos)
    {
      url = url.substr(0, qm);
    }
  }

  {
    auto separ = '?';
    for (auto qparam : request.GetUrl().GetQueryParameters())
    {
      url += separ + qparam.first + '=';

      if (options.AllowedHttpQueryParameters.find(qparam.first)
          != options.AllowedHttpQueryParameters.end())
      {
        url += TruncateIfLengthy(qparam.second);
      }
      else
      {
        url += "[hidden]";
      }

      separ = '&';
    }
  }

  log << "HTTP Request : " << HttpMethodToString(request.GetMethod()) << " " << url;

  for (auto header : request.GetHeaders())
  {
    log << "\n\t" << header.first;

    if (header.second.empty())
    {
      log << " [empty]";
    }
    else if (
        options.AllowedHttpRequestHeaders.find(header.first)
        != options.AllowedHttpRequestHeaders.end())
    {
      log << " : " << TruncateIfLengthy(header.second);
    }
    else
    {
      log << " [hidden]";
    }
  }

  return log.str();
}

std::string GetResponseLogMessage(
    Azure::Core::Http::LoggingPolicyOptions const& options,
    Request const& request,
    RawResponse const& response,
    std::chrono::system_clock::duration const& duration)
{
  std::ostringstream log;

  log << "HTTP Response ("
      << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()
      << "ms) : " << static_cast<int>(response.GetStatusCode()) << " "
      << response.GetReasonPhrase();

  for (auto header : response.GetHeaders())
  {
    log << "\n\t" << header.first;
    if (header.second.empty())
    {
      log << " [empty]";
    }
    else if (
        options.AllowedHttpResponseHeaders.find(header.first)
        != options.AllowedHttpResponseHeaders.end())
    {
      log << " : " << TruncateIfLengthy(header.second);
    }
    else
    {
      log << " [hidden]";
    }
  }

  log << "\n\n -> " << GetRequestLogMessage(options, request);

  return log.str();
}
} // namespace

std::unique_ptr<RawResponse> Azure::Core::Http::LoggingPolicy::Send(
    Context const& ctx,
    Request& request,
    NextHttpPolicy nextHttpPolicy) const
{
  using Azure::Core::Logger;
  using Azure::Core::Internal::Log;
  using Azure::Core::Internal::ShouldLog;

  if (ShouldLog(Logger::Level::Verbose))
  {
    Log(Logger::Level::Verbose, GetRequestLogMessage(m_options, request));
  }
  else
  {
    return nextHttpPolicy.Send(ctx, request);
  }

  auto const start = std::chrono::system_clock::now();
  auto response = nextHttpPolicy.Send(ctx, request);
  auto const end = std::chrono::system_clock::now();

  Log(Logger::Level::Verbose, GetResponseLogMessage(m_options, request, *response, end - start));

  return response;
}
