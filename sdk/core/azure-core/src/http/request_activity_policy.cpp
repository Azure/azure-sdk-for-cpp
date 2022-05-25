// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/diagnostics/log.hpp"
#include "azure/core/internal/tracing/service_tracing.hpp"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <sstream>
#include <thread>

using Azure::Core::Context;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Core::Tracing::_internal;

std::unique_ptr<RawResponse> RequestActivityPolicy::Send(
    Request& request,
    NextHttpPolicy nextPolicy,
    Context const& context) const
{
  // Create a tracing span over the HTTP request.
  std::stringstream ss;
  ss << "HTTP " << request.GetMethod().ToString() << " #" << m_retryCount;
  auto contextAndSpan
      = Azure::Core::Tracing::_internal::DiagnosticTracingFactory::CreateSpanFromContext(
          ss.str(), SpanKind::Client, context);
  auto scope = std::move(contextAndSpan.second);

  scope.AddAttribute(TracingAttributes::HttpMethod.ToString(), request.GetMethod().ToString());
  scope.AddAttribute(
      "http.url",
      /* _sanitizer.SanitizeUrl(message.Request.Uri.ToString())*/
      request.GetUrl().GetAbsoluteUrl());
  {
    Azure::Nullable<std::string> requestId = request.GetHeader("x-ms-client-request-id");
    if (requestId.HasValue())
    {
      scope.AddAttribute(TracingAttributes::RequestId.ToString(), requestId.Value());
    }
  }

  {
    auto userAgent = request.GetHeader("User-Agent");
    if (userAgent.HasValue())
    {
      scope.AddAttribute(TracingAttributes::HttpUserAgent.ToString(), userAgent.Value());
    }
  }
  try
  {
    auto response = nextPolicy.Send(request, contextAndSpan.first);

    scope.AddAttribute(
        TracingAttributes::HttpStatusCode.ToString(),
        std::to_string(static_cast<int>(response->GetStatusCode())));
    auto& responseHeaders = response->GetHeaders();
    auto serviceRequestId = responseHeaders.find("serviceRequestId");
    if (serviceRequestId != responseHeaders.end())
    {
      scope.AddAttribute(TracingAttributes::ServiceRequestId.ToString(), serviceRequestId->second);
    }

    m_retryCount += 1;

    return response;
  }
  catch (const TransportException& e)
  {
    scope.AddEvent(e);

    // Rethrow the exception.
    throw;
  }
}