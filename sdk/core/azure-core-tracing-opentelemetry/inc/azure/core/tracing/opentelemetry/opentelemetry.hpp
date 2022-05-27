// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <azure/core/tracing/tracing.hpp>

#if defined(_MSC_VER)
// The OpenTelemetry headers generate a couple of warnings on MSVC in the OTel 1.2 package, suppress
// the warnings across the includes.
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4244)
#pragma warning(disable : 6323) // Disable "Use of arithmetic operator on Boolean type" warning.
#endif
#include <opentelemetry/common/kv_properties.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/tracer.h>
#include <opentelemetry/trace/tracer_provider.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace Azure { namespace Core { namespace Tracing { namespace OpenTelemetry {

  namespace _detail {
    class OpenTelemetryAttributeSet final : public Azure::Core::Tracing::_internal::AttributeSet,
                                            public opentelemetry::common::KeyValueIterable {
      std::map<std::string, opentelemetry::common::AttributeValue> m_propertySet;

      template <typename T> void AddAttributeToSet(std::string const& attributeName, T value)
      {
        m_propertySet.emplace(
            std::make_pair(attributeName, opentelemetry::common::AttributeValue(value)));
      }

    public:
      void AddAttribute(std::string const& attributeName, int32_t value) override
      {
        AddAttributeToSet(attributeName, value);
      }

      void AddAttribute(std::string const& attributeName, int64_t value) override
      {
        AddAttributeToSet(attributeName, value);
      }

      void AddAttribute(std::string const& attributeName, uint64_t value) override
      {
        AddAttributeToSet(attributeName, value);
      }
      void AddAttribute(std::string const& attributeName, double value) override
      {
        AddAttributeToSet(attributeName, value);
      }

      void AddAttribute(std::string const& attributeName, std::string const& value) override
      {
        AddAttributeToSet<std::string const&>(attributeName, value);
      }
      void AddAttribute(std::string const& attributeName, const char* value) override
      {
        AddAttributeToSet(attributeName, value);
      }

      void AddAttribute(std::string const& attributeName, bool value) override
      {
        AddAttributeToSet(attributeName, value);
      }

      /**
       * Iterate over key-value pairs
       * @param callback a callback to invoke for each key-value. If the callback returns false,
       * the iteration is aborted.
       * @return true if every key-value pair was iterated over
       */
      bool ForEachKeyValue(
          opentelemetry::nostd::function_ref<
              bool(opentelemetry::nostd::string_view, opentelemetry::common::AttributeValue)>
              callback) const noexcept override
      {
        for (auto& value : m_propertySet)
        {
          if (!callback(value.first, value.second))
          {
            return false;
          }
        }
        return true;
      }

      /**
       * @return the number of key-value pairs
       */
      size_t size() const noexcept override { return m_propertySet.size(); }

      ~OpenTelemetryAttributeSet() {}
    };
    /**
     * @brief Span - represents a span in tracing.
     */
    class OpenTelemetrySpan final : public Azure::Core::Tracing::_internal::Span {
      opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> m_span;

    public:
      OpenTelemetrySpan(opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span);

      ~OpenTelemetrySpan();

      /**
       * @brief Signals that the span has now ended.
       */
      virtual void End(Azure::Nullable<Azure::DateTime> endTime) override;

      virtual void AddAttributes(
          Azure::Core::Tracing::_internal::AttributeSet const& attributeToAdd) override;
      virtual void AddAttribute(std::string const& attributeName, std::string const& attributeValue)
          override;

      /**
       * Add an Event to the span. An event is identified by a name and an optional set of
       * attributes associated with the event.
       */
      virtual void AddEvent(
          std::string const& eventName,
          Azure::Core::Tracing::_internal::AttributeSet const& eventAttributes) override;
      virtual void AddEvent(std::string const& eventName) override;
      virtual void AddEvent(std::exception const& exception) override;

      virtual void SetStatus(
          Azure::Core::Tracing::_internal::SpanStatus const& status,
          std::string const& statusMessage) override;

      /**
       * @brief Propogate information from the current span to the HTTP request headers.
       *
       * @param request HTTP Request to the service. If there is an active tracing span, this will
       * add required headers to the HTTP Request.
       */
      virtual void PropagateToHttpHeaders(Azure::Core::Http::Request& request) override;

      opentelemetry::trace::SpanContext GetContext() { return m_span->GetContext(); }
    };

    class OpenTelemetryTracer final : public Azure::Core::Tracing::_internal::Tracer {
      opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> m_tracer;

    public:
      OpenTelemetryTracer(opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer);
      std::shared_ptr<Azure::Core::Tracing::_internal::Span> CreateSpan(
          std::string const& spanName,
          Azure::Core::Tracing::_internal::CreateSpanOptions const& options) const override;

      std::unique_ptr<Azure::Core::Tracing::_internal::AttributeSet> CreateAttributeSet()
          const override;
    };
  } // namespace _detail

  /**
   * @brief Trace Provider - factory for creating Tracer objects.
   *
   * An OpenTelemetryProvider object wraps an opentelemetry-cpp TracerProvider object
   * and provides an abstraction of the opentelemetry APIs which can be consumed by Azure Core and
   * other Azure services.
   *
   */
  class OpenTelemetryProvider final : public Azure::Core::Tracing::TracerProvider {
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> m_tracerProvider;

  public:
    OpenTelemetryProvider(
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider);
    OpenTelemetryProvider();

    /**
     * @brief Create a Tracer object
     *
     * @param name Name of the tracer object, typically the name of the Service client
     * (Azure.Storage.Blobs, for example)
     * @param version Version of the service client.
     * @return std::shared_ptr<Azure::Core::Tracing::Tracer>
     */
    virtual std::shared_ptr<Azure::Core::Tracing::_internal::Tracer> CreateTracer(
        std::string const& name,
        std::string const& version = "") const override;
  };
}}}} // namespace Azure::Core::Tracing::OpenTelemetry
