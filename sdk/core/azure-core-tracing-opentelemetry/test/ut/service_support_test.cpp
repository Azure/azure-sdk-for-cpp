// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#define USE_MEMORY_EXPORTER 1
#include "azure/core/internal/tracing/service_tracing.hpp"
#include "azure/core/tracing/opentelemetry/opentelemetry.hpp"
#include <azure/core/internal/http/pipeline.hpp>
#include <azure/core/internal/json/json.hpp>
#include <azure/core/test/test_base.hpp>

#if defined(_MSC_VER)
// The OpenTelemetry headers generate a couple of warnings on MSVC in the OTel 1.2 package, suppress
// the warnings across the includes.
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4244)
#pragma warning(disable : 6323) // Disable "Use of arithmetic operator on Boolean type" warning.
#endif
#include <opentelemetry/exporters/memory/in_memory_span_data.h>
#include <opentelemetry/exporters/memory/in_memory_span_exporter.h>
#include <opentelemetry/exporters/ostream/span_exporter.h>
#include <opentelemetry/sdk/common/global_log_handler.h>
#include <opentelemetry/sdk/trace/exporter.h>
#include <opentelemetry/sdk/trace/processor.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/trace/propagation/http_trace_context.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#include <chrono>
#include <gtest/gtest.h>
#include <regex>

using namespace Azure::Core::Http::Policies;
using namespace Azure::Core::Http::Policies::_internal;
using namespace Azure::Core::Http;

class CustomLogHandler : public opentelemetry::sdk::common::internal_log::LogHandler {
  void Handle(
      opentelemetry::sdk::common::internal_log::LogLevel,
      const char* file,
      int line,
      const char* msg,
      const opentelemetry::sdk::common::AttributeMap& attributes) noexcept override
  {
    GTEST_LOG_(INFO) << "File: " << std::string(file) << " (" << line << "): " << std::string(msg)
                     << std::endl;
    if (!attributes.empty())
    {
      for (auto& attribute : attributes)
      {
        GTEST_LOG_(INFO) << "Attribute " << attribute.first << ": ";
        switch (attribute.second.index())
        {
          case opentelemetry::sdk::common::kTypeBool:
            GTEST_LOG_(INFO) << opentelemetry::nostd::get<bool>(attribute.second);
            break;
          case opentelemetry::sdk::common::kTypeInt:
            GTEST_LOG_(INFO) << opentelemetry::nostd::get<int>(attribute.second);
            break;
          case opentelemetry::sdk::common::kTypeUInt:
            GTEST_LOG_(INFO) << opentelemetry::nostd::get<uint32_t>(attribute.second);
            break;
          case opentelemetry::sdk::common::kTypeInt64:
            GTEST_LOG_(INFO) << opentelemetry::nostd::get<int64_t>(attribute.second);
            break;
          case opentelemetry::sdk::common::kTypeDouble:
            GTEST_LOG_(INFO) << opentelemetry::nostd::get<double>(attribute.second);
            break;

          case opentelemetry::sdk::common::kTypeString:
            GTEST_LOG_(INFO) << opentelemetry::nostd::get<std::string>(attribute.second);
            break;

          case opentelemetry::sdk::common::kTypeSpanBool:
          case opentelemetry::sdk::common::kTypeSpanInt:
          case opentelemetry::sdk::common::kTypeSpanUInt:
          case opentelemetry::sdk::common::kTypeSpanInt64:
          case opentelemetry::sdk::common::kTypeSpanDouble:
          case opentelemetry::sdk::common::kTypeSpanString:
          case opentelemetry::sdk::common::kTypeUInt64:
          case opentelemetry::sdk::common::kTypeSpanUInt64:
          case opentelemetry::sdk::common::kTypeSpanByte:
            GTEST_LOG_(INFO) << opentelemetry::nostd::get<bool>(attribute.second);
            GTEST_LOG_(INFO) << "SPAN";
            break;
        }
        GTEST_LOG_(INFO) << std::endl;
      }
    }
  }
};

