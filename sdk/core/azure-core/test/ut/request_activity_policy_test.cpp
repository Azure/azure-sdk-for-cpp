// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/http/policies/policy.hpp"
#include "azure/core/internal/http/pipeline.hpp"
#include "azure/core/internal/tracing/service_tracing.hpp"
#include "azure/core/tracing/tracing.hpp"
#include <gtest/gtest.h>
#include <list>

using namespace Azure::Core;
using namespace Azure::Core::Http;
using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Core::Tracing::_internal;
using namespace Azure::Core::Tracing;

namespace {
class NoOpPolicy final : public HttpPolicy {
  std::function<std::unique_ptr<RawResponse>(Request&)> m_createResponse{};

public:
  std::unique_ptr<HttpPolicy> Clone() const override { return std::make_unique<NoOpPolicy>(*this); }

  std::unique_ptr<RawResponse> Send(Request& request, NextHttpPolicy, Azure::Core::Context const&)
      const override
  {
    if (m_createResponse)
    {
      return m_createResponse(request);
    }
    else
    {
      return std::make_unique<RawResponse>(1, 1, HttpStatusCode::Ok, "Something");
    }
  }

  NoOpPolicy() = default;
  NoOpPolicy(std::function<std::unique_ptr<RawResponse>(Request&)> createResponse)
      : HttpPolicy(), m_createResponse(createResponse){};
};

class TestAttributeSet : public Azure::Core::Tracing::_internal::AttributeSet {
  std::map<std::string, std::string> m_attributes;

public:
  TestAttributeSet() : Azure::Core::Tracing::_internal::AttributeSet() {}

  // Inherited via AttributeSet
  virtual void AddAttribute(std::string const&, bool) override {}
  virtual void AddAttribute(std::string const&, int32_t) override {}
  virtual void AddAttribute(std::string const&, int64_t) override {}
  virtual void AddAttribute(std::string const&, uint64_t) override {}
  virtual void AddAttribute(std::string const&, double) override {}
  virtual void AddAttribute(std::string const& key, const char* val) override
  {
    m_attributes.emplace(std::make_pair(key, std::string(val)));
  }

  virtual void AddAttribute(std::string const& key, std::string const& val) override
  {
    m_attributes.emplace(std::make_pair(key, val));
  }

  std::map<std::string, std::string> const& GetAttributes() { return m_attributes; }
};

// Dummy service tracing class.
class TestSpan final : public Azure::Core::Tracing::_internal::Span {
  std::vector<std::string> m_events;
  std::map<std::string, std::string> m_stringAttributes;
  std::string m_spanName;

public:
  TestSpan(std::string const& spanName, CreateSpanOptions const& options)
      : Azure::Core::Tracing::_internal::Span(), m_spanName(spanName)
  {
    if (options.Attributes)
    {
      TestAttributeSet* testAttributes = static_cast<TestAttributeSet*>(options.Attributes.get());

      for (auto const& attribute : testAttributes->GetAttributes())
      {
        m_stringAttributes.emplace(attribute);
      }
    }
  }

  // Inherited via Span
  virtual void AddAttributes(AttributeSet const&) override {}
  virtual void AddAttribute(std::string const& attributeName, std::string const& attributeValue)
      override
  {
    m_stringAttributes.emplace(std::make_pair(attributeName, attributeValue));
  }
  virtual void AddEvent(std::string const& eventName, AttributeSet const&) override
  {
    m_events.push_back(eventName);
  }
  virtual void AddEvent(std::string const& eventName) override { m_events.push_back(eventName); }
  virtual void AddEvent(std::exception const& ex) override { m_events.push_back(ex.what()); }
  virtual void SetStatus(SpanStatus const&, std::string const&) override {}

  // Inherited via Span
  virtual void End(Azure::Nullable<Azure::DateTime>) override {}

  // Inherited via Span
  virtual void PropagateToHttpHeaders(Azure::Core::Http::Request&) override {}

