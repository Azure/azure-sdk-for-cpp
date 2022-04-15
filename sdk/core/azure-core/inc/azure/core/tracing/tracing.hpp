// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Handling log messages from Azure SDK.
 */

#pragma once

#include "azure/core/context.hpp"
#include "azure/core/datetime.hpp"
#include "azure/core/internal/extendable_enumeration.hpp"
#include "azure/core/nullable.hpp"
#include "azure/core/url.hpp"
#include <array>
#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Core { namespace Tracing {

  /** The set of attributes to be applied to a Span.
   *
   * @details
   * OpenTelemetry property bags can hold:
   *    - bool
   *    - int32_t
   *    - int64_t
   *    - uint64_t
   *    - double
   *    - const char *
   *    - std::string/std::string_view
   *    - std::span<const bool>
   *    - std::span<const int32_t>
   *    - std::span<const int64_t>
   *    - std::span<const uint32_t>
   *    - std::span<const double>
   *    - std::span<std::string/std::string_view>
   *    - uint64_t (not fully supported)
   *    - std::span<uint64_t> (not fully supported)
   *    - std::span<const uint8_t> (not fully supported).
   *
   *
   */
  class AttributeSet
  {
  public:
    virtual void AddAttribute(std::string const& attributeName, bool value) = 0;
    virtual void AddAttribute(std::string const& attributeName, int32_t value) = 0;
    virtual void AddAttribute(std::string const& attributeName, int64_t value) = 0;
    virtual void AddAttribute(std::string const& attributeName, uint64_t value) = 0;
    virtual void AddAttribute(std::string const& attributeName, double value) = 0;
    virtual void AddAttribute(std::string const& attributeName, const char* value) = 0;
    virtual void AddAttribute(std::string const& attributeName, std::string const& value) = 0;
    virtual ~AttributeSet(){};
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
    explicit SpanStatus(std::string const& status) : ExtendableEnumeration(status) {}
    SpanStatus() = default;

    AZ_CORE_DLLEXPORT const static SpanStatus Unset;
    AZ_CORE_DLLEXPORT const static SpanStatus Ok;
    AZ_CORE_DLLEXPORT const static SpanStatus Error;
  };

  /**
   * @brief Span - represents a span in tracing.
   */
  class Span {
  public:
    /**
     * @brief Signals that the span has now ended.
     */
    virtual void End(Azure::Nullable<Azure::DateTime> endTime = {}) = 0;

    virtual void AddAttributes(AttributeSet const& attributeToAdd) = 0;

    /**
     * Add an Event to the span. An event is identified by a name and an optional set of attributes
     * associated with the event.
     */
    virtual void AddEvent(std::string const& eventName, AttributeSet const& eventAttributes) = 0;
    virtual void AddEvent(std::string const& eventName) = 0;

    /**
     * @brief Records an exception.
     *
     * @note This might be better as std::runtime_error instead of std::exception. To be discussed.
     */
    virtual void RecordException(std::exception const& exceptionToRecord) = 0;
    virtual void RecordException(
        std::exception const& exceptionToRecord,
        AttributeSet const& eventAttributes)
        = 0;

    virtual void SetStatus(SpanStatus const& status, std::string const& description = "") = 0;
  };

  struct CreateSpanOptions final
  {
    SpanKind SpanKind{SpanKind::Internal};
    std::unique_ptr<AttributeSet> Attributes;
    // Links
    // Start Timestamp
  };

  /**
   * @brief Tracer - factory for creating span objects.
   *
   */
  class Tracer {
  public:
    virtual std::shared_ptr<Azure::Core::Tracing::Span> CreateSpan(
        std::string const& spanName,
        CreateSpanOptions const& options = {}) const = 0;
  };

  /**
   * @brief Trace Provider - factory for creating Tracer objects.
   */
  class TracerProvider {
  public:
    virtual std::shared_ptr<Azure::Core::Tracing::Tracer> CreateTracer(
        std::string const& name,
        std::string const& version = "",
        Azure::Nullable<Azure::Core::Url> const& scheme_url = {}) const = 0;
  };
}}} // namespace Azure::Core::Tracing