class OpenTelemetryServiceTests : public Azure::Core::Test::TestBase {
private:
protected:
  std::shared_ptr<opentelemetry::exporter::memory::InMemorySpanData> m_spanData;

  opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider>
  CreateOpenTelemetryProvider()
  {
#if USE_MEMORY_EXPORTER
    auto exporter = std::make_unique<opentelemetry::exporter::memory::InMemorySpanExporter>();
    m_spanData = exporter->GetData();
#else
    // logging exporter
    auto exporter = std::make_unique<opentelemetry::exporter::trace::OStreamSpanExporter>();
#endif

    // simple processor
    auto simple_processor = std::unique_ptr<opentelemetry::sdk::trace::SpanProcessor>(
        new opentelemetry::sdk::trace::SimpleSpanProcessor(std::move(exporter)));

    auto always_on_sampler = std::unique_ptr<opentelemetry::sdk::trace::AlwaysOnSampler>(
        new opentelemetry::sdk::trace::AlwaysOnSampler);

    auto resource_attributes = opentelemetry::sdk::resource::ResourceAttributes{
        {"service.name", "telemetryTest"}, {"service.instance.id", "instance-1"}};
    auto resource = opentelemetry::sdk::resource::Resource::Create(resource_attributes);
    // Create using SDK configurations as parameter
    return opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider>(
        new opentelemetry::sdk::trace::TracerProvider(
            std::move(simple_processor), resource, std::move(always_on_sampler)));
  }

  // Create
  virtual void SetUp() override
  {
    Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);

