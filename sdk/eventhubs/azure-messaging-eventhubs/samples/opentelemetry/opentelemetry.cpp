// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/diagnostics/logger.hpp>
#include <azure/core/tracing/opentelemetry/opentelemetry.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include <opentelemetry/exporters/otlp/otlp_http_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter_options.h>
#include <opentelemetry/exporters/otlp/otlp_http_log_record_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_log_record_exporter_options.h>
#include <opentelemetry/sdk/logs/batch_log_record_processor.h>
#include <opentelemetry/sdk/logs/logger_provider.h>
#include <opentelemetry/sdk/resource/resource.h>
#include <opentelemetry/sdk/trace/batch_span_processor.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>

namespace {
namespace otel = opentelemetry;
using AzureLogger = Azure::Core::Diagnostics::Logger;

// Azure's numeric levels do not match OpenTelemetry severity numbers.
otel::logs::Severity MapSeverity(AzureLogger::Level level)
{
  switch (level)
  {
    case AzureLogger::Level::Verbose:
      return otel::logs::Severity::kDebug;
    case AzureLogger::Level::Informational:
      return otel::logs::Severity::kInfo;
    case AzureLogger::Level::Warning:
      return otel::logs::Severity::kWarn;
    case AzureLogger::Level::Error:
      return otel::logs::Severity::kError;
  }
  return otel::logs::Severity::kInfo;
}

class Telemetry final {
private:
  std::shared_ptr<otel::sdk::trace::TracerProvider> m_traces;
  std::shared_ptr<otel::sdk::logs::LoggerProvider> m_logs;
  bool m_stopped = false;

public:
  Telemetry()
  {
    auto resource = otel::sdk::resource::Resource::Create(
        {{"service.name", "eventhubs-opentelemetry-sample"}});

    // Options read OTEL_EXPORTER_OTLP_* environment variables. The default transport is
    // OTLP/HTTP with binary protobuf, using /v1/traces and /v1/logs on localhost:4318.
    otel::exporter::otlp::OtlpHttpExporterOptions traceOptions;
    traceOptions.content_type = otel::exporter::otlp::HttpRequestContentType::kBinary;
    auto traceExporter = otel::exporter::otlp::OtlpHttpExporterFactory::Create(traceOptions);
    auto traceProcessor = std::make_unique<otel::sdk::trace::BatchSpanProcessor>(
        std::move(traceExporter), otel::sdk::trace::BatchSpanProcessorOptions{});
    m_traces
        = std::make_shared<otel::sdk::trace::TracerProvider>(std::move(traceProcessor), resource);

    otel::exporter::otlp::OtlpHttpLogRecordExporterOptions logOptions;
    logOptions.content_type = otel::exporter::otlp::HttpRequestContentType::kBinary;
    auto logExporter = otel::exporter::otlp::OtlpHttpLogRecordExporterFactory::Create(logOptions);
    auto logProcessor = std::make_unique<otel::sdk::logs::BatchLogRecordProcessor>(
        std::move(logExporter), otel::sdk::logs::BatchLogRecordProcessorOptions{});
    m_logs = std::make_shared<otel::sdk::logs::LoggerProvider>(std::move(logProcessor), resource);
    auto logger = m_logs->GetLogger("azure-sdk", "eventhubs-opentelemetry-sample", "1.0.0");

    // The listener is process-wide. Capture an owning pointer, never a local reference.
    // Batch processing queues records; this callback does not perform network export.
    AzureLogger::SetListener([logger](AzureLogger::Level level, std::string const& message) {
      auto record = logger->CreateLogRecord();
      if (record)
      {
        record->SetTimestamp(std::chrono::system_clock::now());
        record->SetSeverity(MapSeverity(level));
        record->SetBody(message);
        logger->EmitLogRecord(std::move(record));
      }
      // Do not call Azure SDK APIs or log through Azure Logger here: it can recurse/deadlock.
    });
    // Verbose includes the producer-close record used by the no-Azure smoke check.
    // Prefer Informational or Warning for normal production use.
    AzureLogger::SetLevel(AzureLogger::Level::Verbose);
  }

  std::shared_ptr<Azure::Core::Tracing::TracerProvider> AzureProvider() const
  {
    return Azure::Core::Tracing::OpenTelemetry::OpenTelemetryProvider::Create(
        std::static_pointer_cast<otel::trace::TracerProvider>(m_traces));
  }

  void SmokeSpan() const
  {
    auto tracer = m_traces->GetTracer("eventhubs-opentelemetry-sample", "1.0.0");
    auto span = tracer->StartSpan("sample.exporter-check");
    span->SetAttribute("sample.mode", "no-azure");
    span->End();
  }

  bool Shutdown()
  {
    if (m_stopped)
    {
      return true;
    }
    m_stopped = true;
    // Call only after clients and their background work have stopped. Unregistering waits
    // for active Azure Logger callbacks before we flush and shut down the providers.
    AzureLogger::SetListener(nullptr);
    auto const timeout = std::chrono::seconds(5);
    bool const tracesFlushed = m_traces->ForceFlush(timeout);
    bool const logsFlushed = m_logs->ForceFlush(timeout);
    bool const tracesStopped = m_traces->Shutdown(timeout);
    bool const logsStopped = m_logs->Shutdown(timeout);
    return tracesFlushed && logsFlushed && tracesStopped && logsStopped;
  }

  ~Telemetry() { Shutdown(); }
};

std::string RequiredEnvironment(char const* name)
{
  auto value = std::getenv(name);
  if (value == nullptr || *value == '\0')
  {
    throw std::runtime_error(std::string("Set environment variable ") + name);
  }
  return value;
}
} // namespace

int main(int argc, char** argv)
{
  bool const send = argc == 2 && std::string(argv[1]) == "--send";
  if (argc > 2 || (argc == 2 && !send && std::string(argv[1]) != "--smoke"))
  {
    std::cerr << "Usage: eventhubs-opentelemetry [--smoke|--send]\n";
    return 1;
  }
  try
  {
    Telemetry telemetry;
    {
      Azure::Messaging::EventHubs::ProducerClientOptions options;
      options.TracingProvider = telemetry.AzureProvider();
      // Constructing a credential/client does not authenticate or connect. --smoke never sends.
      auto credential = std::make_shared<Azure::Identity::AzureCliCredential>();
      Azure::Messaging::EventHubs::ProducerClient producer(
          send ? RequiredEnvironment("EVENTHUBS_HOST") : "unused.invalid",
          send ? RequiredEnvironment("EVENTHUB_NAME") : "sample-hub",
          credential,
          options);
      if (send)
      {
        auto context = Azure::Core::Context{}.WithDeadline(
            std::chrono::system_clock::now() + std::chrono::seconds(30));
        producer.Send(
            Azure::Messaging::EventHubs::Models::EventData{"Hello OpenTelemetry!"}, context);
      }
      else
      {
        telemetry.SmokeSpan();
      }
      // The producer destructor emits a real Azure SDK close log, including in smoke mode.
    }
    if (!telemetry.Shutdown())
    {
      std::cerr << "Telemetry flush or shutdown did not complete successfully.\n";
      return 1;
    }
    std::cout << "Finished. Check the collector for traces and logs.\n";
    return 0;
  }
  catch (std::exception const& ex)
  {
    std::cerr << ex.what() << '\n';
    return 1;
  }
}
