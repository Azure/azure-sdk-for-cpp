//  Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/test/interceptor_manager.hpp"
#include "azure/core/test/network_models.hpp"
#include "azure/core/test/record_network_call_policy.hpp"

#include <azure/core/internal/strings.hpp>

#include <string>
#include <vector>

using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http;
using namespace Azure::Core::Test;
using namespace Azure::Core::_internal;

// 2 Kb max
#define MAX_SUPPORTED_BODYSTREAM_SIZE 1024 * 2

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
  if (m_interceptorManager->GetTestMode() != TestMode::RECORD
      || m_interceptorManager->GetTestContext().LiveOnly)
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
      record.Response.emplace(header.first, headerValue);
    }
  }

  // BodyStreams are currently supported ony up to MAX_SUPPORTED_BODYSTREAM_SIZE
  // The content is downloaded to the response body and the returned body stream from playback will
  // stream from the memory buffer instead of the network.
  // When bigger than MAX_SUPPORTED_BODYSTREAM_SIZE, the recording will use the symbol captured from
  // the last request with a bodyStream as a sentinel to tell the playback transport adapter how to
  // generate a bodyStream
  auto bodyStream = response->ExtractBodyStream();
  if (bodyStream != nullptr)
  {
    auto const bodyStreamLen = bodyStream->Length();
    if (bodyStreamLen > MAX_SUPPORTED_BODYSTREAM_SIZE)
    {
      // Avoid recording a long stream response, instead, let's use the first byte from the payload
      // and record the size of the expected payload. This will work for Upload/Download big data
      // Write body for recording
      std::string bodyResponseStr(
          RECORDING_BODY_STREAM_SENTINEL + std::to_string(bodyStreamLen) + "_"
          + std::to_string(*m_symbol));
      std::vector<uint8_t> bodyResponseBytes(bodyResponseStr.begin(), bodyResponseStr.end());
      response->SetBody(bodyResponseBytes);

      // Let the response to own the body stream again
      response->SetBodyStream(std::move(bodyStream));
    }
    else
    {
      // SelfMemoryBodyStream would copy the response to memory
      response->SetBody(bodyStream->ReadToEnd());
      response->SetBodyStream(std::make_unique<WithMemoryBodyStream>(response->GetBody()));
    }
  }

  // Capture response
  auto const& body = response->GetBody();
  std::string bodystr(body.begin(), body.end());
  record.Response.emplace("BODY", bodystr);
  m_interceptorManager->GetRecordedData().NetworkCallRecords.push_back(record);

  return response;
}