    opentelemetry::sdk::common::internal_log::GlobalLogHandler::SetLogHandler(
        opentelemetry::nostd::shared_ptr<opentelemetry::sdk::common::internal_log::LogHandler>(
            new CustomLogHandler()));
    opentelemetry::sdk::common::internal_log::GlobalLogHandler::SetLogLevel(
        opentelemetry::sdk::common::internal_log::LogLevel::Debug);
  }

  virtual void TearDown() override
  {
    // Make sure you call the base classes TearDown method to ensure recordings are made.
    TestBase::TearDown();
  }

  bool VerifySpan(
      std::unique_ptr<opentelemetry::sdk::trace::SpanData> const& span,
      std::string const& expectedSpanContentsJson)
  {
    Azure::Core::Json::_internal::json expectedSpanContents(
        Azure::Core::Json::_internal::json::parse(expectedSpanContentsJson));
    EXPECT_EQ(expectedSpanContents["name"].get<std::string>(), span->GetName());
    if (expectedSpanContents.contains("statusCode"))
    {
      std::string expectedStatus = expectedSpanContents["statusCode"].get<std::string>();
      switch (span->GetStatus())
      {
        case opentelemetry::trace::StatusCode::kOk:
          EXPECT_EQ(expectedStatus, "ok");
          break;
        case opentelemetry::trace::StatusCode::kError:
          EXPECT_EQ(expectedStatus, "error");
          break;
        case opentelemetry::trace::StatusCode::kUnset:
          EXPECT_EQ(expectedStatus, "unset");
          break;
        default:
          throw std::runtime_error("Unknown span status");
      }
    }
    if (expectedSpanContents.contains("kind"))
    {
      std::string expectedKind = expectedSpanContents["kind"].get<std::string>();
      switch (span->GetSpanKind())
      {
        case opentelemetry::trace::SpanKind::kClient:
          EXPECT_EQ(expectedKind, "client");
          break;
        case opentelemetry::trace::SpanKind::kConsumer:
          EXPECT_EQ(expectedKind, "consumer");
          break;
        case opentelemetry::trace::SpanKind::kInternal:
          EXPECT_EQ(expectedKind, "internal");
          break;
        case opentelemetry::trace::SpanKind::kProducer:
          EXPECT_EQ(expectedKind, "producer");
          break;
        case opentelemetry::trace::SpanKind::kServer:
          EXPECT_EQ(expectedKind, "server");
          break;
        default:
          throw std::runtime_error("Unknown span kind");
      }
    }
    if (expectedSpanContents.contains("attributes"))
    {
      auto& expectedAttributes = expectedSpanContents["attributes"];
      EXPECT_TRUE(expectedAttributes.is_object());
      auto attributes(span->GetAttributes());

      EXPECT_EQ(expectedAttributes.size(), attributes.size());

      for (const auto& foundAttribute : attributes)
      {
        EXPECT_TRUE(expectedAttributes.contains(foundAttribute.first));
        switch (foundAttribute.second.index())
        {
          case opentelemetry::common::kTypeBool: {

            EXPECT_TRUE(expectedAttributes[foundAttribute.first].is_boolean());
            auto actualVal = opentelemetry::nostd::get<bool>(foundAttribute.second);
            EXPECT_EQ(expectedAttributes[foundAttribute.first].get<bool>(), actualVal);
            break;
          }
          case opentelemetry::common::kTypeCString:
          case opentelemetry::common::kTypeString: {
            EXPECT_TRUE(expectedAttributes[foundAttribute.first].is_string());
            const auto& actualVal = opentelemetry::nostd::get<std::string>(foundAttribute.second);
            std::string expectedVal(expectedAttributes[foundAttribute.first].get<std::string>());
            std::regex expectedRegex(expectedVal);
            GTEST_LOG_(INFO) << "expected Regex: " << expectedVal << std::endl;
            GTEST_LOG_(INFO) << "actual val: " << actualVal << std::endl;
            EXPECT_TRUE(std::regex_match(actualVal, expectedRegex));
            break;
          }
          case opentelemetry::common::kTypeDouble: {

            EXPECT_TRUE(expectedAttributes[foundAttribute.first].is_number());
            auto actualVal = opentelemetry::nostd::get<double>(foundAttribute.second);
            EXPECT_EQ(expectedAttributes[foundAttribute.first].get<double>(), actualVal);
            break;
          }

          case opentelemetry::common::kTypeInt:
          case opentelemetry::common::kTypeInt64:
            EXPECT_TRUE(expectedAttributes[foundAttribute.first].is_number_integer());
            break;
          case opentelemetry::common::kTypeSpanBool:
          case opentelemetry::common::kTypeSpanByte:
          case opentelemetry::common::kTypeSpanDouble:
          case opentelemetry::common::kTypeSpanInt:
          case opentelemetry::common::kTypeSpanInt64:
          case opentelemetry::common::kTypeSpanString:
          case opentelemetry::common::kTypeSpanUInt:
          case opentelemetry::common::kTypeSpanUInt64:
            EXPECT_TRUE(expectedAttributes[foundAttribute.first].is_array());
            throw std::runtime_error("Unsupported attribute kind");
            break;

          case opentelemetry::common::kTypeUInt:
          case opentelemetry::common::kTypeUInt64:
            EXPECT_TRUE(expectedAttributes[foundAttribute.first].is_number_unsigned());
            break;
          default:
            throw std::runtime_error("Unknown attribute kind");
            break;
        }
      }

      //      const auto& namespaceVal = opentelemetry::nostd::get<std::string>(azNamespace);
    }
    if (expectedSpanContents.contains("library"))
    {
      EXPECT_EQ(
          expectedSpanContents["library"]["name"].get<std::string>(),
          span->GetInstrumentationLibrary().GetName());
      EXPECT_EQ(
          expectedSpanContents["library"]["version"].get<std::string>(),
          span->GetInstrumentationLibrary().GetVersion());
    }
    return true;
  }
};

