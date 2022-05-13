// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#define USE_MEMORY_EXPORTER 1
#include "azure/core-tracing-opentelemetry/opentelemetry.hpp"
#include "azure/core/internal/tracing/service_tracing.hpp"
#include <azure/core/test/test_base.hpp>

#if defined(_MSC_VER)
// The OpenTelemetry headers generate a couple of warnings on MSVC in the OTel 1.2 package, suppress
// the warnings across the includes.
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4244)
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
};

TEST_F(OpenTelemetryServiceTests, SimplestTest)
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

TEST_F(OpenTelemetryServiceTests, CreateWithExplicitProvider)
{
  // Create a serviceTrace, set it and retrieve it via a Context object. This verifies that we can
  // round-trip telemetry providers through a Context (which allows us to hook this up to the
  // ApplicationContext later).
  //
  {
    auto tracerProvider(CreateOpenTelemetryProvider());
    auto provider(std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
        tracerProvider));

    Azure::Core::Context rootContext;
    rootContext.SetTracerProvider(provider);
    EXPECT_EQ(provider, rootContext.GetTracerProvider());
  }

  {
    auto tracerProvider(CreateOpenTelemetryProvider());
    auto provider(std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
        tracerProvider));

    // Create a serviceTrace and span using a provider specified in the ClientOptions.
    {
      Azure::Core::_internal::ClientOptions clientOptions;
      clientOptions.Telemetry.TracingProvider = provider;
      clientOptions.Telemetry.ApplicationId = "MyApplication";

      Azure::Core::Tracing::_internal::ServiceTracing serviceTrace(
          clientOptions, "my-service", "1.0beta-2");

      Azure::Core::Context clientContext;
      auto contextAndSpan = serviceTrace.CreateSpan("My API", clientContext);
      EXPECT_FALSE(contextAndSpan.first.IsCancelled());
    }
    // Now let's verify what was logged via OpenTelemetry.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());

    EXPECT_EQ("My API", spans[0]->GetName());
    const auto& attributes = spans[0]->GetAttributes();
    const auto& azNamespace = attributes.at("az.namespace");
    const auto& namespaceVal = opentelemetry::nostd::get<std::string>(azNamespace);
    EXPECT_EQ("my-service", namespaceVal);
    const auto& library = spans[0]->GetInstrumentationLibrary();
    EXPECT_EQ("my-service", library.GetName());
    EXPECT_EQ("1.0beta-2", library.GetVersion());
  }
}

TEST_F(OpenTelemetryServiceTests, CreateWithImplicitProvider)
{
  {
    auto tracerProvider(CreateOpenTelemetryProvider());
    auto provider(std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
        tracerProvider));

    Azure::Core::Context::ApplicationContext.SetTracerProvider(provider);

    {
      Azure::Core::_internal::ClientOptions clientOptions;
      clientOptions.Telemetry.ApplicationId = "MyApplication";

      Azure::Core::Tracing::_internal::ServiceTracing serviceTrace(
          clientOptions, "my-service", "1.0beta-2");

      Azure::Core::Context clientContext;
      auto contextAndSpan = serviceTrace.CreateSpan("My API", clientContext);
      EXPECT_FALSE(contextAndSpan.first.IsCancelled());
    }

    // Now let's verify what was logged via OpenTelemetry.
    auto spans = m_spanData->GetSpans();
    EXPECT_EQ(1ul, spans.size());
    EXPECT_EQ("My API", spans[0]->GetName());
    const auto& attributes = spans[0]->GetAttributes();
    EXPECT_EQ("my-service", opentelemetry::nostd::get<std::string>(attributes.at("az.namespace")));
    EXPECT_EQ("my-service", spans[0]->GetInstrumentationLibrary().GetName());
    EXPECT_EQ("1.0beta-2", spans[0]->GetInstrumentationLibrary().GetVersion());
  }

  // Clear the global tracer provider set earlier in the test.
  Azure::Core::Context::ApplicationContext.SetTracerProvider(nullptr);
}

