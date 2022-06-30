// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#define USE_MEMORY_EXPORTER 1
#include "../src/opentelemetry_private.hpp"
#include "azure/core/tracing/opentelemetry/opentelemetry.hpp"
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
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

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
  // Simple create an OTel telemetry provider and call a method on the concrete implementation.
  {
    auto provider = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create();

    auto tracer
        = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(provider)
              ->CreateTracer("TracerName", "1.0");
    EXPECT_TRUE(tracer);
  }

  // Create a provider using the tracing abstract classes.
  {
    auto otProvider = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create();
    auto provider = otProvider;

    std::shared_ptr<Azure::Core::Tracing::_internal::TracerProviderImpl> providerImpl
        = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(provider);
    auto tracer = providerImpl->CreateTracer("TracerName", "1.0");
    EXPECT_TRUE(tracer);
  }

  // Create a provider using the OpenTelemetry default provider (this will be a "noop" provider).
  {
    auto rawTracer(opentelemetry::trace::Provider::GetTracerProvider());

    auto traceProvider
        = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(rawTracer);

    auto tracer = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(
                      traceProvider)
                      ->CreateTracer("TracerName", "1.0");
    EXPECT_TRUE(tracer);
  }

  // Create a provider using the OpenTelemetry reference provider (this will be a working provider
  // using the ostream logger).
  {
    auto otelProvider(CreateOpenTelemetryProvider());
    auto traceProvider
        = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(otelProvider);

    auto tracer = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(
                      traceProvider)
                      ->CreateTracer("TracerName", {});
  }
}

