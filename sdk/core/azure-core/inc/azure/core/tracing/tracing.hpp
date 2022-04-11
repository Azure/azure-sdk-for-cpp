// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Handling log messages from Azure SDK.
 */

#pragma once

#include "azure/core/internal/extendable_enumeration.hpp"
#include "azure/core/nullable.hpp"
#include "azure/core/url.hpp"
#include <array>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Tracing {

  /** An attribute applied to a Span, Event, or TraceContext.
   */
  struct Attribute final
  {
  };
  /**
   */
  struct Event final
  {
    std::string Name;
    Azure::DateTime Timestamp;
    std::vector<Attribute> Attributes;
  };

  class SpanKind final : public Azure::Core::_internal::ExtendableEnumeration<SpanKind> {
  public:
    explicit SpanKind(std::string const& kind) : ExtendableEnumeration(kind) {}
    SpanKind() = default;

    AZ_CORE_DLLEXPORT const static SpanKind Internal;
    AZ_CORE_DLLEXPORT const static SpanKind Client;
    AZ_CORE_DLLEXPORT const static SpanKind Server;
    AZ_CORE_DLLEXPORT const static SpanKind Producer;
    AZ_CORE_DLLEXPORT const static SpanKind Consumer;
  };

  class SpanStatus final : public Azure::Core::_internal::ExtendableEnumeration<SpanStatus> {

  public:
    explicit SpanStatus(SpanStatus const& status) : ExtendableEnumeration(status) {}
    SpanStatus() = default;

    AZ_CORE_DLLEXPORT const static SpanStatus Unset;
    AZ_CORE_DLLEXPORT const static SpanStatus Ok;
    AZ_CORE_DLLEXPORT const static SpanStatus Error;
  };

  class SpanContext final {
  public:
    std::array<uint8_t, 16> m_TraceId;
    std::array<uint8_t, 8> m_SpanId;

    virtual std::array<uint8_t, 16> TraceIdBinary() const = 0;
    virtual std::array<uint8_t, 8> SpanIdBinary() const = 0;
    virtual std::array<char, 32> TraceIdHex() const = 0;
    virtual std::array<char, 16> SpanIdHex() const = 0;
    virtual bool IsValid() const = 0;
    virtual bool IsRemote() const = 0;
  };

  class SpanLink {
  public:
    std::unique_ptr<SpanContext> LinkedSpan;
    std::vector<Attribute> LinkAttributes;
  };

  /**
   * @brief Span - represents a span in tracing.
   */
  class Span final {
    Azure::Core::Context::Key m_contextKey;
    std::unique_ptr<SpanContext> m_spanContext;
    std::vector<Event> m_events;
    SpanStatus m_status{SpanStatus::Unset};
    std::string m_name;
    std::vector<Attribute> m_spanAttributes;
    bool m_isRecording{true};

  public:
    /**
     * @brief Retrieves the Span associated with the specified Context.
     *
     * @note: This may not be the correct implementation because of the pluggable trace provider
     * requirement.
     */
    static std::shared_ptr<Span> FromContext(Azure::Core::Context const& context);

    /**
     * @brief Inserts the current Span into the specified context.
     */
    void InsertIntoContext(Azure::Core::Context& context);

    /**
     * @brief Signals that the span has now ended.
     */
    void End(Azure::Nullable<Azure::DateTime> endTime);

    /**
     * @brief Returns the SpanContext associated with this span.
     */
    std::unique_ptr<SpanContext> const& GetContext() const { return m_spanContext; }

    bool IsRecording() { return m_isRecording; }

    void AddAttribute(Attribute& attributeToAdd);
    void AddAttributes(std::vector<Attribute>& attributeToAdd);

    /**
     * Add an Event to the span. An event is identified by a name and an optional set of attributes
     * associated with the event.
     */
    void AddEvent(std::string const& eventName, std::vector<Attribute> eventAttributes);
    void AddEvent(std::string const& eventName, Attribute eventAttributes);
    void AddEvent(std::string const& eventName);
    void AddEvent(Event& eventToAdd) { m_events.push_back(eventToAdd); }
    std::vector<Event> const& GetEvents() const { return m_events; }

    /**
     * @brief Records an exception.
     *
     * @note This might be better as std::runtime_error instead of std::exception. To be discussed.
     */
    void RecordException(std::exception const& exceptionToRecord);
    void RecordException(std::exception const& exceptionToRecord, Attribute eventAttributes);
    void RecordException(
        std::exception const& exceptionToRecord,
        std::vector<Attribute> eventAttributes);

    void SetStatus(SpanStatus const& status);
    SpanStatus const& GetStatus() const { return m_status; };

    void UpdateName(std::string const& newName) { m_name = newName; }

    Azure::DateTime StartTime;
    Azure::DateTime EndTime;
  };

  struct CreateSpanOptions final
  {
    Azure::Nullable<Azure::Core::Context> ParentContext;
    // Attributes
    // Links
    // Start Timestamp
  };

  /**
   * @brief Tracer - factory for creating span objects.
   *
   */
  class Tracer final {
  public:
    std::shared_ptr<Span> CreateSpan(
        std::string const& spanName,
        SpanKind const& spanKind,
        CreateSpanOptions const& options);
  };

  /**
   * @brief Trace Provider - factory for creating Tracer objects.
   */
  class TracerProvider final {
  public:
    virtual std::shared_ptr<Tracer> CreateTracer(
        std::string const& name,
        std::string const& version,
        Azure::Nullable<Azure::Core::Url> const& scheme_url = {})
        = 0;
  };
}}} // namespace Azure::Core::Tracing
