// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#define USE_MEMORY_EXPORTER 1
#include "azure/core-opentelemetry/opentelemetry.hpp"
#include <azure/core/test/test_base.hpp>
#include <opentelemetry/exporters/memory/in_memory_span_data.h>
#include <opentelemetry/exporters/memory/in_memory_span_exporter.h>
#include <opentelemetry/exporters/ostream/span_exporter.h>
#include <opentelemetry/sdk/common/global_log_handler.h>
#include <opentelemetry/sdk/trace/exporter.h>
#include <opentelemetry/sdk/trace/processor.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>

#include <chrono>
#include <gtest/gtest.h>

class OpenTelemetryTests : public Azure::Core::Test::TestBase {
private:
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
      attributes;
    }
  };

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
};

TEST_F(OpenTelemetryTests, Basic)
{
  // Simple create an OTel telemetry provider as a static member variable.
  {
    Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider provider;
    auto tracer = provider.CreateTracer("TracerName", "1.0");
    EXPECT_TRUE(tracer);
  }

  // Create a shared provider using the tracing abstract classes.
  {
    std::shared_ptr<Azure::Core::Tracing::TracerProvider> provider
        = std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>();
    auto tracer = provider->CreateTracer("TracerName", "1.0");
    EXPECT_TRUE(tracer);
  }

  // Create a provider using the OpenTelemetry default provider (this will be a "noop" provider).
  {
    auto rawTracer(opentelemetry::trace::Provider::GetTracerProvider());

    auto traceProvider
        = std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(rawTracer);

    auto tracer = traceProvider->CreateTracer("TracerName");
    EXPECT_TRUE(tracer);
  }

  // Create a provider using the OpenTelemetry reference provider (this will be a working provider
  // using the ostream logger).
  {
    auto otelProvider(CreateOpenTelemetryProvider());
    auto traceProvider
        = std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
            otelProvider);

    auto tracer = traceProvider->CreateTracer("TracerName");
  }
}

TEST_F(OpenTelemetryTests, CreateSpanSimple)
{
  // Simple create an OTel telemetry provider as a static member variable.
  {
    Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider provider;
    auto tracer = provider.CreateTracer("TracerName", "1.0");
    EXPECT_TRUE(tracer);
    auto span = tracer->CreateSpan("My Span");
    EXPECT_TRUE(span);

    span->End();
  }

  // Create a provider using the OpenTelemetry reference provider (this will be a working provider
  // using the ostream logger).
  {
    std::shared_ptr<Azure::Core::Tracing::TracerProvider> traceProvider
        = std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
            CreateOpenTelemetryProvider());

    auto tracer = traceProvider->CreateTracer("TracerName");
    {
      auto span = tracer->CreateSpan("My Span2");
      EXPECT_TRUE(span);

      span->End();
    }
    // Return the collected spans.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());
    // Make sure that the span we collected looks right.
    EXPECT_EQ("My Span2", spans[0]->GetName());
    EXPECT_EQ(opentelemetry::trace::StatusCode::kUnset, spans[0]->GetStatus());
    auto spanContext(spans[0]->GetSpanContext());
    EXPECT_TRUE(spanContext.IsValid());
  }
}

TEST_F(OpenTelemetryTests, TestAttributeSet)
{
  {
    Azure::Core::Tracing::OpenTelemetry::OpenTelemetryAttributeSet attributeSet;
  }

  {
    Azure::Core::Tracing::OpenTelemetry::OpenTelemetryAttributeSet attributeSet;
    // Add a C style string.
    attributeSet.AddAttribute("String", "StringValue");

    attributeSet.ForEachKeyValue(
        [](opentelemetry::nostd::string_view name, opentelemetry::common::AttributeValue value) {
          EXPECT_EQ(name, "String");
          EXPECT_EQ(0, strcmp("StringValue", opentelemetry::nostd::get<const char*>(value)));
          return true;
        });
  }

  {
    Azure::Core::Tracing::OpenTelemetry::OpenTelemetryAttributeSet attributeSet;
    attributeSet.AddAttribute("boolTrue", true);
    attributeSet.AddAttribute("boolFalse", false);

    attributeSet.ForEachKeyValue(
        [](opentelemetry::nostd::string_view name, opentelemetry::common::AttributeValue value) {
          if (name == "boolTrue")
          {
            EXPECT_TRUE(opentelemetry::nostd::get<bool>(value));
          }
          else if (name == "boolFalse")
          {
            EXPECT_FALSE(opentelemetry::nostd::get<bool>(value));
          }
          else
          {
            EXPECT_TRUE(false);
          }
          return true;
        });
  }
  {
    Azure::Core::Tracing::OpenTelemetry::OpenTelemetryAttributeSet attributeSet;
    attributeSet.AddAttribute("int1", 1);
    attributeSet.AddAttribute("pi", 3.1415926);
    attributeSet.AddAttribute("int64", 151031ll);
    attributeSet.AddAttribute("uint64", 1ull);
    attributeSet.AddAttribute("charstring", "char * string.");
    // Note that the attribute set doesn't take ownership of the input value, so we need to ensure
    // the lifetime of any std::string values put into the set.
    std::string stringValue("std::string.");
    attributeSet.AddAttribute("stdstring", stringValue);

    attributeSet.ForEachKeyValue([](opentelemetry::nostd::string_view name,
                                    opentelemetry::common::AttributeValue value) {
      if (name == "int1")
      {
        EXPECT_EQ(1, opentelemetry::nostd::get<int32_t>(value));
      }
      else if (name == "pi")
      {
        EXPECT_EQ(3.1415926, opentelemetry::nostd::get<double>(value));
      }
      else if (name == "int64")
      {
        EXPECT_EQ(151031, opentelemetry::nostd::get<int64_t>(value));
      }
      else if (name == "uint64")
      {
        EXPECT_EQ(1, opentelemetry::nostd::get<uint64_t>(value));
      }
      else if (name == "charstring")
      {
        const char* cstrVal(opentelemetry::nostd::get<const char*>(value));
        EXPECT_EQ(0, strcmp(cstrVal, "char * string."));
      }
      else if (name == "stdstring")
      {
        EXPECT_EQ(
            "std::string.", opentelemetry::nostd::get<opentelemetry::nostd::string_view>(value));
      }
      else
      {
        EXPECT_TRUE(false);
      }
      return true;
    });
  }
}