TEST_F(OpenTelemetryTests, CreateSpanSimple)
{
  // Simple create an OTel telemetry provider as a static member variable.
  {
    auto provider(Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create());
    auto tracer
        = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(provider)
              ->CreateTracer("TracerName", "1.0");
    EXPECT_TRUE(tracer);
    auto span = tracer->CreateSpan("My Span", {});
    EXPECT_TRUE(span);

    span->End({});
  }

  // Create a provider using the OpenTelemetry reference provider (this will be a working provider
  // using the ostream logger).
  {
    auto otProvider = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(
        CreateOpenTelemetryProvider());
    std::shared_ptr<Azure::Core::Tracing::TracerProvider> traceProvider(otProvider);

    auto tracer = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(
                      traceProvider)
                      ->CreateTracer("TracerName", {});
    {
      auto span = tracer->CreateSpan("My Span2", {});
      EXPECT_TRUE(span);

      span->End({});
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
    Azure::Core::Tracing::OpenTelemetry::_detail::OpenTelemetryAttributeSet attributeSet;
  }

  {
    Azure::Core::Tracing::OpenTelemetry::_detail::OpenTelemetryAttributeSet attributeSet;
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
    Azure::Core::Tracing::OpenTelemetry::_detail::OpenTelemetryAttributeSet attributeSet;
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
    Azure::Core::Tracing::OpenTelemetry::_detail::OpenTelemetryAttributeSet attributeSet;
    attributeSet.AddAttribute("int1", 1);
    attributeSet.AddAttribute("pi", 3.1415926);
    attributeSet.AddAttribute("int64", static_cast<int64_t>(151031ll));
    attributeSet.AddAttribute("uint64", static_cast<uint64_t>(1ull));
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
    auto provider = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(
        opentelemetry::trace::Provider::GetTracerProvider());
    auto tracer
        = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(provider)
              ->CreateTracer("TracerName", "1.0");
    EXPECT_TRUE(tracer);
    Azure::Core::Tracing::_internal::CreateSpanOptions options;
    auto span = tracer->CreateSpan("My Span", options);
    EXPECT_TRUE(span);

    span->End({});
  }

  // Create a provider using the OpenTelemetry reference provider (this will be a working provider
  // using the ostream logger).
  {
    std::shared_ptr<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider> traceProvider
        = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(
            CreateOpenTelemetryProvider());

    auto tracerImpl
        = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(
            traceProvider);

    auto tracer = tracerImpl->CreateTracer("TracerName", {});
    {
      Azure::Core::Tracing::_internal::CreateSpanOptions options;
      options.Kind = Azure::Core::Tracing::_internal::SpanKind::Client;
      auto span = tracer->CreateSpan("Client Span", options);
      EXPECT_TRUE(span);

      span->End({});
    }
    {
      Azure::Core::Tracing::_internal::CreateSpanOptions options;
      options.Kind = Azure::Core::Tracing::_internal::SpanKind::Consumer;
      auto span = tracer->CreateSpan("Consumer Span", options);
      EXPECT_TRUE(span);

      span->End({});
    }
    {
      Azure::Core::Tracing::_internal::CreateSpanOptions options;
      options.Kind = Azure::Core::Tracing::_internal::SpanKind::Internal;
      auto span = tracer->CreateSpan("Internal Span", options);
      EXPECT_TRUE(span);

      span->End({});
    }
    {
      Azure::Core::Tracing::_internal::CreateSpanOptions options;
      options.Kind = Azure::Core::Tracing::_internal::SpanKind::Producer;
      auto span = tracer->CreateSpan("Producer Span", options);
      EXPECT_TRUE(span);

      span->End({});
    }
    {
      Azure::Core::Tracing::_internal::CreateSpanOptions options;
      options.Kind = Azure::Core::Tracing::_internal::SpanKind::Server;
      auto span = tracer->CreateSpan("Server Span", options);
      EXPECT_TRUE(span);

      span->End({});
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
      auto otProvider = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(
          CreateOpenTelemetryProvider());
      std::shared_ptr<Azure::Core::Tracing::TracerProvider> traceProvider(otProvider);

      auto tracer = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(
                        traceProvider)
                        ->CreateTracer("TracerName", {});
      {
        Azure::Core::Tracing::_internal::CreateSpanOptions options;
        options.Attributes = std::make_unique<
            Azure::Core::Tracing::OpenTelemetry::_detail::OpenTelemetryAttributeSet>();
        options.Attributes->AddAttribute("SimpleStringAttribute", "Simple String");
        options.Kind = Azure::Core::Tracing::_internal::SpanKind::Client;
        auto span = tracer->CreateSpan("Client Span", options);
        EXPECT_TRUE(span);

        span->End({});

        // Return the collected spans.
        auto spans = m_spanData->GetSpans();
        EXPECT_EQ(static_cast<size_t>(1), spans.size());
        // Make sure that the span we collected looks right.
        EXPECT_EQ("Client Span", spans[0]->GetName());
        EXPECT_EQ(static_cast<size_t>(1), spans[0]->GetAttributes().size());
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
        = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(
            CreateOpenTelemetryProvider());

    auto tracer = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(
                      traceProvider)
                      ->CreateTracer("TracerName", {});
    auto span = tracer->CreateSpan("SpanOuter", {});
    EXPECT_TRUE(span);
    {
      Azure::Core::Tracing::_internal::CreateSpanOptions so;
      so.ParentSpan = span;
      auto span2 = tracer->CreateSpan("SpanInner", so);
      so.ParentSpan = span2;
      auto span3 = tracer->CreateSpan("SpanInner2", so);
      // Span 3's parent is SpanOuter.
      so.ParentSpan = span;
      auto span4 = tracer->CreateSpan("SpanInner4", so);
      span2->End({});

      span->End({});
      span4->End({});
      span3->End({});
    }
    {
      Azure::Core::Tracing::_internal::CreateSpanOptions so;
      so.ParentSpan = span;
      auto span5 = tracer->CreateSpan("SequentialInner", so);
      auto span6 = tracer->CreateSpan("SequentialInner2", so);
      span5->End({});
      span6->End({});
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

    // SpanInner2 should have SpanOuter as a parent.
    EXPECT_EQ(spans[3]->GetParentSpanId(), spans[0]->GetSpanId());

    // SpanInner4 should have SpanInner2 as a parent.
    EXPECT_EQ(spans[2]->GetParentSpanId(), spans[1]->GetSpanId());

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
        = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(
            CreateOpenTelemetryProvider());

    auto tracer = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(
                      traceProvider)
                      ->CreateTracer("TracerName", {});
    auto span = tracer->CreateSpan("StatusSpan", {});
    EXPECT_TRUE(span);

    span->SetStatus(Azure::Core::Tracing::_internal::SpanStatus::Error, {});
    span->SetStatus(Azure::Core::Tracing::_internal::SpanStatus::Ok, {});

    span->End({});

    // Return the collected spans.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());

    EXPECT_EQ(opentelemetry::trace::StatusCode::kOk, spans[0]->GetStatus());
  }

  {
    auto traceProvider = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(
        CreateOpenTelemetryProvider());

    auto tracer = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(
                      traceProvider)
                      ->CreateTracer("TracerName", {});
    auto span = tracer->CreateSpan("StatusSpan", {});
    EXPECT_TRUE(span);

    span->SetStatus(Azure::Core::Tracing::_internal::SpanStatus::Error, "Something went wrong.");

    span->End({});

    // Return the collected spans.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());

    EXPECT_EQ(opentelemetry::trace::StatusCode::kError, spans[0]->GetStatus());
    EXPECT_EQ("Something went wrong.", spans[0]->GetDescription());
  }

  // Set to Unset.
  {
    auto traceProvider = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(
        CreateOpenTelemetryProvider());

    auto tracer = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(
                      traceProvider)
                      ->CreateTracer("TracerName", {});
    auto span = tracer->CreateSpan("StatusSpan", {});
    EXPECT_TRUE(span);

    span->SetStatus(Azure::Core::Tracing::_internal::SpanStatus::Unset, {});

    span->End({});

    // Return the collected spans.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());

    EXPECT_EQ(opentelemetry::trace::StatusCode::kUnset, spans[0]->GetStatus());
  }

  // Not set.
  {
    auto traceProvider = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(
        CreateOpenTelemetryProvider());

    auto tracer = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(
                      traceProvider)
                      ->CreateTracer("TracerName", {});
    auto span = tracer->CreateSpan("StatusSpan", {});
    EXPECT_TRUE(span);

    span->End({});

    // Return the collected spans.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());

    EXPECT_EQ(opentelemetry::trace::StatusCode::kUnset, spans[0]->GetStatus());
  }

  // Invalid status.
  {
    auto traceProvider = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(
        CreateOpenTelemetryProvider());

    auto tracer = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(
                      traceProvider)
                      ->CreateTracer("TracerName", {});
    auto span = tracer->CreateSpan("StatusSpan", {});
    EXPECT_TRUE(span);

    // Return the collected spans.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(0ul, spans.size());
  }
}

TEST_F(OpenTelemetryTests, AddSpanAttributes)
{

  {
    auto traceProvider = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(
        CreateOpenTelemetryProvider());

    auto tracer = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(
                      traceProvider)
                      ->CreateTracer("TracerName", {});
    auto span = tracer->CreateSpan("AttributeSpan", {});
    EXPECT_TRUE(span);

    Azure::Core::Tracing::OpenTelemetry::_detail::OpenTelemetryAttributeSet attributeSet;
    attributeSet.AddAttribute("int1", 1);
    attributeSet.AddAttribute("pi", 3.1415926);
    attributeSet.AddAttribute("int64", static_cast<int64_t>(151031ll));
    attributeSet.AddAttribute("uint64", static_cast<uint64_t>(1ull));
    attributeSet.AddAttribute("charstring", "char * string.");
    // Note that the attribute set doesn't take ownership of the input value, so we need to ensure
    // the lifetime of any std::string values put into the set.
    std::string stringValue("std::string.");
    attributeSet.AddAttribute("stdstring", stringValue);
    span->AddAttributes(attributeSet);
    span->End({});

    // Return the collected spans.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());

    // Make sure that the span we collected looks right.
    EXPECT_EQ("AttributeSpan", spans[0]->GetName());
    EXPECT_EQ(6ul, spans[0]->GetAttributes().size());
    EXPECT_NE(spans[0]->GetAttributes().end(), spans[0]->GetAttributes().find("int1"));
    EXPECT_NE(spans[0]->GetAttributes().end(), spans[0]->GetAttributes().find("pi"));
    EXPECT_NE(spans[0]->GetAttributes().end(), spans[0]->GetAttributes().find("int64"));
    EXPECT_NE(spans[0]->GetAttributes().end(), spans[0]->GetAttributes().find("uint64"));
    EXPECT_NE(spans[0]->GetAttributes().end(), spans[0]->GetAttributes().find("charstring"));
    EXPECT_NE(spans[0]->GetAttributes().end(), spans[0]->GetAttributes().find("stdstring"));
  }
}

TEST_F(OpenTelemetryTests, AddSpanEvents)
{
  {
    auto traceProvider = Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(
        CreateOpenTelemetryProvider());

    auto tracer = Azure::Core::Tracing::_internal::TracerProviderImplGetter::TracerImplFromTracer(
                      traceProvider)
                      ->CreateTracer("TracerName", {});
    auto span = tracer->CreateSpan("SpanWithEvents", {});
    EXPECT_TRUE(span);

    span->AddEvent("String Event");
    span->AddEvent(std::runtime_error("Exception message"));

    {
      Azure::Core::Tracing::OpenTelemetry::_detail::OpenTelemetryAttributeSet attributeSet;
      attributeSet.AddAttribute("int1", 1);
      attributeSet.AddAttribute("pi", 3.1415926);
      attributeSet.AddAttribute("int64", static_cast<int64_t>(151031ll));
      attributeSet.AddAttribute("uint64", static_cast<uint64_t>(1ull));
      attributeSet.AddAttribute("charstring", "char * string.");
      // Note that the attribute set doesn't take ownership of the input value, so we need to ensure
      // the lifetime of any std::string values put into the set.
      std::string stringValue("std::string.");
      attributeSet.AddAttribute("stdstring", stringValue);
      span->AddEvent("Event With Attributes", attributeSet);

      span->End({});

      // Return the collected spans.
      auto spans = m_spanData->GetSpans();
      EXPECT_EQ(1ul, spans.size());
      EXPECT_EQ(3UL, spans[0]->GetEvents().size());

      EXPECT_EQ("String Event", spans[0]->GetEvents()[0].GetName());
      EXPECT_EQ("Exception message", spans[0]->GetEvents()[1].GetName());
      EXPECT_EQ("Event With Attributes", spans[0]->GetEvents()[2].GetName());

      const auto& attributes = spans[0]->GetEvents()[2].GetAttributes();

      // Make sure that the span we collected looks right.
      EXPECT_EQ(6ul, attributes.size());
      EXPECT_NE(attributes.end(), attributes.find("int1"));
      EXPECT_NE(attributes.end(), attributes.find("pi"));
      EXPECT_NE(attributes.end(), attributes.find("int64"));
      EXPECT_NE(attributes.end(), attributes.find("uint64"));
      EXPECT_NE(attributes.end(), attributes.find("charstring"));
      EXPECT_NE(attributes.end(), attributes.find("stdstring"));
    }
  }
}
