// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/diagnostics/log.hpp"
#include "azure/core/internal/input_sanitizer.hpp"
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
  // Find a tracing factory from our context. Note that the factory value is owned by the
  // context chain so we can manage a raw pointer to the factory.
  auto tracingFactory = DiagnosticTracingFactory::DiagnosticFactoryFromContext(context);
  if (tracingFactory)
  {
    // Create a tracing span over the HTTP request.
    std::stringstream ss;
    ss << "HTTP " << request.GetMethod().ToString();

    CreateSpanOptions createOptions;
    createOptions.Kind = SpanKind::Client;
    createOptions.Attributes = tracingFactory->CreateAttributeSet();
    // Note that the AttributeSet takes a *reference* to the values passed into the AttributeSet.
    // This means that all the values passed into the AttributeSet MUST be stabilized across the
    // lifetime of the AttributeSet.
    std::string httpMethod = request.GetMethod().ToString();
    createOptions.Attributes->AddAttribute(TracingAttributes::HttpMethod.ToString(), httpMethod);

    std::string sanitizedUrl = m_inputSanitizer.SanitizeUrl(request.GetUrl()).GetAbsoluteUrl();
    createOptions.Attributes->AddAttribute("http.url", sanitizedUrl);
    Azure::Nullable<std::string> requestId = request.GetHeader("x-ms-client-request-id");
    if (requestId.HasValue())
    {
      createOptions.Attributes->AddAttribute(
          TracingAttributes::RequestId.ToString(), requestId.Value());
    }

    auto userAgent = request.GetHeader("User-Agent");
    if (userAgent.HasValue())
    {
      createOptions.Attributes->AddAttribute(
          TracingAttributes::HttpUserAgent.ToString(), userAgent.Value());
    }

    auto contextAndSpan = tracingFactory->CreateSpan(ss.str(), createOptions, context);
    auto scope = std::move(contextAndSpan.second);

    // Propagate information from the scope to the HTTP headers.
    //
    // This will add the "traceparent" header and any other OpenTelemetry related headers.
    scope.PropagateToHttpHeaders(request);

    try
    {
      // Send the request on to the service.
      auto response = nextPolicy.Send(request, contextAndSpan.first);

      // And register the headers we received from the service.
      scope.AddAttribute(
          TracingAttributes::HttpStatusCode.ToString(),
          std::to_string(static_cast<int>(response->GetStatusCode())));
      auto const& responseHeaders = response->GetHeaders();
      auto serviceRequestId = responseHeaders.find("x-ms-request-id");
      if (serviceRequestId != responseHeaders.end())
      {
        scope.AddAttribute(
            TracingAttributes::ServiceRequestId.ToString(), serviceRequestId->second);
      }

      return response;
    }
    catch (const TransportException& e)
    {
      scope.AddEvent(e);
      scope.SetStatus(SpanStatus::Error);

      // Rethrow the exception.
      throw;
    }
  }
  else
  {
    return nextPolicy.Send(request, context);
  }
}