TEST_F(OpenTelemetryTests, CreateSpanWithOptions)
{
  // Simple create an OTel telemetry provider as a static member variable.
  {
    Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider provider;
    auto tracer = provider.CreateTracer("TracerName", "1.0");
    EXPECT_TRUE(tracer);
    Azure::Core::Tracing::CreateSpanOptions options;
    auto span = tracer->CreateSpan("My Span", options);
    EXPECT_TRUE(span);

    span->End();
  }

  // Create a provider using the OpenTelemetry reference provider (this will be a working provider
  // using the ostream logger).
  {
    std::shared_ptr<Azure::Core::Tracing::TracerProvider> traceProvider
        = std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
            CreateOpenTelemetryProvider());

    auto tracer = traceProvider->CreateTracer("TracerName");
    {
      Azure::Core::Tracing::CreateSpanOptions options;
      options.SpanKind = Azure::Core::Tracing::SpanKind::Client;
      auto span = tracer->CreateSpan("Client Span", options);
      EXPECT_TRUE(span);

      span->End();
    }
    {
      Azure::Core::Tracing::CreateSpanOptions options;
      options.SpanKind = Azure::Core::Tracing::SpanKind::Consumer;
      auto span = tracer->CreateSpan("Consumer Span", options);
      EXPECT_TRUE(span);

      span->End();
    }
    {
      Azure::Core::Tracing::CreateSpanOptions options;
      options.SpanKind = Azure::Core::Tracing::SpanKind::Internal;
      auto span = tracer->CreateSpan("Internal Span", options);
      EXPECT_TRUE(span);

      span->End();
    }
    {
      Azure::Core::Tracing::CreateSpanOptions options;
      options.SpanKind = Azure::Core::Tracing::SpanKind::Producer;
      auto span = tracer->CreateSpan("Producer Span", options);
      EXPECT_TRUE(span);

      span->End();
    }
    {
      Azure::Core::Tracing::CreateSpanOptions options;
      options.SpanKind = Azure::Core::Tracing::SpanKind::Server;
      auto span = tracer->CreateSpan("Server Span", options);
      EXPECT_TRUE(span);

      span->End();
    }
    {
      Azure::Core::Tracing::CreateSpanOptions options;
      options.SpanKind = Azure::Core::Tracing::SpanKind("Bogus");
      EXPECT_THROW(tracer->CreateSpan("Bogus Span", options), std::runtime_error);
    }
    // Return the collected spans.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(5ul, spans.size());
    // Make sure that the span we collected looks right.
    EXPECT_EQ("Client Span", spans[0]->GetName());
    EXPECT_EQ(opentelemetry::trace::SpanKind::kClient, spans[0]->GetSpanKind());
    EXPECT_EQ("Consumer Span", spans[1]->GetName());
    EXPECT_EQ(opentelemetry::trace::SpanKind::kConsumer, spans[1]->GetSpanKind());
    EXPECT_EQ("Internal Span", spans[2]->GetName());
    EXPECT_EQ(opentelemetry::trace::SpanKind::kInternal, spans[2]->GetSpanKind());
    EXPECT_EQ("Producer Span", spans[3]->GetName());
    EXPECT_EQ(opentelemetry::trace::SpanKind::kProducer, spans[3]->GetSpanKind());
    EXPECT_EQ("Server Span", spans[4]->GetName());
    EXPECT_EQ(opentelemetry::trace::SpanKind::kServer, spans[4]->GetSpanKind());
  }

  {
    // Create a provider using the OpenTelemetry reference provider (this will be a working provider
    // using the ostream logger).
    {
      std::shared_ptr<Azure::Core::Tracing::TracerProvider> traceProvider
          = std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
              CreateOpenTelemetryProvider());

      auto tracer = traceProvider->CreateTracer("TracerName");
      {
        Azure::Core::Tracing::CreateSpanOptions options;
        options.Attributes
            = std::make_unique<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryAttributeSet>();
        options.Attributes->AddAttribute("SimpleStringAttribute", "Simple String");
        options.SpanKind = Azure::Core::Tracing::SpanKind::Client;
        auto span = tracer->CreateSpan("Client Span", options);
        EXPECT_TRUE(span);

        span->End();

        // Return the collected spans.
        auto spans = m_spanData->GetSpans();
        EXPECT_EQ(1ul, spans.size());
        // Make sure that the span we collected looks right.
        EXPECT_EQ("Client Span", spans[0]->GetName());
        EXPECT_EQ(1l, spans[0]->GetAttributes().size());
        EXPECT_NE(
            spans[0]->GetAttributes().end(),
            spans[0]->GetAttributes().find("SimpleStringAttribute"));
        EXPECT_EQ(
            "Simple String",
            opentelemetry::nostd::get<std::string>(
                spans[0]->GetAttributes().at("SimpleStringAttribute")));
      }
    }
  }
}

