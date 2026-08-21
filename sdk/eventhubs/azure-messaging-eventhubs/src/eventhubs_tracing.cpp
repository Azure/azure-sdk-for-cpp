// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "private/eventhubs_tracing.hpp"

#include "private/package_version.hpp"

#include <azure/core/internal/client_options.hpp>

#include <cstdint>

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
      Azure::Core::Context const& context,
      MessagingEntityKind entityKind)
  {
    Azure::Core::Tracing::_internal::CreateSpanOptions createOptions;
    createOptions.Kind = spanKind;

    // The az.namespace attribute comes from the tracing context factory.
    auto tracingContext = tracingFactory.CreateTracingContext(spanName, createOptions, context);
    // A factory with no tracer drops every attribute, so skip the string temporaries.
    if (!tracingFactory.HasTracer())
    {
      return tracingContext;
    }
    tracingContext.Span.AddAttribute("messaging.system", "eventhubs");
    tracingContext.Span.AddAttribute(
        entityKind == MessagingEntityKind::Source ? "messaging.source.name"
                                                  : "messaging.destination.name",
        eventHubName);
    tracingContext.Span.AddAttribute("messaging.operation", operationName);
    tracingContext.Span.AddAttribute(
        Azure::Core::Tracing::_internal::TracingAttributes::NetPeerName.ToString(),
        fullyQualifiedNamespace);
    if (messageCount.HasValue())
    {
      SetMessageCount(tracingFactory, tracingContext.Span, messageCount.Value());
    }
    return tracingContext;
  }

  Azure::Core::Tracing::_internal::TracingContextFactory::TracingContext StartAmqpSpan(
      Azure::Core::Tracing::_internal::TracingContextFactory const& tracingFactory,
      std::string const& spanName,
      std::string const& operationName,
      std::string const& eventHubName,
      std::string const& fullyQualifiedNamespace,
      Azure::Nullable<size_t> messageCount,
      AmqpDiagnosticsContext const& diagnosticsContext,
      Azure::Nullable<std::uint64_t> retryAttempt,
      Azure::Core::Context const& context,
      MessagingEntityKind entityKind)
  {
    auto tracingContext = StartSpan(
        tracingFactory,
        spanName,
        Azure::Core::Tracing::_internal::SpanKind::Client,
        operationName,
        eventHubName,
        fullyQualifiedNamespace,
        messageCount,
        context,
        entityKind);
    if (!tracingFactory.HasTracer())
    {
      return tracingContext;
    }

    auto attributes = tracingFactory.CreateAttributeSet();
    if (!attributes)
    {
      return tracingContext;
    }
    attributes->AddAttribute("az.eventhubs.client.id", diagnosticsContext.ClientId);
    attributes->AddAttribute(
        "az.eventhubs.partition.id",
        diagnosticsContext.PartitionId.empty() ? "<gateway>" : diagnosticsContext.PartitionId);
    attributes->AddAttribute("az.eventhubs.amqp.component.type", diagnosticsContext.ComponentType);
    attributes->AddAttribute(
        "az.eventhubs.amqp.component.name",
        diagnosticsContext.ComponentName.empty() ? "<unnamed>" : diagnosticsContext.ComponentName);
    attributes->AddAttribute(
        "az.eventhubs.amqp.component.id", GetAmqpComponentIdentifier(diagnosticsContext));
    attributes->AddAttribute(
        "az.eventhubs.amqp.component.generation", diagnosticsContext.ComponentGeneration);
    if (retryAttempt.HasValue())
    {
      attributes->AddAttribute("az.eventhubs.retry.attempt", retryAttempt.Value());
    }
    tracingContext.Span.AddAttributes(*attributes);
    return tracingContext;
  }

  void SetMessageCount(
      Azure::Core::Tracing::_internal::TracingContextFactory const& tracingFactory,
      Azure::Core::Tracing::_internal::ServiceSpan& span,
      size_t messageCount)
  {
    // A factory with no tracer returns a null attribute set, and that pointer is not null safe.
    auto attributeSet = tracingFactory.CreateAttributeSet();
    if (!attributeSet)
    {
      return;
    }
    attributeSet->AddAttribute(
        "messaging.batch.message_count", static_cast<uint64_t>(messageCount));
    span.AddAttributes(*attributeSet);
  }

}}}} // namespace Azure::Messaging::EventHubs::_detail
