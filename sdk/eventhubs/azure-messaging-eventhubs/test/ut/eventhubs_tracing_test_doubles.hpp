// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Recording tracing test doubles shared by the offline and the live tracing tests.
#pragma once

#include <azure/core/internal/tracing/service_tracing.hpp>
#include <azure/core/tracing/tracing.hpp>

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  // The type tag of the overload that delivered an attribute. A string "3" and a uint64 3 both
  // stringify to "3", so the value alone cannot show the type on the wire.
  constexpr const char* AttributeTypeBool = "bool";
  constexpr const char* AttributeTypeInt32 = "int32";
  constexpr const char* AttributeTypeInt64 = "int64";
  constexpr const char* AttributeTypeUInt64 = "uint64";
  constexpr const char* AttributeTypeDouble = "double";
  constexpr const char* AttributeTypeCString = "cstring";
  constexpr const char* AttributeTypeString = "string";

  class TestAttributeSet final : public Azure::Core::Tracing::_internal::AttributeSet {
    std::map<std::string, std::string> m_attributes;
    std::map<std::string, std::string> m_attributeTypes;

  public:
    TestAttributeSet() : Azure::Core::Tracing::_internal::AttributeSet() {}

    void AddAttribute(std::string const& key, bool value) override
    {
      m_attributes[key] = std::to_string(value);
      m_attributeTypes[key] = AttributeTypeBool;
    }
    void AddAttribute(std::string const& key, int32_t value) override
    {
      m_attributes[key] = std::to_string(value);
      m_attributeTypes[key] = AttributeTypeInt32;
    }
    void AddAttribute(std::string const& key, int64_t value) override
    {
      m_attributes[key] = std::to_string(value);
      m_attributeTypes[key] = AttributeTypeInt64;
    }
    void AddAttribute(std::string const& key, uint64_t value) override
    {
      m_attributes[key] = std::to_string(value);
      m_attributeTypes[key] = AttributeTypeUInt64;
    }
    void AddAttribute(std::string const& key, double value) override
    {
      m_attributes[key] = std::to_string(value);
      m_attributeTypes[key] = AttributeTypeDouble;
    }
    void AddAttribute(std::string const& key, const char* value) override
    {
      m_attributes[key] = std::string(value);
      m_attributeTypes[key] = AttributeTypeCString;
    }
    void AddAttribute(std::string const& key, std::string const& value) override
    {
      m_attributes[key] = value;
      m_attributeTypes[key] = AttributeTypeString;
    }

    std::map<std::string, std::string> const& GetAttributes() const { return m_attributes; }
    std::map<std::string, std::string> const& GetAttributeTypes() const { return m_attributeTypes; }
  };

  class TestSpan final : public Azure::Core::Tracing::_internal::Span {
    std::string m_spanName;
    Azure::Core::Tracing::_internal::SpanKind m_kind;
    std::map<std::string, std::string> m_attributes;
    std::map<std::string, std::string> m_attributeTypes;
    std::vector<std::string> m_events;
    std::vector<Azure::Core::Tracing::_internal::SpanStatus> m_statuses;
    bool m_hasParent;

    void Merge(TestAttributeSet const& attributes)
    {
      for (auto const& attribute : attributes.GetAttributes())
      {
        m_attributes[attribute.first] = attribute.second;
      }
      for (auto const& attributeType : attributes.GetAttributeTypes())
      {
        m_attributeTypes[attributeType.first] = attributeType.second;
      }
    }

  public:
    TestSpan(
        std::string const& spanName,
        Azure::Core::Tracing::_internal::CreateSpanOptions const& options)
        : Azure::Core::Tracing::_internal::Span(), m_spanName(spanName), m_kind(options.Kind),
          m_hasParent(options.ParentSpan != nullptr)
    {
      if (options.Attributes)
      {
        Merge(*static_cast<TestAttributeSet const*>(options.Attributes.get()));
      }
    }

    void AddAttributes(Azure::Core::Tracing::_internal::AttributeSet const& attributeToAdd) override
    {
      Merge(static_cast<TestAttributeSet const&>(attributeToAdd));
    }
    void AddAttribute(std::string const& attributeName, std::string const& attributeValue) override
    {
      m_attributes[attributeName] = attributeValue;
      m_attributeTypes[attributeName] = AttributeTypeString;
    }
    void AddEvent(
        std::string const& eventName,
        Azure::Core::Tracing::_internal::AttributeSet const&) override
    {
      m_events.push_back(eventName);
    }
    void AddEvent(std::string const& eventName) override { m_events.push_back(eventName); }
    void AddEvent(std::exception const& ex) override { m_events.push_back(ex.what()); }
    void SetStatus(Azure::Core::Tracing::_internal::SpanStatus const& status, std::string const&)
        override
    {
      m_statuses.push_back(status);
    }
    void End(Azure::Nullable<Azure::DateTime>) override {}
    void PropagateToHttpHeaders(Azure::Core::Http::Request&) override {}

    std::string const& GetName() const { return m_spanName; }
    Azure::Core::Tracing::_internal::SpanKind GetKind() const { return m_kind; }
    bool HasParent() const { return m_hasParent; }
    std::map<std::string, std::string> const& GetAttributes() const { return m_attributes; }
    std::map<std::string, std::string> const& GetAttributeTypes() const { return m_attributeTypes; }
    std::vector<std::string> const& GetEvents() const { return m_events; }
    std::vector<Azure::Core::Tracing::_internal::SpanStatus> const& GetStatuses() const
    {
      return m_statuses;
    }
  };

  class TestTracer final : public Azure::Core::Tracing::_internal::Tracer {
    std::string m_name;
    std::string m_version;
    mutable std::vector<std::shared_ptr<TestSpan>> m_spans;

  public:
    TestTracer(std::string const& name, std::string const& version)
        : Azure::Core::Tracing::_internal::Tracer(), m_name(name), m_version(version)
    {
    }

    std::shared_ptr<Azure::Core::Tracing::_internal::Span> CreateSpan(
        std::string const& spanName,
        Azure::Core::Tracing::_internal::CreateSpanOptions const& options) const override
    {
      auto span = std::make_shared<TestSpan>(spanName, options);
      m_spans.push_back(span);
      return span;
    }

    // azure-core dereferences the returned attribute set when a tracer exists, so this must
    // never return null. See sdk/core/azure-core/src/tracing/tracing.cpp.
    std::unique_ptr<Azure::Core::Tracing::_internal::AttributeSet> CreateAttributeSet()
        const override
    {
      return std::make_unique<TestAttributeSet>();
    }

    std::string const& GetName() const { return m_name; }
    std::string const& GetVersion() const { return m_version; }
    std::vector<std::shared_ptr<TestSpan>> const& GetSpans() const { return m_spans; }
  };

  class TestTracingProvider final : public Azure::Core::Tracing::TracerProvider {
    mutable std::list<std::shared_ptr<TestTracer>> m_tracers;

  public:
    TestTracingProvider() : Azure::Core::Tracing::TracerProvider() {}
    ~TestTracingProvider() override {}

    std::shared_ptr<Azure::Core::Tracing::_internal::Tracer> CreateTracer(
        std::string const& name,
        std::string const& version) const override
    {
      auto tracer = std::make_shared<TestTracer>(name, version);
      m_tracers.push_back(tracer);
      return tracer;
    }

    std::list<std::shared_ptr<TestTracer>> const& GetTracers() const { return m_tracers; }
  };

  inline std::shared_ptr<TestSpan> SingleSpan(std::shared_ptr<TestTracingProvider> const& provider)
  {
    EXPECT_EQ(1u, provider->GetTracers().size());
    if (provider->GetTracers().size() != 1u)
    {
      return nullptr;
    }
    auto const& tracer = provider->GetTracers().front();
    EXPECT_EQ(1u, tracer->GetSpans().size());
    if (tracer->GetSpans().size() != 1u)
    {
      return nullptr;
    }
    return tracer->GetSpans().front();
  }

  inline std::shared_ptr<TestSpan> FindSpan(
      std::shared_ptr<TestTracingProvider> const& provider,
      std::string const& spanName)
  {
    EXPECT_EQ(1u, provider->GetTracers().size());
    if (provider->GetTracers().size() != 1u)
    {
      return nullptr;
    }

    std::shared_ptr<TestSpan> match;
    for (auto const& span : provider->GetTracers().front()->GetSpans())
    {
      if (span->GetName() == spanName)
      {
        EXPECT_EQ(nullptr, match) << "More than one span named " << spanName;
        match = span;
      }
    }
    EXPECT_NE(nullptr, match) << "No span named " << spanName;
    return match;
  }

}}}} // namespace Azure::Messaging::EventHubs::Test
