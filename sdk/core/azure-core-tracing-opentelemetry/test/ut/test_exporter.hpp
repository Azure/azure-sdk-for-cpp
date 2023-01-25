// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>
#include <opentelemetry/sdk/trace/exporter.h>

class RecordedSpan : public opentelemetry::sdk::trace::Recordable {
public:
  struct Attribute
  {
    enum class AttributeType
    {
      Bool,
      Int32,
      Int64,
      UInt32,
      UInt64,
      Double,
      CString,
      String,
      BoolArray,
      Int32Array,
      UInt32Array,
      Int64Array,
      UInt64Array,
      DoubleArray,
      StringArray,
      ByteArray
    };
    AttributeType Type;
    bool BoolValue{};
    int32_t Int32Value{};
    int64_t Int64Value{};
    uint32_t UInt32Value{};
    uint64_t UInt64Value{};
    double DoubleValue{};
    const char* CStringValue{};
    std::string StringValue;
    std::vector<bool> BoolArrayValue;
    std::vector<int32_t> Int32ArrayValue;
    std::vector<int64_t> Int64ArrayValue;
    std::vector<uint32_t> UInt32ArrayValue;
    std::vector<uint64_t> UInt64ArrayValue;
    std::vector<double> DoubleArrayValue;
    std::vector<std::string> StringArrayValue;
    std::vector<uint8_t> ByteArrayValue;
    Attribute(bool val) : Type{AttributeType::Bool}, BoolValue{val} {}
    Attribute(const char* val) : Type{AttributeType::CString}, CStringValue{val} {}
    Attribute(uint32_t val) : Type{AttributeType::UInt32}, UInt32Value{val} {}
    Attribute(int32_t val) : Type{AttributeType::Int32}, Int32Value{val} {}
    Attribute(int64_t val) : Type{AttributeType::Int64}, Int64Value{val} {}
    Attribute(uint64_t val) : Type{AttributeType::UInt64}, UInt64Value{val} {}
    Attribute(double val) : Type{AttributeType::Double}, DoubleValue{val} {}
    Attribute(opentelemetry::nostd::string_view const val)
        : Type{AttributeType::String}, StringValue{val}
    {
    }
    Attribute(opentelemetry::nostd::span<const bool> val)
        : Type{AttributeType::BoolArray}, BoolArrayValue(val.size(), val.data())
    {
    }
    Attribute(opentelemetry::nostd::span<const int32_t> val)
        : Type{AttributeType::Int32Array}, Int32ArrayValue(val.begin(), val.end())
    {
    }
    Attribute(opentelemetry::nostd::span<const uint32_t> val)
        : Type{AttributeType::UInt32Array}, UInt32ArrayValue(val.begin(), val.end())
    {
    }
    Attribute(opentelemetry::nostd::span<const int64_t> val)
        : Type{AttributeType::Int64Array}, Int64ArrayValue(val.begin(), val.end())
    {
    }
    Attribute(opentelemetry::nostd::span<const uint64_t> val)
        : Type{AttributeType::UInt64Array}, UInt64ArrayValue(val.begin(), val.end())
    {
    }
    Attribute(opentelemetry::nostd::span<const double> val)
        : Type{AttributeType::DoubleArray}, DoubleArrayValue(val.begin(), val.end())
    {
    }
    Attribute(opentelemetry::nostd::span<const opentelemetry::nostd::string_view> val)
        : Type{AttributeType::StringArray}, StringArrayValue(val.begin(), val.end())
    {
    }
    Attribute(opentelemetry::nostd::span<const uint8_t> val)
        : Type{AttributeType::ByteArray}, ByteArrayValue(val.begin(), val.end())
    {
    }
    AttributeType index() const { return Type; }

    operator bool() const
    {
      assert(Type == AttributeType::Bool);
      return BoolValue;
    }
  };
  class AttributeMap {
    std::map<std::string, Attribute> m_attributes;

