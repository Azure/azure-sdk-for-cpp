// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "azure/core/context.hpp"
#include "azure/core/internal/client_options.hpp"
#include "azure/core/tracing/tracing.hpp"

#pragma once

/**
 *
 * @brief Helper classes to enable service client distributed tracing implementations.
 *
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

    friend class DiagnosticTracingFactory;
    ServiceSpan() = default;
    explicit ServiceSpan(std::shared_ptr<Span> span) : m_span(span) {}

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
     * @brief Adds a single attributes to the span.
     *
     * @param attributeName Name of the attribute to be added.
     * @param attributeValue Value of the attribute to be added.
     */
    virtual void AddAttribute(std::string const& attributeName, std::string const& attributeValue)
        override
    {
      if (m_span)
      {
        m_span->AddAttribute(attributeName, attributeValue);
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

    /**
     * @brief Propogate information from the current span to the HTTP request headers.
     *
     * @param request HTTP Request to the service. If there is an active tracing span, this will
     * add required headers to the HTTP Request.
     */
    virtual void PropagateToHttpHeaders(Azure::Core::Http::Request& request) override
    {
      if (m_span)
      {
        m_span->PropagateToHttpHeaders(request);
      }
    }
  };

  /**
   * @brief Helper class to enable distributed tracing for the service.
   *
   * @details Each service implementation SHOULD have a member variable which aids in managing
   * the distributed tracing for the service.
   */
  class DiagnosticTracingFactory final {
  private:
    std::string m_serviceName;
    std::string m_serviceVersion;
    std::shared_ptr<Azure::Core::Tracing::_internal::Tracer> m_serviceTracer;

    /** @brief The key used to retrieve the span and tracer associated with a context object.
     *
     *  The value stored in the context with this key is a `std::pair<std::shared_ptr<Span>,
     * std::shared_ptr<Tracer>>`.
     *
     * A caller can use the Span and Tracer to create a new span associated with the current
     * context span.
     */
    static Azure::Core::Context::Key ContextSpanKey;
    static Azure::Core::Context::Key TracingFactoryContextKey;
    //    using TracingContext = std::pair<std::shared_ptr<Span>, std::shared_ptr<Tracer>>;
    using TracingContext = std::shared_ptr<Span>;

  public:
    DiagnosticTracingFactory(
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

    DiagnosticTracingFactory() = default;
    DiagnosticTracingFactory(DiagnosticTracingFactory const&) = default;

    /** @brief A ContextAndSpan provides an updated Context object and a new span object
     * which can be used to add events and attributes to the span.
     */
    using ContextAndSpan = std::pair<Azure::Core::Context, ServiceSpan>;

    ContextAndSpan CreateSpan(
        std::string const& spanName,
        Azure::Core::Tracing::_internal::SpanKind const& spanKind,
        Azure::Core::Context const& clientContext);

    ContextAndSpan CreateSpan(
        std::string const& spanName,
        Azure::Core::Tracing::_internal::CreateSpanOptions& spanOptions,
        Azure::Core::Context const& clientContext);

    std::unique_ptr<Azure::Core::Tracing::_internal::AttributeSet> CreateAttributeSet();

    static std::unique_ptr<DiagnosticTracingFactory> DiagnosticFactoryFromContext(
        Azure::Core::Context const& context);
  };

  /**
   * @brief Attributes emitted as a part of distributed tracing spans.
   *
   * List taken from here:
   * https://github.com/Azure/azure-sdk/blob/main/docs/tracing/distributed-tracing-conventions.yml
   *
   */
  class TracingAttributes
      : public Azure::Core::_internal::ExtendableEnumeration<TracingAttributes> {
  public:
    explicit TracingAttributes(std::string const& that) : ExtendableEnumeration(that) {}

    /**
     * @brief
     * [Namespace](https://docs.microsoft.com/azure/azure-resource-manager/management/azure-services-resource-providers)
     * of Azure service request is made against.
     *
     */
    AZ_CORE_DLLEXPORT const static TracingAttributes AzNamespace;

    /**
     * @brief HTTP request method.
     *
     */
    AZ_CORE_DLLEXPORT const static TracingAttributes HttpMethod;

    /**
     * @brief Full HTTP request URL in the form `scheme://host[:port]/path?query[#fragment]`.
     *
     */
    AZ_CORE_DLLEXPORT const static TracingAttributes HttpUrl;

    /**
     * @brief [HTTP response status code](https://tools.ietf.org/html/rfc7231#section-6).
     *
     */
    AZ_CORE_DLLEXPORT const static TracingAttributes HttpStatusCode;

    /**
     * @brief Value of the [HTTP User-Agent](https://tools.ietf.org/html/rfc7231#section-5.5.3)
     * header sent by the client.
     *
     */
    AZ_CORE_DLLEXPORT const static TracingAttributes HttpUserAgent;

    /** @brief  Value of the[x - ms - client - request - id] header(or other request - id header,
     * depending on the service) sent by the client.
     */
    AZ_CORE_DLLEXPORT const static TracingAttributes RequestId;

    /** @brief Value of the [x-ms-request-id]  header (or other request-id header, depending on the
     * service) sent by the server in response.
     */
    AZ_CORE_DLLEXPORT const static TracingAttributes ServiceRequestId;
  };

}}}} // namespace Azure::Core::Tracing::_internal
