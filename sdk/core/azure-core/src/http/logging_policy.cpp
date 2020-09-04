// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/http/policy.hpp>
#include <azure/core/internal/log.hpp>

#include <chrono>
#include <sstream>

using namespace Azure::Core;
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

std::string GetRequestLogMessage(Request const& request)
{
  std::ostringstream log;

  log << "HTTP Request : " << HttpMethodToString(request.GetMethod()) << " "
      << request.GetUrl().GetAbsoluteUrl();

  for (auto header : request.GetHeaders())
  {
    log << "\n\t" << header.first << " : " << TruncateIfLengthy(header.second);
  }

  return log.str();
}

std::string GetResponseLogMessage(
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
    if (!header.second.empty() && header.first != "authorization")
    {
      log << " : " << TruncateIfLengthy(header.second);
    }
  }

  log << "\n\n -> " << GetRequestLogMessage(request);

  return log.str();
}
} // namespace

std::unique_ptr<RawResponse> Azure::Core::Http::LoggingPolicy::Send(
    Context const& ctx,
    Request& request,
    NextHttpPolicy nextHttpPolicy) const
{
  if (Logging::Details::ShouldWrite(LogClassification::Request))
  {
    Logging::Details::Write(LogClassification::Request, GetRequestLogMessage(request));
  }

  if (!Logging::Details::ShouldWrite(LogClassification::Response))
  {
    return nextHttpPolicy.Send(ctx, request);
  }

  auto const start = std::chrono::system_clock::now();
  auto response = nextHttpPolicy.Send(ctx, request);
  auto const end = std::chrono::system_clock::now();

  Logging::Details::Write(
      LogClassification::Response, GetResponseLogMessage(request, *response, end - start));

  return response;
}
