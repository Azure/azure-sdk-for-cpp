
#include "azure/core-opentelemetry/opentelemetry.hpp"
#include <azure/core/nullable.hpp>
#include <azure/core/tracing/tracing.hpp>
#include <memory>
#ifdef _MSC_VER
// The OpenTelemetry headers generate a couple of warnings on MSVC in the OTel 1.2 package, suppress
// the warnings across the includes.
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4244)
#endif
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/tracer_provider.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
namespace Azure { namespace Core { namespace Tracing { namespace OpenTelemetry {

  OpenTelemetryProvider::OpenTelemetryProvider(
      opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider)
      : m_tracerProvider(tracerProvider)
  {
  }

  OpenTelemetryProvider::OpenTelemetryProvider()
      : m_tracerProvider(opentelemetry::trace::Provider::GetTracerProvider())
  {
  }

  std::shared_ptr<Azure::Core::Tracing::Tracer> OpenTelemetryProvider::CreateTracer(
      std::string const& name,
      std::string const& version,
      Azure::Nullable<Azure::Core::Url> const& schema_url) const
  {
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> returnTracer;
    if (schema_url)
    {
      returnTracer
          = m_tracerProvider->GetTracer(name, version, schema_url.Value().GetAbsoluteUrl());
    }
    else
    {
      returnTracer = m_tracerProvider->GetTracer(name, version);
    }
    return std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryTracer>(returnTracer);
  }

  OpenTelemetryTracer::OpenTelemetryTracer(
      opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer)
      : m_tracer(tracer)
  {
  }

  std::shared_ptr<Azure::Core::Tracing::Span> OpenTelemetryTracer::CreateSpan(
      std::string const& spanName,
      CreateSpanOptions const& options = {}) const
  {
    opentelemetry::trace::StartSpanOptions spanOptions;
    spanOptions.kind = opentelemetry::trace::SpanKind::kInternal;
    if (options.Kind == Azure::Core::Tracing::SpanKind::Client)
    {
      spanOptions.kind = opentelemetry::trace::SpanKind::kClient;
    }
    else if (options.Kind == Azure::Core::Tracing::SpanKind::Consumer)
    {
      spanOptions.kind = opentelemetry::trace::SpanKind::kConsumer;
    }
    else if (options.Kind == Azure::Core::Tracing::SpanKind::Producer)
    {
      spanOptions.kind = opentelemetry::trace::SpanKind::kProducer;
    }
    else if (options.Kind == Azure::Core::Tracing::SpanKind::Server)
    {
      spanOptions.kind = opentelemetry::trace::SpanKind::kServer;
    }
    else if (options.Kind == Azure::Core::Tracing::SpanKind::Internal)
    {
      spanOptions.kind = opentelemetry::trace::SpanKind::kInternal;
    }
    else
    {
      throw std::runtime_error("Unknown SpanOptions Kind: " + options.Kind.ToString());
    }

    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> newSpan;
    if (options.Attributes)
    {
      // Note: We make a huge assumption here: That if you're calling into the OpenTelemetry version
      // of Azure::Core::Tracing, the Attributes passed in will be an OpenTelemetryAttributeSet
      Azure::Core::Tracing::OpenTelemetry::OpenTelemetryAttributeSet* attributes
          = static_cast<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryAttributeSet*>(
              options.Attributes.get());
      newSpan = m_tracer->StartSpan(spanName, *attributes, spanOptions);
    }
    else
    {
      newSpan = m_tracer->StartSpan(spanName, spanOptions);
    }

    return std::make_shared<Azure::Core::Tracing::OpenTelemetry::OpenTelemetrySpan>(
        newSpan, m_tracer);
  }

  OpenTelemetrySpan::~OpenTelemetrySpan() {}

  OpenTelemetrySpan::OpenTelemetrySpan(
      opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span,
      opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> const& tracer)
      : m_scope(tracer->WithActiveSpan(span)), m_span(span)
  {
  }

  void OpenTelemetrySpan::End(Azure::Nullable<Azure::DateTime> endTime)
  {
    opentelemetry::trace::EndSpanOptions options;
    if (endTime)
    {
      options.end_steady_time = opentelemetry::common::SteadyTimestamp(
          std::chrono::steady_clock::time_point(endTime.Value().time_since_epoch()));
    }
    m_span->End(options);
  }

  /**
   * @brief Add the set of attributes provided to the current span.
   */
  void OpenTelemetrySpan::AddAttributes(AttributeSet const& attributesToAdd)
  {
    // Note: We make a huge assumption here: That if you're calling into the OpenTelemetry version
    // of Azure::Core::Tracing, the Attributes passed in will be an OpenTelemetryAttributeSet
    Azure::Core::Tracing::OpenTelemetry::OpenTelemetryAttributeSet const& attributes
        = static_cast<Azure::Core::Tracing::OpenTelemetry::OpenTelemetryAttributeSet const&>(
            attributesToAdd);
    attributes.ForEachKeyValue(
        [this](
            opentelemetry::nostd::string_view name, opentelemetry::common::AttributeValue value) {
          m_span->SetAttribute(name, value);
          return true;
        });
  }

  /**
   * Add an Event to the span. An event is identified by a name and an optional set of attributes
   * associated with the event.
   */
  void OpenTelemetrySpan::AddEvent(
      std::string const& /* eventName*/,
      AttributeSet const& /* eventAttributes*/)
  {
    throw std::runtime_error("Not implemented");
  }

  void OpenTelemetrySpan::AddEvent(std::string const& /* eventName*/)
  {
    throw std::runtime_error("Not implemented");
  }

  /**
   * @brief Records an exception.
   *
   * @note This might be better as std::runtime_error instead of std::exception. To be discussed.
   */
  void OpenTelemetrySpan::RecordException(std::exception const& /* exceptionToRecord*/)
  {
    throw std::runtime_error("Not implemented");
  }

  void OpenTelemetrySpan::RecordException(
      std::exception const& /* exceptionToRecord*/,
      AttributeSet const& /* eventAttributes*/)
  {
    throw std::runtime_error("Not implemented");
  }

  void OpenTelemetrySpan::SetStatus(SpanStatus const& status, std::string const& statusMessage)
  {
    opentelemetry::trace::StatusCode statusCode = opentelemetry::trace::StatusCode::kUnset;
    if (status == Azure::Core::Tracing::SpanStatus::Error)
    {
      statusCode = opentelemetry::trace::StatusCode::kError;
    }
    else if (status == Azure::Core::Tracing::SpanStatus::Ok)
    {
      statusCode = opentelemetry::trace::StatusCode::kOk;
    }
    else if (status == Azure::Core::Tracing::SpanStatus::Unset)
    {
      statusCode = opentelemetry::trace::StatusCode::kUnset;
    }
    else
    {
      throw std::runtime_error("Unknown status code: " + status.ToString());
    }

    m_span->SetStatus(statusCode, statusMessage);
  }

}}}} // namespace Azure::Core::Tracing::OpenTelemetry
