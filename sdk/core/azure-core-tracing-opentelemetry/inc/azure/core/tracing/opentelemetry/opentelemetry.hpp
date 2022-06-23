// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/internal/tracing/tracing_impl.hpp>
#include <azure/core/tracing/tracing.hpp>
#if defined(_MSC_VER)
// The OpenTelemetry headers generate a couple of warnings on MSVC in the OTel 1.2 package, suppress
// the warnings across the includes.
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4244)
#pragma warning(disable : 6323) // Disable "Use of arithmetic operator on Boolean type" warning.
#endif
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/tracer_provider.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace Azure { namespace Core { namespace Tracing { namespace OpenTelemetry {

  /**
   * @brief Trace Provider - factory for creating Tracer objects.
   *
   * An OpenTelemetryProvider object wraps an opentelemetry-cpp TracerProvider object
   * and provides an abstraction of the opentelemetry APIs which can be consumed by Azure Core and
   * other Azure services.
   *
   */
  class OpenTelemetryProvider : public Azure::Core::Tracing::TracerProvider {
  private:
    std::shared_ptr<Azure::Core::Tracing::_internal::Tracer> CreateTracer(
        std::string const& name,
        std::string const& version) const override;

    opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> m_tracerProvider;

    explicit OpenTelemetryProvider(
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider
        = opentelemetry::trace::Provider::GetTracerProvider());

  public:
    static std::shared_ptr<OpenTelemetryProvider> Create(
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider
        = opentelemetry::trace::Provider::GetTracerProvider());

    virtual ~OpenTelemetryProvider() = default;
  };

}}}} // namespace Azure::Core::Tracing::OpenTelemetry
