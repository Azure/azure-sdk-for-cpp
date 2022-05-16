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

// Dummy service tracing class.
class TestSpan final : public Azure::Core::Tracing::_internal::Span {
public:
  TestSpan() : Azure::Core::Tracing::_internal::Span() {}
  void Azure::Core::Tracing::_internal::Span::End(Azure::Nullable<Azure::DateTime>) override {}
  void Azure::Core::Tracing::_internal::Span::AddAttributes(
      const Azure::Core::Tracing::_internal::AttributeSet&) override
  {
  }
  void Azure::Core::Tracing::_internal::Span::AddEvent(const std::exception&) override {}
  void Azure::Core::Tracing::_internal::Span::AddEvent(const std::string&) override {}
  void Azure::Core::Tracing::_internal::Span::AddEvent(
      const std::string&,
      const Azure::Core::Tracing::_internal::AttributeSet&) override
  {
  }
  void Azure::Core::Tracing::_internal::Span::SetStatus(
      const Azure::Core::Tracing::_internal::SpanStatus&,
      const std::string&) override
  {
  }
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
  {
    Azure::Core::_internal::ClientOptions clientOptions;
    auto testTracer = std::make_shared<TestTracingProvider>();
    clientOptions.Telemetry.TracingProvider = testTracer;
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