TEST_F(OpenTelemetryTests, NestSpans)
{

  {
    std::shared_ptr<Azure::Core::Tracing::TracerProvider> traceProvider
        = std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
            CreateOpenTelemetryProvider());

    auto tracer = traceProvider->CreateTracer("TracerName");
    auto span = tracer->CreateSpan("SpanOuter");
    EXPECT_TRUE(span);
    {
      auto span2 = tracer->CreateSpan("SpanInner");
      auto span3 = tracer->CreateSpan("SpanInner2");
      auto span4 = tracer->CreateSpan("SpanInner4");
      span2->End();

      span->End();
      span4->End();
      span3->End();
    }
    {
      auto span5 = tracer->CreateSpan("SequentialInner");
      span5->End();
    }
    {
      auto span6 = tracer->CreateSpan("SequentialInner2");
      span6->End();
    }

    // Return the collected spans.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(6ul, spans.size());
    // Make sure that the span we collected looks right.
    // The spans are ordered in the order they called "End", since a span that hasn't ended cannot
    // be recorded.
    EXPECT_EQ("SpanInner", spans[0]->GetName());
    EXPECT_EQ("SpanOuter", spans[1]->GetName());
    EXPECT_EQ("SpanInner4", spans[2]->GetName());
    EXPECT_EQ("SpanInner2", spans[3]->GetName());
    EXPECT_EQ("SequentialInner", spans[4]->GetName());
    EXPECT_EQ("SequentialInner2", spans[5]->GetName());
    EXPECT_FALSE(spans[1]->GetParentSpanId().IsValid()); // Span 1 should be a root span.
    EXPECT_TRUE(spans[0]->GetParentSpanId().IsValid()); // Span 0 should not be a root span.
    EXPECT_TRUE(spans[2]->GetParentSpanId().IsValid()); // Span 2 should not be a root span.
    EXPECT_TRUE(spans[3]->GetParentSpanId().IsValid()); // Span 3 should not be a root span.
    EXPECT_TRUE(spans[4]->GetParentSpanId().IsValid()); // Span 4 should not be a root span.
    EXPECT_TRUE(spans[5]->GetParentSpanId().IsValid()); // Span 5 should not be a root span.

    // SpanInner should have SpanOuter as a parent.
    EXPECT_EQ(spans[0]->GetParentSpanId(), spans[1]->GetSpanId());

    // SpanInner2 should have SpanInner as a parent.
    EXPECT_EQ(spans[3]->GetParentSpanId(), spans[0]->GetSpanId());

    // SpanInner4 should have SpanInner2 as a parent.
    EXPECT_EQ(spans[2]->GetParentSpanId(), spans[3]->GetSpanId());

    // SequentialInner should have SpanOuter as a parent.
    EXPECT_EQ(spans[4]->GetParentSpanId(), spans[1]->GetSpanId());

    // SequentialInner2 should have SpanOuter as a parent.
    EXPECT_EQ(spans[5]->GetParentSpanId(), spans[1]->GetSpanId());
  }
}