TEST_F(OpenTelemetryServiceTests, SimplestTest)
{
  {
    Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace;
  }
  {
    Azure::Core::_internal::ClientOptions clientOptions;
    Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace(
        clientOptions, "my-service-cpp", "1.0b2");
  }

  {
    Azure::Core::_internal::ClientOptions clientOptions;
    Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace(
        clientOptions, "my-service-cpp", "1.0b2");

    auto contextAndSpan = serviceTrace.CreateTracingContext("My API", {});
    EXPECT_FALSE(contextAndSpan.Context.IsCancelled());
  }
}

TEST_F(OpenTelemetryServiceTests, CreateWithExplicitProvider)
{
  // Create a serviceTrace, set it and retrieve it via a Context object. This verifies that we can
  // round-trip telemetry providers through a Context (which allows us to hook this up to the
  // ApplicationContext later).
  //
  {
    auto tracerProvider(CreateOpenTelemetryProvider());
    auto provider(
        Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(tracerProvider));

    Azure::Core::Context rootContext;
    rootContext.SetTracerProvider(
        std::static_pointer_cast<Azure::Core::Tracing::TracerProvider>(provider));
    EXPECT_EQ(provider, rootContext.GetTracerProvider());
  }

  {
    auto tracerProvider(CreateOpenTelemetryProvider());
    auto provider(
        Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(tracerProvider));

    // Create a serviceTrace and span using a provider specified in the ClientOptions.
    {
      Azure::Core::_internal::ClientOptions clientOptions;
      clientOptions.Telemetry.TracingProvider = provider;
      clientOptions.Telemetry.ApplicationId = "MyApplication";

      Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace(
          clientOptions, "my-service", "1.0beta-2");

      Azure::Core::Context clientContext;
      auto contextAndSpan = serviceTrace.CreateTracingContext("My API", clientContext);
      EXPECT_FALSE(contextAndSpan.Context.IsCancelled());
    }
    // Now let's verify what was logged via OpenTelemetry.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());

    VerifySpan(spans[0], R"(
{
  "name": "My API",
  "kind": "internal",
  "attributes": {
     "az.namespace": "my-service"
  },
  "library": {
    "name": "my-service",
    "version": "1.0beta-2"
  }
})");
  }
}

TEST_F(OpenTelemetryServiceTests, CreateWithImplicitProvider)
{
  {
    auto tracerProvider(CreateOpenTelemetryProvider());
    auto provider(
        Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(tracerProvider));

    Azure::Core::Context::ApplicationContext.SetTracerProvider(
        std::static_pointer_cast<Azure::Core::Tracing::TracerProvider>(provider));

    {
      Azure::Core::_internal::ClientOptions clientOptions;
      clientOptions.Telemetry.ApplicationId = "MyApplication";

      Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace(
          clientOptions, "my-service", "1.0beta-2");

      Azure::Core::Context clientContext;
      auto contextAndSpan = serviceTrace.CreateTracingContext("My API", clientContext);
      EXPECT_FALSE(contextAndSpan.Context.IsCancelled());
    }

    // Now let's verify what was logged via OpenTelemetry.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());

    VerifySpan(spans[0], R"(
{
  "name": "My API",
  "kind": "internal",
  "attributes": {
     "az.namespace": "my-service"
  },
  "library": {
    "name": "my-service",
    "version": "1.0beta-2"
  }
})");
  }

  // Clear the global tracer provider set earlier in the test.
  Azure::Core::Context::ApplicationContext.SetTracerProvider(nullptr);
}