  public:
    void SetAttribute(std::string const& key, const opentelemetry::common::AttributeValue& value)
    {
      switch (value.index())
      {
        case opentelemetry::common::AttributeType::kTypeBool: {
          ;
          m_attributes.emplace(key, Attribute{opentelemetry::nostd::get<bool>(value)});
          break;
        }
        case opentelemetry::common::AttributeType::kTypeCString: {
          m_attributes.emplace(key, Attribute{opentelemetry::nostd::get<const char*>(value)});
          break;
        }
        case opentelemetry::common::AttributeType::kTypeInt: {
          m_attributes.emplace(key, Attribute{opentelemetry::nostd::get<int32_t>(value)});
          break;
        }
        case opentelemetry::common::AttributeType::kTypeInt64: {
          m_attributes.emplace(key, Attribute{opentelemetry::nostd::get<int64_t>(value)});
          break;
        }
        case opentelemetry::common::AttributeType::kTypeUInt: {
          m_attributes.emplace(key, Attribute{opentelemetry::nostd::get<uint32_t>(value)});
          break;
        }
        case opentelemetry::common::AttributeType::kTypeDouble: {
          m_attributes.emplace(key, Attribute{opentelemetry::nostd::get<double>(value)});
          break;
        }
        case opentelemetry::common::AttributeType::kTypeString: {
          m_attributes.emplace(
              key, Attribute{opentelemetry::nostd::get<opentelemetry::nostd::string_view>(value)});
          break;
        }
        case opentelemetry::common::AttributeType::kTypeSpanBool: {
          m_attributes.emplace(
              key,
              Attribute{opentelemetry::nostd::get<opentelemetry::nostd::span<const bool>>(value)});
          break;
        }
        case opentelemetry::common::AttributeType::kTypeSpanInt: {
          m_attributes.emplace(
              key,
              Attribute{
                  opentelemetry::nostd::get<opentelemetry::nostd::span<const int32_t>>(value)});
          break;
        }
        case opentelemetry::common::AttributeType::kTypeSpanInt64: {
          m_attributes.emplace(
              key,
              Attribute{
                  opentelemetry::nostd::get<opentelemetry::nostd::span<const int64_t>>(value)});
          break;
        }
        case opentelemetry::common::AttributeType::kTypeSpanUInt: {
          m_attributes.emplace(
              key,
              Attribute{
                  opentelemetry::nostd::get<opentelemetry::nostd::span<const uint32_t>>(value)});
          break;
        }
        case opentelemetry::common::AttributeType::kTypeSpanDouble: {
          m_attributes.emplace(
              key,
              Attribute{
                  opentelemetry::nostd::get<opentelemetry::nostd::span<const double>>(value)});
          break;
        }
        case opentelemetry::common::AttributeType::kTypeSpanString: {
          m_attributes.emplace(
              key,
              Attribute{opentelemetry::nostd::get<
                  opentelemetry::nostd::span<const opentelemetry::nostd::string_view>>(value)});
          break;
        }
        case opentelemetry::common::AttributeType::kTypeUInt64: {
          m_attributes.emplace(key, Attribute{opentelemetry::nostd::get<uint64_t>(value)});
          break;
        }
        case opentelemetry::common::AttributeType::kTypeSpanUInt64: {
          m_attributes.emplace(
              key,
              Attribute{
                  opentelemetry::nostd::get<opentelemetry::nostd::span<const uint64_t>>(value)});
          break;
        }
        case opentelemetry::common::AttributeType::kTypeSpanByte: {
          m_attributes.emplace(
              key,
              Attribute{
                  opentelemetry::nostd::get<opentelemetry::nostd::span<const uint8_t>>(value)});
          break;
        }

        break;
      }
    }
    size_t size() const { return m_attributes.size(); }
    decltype(m_attributes)::iterator begin() { return m_attributes.begin(); }
    decltype(m_attributes)::iterator end() { return m_attributes.end(); }
    Attribute const& at(std::string const& key) const { return m_attributes.at(key); }
  };
  struct Event
  {
    std::string Name;
    std::chrono::system_clock::time_point Timestamp;
    AttributeMap Attributes;
  };
  opentelemetry::trace::SpanId m_parentSpan;
  opentelemetry::trace::SpanId m_spanId;
  AttributeMap m_attributes;
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
   * @param key the name of the attribute
   * @param value the attribute value
   */
  void SetAttribute(
      opentelemetry::nostd::string_view key,
      const opentelemetry::common::AttributeValue& value) noexcept override
  {
    m_attributes.SetAttribute(std::string{key}, value);
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
          event.Attributes.SetAttribute(std::string{name}, value);
          return true;
        });
    m_events.push_back(event);
  };

  /**
   * Add a link to a span.
   */
  void AddLink(
      const opentelemetry::trace::SpanContext&,
      const opentelemetry::common::KeyValueIterable&) noexcept override{
      // TODO, when we use this, we need to test this.
      // NO-OP since this exporter silences link data.
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
   * @param resource the resource to set
   */
  void SetResource(const opentelemetry::sdk::resource::Resource& resource) noexcept override
  {
    m_resource = std::make_unique<opentelemetry::sdk::resource::Resource>(resource);
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
  AttributeMap const& GetAttributes() { return m_attributes; }
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
   * @return return the status of the operation.
   */
  bool Shutdown(std::chrono::microseconds) noexcept override { return true; }

private:
  std::shared_ptr<TestData> m_testData;
};
