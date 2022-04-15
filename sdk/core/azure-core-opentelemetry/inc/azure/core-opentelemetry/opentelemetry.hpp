#pragma once

#include <azure/core/tracing/tracing.hpp>
#ifdef _MSC_VER
// The OpenTelemetry headers generate a couple of warnings on MSVC in the OTel 1.2 package, suppress
// the warnings across the includes.
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4244)
#include <opentelemetry/common/kv_properties.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/tracer.h>
#include <opentelemetry/trace/tracer_provider.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace Azure { namespace Core { namespace Tracing { namespace OpenTelemetry {

  class OpenTelemetryAttributeSet final : public Azure::Core::Tracing::AttributeSet,
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
  class OpenTelemetrySpan final : public Azure::Core::Tracing::Span {
    opentelemetry::trace::Scope m_scope;
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> m_span;

  public:
    OpenTelemetrySpan(
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::Span> span,
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> const& tracer);

    ~OpenTelemetrySpan();

    /**
     * @brief Signals that the span has now ended.
     */
    virtual void End(Azure::Nullable<Azure::DateTime> endTime) override;

    virtual void AddAttributes(AttributeSet const& attributeToAdd) override;

    /**
     * Add an Event to the span. An event is identified by a name and an optional set of attributes
     * associated with the event.
     */
    virtual void AddEvent(std::string const& eventName, AttributeSet const& eventAttributes)
        override;
    virtual void AddEvent(std::string const& eventName) override;

    /**
     * @brief Records an exception.
     *
     * @note This might be better as std::runtime_error instead of std::exception. To be discussed.
     */
    virtual void RecordException(std::exception const& exceptionToRecord) override;
    virtual void RecordException(
        std::exception const& exceptionToRecord,
        AttributeSet const& eventAttributes) override;

    virtual void SetStatus(SpanStatus const& status, std::string const& statusMessage) override;
  };

  class OpenTelemetryTracer final : public Azure::Core::Tracing::Tracer {
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> m_tracer;

  public:
    OpenTelemetryTracer(opentelemetry::nostd::shared_ptr<opentelemetry::trace::Tracer> tracer);
    std::shared_ptr<Azure::Core::Tracing::Span> CreateSpan(
        std::string const& spanName,
        CreateSpanOptions const& options) const override;
  };

  class OpenTelemetryProvider final : public Azure::Core::Tracing::TracerProvider {
    opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> m_tracerProvider;

  public:
    OpenTelemetryProvider(
        opentelemetry::nostd::shared_ptr<opentelemetry::trace::TracerProvider> tracerProvider);
    OpenTelemetryProvider();

    virtual std::shared_ptr<Azure::Core::Tracing::Tracer> CreateTracer(
        std::string const& name,
        std::string const& version = "",
        Azure::Nullable<Azure::Core::Url> const& scheme_url = {}) const override;
  };
}}}} // namespace Azure::Core::Tracing::OpenTelemetry