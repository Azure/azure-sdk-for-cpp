// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/eventhubs_tracing.hpp"

#include "private/package_version.hpp"

#include <azure/core/internal/client_options.hpp>

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {

  Azure::Core::Tracing::_internal::TracingContextFactory CreateTracingContextFactory(
      std::shared_ptr<Azure::Core::Tracing::TracerProvider> const& tracingProvider)
  {
    Azure::Core::_internal::ClientOptions clientOptions;
    clientOptions.Telemetry.TracingProvider = tracingProvider;
    return Azure::Core::Tracing::_internal::TracingContextFactory{
        clientOptions,
        "Microsoft.EventHub",
        "azure-messaging-eventhubs-cpp",
        PackageVersion::ToString()};
  }

  Azure::Core::Tracing::_internal::TracingContextFactory::TracingContext StartSpan(
      Azure::Core::Tracing::_internal::TracingContextFactory const& tracingFactory,
      std::string const& spanName,
      Azure::Core::Tracing::_internal::SpanKind spanKind,
      std::string const& operationName,
      std::string const& eventHubName,
      std::string const& fullyQualifiedNamespace,
      Azure::Nullable<size_t> messageCount,
      Azure::Core::Context const& context)
  {
    Azure::Core::Tracing::_internal::CreateSpanOptions createOptions;
    createOptions.Kind = spanKind;

    // The az.namespace attribute comes from the tracing context factory.
    auto tracingContext = tracingFactory.CreateTracingContext(spanName, createOptions, context);
    tracingContext.Span.AddAttribute("messaging.system", "eventhubs");
    tracingContext.Span.AddAttribute("messaging.destination.name", eventHubName);
    tracingContext.Span.AddAttribute("messaging.operation", operationName);
    tracingContext.Span.AddAttribute(
        Azure::Core::Tracing::_internal::TracingAttributes::NetPeerName.ToString(),
        fullyQualifiedNamespace);
    if (messageCount.HasValue())
    {
      SetMessageCount(tracingContext.Span, messageCount.Value());
    }
    return tracingContext;
  }

  void SetMessageCount(Azure::Core::Tracing::_internal::ServiceSpan& span, size_t messageCount)
  {
    // ServiceSpan has only a string AddAttribute overload, so the count goes in as text.
    span.AddAttribute("messaging.batch.message_count", std::to_string(messageCount));
  }

}}}} // namespace Azure::Messaging::EventHubs::_detail
