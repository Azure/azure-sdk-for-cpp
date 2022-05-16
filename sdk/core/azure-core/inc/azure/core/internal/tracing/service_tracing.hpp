// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/context.hpp"
#include "azure/core/internal/client_options.hpp"
#include "azure/core/tracing/tracing.hpp"

#pragma once

/**
 *
 * @brief Helper classes to enable service client distributed tracing implementations.
 * @remark See #policy.hpp
 */

namespace Azure { namespace Core { namespace Tracing { namespace _internal {

  /**
   * @brief RAII Helper class for Azure::Core::Tracing::Span objects.
   *
   * @details The ServiceSpan object is an RAII helper object used to manage Span objects.
   *
   * Before a Span is registered with OpenTelemetry, the span needs to have called the
   * "Azure::Core::Tracing::Span::End" method. The ServiceSpan method wraps an
   * Azure::Core::Tracing::Span object and ensures that the "End" method is called in the destructor
   * for the span.
   */
  class ServiceSpan final : public Span {
  private:
    std::shared_ptr<Span> m_span;

    friend class ServiceTracing;
    explicit ServiceSpan(std::shared_ptr<Span>& span) : m_span(span) {}

    ServiceSpan() = default;

    ServiceSpan(const ServiceSpan&) = delete;
    ServiceSpan& operator=(ServiceSpan const&) = delete;

    ServiceSpan& operator=(ServiceSpan&&) noexcept = default;

  public:
    ServiceSpan(ServiceSpan&& that) = default;

    ~ServiceSpan()
    {
      if (m_span)
      {
        m_span->End();
      }
    }

    void End(Azure::Nullable<Azure::DateTime> = Azure::Nullable<Azure::DateTime>{}) override
    {
      if (m_span)
      {
        m_span->End();
      }
    }
    void SetStatus(
        Azure::Core::Tracing::_internal::SpanStatus const& status,
        std::string const& description = "") override
    {
      if (m_span)
      {
        m_span->SetStatus(status, description);
      }
    }
    /**
     * @brief Adds a set of attributes to the span.
     *
     * @param attributeToAdd Attributes to be added to the span.
     */
    virtual void AddAttributes(AttributeSet const& attributeToAdd) override
    {
      if (m_span)
      {
        m_span->AddAttributes(attributeToAdd);
      }
    }

    /**
     * @brief Adds an event to the span.
     *
     * Add an Event to the span. An event is identified by a name and an optional set of attributes
     * associated with the event.
     *
     * @param eventName Name of the event to add.
     * @param eventAttributes Attributes associated with the event.
     */
    virtual void AddEvent(std::string const& eventName, AttributeSet const& eventAttributes)
        override
    {
      if (m_span)
      {
        m_span->AddEvent(eventName, eventAttributes);
      }
    }

    /**
     * @brief Adds an event to the span.
     *
     * Add an Event to the span. An event is identified by a name
     *
     * @param eventName Name of the event to add.
     */
    virtual void AddEvent(std::string const& eventName) override
    {
      if (m_span)
      {
        m_span->AddEvent(eventName);
      }
    }

    /**
     * @brief Records an exception occurring in the span.
     *
     * @param exception Exception which has occurred.
     */
    virtual void AddEvent(std::exception const& exception) override
    {
      if (m_span)
      {
        m_span->AddEvent(exception);
      }
    }
  };

  /**
   * @brief Helper class to enable distributed tracing for the service.
   *
   * @details Each service implementation SHOULD have a member variable which aids in managing the
   * distributed tracing for the service.
   */
  class ServiceTracing final {
  private:
    std::string m_serviceName;
    std::string m_serviceVersion;
    std::shared_ptr<Azure::Core::Tracing::_internal::Tracer> m_serviceTracer;
    Azure::Core::Context::Key SpanKey;

  public:
    ServiceTracing(
        Azure::Core::_internal::ClientOptions const& options,
        std::string serviceName,
        std::string serviceVersion)
        : m_serviceName(serviceName), m_serviceVersion(serviceVersion),
          m_serviceTracer(
              options.Telemetry.TracingProvider
                  ? options.Telemetry.TracingProvider->CreateTracer(serviceName, serviceVersion)
                  : nullptr)
    {
    }

    ServiceTracing() = default;

    std::pair<Azure::Core::Context, ServiceSpan> CreateSpan(
        std::string const& spanName,
        Azure::Core::Context const& clientContext);

    std::unique_ptr<Azure::Core::Tracing::_internal::AttributeSet> CreateAttributeSet();
  };

  class TracingAttributes
      : public Azure::Core::_internal::ExtendableEnumeration<TracingAttributes> {
  public:
    explicit TracingAttributes(std::string const& that) : ExtendableEnumeration(that) {}

    /**
     * @brief Represents an "Internal" operation.
     *
     */
    AZ_CORE_DLLEXPORT const static TracingAttributes AzNamespace;
  };

}}}} // namespace Azure::Core::Tracing::_internal