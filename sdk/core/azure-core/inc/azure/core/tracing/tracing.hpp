// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 * @brief Handling log messages from Azure SDK.
 */

#pragma once

#include "azure/core/datetime.hpp"
#include "azure/core/internal/extendable_enumeration.hpp"
#include "azure/core/nullable.hpp"
#include "azure/core/url.hpp"
#include <array>
#include <memory>
#include <string>
#include <vector>

// Forward declare Azure::Core::Http::Request to resolve an include file ordering problem.
namespace Azure { namespace Core { namespace Http {
  class Request;
}}} // namespace Azure::Core::Http

namespace Azure { namespace Core { namespace Tracing {

  namespace _internal {

    /** The set of attributes to be applied to a Span or Event.
     *
     * @details
     * An AttributeSet represents a set of attributes to be added to a span or
     * event.
     *
     * @note Note that AttributeSets do *NOT* take a copy of their input values,
     * it is the responsibility of the caller to ensure that the object remains
     * valid between when it is added to the AttributeSet and when it is added to
     * a span or event.
     *
     * OpenTelemetry property bags can hold:
     *    - bool
     *    - int32_t
     *    - int64_t
     *    - uint64_t
     *    - double
     *    - const char *
     *    - std::string/std::string_view                      *** Not Implemented
     *    - std::span<const bool>                             *** Not Implemented
     *    - std::span<const int32_t>                          *** Not Implemented
     *    - std::span<const int64_t>                          *** Not Implemented
     *    - std::span<const uint32_t>                         *** Not Implemented
     *    - std::span<const double>                           *** Not Implemented
     *    - std::span<std::string/std::string_view>           *** Not Implemented
     *    - uint64_t (not fully supported)                    *** Not Implemented
     *    - std::span<uint64_t> (not fully supported)         *** Not Implemented
     *    - std::span<const uint8_t> (not fully supported)    *** Not Implemented.
     *
     */
    class AttributeSet {
    public:
      /**
       * @brief Adds a Boolean attribute to the attribute set.
       *
       * @param attributeName Name of attribute to add.
       * @param value Value of attribute.
       */
      virtual void AddAttribute(std::string const& attributeName, bool value) = 0;
      /**
       * @brief Adds a 32bit integer attribute to the attribute set.
       *
       * @param attributeName Name of attribute to add.
       * @param value Value of attribute.
       */
      virtual void AddAttribute(std::string const& attributeName, int32_t value) = 0;
      /**
       * @brief Adds a 64bit integer attribute to the attribute set.
       *
       * @param attributeName Name of attribute to add.
       * @param value Value of attribute.
       */
      virtual void AddAttribute(std::string const& attributeName, int64_t value) = 0;
      /**
       * @brief Adds an unsigned 64bit integer attribute to the attribute set.
       *
       * @param attributeName Name of attribute to add.
       * @param value Value of attribute.
       */
      virtual void AddAttribute(std::string const& attributeName, uint64_t value) = 0;
      /**
       * @brief Adds a 64bit floating point attribute to the attribute set.
       *
       * @param attributeName Name of attribute to add.
       * @param value Value of attribute.
       */
      virtual void AddAttribute(std::string const& attributeName, double value) = 0;
      /**
       * @brief Adds a C style string attribute to the attribute set.
       *
       * @param attributeName Name of attribute to add.
       * @param value Value of attribute.
       */
      virtual void AddAttribute(std::string const& attributeName, const char* value) = 0;
      /**
       * @brief Adds a C++ string attribute to the attribute set.
       *
       * @param attributeName Name of attribute to add.
       * @param value Value of attribute.
       */
      virtual void AddAttribute(std::string const& attributeName, std::string const& value) = 0;

      /**
       * @brief destroys an AttributeSet - virtual destructor to enable base class users to
       * destroy derived classes.
       */
      virtual ~AttributeSet() = default;
    };

    /** @brief The Type of Span.
     */
    class SpanKind final : public Azure::Core::_internal::ExtendableEnumeration<SpanKind> {
    public:
      explicit SpanKind(std::string const& kind) : ExtendableEnumeration(kind) {}
      SpanKind() = default;