  std::string const& GetName() { return m_spanName; }
  std::vector<std::string> const& GetEvents() { return m_events; }
  std::map<std::string, std::string> const& GetAttributes() { return m_stringAttributes; }
};

class TestTracer final : public Azure::Core::Tracing::_internal::Tracer {
  mutable std::vector<std::shared_ptr<TestSpan>> m_spans;

public:
  TestTracer(std::string const&, std::string const&) : Azure::Core::Tracing::_internal::Tracer() {}
  std::shared_ptr<Span> CreateSpan(std::string const& spanName, CreateSpanOptions const& options)
      const override
  {
    auto returnSpan(std::make_shared<TestSpan>(spanName, options));
    m_spans.push_back(returnSpan);
    return returnSpan;
  }

  std::unique_ptr<AttributeSet> CreateAttributeSet() const override
  {
    return std::make_unique<TestAttributeSet>();
  };

  std::vector<std::shared_ptr<TestSpan>> const& GetSpans() { return m_spans; }
};

class TestTracingProvider final : public Azure::Core::Tracing::TracerProvider {
  mutable std::list<std::shared_ptr<TestTracer>> m_tracers;

public:
  TestTracingProvider() : TracerProvider() {}
  ~TestTracingProvider() {}
  std::shared_ptr<Azure::Core::Tracing::_internal::Tracer> CreateTracer(
      std::string const& serviceName,
      std::string const& serviceVersion) const override
  {
    auto returnTracer = std::make_shared<TestTracer>(serviceName, serviceVersion);
    m_tracers.push_back(returnTracer);
    return returnTracer;
  };

  std::list<std::shared_ptr<TestTracer>> const& GetTracers() { return m_tracers; }
};
} // namespace

TEST(RequestActivityPolicy, Basic)
{
  {
    auto testTracer = std::make_shared<TestTracingProvider>();

    Azure::Core::_internal::ClientOptions clientOptions;
    clientOptions.Telemetry.TracingProvider = testTracer;
    Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace(
        clientOptions, "my-service-cpp", "1.0b2");

    auto contextAndSpan = serviceTrace.CreateTracingContext("My API", {});
    Azure::Core::Context callContext = std::move(contextAndSpan.Context);
    Request request(HttpMethod::Get, Url("https://www.microsoft.com"));

    {
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
      // Add the request ID policy - this adds the x-ms-request-id attribute to the pipeline.
      policies.emplace_back(
          std::make_unique<RequestActivityPolicy>(Azure::Core::_internal::InputSanitizer{}));
      // Final policy - equivalent to HTTP policy.
      policies.emplace_back(std::make_unique<NoOpPolicy>());

      Azure::Core::Http::_internal::HttpPipeline(policies).Send(request, callContext);
    }

    EXPECT_EQ(1ul, testTracer->GetTracers().size());
    auto& tracer = testTracer->GetTracers().front();
    EXPECT_EQ(2ul, tracer->GetSpans().size());
    EXPECT_EQ("My API", tracer->GetSpans()[0]->GetName());
    EXPECT_EQ("HTTP GET", tracer->GetSpans()[1]->GetName());
    EXPECT_EQ("GET", tracer->GetSpans()[1]->GetAttributes().at("http.method"));
  }

  // Now try with the request ID and telemetry policies (simulating a more complete pipeline).
  {
    auto testTracer = std::make_shared<TestTracingProvider>();

    Azure::Core::_internal::ClientOptions clientOptions;
    clientOptions.Telemetry.TracingProvider = testTracer;
    Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace(
        clientOptions, "my-service-cpp", "1.0b2");
    auto contextAndSpan = serviceTrace.CreateTracingContext("My API", {});
    Azure::Core::Context callContext = std::move(contextAndSpan.Context);
    Request request(HttpMethod::Get, Url("https://www.microsoft.com"));

    {
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;
      // Add the request ID policy - this adds the x-ms-request-id attribute to the pipeline.
      policies.emplace_back(std::make_unique<RequestIdPolicy>());
      policies.emplace_back(
          std::make_unique<TelemetryPolicy>("my-service-cpp", "1.0b2", clientOptions.Telemetry));
      policies.emplace_back(std::make_unique<RetryPolicy>(RetryOptions{}));
      policies.emplace_back(
          std::make_unique<RequestActivityPolicy>(Azure::Core::_internal::InputSanitizer{}));
      // Final policy - equivalent to HTTP policy.
      policies.emplace_back(std::make_unique<NoOpPolicy>());

      Azure::Core::Http::_internal::HttpPipeline(policies).Send(request, callContext);
    }

    EXPECT_EQ(1ul, testTracer->GetTracers().size());
    auto& tracer = testTracer->GetTracers().front();
    EXPECT_EQ(2ul, tracer->GetSpans().size());
    EXPECT_EQ("My API", tracer->GetSpans()[0]->GetName());
    EXPECT_EQ("HTTP GET", tracer->GetSpans()[1]->GetName());
    EXPECT_EQ("GET", tracer->GetSpans()[1]->GetAttributes().at("http.method"));
  }
}