TEST_F(OpenTelemetryServiceTests, CreateSpanWithOptions)
{
  {
    auto tracerProvider(CreateOpenTelemetryProvider());
    auto provider(
        Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(tracerProvider));

    Azure::Core::Context::ApplicationContext.SetTracerProvider(provider);

    {
      Azure::Core::_internal::ClientOptions clientOptions;
      clientOptions.Telemetry.ApplicationId = "MyApplication";

      Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace(
          clientOptions, "my-service", "1.0beta-2");

      Azure::Core::Context clientContext;
      Azure::Core::Tracing::_internal::CreateSpanOptions createOptions;
      createOptions.Kind = Azure::Core::Tracing::_internal::SpanKind::Internal;
      createOptions.Attributes = serviceTrace.CreateAttributeSet();
      createOptions.Attributes->AddAttribute("TestAttribute", 3);
      auto contextAndSpan
          = serviceTrace.CreateTracingContext("My API", createOptions, clientContext);
      EXPECT_FALSE(contextAndSpan.Context.IsCancelled());
    }

    // Now let's verify what was logged via OpenTelemetry.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());

    VerifySpan(spans[0], R"(
{
  "name": "My API",
  "kind": "internal",
  "attributes": {
     "az.namespace": "my-service",
     "TestAttribute": 3
  },
  "library": {
    "name": "my-service",
    "version": "1.0beta-2"
  }
})");
  }

  // Clear the global tracer provider set earlier in the test.
  Azure::Core::Context::ApplicationContext.SetTracerProvider(nullptr);
}

TEST_F(OpenTelemetryServiceTests, NestSpans)
{
  {
    auto tracerProvider(CreateOpenTelemetryProvider());
    auto provider(
        Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(tracerProvider));

    Azure::Core::Context::ApplicationContext.SetTracerProvider(provider);

    Azure::Core::Http::Request outerRequest(
        HttpMethod::Post, Azure::Core::Url("https://www.microsoft.com"));
    Azure::Core::Http::Request innerRequest(
        HttpMethod::Post, Azure::Core::Url("https://www.microsoft.com"));
    {
      Azure::Core::_internal::ClientOptions clientOptions;
      clientOptions.Telemetry.ApplicationId = "MyApplication";

      Azure::Core::Tracing::_internal::TracingContextFactory serviceTrace(
          clientOptions, "my-service", "1.0beta-2");

      Azure::Core::Context parentContext;
      Azure::Core::Tracing::_internal::CreateSpanOptions createOptions;
      createOptions.Kind = Azure::Core::Tracing::_internal::SpanKind::Client;
      auto contextAndSpan
          = serviceTrace.CreateTracingContext("My API", createOptions, parentContext);
      EXPECT_FALSE(contextAndSpan.Context.IsCancelled());
      parentContext = contextAndSpan.Context;
      contextAndSpan.Span.PropagateToHttpHeaders(outerRequest);

      {
        Azure::Core::Tracing::_internal::CreateSpanOptions innerOptions;
        innerOptions.Kind = Azure::Core::Tracing::_internal::SpanKind::Server;
        auto innerContextAndSpan
            = serviceTrace.CreateTracingContext("Nested API", innerOptions, parentContext);
        EXPECT_FALSE(innerContextAndSpan.Context.IsCancelled());
        innerContextAndSpan.Span.PropagateToHttpHeaders(innerRequest);
      }
    }
    // Now let's verify what was logged via OpenTelemetry.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(2ul, spans.size());

    // Because Nested API goes out of scope before My API, it will be logged first in the
    // tracing spans.
    {
      EXPECT_EQ("Nested API", spans[0]->GetName());
      EXPECT_TRUE(spans[0]->GetParentSpanId().IsValid());
      // The nested span should have the outer span as a parent.
      EXPECT_EQ(spans[1]->GetSpanId(), spans[0]->GetParentSpanId());

      const auto& attributes = spans[0]->GetAttributes();
      EXPECT_EQ(1ul, attributes.size());
      EXPECT_EQ(
          "my-service", opentelemetry::nostd::get<std::string>(attributes.at("az.namespace")));
    }
    {
      EXPECT_EQ("My API", spans[1]->GetName());
      EXPECT_FALSE(spans[1]->GetParentSpanId().IsValid());

      const auto& attributes = spans[1]->GetAttributes();
      EXPECT_EQ(1ul, attributes.size());
      EXPECT_EQ(
          "my-service", opentelemetry::nostd::get<std::string>(attributes.at("az.namespace")));
    }

    EXPECT_EQ("my-service", spans[0]->GetInstrumentationLibrary().GetName());
    EXPECT_EQ("my-service", spans[1]->GetInstrumentationLibrary().GetName());
    EXPECT_EQ("1.0beta-2", spans[0]->GetInstrumentationLibrary().GetVersion());
    EXPECT_EQ("1.0beta-2", spans[1]->GetInstrumentationLibrary().GetVersion());

    // The trace ID for the inner and outer requests must be the same, the parent-id/span-id must be
    // different.
    //
    // Returns a 4 element array.
    // Array[0] is the version of the TraceParent header.
    // Array[1] is the trace-id of the TraceParent header.
    // Array[2] is the parent-id/span-id of the TraceParent header.
    // Array[3] is the trace-flags of the TraceParent header.
    auto ParseTraceParent = [](const std::string& traceParent) -> std::array<std::string, 4> {
      std::array<std::string, 4> returnedComponents;
      std::string component;
      size_t index = 0;
      for (auto ch : traceParent)
      {
        if (ch != '-')
        {
          component.push_back(ch);
        }
        else
        {
          returnedComponents[index] = component;
          component.clear();
          index += 1;
        }
      }
      EXPECT_EQ(3ul, index);
      returnedComponents[3] = component;
      return returnedComponents;
    };
    auto outerTraceId = ParseTraceParent(outerRequest.GetHeader("traceparent").Value());
    auto innerTraceId = ParseTraceParent(innerRequest.GetHeader("traceparent").Value());
    // Version should always match.
    EXPECT_EQ(outerTraceId[0], innerTraceId[0]);
    // Trace ID should always match.
    EXPECT_EQ(outerTraceId[1], innerTraceId[1]);
    // Span-Id should never match.
    EXPECT_NE(outerTraceId[2], innerTraceId[2]);
  }
}

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
} // namespace