TEST_F(OpenTelemetryTests, SetStatus)
{

  {
    std::shared_ptr<Azure::Core::Tracing::TracerProvider> traceProvider
        = std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
            CreateOpenTelemetryProvider());

    auto tracer = traceProvider->CreateTracer("TracerName");
    auto span = tracer->CreateSpan("StatusSpan");
    EXPECT_TRUE(span);

    span->SetStatus(Azure::Core::Tracing::SpanStatus::Error);
    span->SetStatus(Azure::Core::Tracing::SpanStatus::Ok);

    span->End();

    // Return the collected spans.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());

    EXPECT_EQ(opentelemetry::trace::StatusCode::kOk, spans[0]->GetStatus());
  }

  {
    std::shared_ptr<Azure::Core::Tracing::TracerProvider> traceProvider
        = std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
            CreateOpenTelemetryProvider());

    auto tracer = traceProvider->CreateTracer("TracerName");
    auto span = tracer->CreateSpan("StatusSpan");
    EXPECT_TRUE(span);

    span->SetStatus(Azure::Core::Tracing::SpanStatus::Error, "Something went wrong.");

    span->End();

    // Return the collected spans.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());

    EXPECT_EQ(opentelemetry::trace::StatusCode::kError, spans[0]->GetStatus());
    EXPECT_EQ("Something went wrong.", spans[0]->GetDescription());
  }

  // Set to Unset.
  {
    std::shared_ptr<Azure::Core::Tracing::TracerProvider> traceProvider
        = std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
            CreateOpenTelemetryProvider());

    auto tracer = traceProvider->CreateTracer("TracerName");
    auto span = tracer->CreateSpan("StatusSpan");
    EXPECT_TRUE(span);

    span->SetStatus(Azure::Core::Tracing::SpanStatus::Unset);

    span->End();

    // Return the collected spans.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());

    EXPECT_EQ(opentelemetry::trace::StatusCode::kUnset, spans[0]->GetStatus());
  }

  // Not set.
  {
    std::shared_ptr<Azure::Core::Tracing::TracerProvider> traceProvider
        = std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
            CreateOpenTelemetryProvider());

    auto tracer = traceProvider->CreateTracer("TracerName");
    auto span = tracer->CreateSpan("StatusSpan");
    EXPECT_TRUE(span);

    span->End();

    // Return the collected spans.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());

    EXPECT_EQ(opentelemetry::trace::StatusCode::kUnset, spans[0]->GetStatus());
  }

  // Invalid status.
  {
    std::shared_ptr<Azure::Core::Tracing::TracerProvider> traceProvider
        = std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
            CreateOpenTelemetryProvider());

    auto tracer = traceProvider->CreateTracer("TracerName");
    auto span = tracer->CreateSpan("StatusSpan");
    EXPECT_TRUE(span);

    EXPECT_THROW(span->SetStatus(Azure::Core::Tracing::SpanStatus("Bogus")), std::runtime_error);

    // Return the collected spans.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(0ul, spans.size());
  }
}

TEST_F(OpenTelemetryTests, AddSpanAttributes)
{

  {
    std::shared_ptr<Azure::Core::Tracing::TracerProvider> traceProvider
        = std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
            CreateOpenTelemetryProvider());

    auto tracer = traceProvider->CreateTracer("TracerName");
    auto span = tracer->CreateSpan("AttributeSpan");
    EXPECT_TRUE(span);

    Azure::Core::Tracing::OpenTelemetry::OpenTelemetryAttributeSet attributeSet;
    attributeSet.AddAttribute("int1", 1);
    attributeSet.AddAttribute("pi", 3.1415926);
    attributeSet.AddAttribute("int64", 151031ll);
    attributeSet.AddAttribute("uint64", 1ull);
    attributeSet.AddAttribute("charstring", "char * string.");
    // Note that the attribute set doesn't take ownership of the input value, so we need to ensure
    // the lifetime of any std::string values put into the set.
    std::string stringValue("std::string.");
    attributeSet.AddAttribute("stdstring", stringValue);
    span->AddAttributes(attributeSet);
    span->End();

    // Return the collected spans.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());

    // Make sure that the span we collected looks right.
    EXPECT_EQ("AttributeSpan", spans[0]->GetName());
    EXPECT_EQ(6l, spans[0]->GetAttributes().size());
    EXPECT_NE(spans[0]->GetAttributes().end(), spans[0]->GetAttributes().find("int1"));
    EXPECT_NE(spans[0]->GetAttributes().end(), spans[0]->GetAttributes().find("pi"));
    EXPECT_NE(spans[0]->GetAttributes().end(), spans[0]->GetAttributes().find("int64"));
    EXPECT_NE(spans[0]->GetAttributes().end(), spans[0]->GetAttributes().find("uint64"));
    EXPECT_NE(spans[0]->GetAttributes().end(), spans[0]->GetAttributes().find("charstring"));
    EXPECT_NE(spans[0]->GetAttributes().end(), spans[0]->GetAttributes().find("stdstring"));
  }
}