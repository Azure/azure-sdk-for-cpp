// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief Stress framework for EventHubs service client.
 *
 */

#include "argagg.hpp"
#include "batch_stress_tests.hpp"
#include "eventhubs_stress_scenarios.hpp"
#include "opentelemetry/sdk/logs/simple_log_record_processor_factory.h"

#include <azure/core/datetime.hpp>
#include <azure/core/platform.hpp>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>

#if defined(AZURE_PLATFORM_WINDOWS)
#include <winsock.h>
#else // Assume POSIX
#include <unistd.h>
#endif

#if defined(_MSC_VER)
// The OpenTelemetry headers generate a couple of warnings on MSVC in the OTel 1.2 package, suppress
// the warnings across the includes.
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4244)
#pragma warning(disable : 6323) // Disable "Use of arithmetic operator on Boolean type" warning.
#endif

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

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace trace_sdk = opentelemetry::sdk::trace;
namespace trace = opentelemetry::trace;
namespace logs_sdk = opentelemetry::sdk::logs;
namespace logs = opentelemetry::logs;
namespace otlp = opentelemetry::exporter::otlp;
namespace internal_log = opentelemetry::sdk::common::internal_log;

auto GetTraceResource(std::string const& stressScenarioName)
{
  char hostname[256];
  if (gethostname(hostname, sizeof(hostname)))
  {
#if _MSC_VER
    char errorBuffer[512];
    strerror_s(errorBuffer, sizeof(errorBuffer), errno);
    throw std::runtime_error("Failed to get hostname." + std::string(errorBuffer));

#else
    throw std::runtime_error("Failed to get hostname." + std::string(strerror(errno)));
#endif
  }

  auto resource_attributes = opentelemetry::sdk::resource::ResourceAttributes{
      {"service.name", stressScenarioName},
      {"service.instance.id", hostname},
  };
  return opentelemetry::sdk::resource::Resource::Create(resource_attributes);
}

void InitTracer(const std::string& stressScenarioName)
{
  opentelemetry::exporter::otlp::OtlpHttpExporterOptions opts;

  // Create OTLP exporter instance
  auto exporter = otlp::OtlpHttpExporterFactory::Create(opts);
  auto processor = trace_sdk::SimpleSpanProcessorFactory::Create(std::move(exporter));

  auto resource{GetTraceResource(stressScenarioName)};

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
  // https://github.com/Azure/azure-sdk-for-cpp/issues/5784

  std::shared_ptr<opentelemetry::trace::TracerProvider> provider
      = trace_sdk::TracerProviderFactory::Create(std::move(processor), std::move(resource));
#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

  // Set the global trace provider
  trace::Provider::SetTracerProvider(provider);
}

// On debug builds, we log to the console. On release builds, we log to OpenTelemetry.
#if defined(_DEBUG)
constexpr const bool LogDefault = true;
#else
constexpr const bool LogDefault = false;
#endif

bool LogToConsole{LogDefault};

// Log level textual representation, including space padding, matches slf4j and log4net.
constexpr const char* ErrorText = "ERROR";
constexpr const char* WarningText = "WARN ";
constexpr const char* InformationalText = "INFO ";
constexpr const char* VerboseText = "DEBUG";
constexpr const char* UnknownText = "?????";

constexpr const char* LogLevelToConsoleString(Azure::Core::Diagnostics::Logger::Level logLevel)
{
  switch (logLevel)
  {
    case Azure::Core::Diagnostics::Logger::Level::Error:
      return ErrorText;

    case Azure::Core::Diagnostics::Logger::Level::Warning:
      return WarningText;

    case Azure::Core::Diagnostics::Logger::Level::Informational:
      return InformationalText;

    case Azure::Core::Diagnostics::Logger::Level::Verbose:
      return VerboseText;

    default:
      return UnknownText;
  }
}

void InitLogger(const std::string& stressScenarioName)
{
  if (LogToConsole)
  {
    std::cout << "Using console to export log records." << std::endl;

    // Integrate the azure logging diagnostics with the OpenTelemetry logger provider we just
    // created.
    Azure::Core::Diagnostics::Logger::SetListener(
        [](Azure::Core::Diagnostics::Logger::Level level, std::string const& message) {
          std::cerr << '['
                    << Azure::DateTime(std::chrono::system_clock::now())
                           .ToString(
                               Azure::DateTime::DateFormat::Rfc3339,
                               Azure::DateTime::TimeFractionFormat::AllDigits)
                    << " T: " << std::this_thread::get_id() << "] "
                    << LogLevelToConsoleString(level) << " : " << message;

          // If the message ends with a new line, flush the stream otherwise insert a new line to
          // terminate the message.
          //
          // If the client of the logger APIs is using the stream form of the logger, then it will
          // insert a \n character when the client uses std::endl. This check ensures that we don't
          // insert unnecessary new lines.
          if (!message.empty() && message.back() == '\n')
          {
            std::cerr << std::flush;
          }
          else
          {
            std::cerr << std::endl;
          }
        });
  }
  else
  {
    opentelemetry::exporter::otlp::OtlpHttpLogRecordExporterOptions logger_opts;

    std::cout << "Using " << logger_opts.url << " to export log records." << std::endl;
    logger_opts.console_debug = false;

    // Create OTLP exporter instance
    auto exporter = otlp::OtlpHttpLogRecordExporterFactory::Create(logger_opts);
    auto processor = logs_sdk::SimpleLogRecordProcessorFactory::Create(std::move(exporter));

    auto resource{GetTraceResource(stressScenarioName)};

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#elif defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    // https://github.com/Azure/azure-sdk-for-cpp/issues/5784

    std::shared_ptr<logs::LoggerProvider> provider
        = logs_sdk::LoggerProviderFactory::Create(std::move(processor), std::move(resource));
#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

    // Set the global log provider.
    logs::Provider::SetLoggerProvider(provider);

    // Integrate the azure logging diagnostics with the OpenTelemetry logger provider we just
    // created.
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
  }
}

