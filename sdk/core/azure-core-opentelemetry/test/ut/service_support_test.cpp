// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#define USE_MEMORY_EXPORTER 1
#include "azure/core-opentelemetry/opentelemetry.hpp"
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

class OpenTelemetryServiceTests : public Azure::Core::Test::TestBase {
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

TEST_F(OpenTelemetryServiceTests, SimplestTest)
{
  {
    Azure::Core::Tracing::_internal::ServiceTracing serviceTrace;
  }
  {
    Azure::Core::_internal::ClientOptions clientOptions;
    Azure::Core::Tracing::_internal::ServiceTracing serviceTrace(
        clientOptions, "myservice-cpp", "1.0b2");
  }

  {
    Azure::Core::_internal::ClientOptions clientOptions;
    Azure::Core::Tracing::_internal::ServiceTracing serviceTrace(
        clientOptions, "myservice-cpp", "1.0b2");

    auto contextAndSpan = serviceTrace.CreateSpan("My API", {});
    EXPECT_FALSE(contextAndSpan.first.IsCancelled());
  }
  {
    auto tracerProvider(CreateOpenTelemetryProvider());
    auto provider(std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider>(
        tracerProvider));

    Azure::Core::Context rootContext;
    rootContext.SetTracerProvider(provider);

    {
      Azure::Core::_internal::ClientOptions clientOptions;
      clientOptions.Telemetry.ApplicationId = "MyApplication";

      Azure::Core::Tracing::_internal::ServiceTracing serviceTrace(
          clientOptions, "my-service", "1.0beta-2");

      Azure::Core::Context clientContext(rootContext);
      auto contextAndSpan = serviceTrace.CreateSpan("My API", clientContext);
      EXPECT_FALSE(contextAndSpan.first.IsCancelled());
    }
  }
}
