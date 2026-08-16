// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Distributed tracing helpers shared by the Event Hubs clients.
#pragma once

#include <azure/core/context.hpp>
#include <azure/core/internal/tracing/service_tracing.hpp>
#include <azure/core/nullable.hpp>

#include <memory>
#include <string>

namespace Azure { namespace Messaging { namespace EventHubs { namespace _detail {

  // Creates the tracing context factory of a client. The provider can be null.
  Azure::Core::Tracing::_internal::TracingContextFactory CreateTracingContextFactory(
      std::shared_ptr<Azure::Core::Tracing::TracerProvider> const& tracingProvider);

  // Starts a span for an operation and adds the messaging attributes. The message count is
  // optional, because a receiver knows it only on return.
  Azure::Core::Tracing::_internal::TracingContextFactory::TracingContext StartSpan(
      Azure::Core::Tracing::_internal::TracingContextFactory const& tracingFactory,
      std::string const& spanName,
      Azure::Core::Tracing::_internal::SpanKind spanKind,
      std::string const& operationName,
      std::string const& eventHubName,
      std::string const& fullyQualifiedNamespace,
      Azure::Nullable<size_t> messageCount,
      Azure::Core::Context const& context);

  // Adds the message count attribute to a span. The factory supplies the attribute set that
  // carries the count as an unsigned integer.
  void SetMessageCount(
      Azure::Core::Tracing::_internal::TracingContextFactory const& tracingFactory,
      Azure::Core::Tracing::_internal::ServiceSpan& span,
      size_t messageCount);

}}}} // namespace Azure::Messaging::EventHubs::_detail
