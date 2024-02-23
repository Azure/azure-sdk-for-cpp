// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "opentelemetry_helpers.hpp"

#include "eventhubs_stress_scenarios.hpp"

opentelemetry::nostd::shared_ptr<opentelemetry::logs::Logger> GetLogger()
{
  auto logger{opentelemetry::logs::Provider::GetLoggerProvider()->GetLogger(EventHubsLoggerName)};
  return logger;
}

opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> GetTracer()
{
  return opentelemetry::trace::Provider::GetTracerProvider()->GetTracer(EventHubsLoggerName);
}

std::pair<opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span>, opentelemetry::trace::Scope>
CreateStressSpan(std::string const& name)
{
  auto tracer = GetTracer();
  opentelemetry::trace::StartSpanOptions options;
  options.parent = tracer->GetCurrentSpan()->GetContext();
  options.kind = opentelemetry::trace::SpanKind::kClient;
  auto newSpan = tracer->StartSpan(name, options);

  auto scope{tracer->WithActiveSpan(newSpan)};

  return std::make_pair<
      opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span>,
      opentelemetry::trace::Scope>(std::move(newSpan), std::move(scope));
}
