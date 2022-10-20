// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#if defined(_MSC_VER)
// The OpenTelemetry headers generate a couple of warnings on MSVC in the OTel 1.2 package, suppress
// the warnings across the includes.
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4244)
#pragma warning(disable : 6323) // Disable "Use of arithmetic operator on Boolean type" warning.
#endif
#include <opentelemetry/exporters/memory/in_memory_span_data.h>
#include <opentelemetry/exporters/memory/in_memory_span_exporter.h>
#include <opentelemetry/exporters/ostream/span_exporter.h>
#include <opentelemetry/nostd/span.h>
#include <opentelemetry/sdk/common/global_log_handler.h>
#include <opentelemetry/sdk/trace/exporter.h>
#include <opentelemetry/sdk/trace/processor.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/trace/propagation/http_trace_context.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

class RecordedSpan : public opentelemetry::sdk::trace::Recordable {
  struct Event
  {
    std::string Name;
    std::chrono::system_clock::time_point Timestamp;
    opentelemetry::sdk::common::AttributeMap Attributes;
  };
  opentelemetry::trace::SpanId m_parentSpan;
  opentelemetry::trace::SpanId m_spanId;
  opentelemetry::sdk::common::AttributeMap m_attributes;
  std::vector<Event> m_events;
  opentelemetry::trace::StatusCode m_statusCode{};
  std::string m_statusDescription;
  std::string m_name;
  opentelemetry::trace::SpanKind m_spanKind{};
  std::chrono::system_clock::time_point m_startTime;
  std::chrono::nanoseconds m_duration{};
  std::unique_ptr<opentelemetry::sdk::instrumentationscope::InstrumentationScope> m_scope;
  std::unique_ptr<opentelemetry::sdk::resource::Resource> m_resource;

public:
  ~RecordedSpan() = default;

  /**
   * Set the span context and parent span id
   * @param span_context the span context to set
   * @param parent_span_id the parent span id to set
   */
  void SetIdentity(
      const opentelemetry::trace::SpanContext& span_context,
      opentelemetry::trace::SpanId parent_span_id) noexcept override
  {
    m_parentSpan = parent_span_id;
    m_spanId = span_context.span_id();
  };

  /**
   * Set an attribute of a span.
   * @param name the name of the attribute
   * @param value the attribute value
   */
  void SetAttribute(
      opentelemetry::nostd::string_view key,
      const opentelemetry::common::AttributeValue& value) noexcept override
  {
    m_attributes.SetAttribute(key, value);
  };

  /**
   * Add an event to a span.
   * @param name the name of the event
   * @param timestamp the timestamp of the event
   * @param attributes the attributes associated with the event
   */
  void AddEvent(
      opentelemetry::nostd::string_view name,
      opentelemetry::common::SystemTimestamp timestamp,
      const opentelemetry::common::KeyValueIterable& attributes) noexcept override
  {
    Event event;
    event.Name = std::string(name);
    event.Timestamp = timestamp;

    attributes.ForEachKeyValue(
        [&event](
            opentelemetry::nostd::string_view name, opentelemetry::common::AttributeValue value) {
          event.Attributes.SetAttribute(name, value);
          return true;
        });
    m_events.push_back(event);
  };

  /**
   * Add a link to a span.
   * @param span_context the span context of the linked span
   * @param attributes the attributes associated with the link
   */
  void AddLink(
      const opentelemetry::trace::SpanContext& span_context,
      const opentelemetry::common::KeyValueIterable& attributes) noexcept override
  {
    span_context;
    attributes;
  };

  /**
   * Set the status of the span.
   * @param code the status code
   * @param description a description of the status
   */
  void SetStatus(
      opentelemetry::trace::StatusCode code,
      opentelemetry::nostd::string_view description) noexcept override
  {
    m_statusCode = code;
    m_statusDescription = std::string(description);
  };

  /**
   * Set the name of the span.
   * @param name the name to set
   */
  void SetName(opentelemetry::nostd::string_view name) noexcept override
  {
    m_name = std::string(name);
  };