      /**
       * @brief Represents an "Internal" operation.
       *
       */
      AZ_CORE_DLLEXPORT const static SpanKind Internal;
      /**
       * @brief Represents a request to a remote service.
       *
       */
      AZ_CORE_DLLEXPORT const static SpanKind Client;
      /**
       * @brief Represents a span covering the server side handling of an API call.
       *
       */
      AZ_CORE_DLLEXPORT const static SpanKind Server;
      /**
       * @brief Represents the initiator of an asynchronous request.
       *
       */
      AZ_CORE_DLLEXPORT const static SpanKind Producer;
      /**
       * @brief Represents a span which describes a child of a producer request.
       *
       */
      AZ_CORE_DLLEXPORT const static SpanKind Consumer;
    };

    /**
     * @brief Represents the status of a span.
     */
    class SpanStatus final : public Azure::Core::_internal::ExtendableEnumeration<SpanStatus> {

    public:
      explicit SpanStatus(std::string const& status) : ExtendableEnumeration(status) {}
      SpanStatus() = default;

      /**
       * @brief The default status of a span.
       */
      AZ_CORE_DLLEXPORT const static SpanStatus Unset;
      /**
       * @brief The operation has completed successfully.
       */
      AZ_CORE_DLLEXPORT const static SpanStatus Ok;
      /**
       * @brief The operation contains an error.
       */
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

      /**
       * @brief Adds a set of attributes to the span.
       *
       * @param attributeToAdd Attributes to be added to the span.
       */
      virtual void AddAttributes(AttributeSet const& attributeToAdd) = 0;

      /**
       * @brief Adds a single string valued attribute to the span.
       *
       * @param attributeName Name of the attribute to add.
       * @param attributeValue value of the attribute.
       */
      virtual void AddAttribute(std::string const& attributeName, std::string const& attributeValue)
          = 0;

      /**
       * @brief Adds an event to the span.
       *
       * Add an Event to the span. An event is identified by a name and an optional set of
       * attributes associated with the event.
       *
       * @param eventName Name of the event to add.
       * @param eventAttributes Attributes associated with the event.
       */
      virtual void AddEvent(std::string const& eventName, AttributeSet const& eventAttributes) = 0;

      /**
       * @brief Adds an event to the span.
       *
       * Add an Event to the span. An event is identified by a name
       *
       * @param eventName Name of the event to add.
       */
      virtual void AddEvent(std::string const& eventName) = 0;
      /**
       * @brief Records an exception occurring in the span.
       *
       * @param exception Exception which has occurred.
       */
      virtual void AddEvent(std::exception const& exception) = 0;

      /**
       * @brief Set the Status of the span
       *
       * @param status Updated status of the span.
       * @param description A description associated with the Status.
       */
      virtual void SetStatus(SpanStatus const& status, std::string const& description = "") = 0;

      /**
       * @brief Propogate information from the current span to the HTTP request headers.
       *
       * @param request HTTP Request to the service. If there is an active tracing span, this will
       * add required headers to the HTTP Request.
       */
      virtual void PropagateToHttpHeaders(Azure::Core::Http::Request& request) = 0;
    };

    /**
     * @brief Options used while creating a span.
     *
     */
    struct CreateSpanOptions final
    {
      /**
       * @brief The kind of span to be created.
       *
       */
      SpanKind Kind{SpanKind::Internal};
      /**
       * @brief Attributes associated with the span.
       *
       */
      std::unique_ptr<AttributeSet> Attributes;

      /**
       * @brief Parent for the newly created span.
       */
      std::shared_ptr<Span> ParentSpan;
    };

    /**
     * @brief Tracer - factory for creating span objects.
     *
     */
    class Tracer {
    public:
      /**
       * @brief Create new Span object.
       *
       * @details Creates a new span object.
       *
       * @note There is no concept of a "current" span, each span created is a top level span,
       * unless the CreateSpanOptions has ParentSpan member, in which case the ParentSpan member
       * will be the parent of the newly created span.
       *
       * @param spanName The name of the span to create.
       * @param options Options to be used when creating the span.
       * @return std::shared_ptr<Azure::Core::Tracing::Span> Newly created span.
       */
      virtual std::shared_ptr<Span> CreateSpan(
          std::string const& spanName,
          CreateSpanOptions const& options = {}) const = 0;

      virtual std::unique_ptr<AttributeSet> CreateAttributeSet() const = 0;
    };
  } // namespace _internal

  /**
   * @brief Trace Provider - factory for creating Tracer objects.
   */
  class TracerProvider {
  public:
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
        std::string const& version = "") const = 0;
  };
}}} // namespace Azure::Core::Tracing