void CleanupTracer()
{
  // We call ForceFlush to prevent to cancel running exports, It's optional.
  opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> provider
      = trace::Provider::GetTracerProvider();
  if (provider)
  {
    static_cast<trace_sdk::TracerProvider*>(provider.get())->ForceFlush();
  }

  std::shared_ptr<opentelemetry::trace::TracerProvider> none;
  trace::Provider::SetTracerProvider(none);
}

void Usage(
    argagg::parser const& argparser,
    const std::vector<std::shared_ptr<EventHubsStressScenario>>& scenarios)
{
  argagg::fmt_ostream fmt(std::cerr);
  fmt << "Usage azure-messaging-eventhubs-stress-test [options] " << std::endl << argparser;

  fmt << std::endl;
  fmt << "Scenario Options:" << std::endl;
  for (const auto& scenario : scenarios)
  {
    fmt << "Scenario: " << scenario->GetStressScenarioName() << std::endl;
    for (const auto& option : scenario->GetScenarioOptions())
    {
      fmt << "    ";
      for (auto& arg : option.Activators)
      {
        fmt << arg;
        if (arg != option.Activators.back())
        {
          fmt << ", ";
        }
      }
      fmt << "\n        " << option.HelpMessage << '\n';
    }
  }
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
        std::make_shared<BatchStressTest>(),
    };

    // Determine the stress scenario to run.
    // Parse the command line in "positional only" mode. The first argument is the scenario name.
    std::shared_ptr<EventHubsStressScenario> scenario;
    {
      argagg::parser argparser;

      argagg::parser_results args;
      try
      {
        args = argparser.parse(argc, argv, true);
      }
      catch (const std::exception& e)
      {
        std::cerr << "Exception thrown parsing scenario command line: " << e.what() << std::endl;

        Usage(argparser, scenarios);

        return -1;
      }

      std::string scenarioName;
      if (args.pos.size() > 0)
      {
        scenarioName = args.pos[0];
      }
      else
      {
        std::cerr << "No scenario name provided." << std::endl;
        Usage(argparser, scenarios);
        return -1;
      }

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

        Usage(argparser, scenarios);
        return -1;
      }
    }

    std::cout << "Running stress scenario " << scenario->GetStressScenarioName() << std::endl;

    // Now we know the scenario, reparse the command line using the scenario specific options. We
    // also support
    auto scenarioOptions{scenario->GetScenarioOptions()};
    argagg::parser argparser{{
        {"console", {"--console"}, "Log output traces to console", 0},
        {"help", {"-?", "-h", "--help"}, "This help message.", 0},
        {"verbose", {"-v", "--verbose"}, "Enable verbose logging", 0},
    }};

    // Add the scenario specific options to the parser.
    for (const auto& option : scenarioOptions)
    {
      argparser.definitions.push_back(
          {option.Name, option.Activators, option.HelpMessage, option.ExpectedArgs});
    }

    // Re-parse the command line with this scenario's options.
    argagg::parser_results args;
    try
    {
      args = argparser.parse(argc, argv);
    }
    catch (const std::exception& e)
    {
      std::cerr << "Exception thrown parsing command line: " << e.what() << std::endl;

      Usage(argparser, scenarios);

      return -1;
    }

    // Log to the console or to OpenTelemetry logs.
    LogToConsole = args["console"] ? args["console"] : LogDefault;

    if (args.has_option("help"))
    {
      Usage(argparser, scenarios);
      return 0;
    }

    // Initialize OpenTelemetry Tracers and Loggers.
    // TBD: Metrics.
    InitTracer(scenario->GetStressScenarioName());
    InitLogger(scenario->GetStressScenarioName());

    if (args.has_option("verbose"))
    {
      std::cerr << "Verbose logging enabled." << std::endl;
      Azure::Core::Diagnostics::Logger::SetLevel(Azure::Core::Diagnostics::Logger::Level::Verbose);
    }
    else
    {
      Azure::Core::Diagnostics::Logger::SetLevel(
          Azure::Core::Diagnostics::Logger::Level::Informational);
    }

    std::cout << "===\tINITIALIZE TEST\t===" << std::endl;
    scenario->Initialize(args);

    std::cout << "===\tRUN TEST\t===" << std::endl;
    scenario->Run();

    std::cout << "===\tCLEANUP TEST\t===" << std::endl;
    scenario->Cleanup();
  }
  catch (std::exception const& ex)
  {
    std::cerr << "Test failed due to exception thrown: " << ex.what() << std::endl;
  }
  CleanupTracer();
  return 0;
}