// Create a serviceTrace and span using a provider specified in the ClientOptions.
class ServiceClientOptions : public Azure::Core::_internal::ClientOptions {
public:
  explicit ServiceClientOptions() : ClientOptions() {}
};

class ServiceClient {
private:
  ServiceClientOptions m_clientOptions;
  Azure::Core::Tracing::_internal::TracingContextFactory m_tracingFactory;
  std::unique_ptr<Azure::Core::Http::_internal::HttpPipeline> m_pipeline;

public:
  explicit ServiceClient(ServiceClientOptions const& clientOptions = ServiceClientOptions{})
      : m_tracingFactory(clientOptions, "Azure.Core.OpenTelemetry.Test.Service", "1.0.0.beta-2")
  {
    std::vector<std::unique_ptr<HttpPolicy>> policies;
    policies.emplace_back(std::make_unique<RequestIdPolicy>());
    policies.emplace_back(std::make_unique<RetryPolicy>(RetryOptions{}));

    // Add the request ID policy - this adds the x-ms-request-id attribute to the pipeline.
    policies.emplace_back(
        std::make_unique<RequestActivityPolicy>(Azure::Core::Http::_internal::HttpSanitizer{}));

    // Final policy - functions as the HTTP transport policy.
    policies.emplace_back(std::make_unique<NoOpPolicy>([&](Request& request) {
      // If the request is for port 12345, throw an exception.
      if (request.GetUrl().GetPort() == 12345)
      {
        throw Azure::Core::RequestFailedException("it all goes wrong here.");
      }
      return std::make_unique<RawResponse>(1, 1, HttpStatusCode::Ok, "Something");
    }));

    m_pipeline = std::make_unique<Azure::Core::Http::_internal::HttpPipeline>(policies);
  }

