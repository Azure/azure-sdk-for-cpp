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

  TracingContextFactory::TracingContext TracingContextFactory::CreateTracingContext(
      std::string const& methodName,
      Azure::Core::Context const& context) const
  {
    if (m_serviceTracer)
    {
      Azure::Core::Context contextToUse = context;
      CreateSpanOptions createOptions;

      createOptions.Kind = SpanKind::Internal;
      createOptions.Attributes = m_serviceTracer->CreateAttributeSet();
      return CreateTracingContext(methodName, createOptions, context);
    }
    else
    {
      return TracingContext{context, ServiceSpan{}};
    }
  }

  TracingContextFactory::TracingContext TracingContextFactory::CreateTracingContext(
      std::string const& methodName,
      Azure::Core::Tracing::_internal::CreateSpanOptions& createOptions,
      Azure::Core::Context const& context) const
  {
    if (m_serviceTracer)
    {
      Azure::Core::Context contextToUse = context;

      // Ensure that the factory is available in the context chain.
      TracingContextFactory const* tracingFactoryFromContext;
      if (!context.TryGetValue(TracingFactoryContextKey, tracingFactoryFromContext))
      {
        contextToUse = context.WithValue(TracingFactoryContextKey, this);
      }

      std::shared_ptr<Span> traceContext;
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

      if (!createOptions.Attributes)
      {
        createOptions.Attributes = m_serviceTracer->CreateAttributeSet();
      }
      createOptions.Attributes->AddAttribute(
          TracingAttributes::AzNamespace.ToString(), m_serviceName);

      std::shared_ptr<Span> newSpan(m_serviceTracer->CreateSpan(methodName, createOptions));
      Azure::Core::Context newContext = contextToUse.WithValue(ContextSpanKey, newSpan);
      ServiceSpan newServiceSpan(newSpan);
      return TracingContext{std::move(newContext), std::move(newServiceSpan)};
    }
    else
    {
      return TracingContext{context, ServiceSpan{}};
    }
  }

  std::unique_ptr<TracingContextFactory> TracingContextFactory::CreateFromContext(
      Azure::Core::Context const& context)
  {
    TracingContextFactory const* factory;
    if (context.TryGetValue(TracingFactoryContextKey, factory))
    {
      return std::make_unique<TracingContextFactory>(*factory);
    }
    else
    {
      return nullptr;
    }
  }

  std::unique_ptr<Azure::Core::Tracing::_internal::AttributeSet>
  TracingContextFactory::CreateAttributeSet() const
  {
    if (m_serviceTracer)
    {
      return m_serviceTracer->CreateAttributeSet();
    }
    return nullptr;
  }

  Azure::Core::Context::Key TracingContextFactory::ContextSpanKey;
  Azure::Core::Context::Key TracingContextFactory::TracingFactoryContextKey;
}}}} // namespace Azure::Core::Tracing::_internal
