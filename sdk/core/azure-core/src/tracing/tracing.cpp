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

  ContextAndSpanFactory::ContextAndSpan ContextAndSpanFactory::CreateSpan(
      std::string const& methodName,
      Azure::Core::Context const& context) const
  {
    if (m_serviceTracer)
    {
      Azure::Core::Context contextToUse = context;
      CreateSpanOptions createOptions;

      createOptions.Kind = SpanKind::Internal;
      createOptions.Attributes = m_serviceTracer->CreateAttributeSet();
      return CreateSpan(methodName, createOptions, context);
    }
    else
    {
      return std::make_pair(context, ServiceSpan{});
    }
  }

  ContextAndSpanFactory::ContextAndSpan ContextAndSpanFactory::CreateSpan(
      std::string const& methodName,
      Azure::Core::Tracing::_internal::CreateSpanOptions& createOptions,
      Azure::Core::Context const& context) const
  {
    if (m_serviceTracer)
    {
      Azure::Core::Context contextToUse = context;

      // Ensure that the factory is available in the context chain.
      ContextAndSpanFactory const* tracingFactoryFromContext;
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

      if (!createOptions.Attributes)
      {
        createOptions.Attributes = m_serviceTracer->CreateAttributeSet();
      }
      createOptions.Attributes->AddAttribute(
          TracingAttributes::AzNamespace.ToString(), m_serviceName);

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

  std::unique_ptr<ContextAndSpanFactory> ContextAndSpanFactory::ContextAndSpanFactoryFromContext(
      Azure::Core::Context const& context)
  {
    ContextAndSpanFactory const* factory;
    if (context.TryGetValue(TracingFactoryContextKey, factory))
    {
      return std::make_unique<ContextAndSpanFactory>(*factory);
    }
    else
    {
      return nullptr;
    }
  }

  std::unique_ptr<Azure::Core::Tracing::_internal::AttributeSet>
  ContextAndSpanFactory::CreateAttributeSet() const
  {
    if (m_serviceTracer)
    {
      return m_serviceTracer->CreateAttributeSet();
    }
    return nullptr;
  }

  Azure::Core::Context::Key ContextAndSpanFactory::ContextSpanKey;
  Azure::Core::Context::Key ContextAndSpanFactory::TracingFactoryContextKey;
}}}} // namespace Azure::Core::Tracing::_internal
