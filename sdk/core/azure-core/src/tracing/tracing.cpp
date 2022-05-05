#include "azure/core/tracing/tracing.hpp"
#include "azure/core/internal/tracing/service_tracing.hpp"

namespace Azure { namespace Core { namespace Tracing {

  const SpanKind SpanKind::Internal("Internal");
  const SpanKind SpanKind::Client("Client");
  const SpanKind SpanKind::Consumer("Consumer");
  const SpanKind SpanKind::Producer("Producer");
  const SpanKind SpanKind::Server("Server");

  const SpanStatus SpanStatus::Unset("Unset");
  const SpanStatus SpanStatus::Ok("Ok");
  const SpanStatus SpanStatus::Error("Error");

}}} // namespace Azure::Core::Tracing

namespace Azure { namespace Core { namespace Tracing { namespace _internal {

  const TracingAttributes TracingAttributes::AzNamespace("az.namespace");

  std::pair<Azure::Core::Context, ServiceSpan> ServiceTracing::CreateSpan(
      std::string const& methodName,
      Azure::Core::Context const& context)
  {
    if (m_serviceTracer)
    {
      Azure::Core::Tracing::CreateSpanOptions createOptions;
      // Find a span in the context hierarchy.
      if (!context.TryGetValue(SpanKey, createOptions.ParentSpan))
      {
        // Please note: Not specifically needed, but make sure that this is a root level
        // span if there is no parent span in the context
        createOptions.ParentSpan = nullptr;
      }
      createOptions.Attributes = m_serviceTracer->CreateAttributeSet();
      createOptions.Attributes->AddAttribute(
          TracingAttributes::AzNamespace.ToString(), m_serviceName);

      std::shared_ptr<Azure::Core::Tracing::Span> newSpan(
          m_serviceTracer->CreateSpan(methodName, createOptions));
      Azure::Core::Context newContext = context.WithValue(SpanKey, newSpan);
      ServiceSpan newServiceSpan(newSpan);
      return std::make_pair<Azure::Core::Context, ServiceSpan>(
          std::move(newContext), std::move(newServiceSpan));
    }
    else
    {
      return std::make_pair(context, ServiceSpan());
    }
  }

}}}} // namespace Azure::Core::Tracing::_internal