// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/tracing/tracing.hpp>

#if defined(_azure_APIVIEW)
#include "azure/core/tracing/opentelemetry/internal/apiview.hpp"
#else
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
  class OpenTelemetryProvider final : public Azure::Core::Tracing::TracerProvider {
  private:
    std::shared_ptr<Azure::Core::Tracing::_internal::Tracer> CreateTracer(
        std::string const& name,
        std::string const& version) const override;

    opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> m_tracerProvider;

    explicit OpenTelemetryProvider(
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider
        = opentelemetry::trace::Provider::GetTracerProvider());

    // Schema URL for OpenTelemetry. Azure SDKs currently support version 1.17.0 only.
    const char* OpenTelemetrySchemaUrl117 = "https://opentelemetry.io/schemas/1.17.0";
    const char* OpenTelemetrySchemaUrlCurrent = OpenTelemetrySchemaUrl117;

  public:
    /**
     * @brief Create a new instance of an OpenTelemetryProvider.
     *
     * @param tracerProvider opentelemetry-cpp TracerProvider object.
     *
     * @returns a new OpenTelemetryProvider object
     */
    static std::shared_ptr<OpenTelemetryProvider> Create(
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider
        = opentelemetry::trace::Provider::GetTracerProvider());

    virtual ~OpenTelemetryProvider() = default;
  };

}}}} // namespace Azure::Core::Tracing::OpenTelemetry
