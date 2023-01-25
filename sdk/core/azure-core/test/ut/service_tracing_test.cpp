// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/internal/tracing/tracing_impl.hpp"
#include <azure/core/internal/tracing/service_tracing.hpp>
#include <gtest/gtest.h>

using namespace Azure::Core;
using namespace Azure::Core::Tracing;
using namespace Azure::Core::Tracing::_internal;

TEST(TracingContextFactory, ServiceTraceEnums)
{
  // Exercise the SpanKind and SpanStatus constructors from the distributed tracing header.
  {
    SpanKind spanKind = Azure::Core::Tracing::_internal::SpanKind::Client;
    spanKind = SpanKind::Consumer;
    spanKind = SpanKind::Internal;
    spanKind = SpanKind::Producer;
    spanKind = Azure::Core::Tracing::_internal::SpanKind::Server;
    int i = static_cast<int>(spanKind);
    i += 1;
  }
  {
    SpanStatus spanStatus = SpanStatus::Unset;
    spanStatus = SpanStatus::Error;
    spanStatus = SpanStatus::Ok;
    int i = static_cast<int>(spanStatus);
    i += 1;
  }
  Azure::Core::Tracing::_internal::CreateSpanOptions options;
  options.Kind = SpanKind::Internal;

  std::string tracingAttributeName = TracingAttributes::AzNamespace.ToString();
}

// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/http/policies/policy.hpp>
#include <azure/core/internal/http/pipeline.hpp>
#include <gtest/gtest.h>

using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::_internal;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;

namespace {

class NoOpPolicy final : public HttpPolicy {
private:
  std::unique_ptr<RawResponse> Send(
      Request& request,
      NextHttpPolicy nextPolicy,
      Context const& context) const override
  {
    (void)context;
    (void)request;
    (void)nextPolicy;

    return std::unique_ptr<RawResponse>();
  }

  std::unique_ptr<HttpPolicy> Clone() const override { return std::make_unique<NoOpPolicy>(*this); }
};

} // namespace

TEST(TracingContextFactory, SimpleServiceSpanTests)
{
  {
    Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace;
  }
  {
    Azure::Core::_internal::ClientOptions clientOptions;
    Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace(
        clientOptions, "my.service", "my-service-cpp", "1.0b2");
  }

  {
    Azure::Core::_internal::ClientOptions clientOptions;
    Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace(
        clientOptions, "my.service", "my-service-cpp", "1.0b2");

    auto contextAndSpan = serviceTrace.CreateTracingContext("My API", {});
    EXPECT_FALSE(contextAndSpan.Context.IsCancelled());
  }
}
namespace {
// Dummy service tracing class.
class TestSpan final : public Azure::Core::Tracing::_internal::Span {
public:
  TestSpan() : Azure::Core::Tracing::_internal::Span() {}

  // Inherited via Span
  virtual void AddAttributes(AttributeSet const&) override {}
  virtual void AddAttribute(std::string const&, std::string const&) override {}
  virtual void AddEvent(std::string const&, AttributeSet const&) override {}
  virtual void AddEvent(std::string const&) override {}
  virtual void AddEvent(std::exception const&) override {}
  virtual void SetStatus(SpanStatus const&, std::string const&) override {}

  // Inherited via Span
  virtual void End(Azure::Nullable<Azure::DateTime>) override {}

  // Inherited via Span
  virtual void PropagateToHttpHeaders(Azure::Core::Http::Request&) override {}
};

class TestAttributeSet : public Azure::Core::Tracing::_internal::AttributeSet {
public:
  TestAttributeSet() : Azure::Core::Tracing::_internal::AttributeSet() {}

  // Inherited via AttributeSet
  virtual void AddAttribute(std::string const&, bool) override {}
  virtual void AddAttribute(std::string const&, int32_t) override {}
  virtual void AddAttribute(std::string const&, int64_t) override {}
  virtual void AddAttribute(std::string const&, uint64_t) override {}
  virtual void AddAttribute(std::string const&, double) override {}
  virtual void AddAttribute(std::string const&, const char*) override {}
  virtual void AddAttribute(std::string const&, std::string const&) override {}
};
class TestTracer final : public Azure::Core::Tracing::_internal::Tracer {
public:
  TestTracer(std::string const&, std::string const&) : Azure::Core::Tracing::_internal::Tracer() {}
  std::shared_ptr<Span> CreateSpan(std::string const&, CreateSpanOptions const&) const override
  {
    return std::make_shared<TestSpan>();
  }

  std::unique_ptr<AttributeSet> CreateAttributeSet() const override
  {
    return std::make_unique<TestAttributeSet>();
  };
};

class TestTracingProvider final : public Azure::Core::Tracing::TracerProvider {
public:
  TestTracingProvider() : TracerProvider() {}
  ~TestTracingProvider() {}
  std::shared_ptr<Azure::Core::Tracing::_internal::Tracer> CreateTracer(
      std::string const& serviceName,
      std::string const& serviceVersion) const override
  {
    return std::make_shared<TestTracer>(serviceName, serviceVersion);
  };
};
} // namespace
TEST(TracingContextFactory, BasicServiceSpanTests)
{
  {
    Azure::Core::_internal::ClientOptions clientOptions;
    Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace(
        clientOptions, "My.Service", "my-service-cpp", "1.0b2");

    auto contextAndSpan = serviceTrace.CreateTracingContext("My API", {});
    ServiceSpan span = std::move(contextAndSpan.Span);

    span.End();
    span.AddEvent("New Event");
    span.AddEvent(std::runtime_error("Exception"));
    span.SetStatus(SpanStatus::Error);
  }

  {
    Azure::Core::_internal::ClientOptions clientOptions;
    auto testTracer = std::make_shared<TestTracingProvider>();
    clientOptions.Telemetry.TracingProvider = testTracer;
    Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace(
        clientOptions, "My.Service", "my-service-cpp", "1.0b2");

    auto contextAndSpan = serviceTrace.CreateTracingContext("My API", {});
    ServiceSpan span = std::move(contextAndSpan.Span);

    span.End();
    span.AddEvent("New Event");
    span.AddEvent(std::runtime_error("Exception"));
    std::unique_ptr<Azure::Core::Tracing::_internal::AttributeSet> attributeSet
        = serviceTrace.CreateAttributeSet();
    attributeSet->AddAttribute("Joe", "Joe'sValue");
    span.AddEvent("AttributeEvent", *attributeSet);
    span.AddAttributes(*attributeSet);
    span.SetStatus(SpanStatus::Error);
  }

  // Now run all the previous tests on a TracingContextFactory created *without* a tracing
  // provider.
  {
    Azure::Core::_internal::ClientOptions clientOptions;
    Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace(
        clientOptions, "My.Service", "my-service-cpp", "1.0b2");

    auto contextAndSpan = serviceTrace.CreateTracingContext("My API", {});
    ServiceSpan span = std::move(contextAndSpan.Span);

    span.End();
    span.AddEvent("New Event");
    span.AddEvent(std::runtime_error("Exception"));
    std::unique_ptr<Azure::Core::Tracing::_internal::AttributeSet> attributeSet
        = serviceTrace.CreateAttributeSet();
    if (attributeSet)
    {
      attributeSet->AddAttribute("Joe", "Joe'sValue");
      span.AddEvent("AttributeEvent", *attributeSet);
      span.AddAttributes(*attributeSet);
    }
    span.SetStatus(SpanStatus::Error);
  }
}
