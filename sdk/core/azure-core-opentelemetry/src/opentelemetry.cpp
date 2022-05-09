
#include "azure/core-opentelemetry/opentelemetry.hpp"
#include <azure/core/nullable.hpp>
#include <azure/core/tracing/tracing.hpp>
#include <memory>
#if defined(_MSC_VER)
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
  using namespace Azure::Core::Tracing::_internal;

  OpenTelemetryProvider::OpenTelemetryProvider(
      opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider)
      : m_tracerProvider(tracerProvider)
  {
  }

  OpenTelemetryProvider::OpenTelemetryProvider()
      : m_tracerProvider(opentelemetry::trace::Provider::GetTracerProvider())
  {
  }

  std::shared_ptr<Azure::Core::Tracing::_internal::Tracer> OpenTelemetryProvider::CreateTracer(
      std::string const& name,
      std::string const& version) const
  {
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> returnTracer(
        m_tracerProvider->GetTracer(name, version));
    return std::make_shared<Azure::Core::Tracing::OpenTelemetry::_internal::OpenTelemetryTracer>(
        returnTracer);
  }
  namespace _internal {
    std::unique_ptr<Azure::Core::Tracing::_internal::AttributeSet>
    OpenTelemetryTracer::CreateAttributeSet() const
    {
      return std::make_unique<OpenTelemetryAttributeSet>();
    }

    OpenTelemetryTracer::OpenTelemetryTracer(
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer)
        : m_tracer(tracer)
    {
    }

    std::shared_ptr<Azure::Core::Tracing::_internal::Span> OpenTelemetryTracer::CreateSpan(
        std::string const& spanName,
        Azure::Core::Tracing::_internal::CreateSpanOptions const& options = {}) const
    {
      opentelemetry::trace::StartSpanOptions spanOptions;
      spanOptions.kind = opentelemetry::trace::SpanKind::kInternal;
      if (options.Kind == Azure::Core::Tracing::_internal::SpanKind::Client)
      {
        spanOptions.kind = opentelemetry::trace::SpanKind::kClient;
      }
      else if (options.Kind == SpanKind::Consumer)
      {
        spanOptions.kind = opentelemetry::trace::SpanKind::kConsumer;
      }
      else if (options.Kind == SpanKind::Producer)
      {
        spanOptions.kind = opentelemetry::trace::SpanKind::kProducer;
      }
      else if (options.Kind == SpanKind::Server)
      {
        spanOptions.kind = opentelemetry::trace::SpanKind::kServer;
      }
      else if (options.Kind == SpanKind::Internal)
      {
        spanOptions.kind = opentelemetry::trace::SpanKind::kInternal;
      }
      else
      {
        throw std::runtime_error("Unknown SpanOptions Kind: " + options.Kind.ToString());
      }

      if (options.ParentSpan)
      {
        spanOptions.parent
            = static_cast<OpenTelemetrySpan*>(options.ParentSpan.get())->GetContext();
      }

      opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> newSpan;
      if (options.Attributes)
      {
        // Note: We make a huge assumption here: That if you're calling into the OpenTelemetry
        // version of Azure::Core::Tracing, the Attributes passed in will be an
        // OpenTelemetryAttributeSet
        OpenTelemetryAttributeSet* attributes
            = static_cast<OpenTelemetryAttributeSet*>(options.Attributes.get());
        newSpan = m_tracer->StartSpan(spanName, *attributes, spanOptions);
      }
      else
      {
        newSpan = m_tracer->StartSpan(spanName, spanOptions);
      }

      return std::make_shared<Azure::Core::Tracing::OpenTelemetry::_internal::OpenTelemetrySpan>(
          newSpan);
    }

    OpenTelemetrySpan::~OpenTelemetrySpan() {}

    OpenTelemetrySpan::OpenTelemetrySpan(
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span)
        : m_span(span)
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
      // Note: We make a huge assumption here: That if you're calling into the OpenTelemetry
      // version of Azure::Core::Tracing, the Attributes passed in will be an
      // OpenTelemetryAttributeSet
      OpenTelemetryAttributeSet const& attributes
          = static_cast<OpenTelemetryAttributeSet const&>(attributesToAdd);
      attributes.ForEachKeyValue(
          [this](
              opentelemetry::nostd::string_view name, opentelemetry::common::AttributeValue value) {
            m_span->SetAttribute(name, value);
            return true;
          });
    }

    /**
     * Add an Event to the span. An event is identified by a name and an optional set of
     * attributes associated with the event.
     */
    void OpenTelemetrySpan::AddEvent(
        std::string const& eventName,
        AttributeSet const& eventAttributes)
    {
      OpenTelemetryAttributeSet const& attributes
          = static_cast<OpenTelemetryAttributeSet const&>(eventAttributes);

      m_span->AddEvent(eventName, attributes);
    }

    void OpenTelemetrySpan::AddEvent(std::string const& eventName) { m_span->AddEvent(eventName); }

    void OpenTelemetrySpan::AddEvent(std::exception const& ex) { m_span->AddEvent(ex.what()); }

    void OpenTelemetrySpan::SetStatus(SpanStatus const& status, std::string const& statusMessage)
    {
      opentelemetry::trace::StatusCode statusCode = opentelemetry::trace::StatusCode::kUnset;
      if (status == SpanStatus::Error)
      {
        statusCode = opentelemetry::trace::StatusCode::kError;
      }
      else if (status == SpanStatus::Ok)
      {
        statusCode = opentelemetry::trace::StatusCode::kOk;
      }
      else if (status == SpanStatus::Unset)
      {
        statusCode = opentelemetry::trace::StatusCode::kUnset;
      }
      else
      {
        throw std::runtime_error("Unknown status code: " + status.ToString());
      }

      m_span->SetStatus(statusCode, statusMessage);
    }

  } // namespace _internal
}}}} // namespace Azure::Core::Tracing::OpenTelemetry