TEST_F(OpenTelemetryServiceTests, NestSpans)
{
  {
    auto tracerProvider(CreateOpenTelemetryProvider());
    auto provider(std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
        tracerProvider));

    Azure::Core::Context::ApplicationContext.SetTracerProvider(provider);

    {
      Azure::Core::_internal::ClientOptions clientOptions;
      clientOptions.Telemetry.ApplicationId = "MyApplication";

      Azure::Core::Tracing::_internal::ServiceTracing serviceTrace(
          clientOptions, "my-service", "1.0beta-2");

      Azure::Core::Context parentContext;
      auto contextAndSpan = serviceTrace.CreateSpan("My API", parentContext);
      EXPECT_FALSE(contextAndSpan.first.IsCancelled());
      parentContext = contextAndSpan.first;

      {
        auto innerContextAndSpan = serviceTrace.CreateSpan("Nested API", parentContext);
        EXPECT_FALSE(innerContextAndSpan.first.IsCancelled());
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
      EXPECT_EQ("my-service", opentelemetry::nostd::get<std::string>(attributes.at("az.namespace")));
    }
    {
      EXPECT_EQ("My API", spans[1]->GetName());
      EXPECT_FALSE(spans[1]->GetParentSpanId().IsValid());

      const auto &attributes = spans[1]->GetAttributes();
      EXPECT_EQ(1ul, attributes.size());
      EXPECT_EQ("my-service", opentelemetry::nostd::get<std::string>(attributes.at("az.namespace")));
    }

    EXPECT_EQ("my-service", spans[0]->GetInstrumentationLibrary().GetName());
    EXPECT_EQ("my-service", spans[1]->GetInstrumentationLibrary().GetName());
    EXPECT_EQ("1.0beta-2", spans[0]->GetInstrumentationLibrary().GetVersion());
    EXPECT_EQ("1.0beta-2", spans[1]->GetInstrumentationLibrary().GetVersion());
  }
}

TEST_F(OpenTelemetryServiceTests, ServiceApiImplementation)
{
  // Create a serviceTrace and span using a provider specified in the ClientOptions.
  class ServiceClientOptions : public Azure::Core::_internal::ClientOptions {
  public:
    explicit ServiceClientOptions() : ClientOptions() {}
  };

  class ServiceClient {
  private:
    ServiceClientOptions m_clientOptions;
    Azure::Core::Tracing::_internal::ServiceTracing m_serviceTrace;

  public:
    explicit ServiceClient(ServiceClientOptions const& clientOptions = ServiceClientOptions{})
        : m_serviceTrace(clientOptions, "Azure.Core.OpenTelemetry.Test.Service", "1.0.0.beta-2")
    {
    }

    Azure::Response<std::string> GetConfigurationString(
        std::string const& inputString,
        Azure::Core::Context const& context = Azure::Core::Context{})
    {
      auto contextAndSpan = m_serviceTrace.CreateSpan("GetConfigurationString", context);

      // <Call Into Service via an HTTP pipeline>

      // Reflect that the operation was successful.
      contextAndSpan.second.SetStatus(Azure::Core::Tracing::_internal::SpanStatus::Ok);
      Azure::Response<std::string> rv(
          inputString,
          std::make_unique<Azure::Core::Http::RawResponse>(
              1, 1, Azure::Core::Http::HttpStatusCode::Ok, "OK"));
      return rv;
      // When contextAndSpan.second goes out of scope, it ends the span, which will record it.
    }

    Azure::Response<std::string> ApiWhichThrows(
        std::string const&,
        Azure::Core::Context const& context = Azure::Core::Context{})
    {
      auto contextAndSpan = m_serviceTrace.CreateSpan("ApiWhichThrows", context);

      try
      {
        // <Call Into Service via an HTTP pipeline>

        throw Azure::Core::RequestFailedException("it all goes wrong here.");
      }
      catch (std::exception& ex)
      {
        // Register that the exception has happened and that the span is now in error.
        contextAndSpan.second.AddEvent(ex);
        contextAndSpan.second.SetStatus(Azure::Core::Tracing::_internal::SpanStatus::Error);
        throw;
      }

      // When contextAndSpan.second goes out of scope, it ends the span, which will record it.
    }
  };

  {
    auto tracerProvider(CreateOpenTelemetryProvider());
    auto provider(std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
        tracerProvider));

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
      EXPECT_EQ(1ul, spans.size());

      EXPECT_EQ("GetConfigurationString", spans[0]->GetName());
      EXPECT_EQ(opentelemetry::trace::SpanKind::kInternal, spans[0]->GetSpanKind());
      EXPECT_EQ(opentelemetry::trace::StatusCode::kOk, spans[0]->GetStatus());

      auto const& attributes = spans[0]->GetAttributes();
      auto const& azNamespace = attributes.at("az.namespace");
      auto const& namespaceVal = opentelemetry::nostd::get<std::string>(azNamespace);
      EXPECT_EQ("Azure.Core.OpenTelemetry.Test.Service", namespaceVal);
      auto const& library = spans[0]->GetInstrumentationLibrary();
      EXPECT_EQ("Azure.Core.OpenTelemetry.Test.Service", library.GetName());
      EXPECT_EQ("1.0.0.beta-2", library.GetVersion());
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