  Azure::Response<std::string> GetConfigurationString(
      std::string const& inputString,
      Azure::Core::Context const& context = Azure::Core::Context{}) const
  {
    auto contextAndSpan = m_tracingFactory.CreateTracingContext("GetConfigurationString", context);

    // <Call Into Service via an HTTP pipeline>
    Azure::Core::Http::Request requestToSend(
        HttpMethod::Get, Azure::Core::Url("https://www.microsoft.com/"));

    std::unique_ptr<Azure::Core::Http::RawResponse> response
        = m_pipeline->Send(requestToSend, contextAndSpan.Context);

    // Reflect that the operation was successful.
    contextAndSpan.Span.SetStatus(Azure::Core::Tracing::_internal::SpanStatus::Ok);
    Azure::Response<std::string> rv(inputString, std::move(response));
    return rv;
    // When contextAndSpan.second goes out of scope, it ends the span, which will record it.
  }

  Azure::Response<std::string> ApiWhichThrows(
      std::string const&,
      Azure::Core::Context const& context = Azure::Core::Context{}) const
  {
    auto contextAndSpan = m_tracingFactory.CreateTracingContext("ApiWhichThrows", context);

    try
    {
      // <Call Into Service via an HTTP pipeline>
      Azure::Core::Http::Request requestToSend(
          HttpMethod::Get, Azure::Core::Url("https://www.microsoft.com/:12345/index.html"));

      std::unique_ptr<Azure::Core::Http::RawResponse> response
          = m_pipeline->Send(requestToSend, contextAndSpan.Context);
      return Azure::Response<std::string>("", std::move(response));
    }
    catch (std::exception const& ex)
    {
      // Register that the exception has happened and that the span is now in error.
      contextAndSpan.Span.AddEvent(ex);
      contextAndSpan.Span.SetStatus(Azure::Core::Tracing::_internal::SpanStatus::Error);
      throw;
    }

    // When contextAndSpan.second goes out of scope, it ends the span, which will record it.
  }
};

TEST_F(OpenTelemetryServiceTests, ServiceApiImplementation)
{
  {
    auto tracerProvider(CreateOpenTelemetryProvider());
    auto provider(
        Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(tracerProvider));

    {
      // Call a simple API and verify telemetry is generated.
      {
        ServiceClientOptions clientOptions;
        clientOptions.Telemetry.TracingProvider = provider;
        clientOptions.Telemetry.ApplicationId = "MyApplication";
        ServiceClient myServiceClient(clientOptions);

        myServiceClient.GetConfigurationString("Fred");
      }
      // Now let's verify what was logged via OpenTelemetry.
      auto spans = m_spanData->GetSpans();
      EXPECT_EQ(2ul, spans.size());

      VerifySpan(spans[0], R"(
{
  "name": "HTTP GET",
  "kind": "client",
  "statusCode": "unset",
  "attributes": {
    "az.namespace": "Azure.Core.OpenTelemetry.Test.Service",
    "http.method": "GET",
    "http.url": "https://www.microsoft.com",
    "requestId": ".*",
    "http.user_agent": "MyApplication azsdk-cpp-Azure.Core.OpenTelemetry.Test.Service/1.0.0.beta-2.*",
    "http.status_code": "200"
  },
  "library": {
    "name": "Azure.Core.OpenTelemetry.Test.Service",
    "version": "1.0.0.beta-2"
  }
})");

      VerifySpan(spans[1], R"(
{
  "name": "GetConfigurationString",
  "kind": "internal",
  "statusCode": "ok",
  "attributes": {
    "az.namespace": "Azure.Core.OpenTelemetry.Test.Service"
  },
  "library": {
    "name": "Azure.Core.OpenTelemetry.Test.Service",
    "version": "1.0.0.beta-2"
  }
})");
    }
  }
  // Call into the fake service client ensuring that no telemetry is generated.
  {
    // Call a simple API and verify no telemetry is generated.
    {
      ServiceClient myServiceClient;

      myServiceClient.GetConfigurationString("George");
    }
    // Now let's verify what was logged via OpenTelemetry.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(0ul, spans.size());
  }
}
