// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief Validates the Azure Core transport adapters with fault responses from server.
 *
 * @note This test requires the Http-fault-injector
 * (https://github.com/Azure/azure-sdk-tools/tree/main/tools/http-fault-injector) running. Follow
 * the instructions to install and run the server before running this test.
 *
 */

#define REQUESTS 100
#define WARMUP 100
#define ROUNDS 100

#include "argagg.hpp"
#include "eventhubs_stress_scenarios.hpp"
#include "opentelemetry/sdk/logs/simple_log_record_processor_factory.h"
#include "produceconsumeevents.hpp"

#include <cctype>
#include <iostream>
#include <algorithm>
#include <memory>

#include <opentelemetry/exporters/otlp/otlp_http_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_exporter_options.h>
#include <opentelemetry/exporters/otlp/otlp_http_log_record_exporter_factory.h>
#include <opentelemetry/exporters/otlp/otlp_http_log_record_exporter_options.h>
#include <opentelemetry/logs/provider.h>
#include <opentelemetry/sdk/common/global_log_handler.h>
#include <opentelemetry/sdk/logs/logger_context.h>
#include <opentelemetry/sdk/logs/logger_provider_factory.h>
#include <opentelemetry/sdk/trace/processor.h>
#include <opentelemetry/sdk/trace/simple_processor_factory.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/sdk/trace/tracer_provider_factory.h>
#include <opentelemetry/trace/provider.h>

namespace trace_sdk = opentelemetry::sdk::trace;
namespace trace = opentelemetry::trace;
namespace logs_sdk = opentelemetry::sdk::logs;
namespace logs = opentelemetry::logs;
namespace otlp = opentelemetry::exporter::otlp;
namespace internal_log = opentelemetry::sdk::common::internal_log;

opentelemetry::exporter::otlp::OtlpHttpExporterOptions opts;
void InitTracer()
{
  // Create OTLP exporter instance
  auto exporter = otlp::OtlpHttpExporterFactory::Create(opts);
  auto processor = trace_sdk::SimpleSpanProcessorFactory::Create(std::move(exporter));
  std::shared_ptr<opentelemetry::trace::TracerProvider> provider
      = trace_sdk::TracerProviderFactory::Create(std::move(processor));
  // Set the global trace provider
  trace::Provider::SetTracerProvider(provider);
}

opentelemetry::exporter::otlp::OtlpHttpLogRecordExporterOptions logger_opts;
void InitLogger()
{

  std::cout << "Using " << logger_opts.url << " to export log records." << std::endl;
  logger_opts.console_debug = false;
  // Create OTLP exporter instance
  auto exporter = otlp::OtlpHttpLogRecordExporterFactory::Create(logger_opts);
  auto processor = logs_sdk::SimpleLogRecordProcessorFactory::Create(std::move(exporter));
  std::shared_ptr<logs::LoggerProvider> provider
      = logs_sdk::LoggerProviderFactory::Create(std::move(processor));

  logs::Provider::SetLoggerProvider(provider);
}

void CleanupTracer()
{
  // We call ForceFlush to prevent to cancel running exportings, It's optional.
  opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> provider
      = trace::Provider::GetTracerProvider();
  if (provider)
  {
    static_cast<trace_sdk::TracerProvider*>(provider.get())->ForceFlush();
  }

  std::shared_ptr<opentelemetry::trace::TracerProvider> none;
  trace::Provider::SetTracerProvider(none);
}

void Usage(argagg::parser const& argparser)
{
  argagg::fmt_ostream fmt(std::cerr);
  fmt << "Usage azure-messaging-eventhubs-stress-test [options] " << std::endl << argparser;
}

bool CompareString(std::string lhs, std::string rhs)
{
  std::transform(lhs.begin(), lhs.end(), lhs.begin(), tolower);
  std::transform(rhs.begin(), rhs.end(), rhs.begin(), tolower);
  return lhs == rhs;
}

  int main(int argc, char** argv)
{
  try
  {
    std::vector<std::shared_ptr<EventHubsStressScenario>> scenarios{
        std::make_shared<ProduceConsumeEvents>(),
    };

    argagg::parser argparser{
        {{"scenario", {"-s", "--scenario"}, "Scenario to run", 1},
         {"warmups", {"-w", "--warmups"}, "Number of pre-measurement iterations to run", 1},
         {"rounds", {"-r", "--rounds"}, "Number of iterations iterations to run the test", 1},
         {"requests", {"-R", "--requests"}, "Number of requests per round", 1},
         {"help", {"-?", "--help"}, "This help message.", 0}}};

    argagg::parser_results args;
    try
    {
      args = argparser.parse(argc, argv);
    }
    catch (const std::exception& e)
    {
      std::cerr << "Exception thrown parsing command line: " << e.what();

      Usage(argparser);

      return -1;
    }

    if (args.has_option("help"))
    {
      Usage(argparser);
      return -1;
    }

    auto rounds = args["rounds"].as<int>(ROUNDS);
    auto warmup = args["warmups"].as<int>(WARMUP);
    auto scenarioName = args["scenario"].as<std::string>("produceconsumeevents");

    std::shared_ptr<EventHubsStressScenario> scenario;
    for (const auto& scenarioToCheck : scenarios)
    {
      if (CompareString(scenarioToCheck->GetStressScenarioName(), scenarioName))
      {
        scenario = scenarioToCheck;
        break;
      }
    }
    if (!scenario)
    {
      std::cerr << "Unknown scenario name " << scenarioName << "." << std::endl;
      std::cerr << "Known scenarios are:" << std::endl;
      for (const auto& scenarioToCheck : scenarios)
      {
        std::cerr << "    " << scenarioToCheck->GetStressScenarioName();
      }
      return -1;
    }

    std::cout << "Running stress scenario " << scenario->GetStressScenarioName() << std::endl;

    InitTracer();
    InitLogger();

    Azure::Core::Diagnostics::Logger::SetLevel(
        Azure::Core::Diagnostics::Logger::Level::Informational);
    Azure::Core::Diagnostics::Logger::SetListener(
        [](Azure::Core::Diagnostics::Logger::Level level, std::string const& message) {
          logs::Severity logSeverity{logs::Severity::kInvalid};
          switch (level)
          {
            case Azure::Core::Diagnostics::Logger::Level::Error:
              logSeverity = logs::Severity::kError;
              break;
            case Azure::Core::Diagnostics::Logger::Level::Informational:
              logSeverity = logs::Severity::kInfo;
              break;
            case Azure::Core::Diagnostics::Logger::Level::Verbose:
              logSeverity = logs::Severity::kDebug;
              break;
            case Azure::Core::Diagnostics::Logger::Level::Warning:
              logSeverity = logs::Severity::kWarn;
              break;
          }
          logs::Provider::GetLoggerProvider()
              ->GetLogger(EventHubsLoggerName)
              ->Log(logSeverity, message);
        });

    internal_log::GlobalLogHandler::SetLogLevel(internal_log::LogLevel::Error);

    auto tracer{trace::Provider::GetTracerProvider()->GetTracer(EventHubsLoggerName)};
    auto logger{opentelemetry::logs::Provider::GetLoggerProvider()->GetLogger(EventHubsLoggerName)};

    logger->Log(opentelemetry::logs::Severity::kDebug, "Starting test.");

    auto span{tracer->StartSpan("span 1")};
    auto scope{trace::Tracer::WithActiveSpan(span)};

    span->AddEvent("EventHubs Stress started.");

    std::cout << "--------------\tSTARTING TEST\t--------------" << std::endl;
    scenario->Initialize();
    std::cout << "--------------\tPRE WARMUP\t--------------" << std::endl;
    {
      trace::StartSpanOptions spanOptions;
      spanOptions.parent = span->GetContext();
      spanOptions.kind = trace::SpanKind::kClient;
      std::map<std::string, size_t> attributes{{"iterations", warmup}};
      auto warmupSpan{tracer->StartSpan("Warmup", attributes, spanOptions)};
      auto warmupScope{trace::Tracer::WithActiveSpan(warmupSpan)};

      warmupSpan->AddEvent("Begin Warmup.");
      scenario->Warmup(warmup);
      warmupSpan->AddEvent("End Warmup.");
      warmupSpan->End();
    }
    std::cout << "--------------\tPOST WARMUP\t--------------" << std::endl;

    for (int i = 0; i < rounds; i++)
    {
      std::cout << "--------------\tTEST ITERATION:" << i << "\t--------------" << std::endl;

      scenario->Run(REQUESTS);

      std::cout << "--------------\tDONE ITERATION:" << i << "\t--------------" << std::endl;
    }

    scenario->Cleanup();
  }
  catch (std::exception const& ex)
  {
    std::cerr << "Test failed due to exception thrown: " << ex.what() << std::endl;

  }
  CleanupTracer();
  return 0;
}
