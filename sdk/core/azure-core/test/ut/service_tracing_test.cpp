// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/tracing/tracing.hpp"
#include <azure/core/internal/tracing/service_tracing.hpp>
#include <gtest/gtest.h>

using namespace Azure::Core;
using namespace Azure::Core::Tracing;
using namespace Azure::Core::Tracing::_internal;

TEST(ServiceTracing, ServiceTraceEnums)
{
  // Exercise the SpanKind and SpanStatus constructors from the distributed tracing header.
  {
    SpanKind spanKind = Azure::Core::Tracing::_internal::SpanKind::Client;
    spanKind = SpanKind::Consumer;
    spanKind = SpanKind::Internal;
    spanKind = SpanKind::Producer;
    spanKind = Azure::Core::Tracing::_internal::SpanKind::Server;
    std::string kindValue = spanKind.ToString();
  }
  {
    SpanStatus spanStatus = SpanStatus::Unset;
    spanStatus = SpanStatus::Error;
    spanStatus = SpanStatus::Ok;
    std::string statusValue = spanStatus.ToString();
  }
  Azure::Core::Tracing::_internal::CreateSpanOptions options;
  options.Kind = SpanKind::Internal;

  std::string tracingAttributeName = TracingAttributes::AzNamespace.ToString();
}

TEST(ServiceTracing, SimpleServiceSpanTests)
{
  {
    Azure::Core::Tracing::_internal::ServiceTracing serviceTrace;
  }
  {
    Azure::Core::_internal::ClientOptions clientOptions;
    Azure::Core::Tracing::_internal::ServiceTracing serviceTrace(
        clientOptions, "my-service-cpp", "1.0b2");
  }

  {
    Azure::Core::_internal::ClientOptions clientOptions;
    Azure::Core::Tracing::_internal::ServiceTracing serviceTrace(
        clientOptions, "my-service-cpp", "1.0b2");

    auto contextAndSpan = serviceTrace.CreateSpan("My API", {});
    EXPECT_FALSE(contextAndSpan.first.IsCancelled());
  }
}

TEST(ServiceTracing, BasicServiceSpanTests)
{
  {
    Azure::Core::_internal::ClientOptions clientOptions;
    Azure::Core::Tracing::_internal::ServiceTracing serviceTrace(
        clientOptions, "my-service-cpp", "1.0b2");

    auto contextAndSpan = serviceTrace.CreateSpan("My API", {});
    auto span = std::move(contextAndSpan.second);

    span.End();
    span.AddEvent("New Event");
    span.AddEvent(std::runtime_error("Exception"));
    span.SetStatus(SpanStatus::Error);
  }
}
