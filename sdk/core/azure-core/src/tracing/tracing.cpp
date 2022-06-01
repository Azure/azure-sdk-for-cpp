#include "azure/core/tracing/tracing.hpp"
#include "azure/core/internal/tracing/service_tracing.hpp"

namespace Azure { namespace Core { namespace Tracing { namespace _internal {

  const SpanKind SpanKind::Internal("Internal");
  const SpanKind SpanKind::Client("Client");
  const SpanKind SpanKind::Consumer("Consumer");
  const SpanKind SpanKind::Producer("Producer");
  const SpanKind SpanKind::Server("Server");

  const SpanStatus SpanStatus::Unset("Unset");
  const SpanStatus SpanStatus::Ok("Ok");
  const SpanStatus SpanStatus::Error("Error");

  const TracingAttributes TracingAttributes::AzNamespace("az.namespace");
  const TracingAttributes TracingAttributes::ServiceRequestId("serviceRequestId");
  const TracingAttributes TracingAttributes::HttpUserAgent("http.user_agent");
  const TracingAttributes TracingAttributes::HttpMethod("http.method");
  const TracingAttributes TracingAttributes::HttpUrl("http.url");
  const TracingAttributes TracingAttributes::RequestId("requestId");
  const TracingAttributes TracingAttributes::HttpStatusCode("http.status_code");

  DiagnosticTracingFactory::ContextAndSpan DiagnosticTracingFactory::CreateSpan(
      std::string const& methodName,
      Azure::Core::Tracing::_internal::SpanKind const& spanKind,
      Azure::Core::Context const& context)
  {
    CreateSpanOptions createOptions;
    if (m_serviceTracer)
    {
      Azure::Core::Context contextToUse = context;

      // Ensure that the factory is available in the context chain.
      DiagnosticTracingFactory* tracingFactoryFromContext;
      if (!context.TryGetValue(TracingFactoryContextKey, tracingFactoryFromContext))
      {
        contextToUse = context.WithValue(TracingFactoryContextKey, this);
      }

      TracingContext traceContext;
      // Find a span in the context hierarchy.
      if (contextToUse.TryGetValue(ContextSpanKey, traceContext))
      {
        createOptions.ParentSpan = traceContext;
      }
      else
      {
        // Please note: Not specifically needed, but make sure that this is a root level
        // span if there is no parent span in the context
        createOptions.ParentSpan = nullptr;
      }
      createOptions.Attributes = m_serviceTracer->CreateAttributeSet();
      createOptions.Attributes->AddAttribute(
          TracingAttributes::AzNamespace.ToString(), m_serviceName);

      createOptions.Kind = spanKind;

      std::shared_ptr<Span> newSpan(m_serviceTracer->CreateSpan(methodName, createOptions));
      TracingContext tracingContext = newSpan;
      Azure::Core::Context newContext = contextToUse.WithValue(ContextSpanKey, tracingContext);
      ServiceSpan newServiceSpan(newSpan);
      return std::make_pair<Azure::Core::Context, ServiceSpan>(
          std::move(newContext), std::move(newServiceSpan));
    }
    else
    {
      return std::make_pair(context, ServiceSpan{});
    }
  }
  DiagnosticTracingFactory::ContextAndSpan DiagnosticTracingFactory::CreateSpanFromContext(
      std::string const& spanName,
      Azure::Core::Tracing::_internal::SpanKind const& spanKind,
      Azure::Core::Context const& context)
  {
    DiagnosticTracingFactory* tracingFactory
        = DiagnosticTracingFactory::DiagnosticFactoryFromContext(context);
    if (tracingFactory)
    {
      return tracingFactory->CreateSpan(spanName, spanKind, context);
    }
    else
    {
      return std::make_pair(context, ServiceSpan{});
    }
  }

  Azure::Nullable<DiagnosticTracingFactory::TracingContext>
  DiagnosticTracingFactory::TracingContextFromContext(Azure::Core::Context const& context)
  {
    TracingContext traceContext;
    if (context.TryGetValue(ContextSpanKey, traceContext))
    {
      return traceContext;
    }
    else
    {
      return Azure::Nullable<TracingContext>{};
    }
  }

  DiagnosticTracingFactory* DiagnosticTracingFactory::DiagnosticFactoryFromContext(
      Azure::Core::Context const& context)
  {
    DiagnosticTracingFactory* factory;
    if (context.TryGetValue(TracingFactoryContextKey, factory))
    {
      return factory;
    }
    else
    {
      return nullptr;
    }
  }

  std::unique_ptr<Azure::Core::Tracing::_internal::AttributeSet>
  DiagnosticTracingFactory::CreateAttributeSet()
  {
    return m_serviceTracer->CreateAttributeSet();
  }

  Azure::Core::Context::Key DiagnosticTracingFactory::ContextSpanKey;
  Azure::Core::Context::Key DiagnosticTracingFactory::TracingFactoryContextKey;

}}}} // namespace Azure::Core::Tracing::_internal