  /**
   * Set the spankind of the span.
   * @param span_kind the spankind to set
   */
  void SetSpanKind(opentelemetry::trace::SpanKind span_kind) noexcept override
  {
    m_spanKind = span_kind;
  };

  /**
   * Set Resource of the span
   * @param Resource the resource to set
   */
  void SetResource(const opentelemetry::sdk::resource::Resource& resource) noexcept override
  {
    m_resource = std::make_unique<opentelemetry::sdk::resource::Resource>(resource);
    resource;
  };

  /**
   * Set the start time of the span.
   * @param start_time the start time to set
   */
  void SetStartTime(opentelemetry::common::SystemTimestamp start_time) noexcept override
  {
    m_startTime = start_time;
  };

  /**
   * Set the duration of the span.
   * @param duration the duration to set
   */
  void SetDuration(std::chrono::nanoseconds duration) noexcept override { m_duration = duration; }

  /**
   * Set the instrumentation scope of the span.
   * @param instrumentation_scope the instrumentation scope to set
   */
  void SetInstrumentationScope(const opentelemetry::sdk::instrumentationscope::InstrumentationScope&
                                   instrumentation_scope) noexcept override
  {
    m_scope = std::make_unique<opentelemetry::sdk::instrumentationscope::InstrumentationScope>(
        instrumentation_scope);
  };

  std::string GetName() { return m_name; }
  opentelemetry::trace::StatusCode GetStatus() { return m_statusCode; }
  opentelemetry::trace::SpanId GetParentSpanId() { return m_parentSpan; }
  opentelemetry::trace::SpanKind GetSpanKind() { return m_spanKind; }
  opentelemetry::trace::SpanId GetSpanId() { return m_spanId; }
  opentelemetry::sdk::common::AttributeMap const& GetAttributes() { return m_attributes; }
  opentelemetry::sdk::instrumentationscope::InstrumentationScope& GetInstrumentationScope()
  {
    return *m_scope;
  }
};

class TestExporter final : public opentelemetry::sdk::trace::SpanExporter {

public:
  class TestData {
    std::vector<std::unique_ptr<RecordedSpan>> m_spans;

  public:
    // Returns a copy of the recorded spans and clears the set of recorded spans.
    std::vector<std::unique_ptr<RecordedSpan>> const ExtractSpans() { return std::move(m_spans); }
    void AddSpan(std::unique_ptr<RecordedSpan>&& span) { m_spans.push_back(std::move(span)); }
  };
  std::shared_ptr<TestData> const& GetTestData() { return m_testData; }

  TestExporter() : m_testData{std::make_shared<TestData>()} {}
  virtual ~TestExporter() = default;

  /**
   * Create a span recordable. This object will be used to record span data and
   * will subsequently be passed to SpanExporter::Export. Vendors can implement
   * custom recordables or use the default SpanData recordable provided by the
   * SDK.
   * @return a newly initialized Recordable object
   *
   * Note: This method must be callable from multiple threads.
   */
  std::unique_ptr<opentelemetry::sdk::trace::Recordable> MakeRecordable() noexcept override
  {
    return std::unique_ptr<opentelemetry::sdk::trace::Recordable>(new (std::nothrow) RecordedSpan);
  }

  /**
   * Exports a batch of span recordables. This method must not be called
   * concurrently for the same exporter instance.
   * @param spans a span of unique pointers to span recordables
   */
  opentelemetry::sdk::common::ExportResult Export(
      const opentelemetry::nostd::span<std::unique_ptr<opentelemetry::sdk::trace::Recordable>>&
          spans) noexcept override
  {
    for (auto& recordable : spans)
    {
      auto span = std::unique_ptr<RecordedSpan>(static_cast<RecordedSpan*>(recordable.release()));
      m_testData->AddSpan(std::move(span));
    }
    return opentelemetry::sdk::common::ExportResult::kSuccess;
  }

  /**
   * Shut down the exporter.
   * @param timeout an optional timeout.
   * @return return the status of the operation.
   */
  bool Shutdown(std::chrono::microseconds) noexcept override { return true; }

private:
  std::shared_ptr<TestData> m_testData;
};