TEST(RequestActivityPolicy, TryRetries)
{
  {
    auto testTracer = std::make_shared<TestTracingProvider>();

    Azure::Core::_internal::ClientOptions clientOptions;
    clientOptions.Telemetry.TracingProvider = testTracer;
    Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace(
        clientOptions, "my-service-cpp", "1.0b2");

    auto contextAndSpan = serviceTrace.CreateTracingContext("My API", {});
    Azure::Core::Context callContext = std::move(contextAndSpan.Context);
    Request request(HttpMethod::Get, Url("https://www.microsoft.com"));

    {
      std::vector<std::unique_ptr<Azure::Core::Http::Policies::HttpPolicy>> policies;

      policies.emplace_back(std::make_unique<RequestIdPolicy>());
      policies.emplace_back(std::make_unique<RetryPolicy>(RetryOptions{}));

      // Add the request ID policy - this adds the x-ms-request-id attribute to the pipeline.
      policies.emplace_back(
          std::make_unique<RequestActivityPolicy>(Azure::Core::_internal::InputSanitizer{}));
      // Final policy - equivalent to HTTP policy.
      int retryCount = 0;
      policies.emplace_back(std::make_unique<NoOpPolicy>([&](Request&) {
        retryCount += 1;
        if (retryCount < 3)
        {
          // Return a response which should trigger a response.
          return std::make_unique<RawResponse>(
              1, 1, *RetryOptions().StatusCodes.begin(), "Something");
        }
        else
        {
          // Return success.
          return std::make_unique<RawResponse>(1, 1, HttpStatusCode::Ok, "Something");
        }
      }));

      Azure::Core::Http::_internal::HttpPipeline pipeline(policies);
      // Simulate retrying an HTTP operation 3 times on the pipeline:
      pipeline.Send(request, callContext);
    }

    EXPECT_EQ(1ul, testTracer->GetTracers().size());
    auto& tracer = testTracer->GetTracers().front();
    EXPECT_EQ(4ul, tracer->GetSpans().size());
    EXPECT_EQ("My API", tracer->GetSpans()[0]->GetName());
    EXPECT_EQ("HTTP GET", tracer->GetSpans()[1]->GetName());
    EXPECT_EQ("HTTP GET", tracer->GetSpans()[2]->GetName());
    EXPECT_EQ("HTTP GET", tracer->GetSpans()[3]->GetName());
    EXPECT_EQ("GET", tracer->GetSpans()[1]->GetAttributes().at("http.method"));
    EXPECT_EQ("408", tracer->GetSpans()[1]->GetAttributes().at("http.status_code"));
    EXPECT_EQ("408", tracer->GetSpans()[2]->GetAttributes().at("http.status_code"));
    EXPECT_EQ("200", tracer->GetSpans()[3]->GetAttributes().at("http.status_code"));
  }
}
