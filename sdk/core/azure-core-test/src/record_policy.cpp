// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/test/interceptor_manager.hpp"
#include "azure/core/test/network_models.hpp"
#include "azure/core/test/record_network_call_policy.hpp"
#include "private/environment.hpp"

#include <azure/core/internal/strings.hpp>

#include <regex>
#include <string>
#include <vector>

using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http;
using namespace Azure::Core::Test;
using namespace Azure::Core::_internal;

/**
 * @brief Records network request and response into RecordedData.
 *
 * @param ctx The context for canceling the request.
 * @param request The HTTP request that is sent.
 * @param nextHttpPolicy The next policy in the pipeline.
 * @return The HTTP raw response.
 */
std::unique_ptr<RawResponse> RecordNetworkCallPolicy::Send(
    Request& request,
    NextHttpPolicy nextHttpPolicy,
    Context const& ctx) const
{
  if (m_interceptorManager->GetTestMode() != TestMode::RECORD)
  {
    return nextHttpPolicy.Send(request, ctx);
  }

  // Init recordedRecord
  NetworkCallRecord record;

  record.Method = request.GetMethod().ToString();
  // Capture headers
  {
    std::vector<std::string> headersToBeCaptured
        = {"x-ms-client-request-id", "Content-Type", "x-ms-version", "User-Agent"};

    for (auto const& header : request.GetHeaders())
    {
      if (std::find_if(
              headersToBeCaptured.begin(),
              headersToBeCaptured.end(),
              [header](std::string const& compare) {
                return StringExtensions::LocaleInvariantCaseInsensitiveEqual(compare, header.first);
              })
          != headersToBeCaptured.end())
        record.Headers.emplace(header.first, header.second);
    }
  }

  // Remove sensitive information such as SAS token signatures from the recording.
  {
    Azure::Core::Url const redactedUrl = m_interceptorManager->RedactUrl(request.GetUrl());
    record.Url = redactedUrl.GetAbsoluteUrl();
  }

  // At this point, the request has been recorded. Send it to capture the response.
  auto response = nextHttpPolicy.Send(request, ctx);

  // BodyStreams are currently not supported
  if (response->ExtractBodyStream() != nullptr)
  {
    throw std::runtime_error("Record mode don't support recording a body stream");
  }

  record.Response.emplace(
      "STATUS_CODE",
      std::to_string(static_cast<typename std::underlying_type<Http::HttpStatusCode>::type>(
          response->GetStatusCode())));

  record.Response.emplace("REASON_PHRASE", response->GetReasonPhrase());

  for (auto const& header : response->GetHeaders())
  {
    if (header.first == "x-ms-encryption-key-sha256")
    {
      record.Response.emplace(header.first, "REDACTED");
    }
    else
    {
      auto headerValue = header.second;
      // Use hardcoded id
      headerValue = std::regex_replace(
          headerValue,
          std::regex("[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}"),
          "14a3ada2-0126-4805-4491-9693eacc9ab6");
      record.Response.emplace(header.first, headerValue);
    }
  }

  // Capture response
  auto const& body = response->GetBody();
  std::string bodystr(body.begin(), body.end());
  // Replace all id-like strings for a hardcoded ID
  bodystr = std::regex_replace(
      bodystr,
      std::regex("[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}"),
      "14a3ada2-0126-4805-4491-9693eacc9ab6");
  record.Response.emplace("BODY", bodystr);
  m_interceptorManager->GetRecordedData().NetworkCallRecords.push_back(record);

  return response;
}
