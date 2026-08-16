// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/private/eventhubs_tracing.hpp"
#include "../src/private/eventhubs_utilities.hpp"
#include "../src/private/package_version.hpp"

#include <azure/core/context.hpp>
#include <azure/core/internal/tracing/service_tracing.hpp>
#include <azure/core/tracing/tracing.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <cstdint>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  namespace {
    // The key below is fake text. cspell tokenizes it into fragments that are not words, so
    // spell checking is disabled across this block.
    // cspell: disable
    constexpr const char* TestConnectionString
        = "Endpoint=sb://fake.example.com/;SharedAccessKeyName=name;SharedAccessKey=key;"
          "EntityPath=unit-test-eh";
    // cspell: enable
    constexpr const char* TestEventHubName = "unit-test-eh";
    constexpr const char* TestFullyQualifiedNamespace = "fake.example.com";

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
      std::map<std::string, std::string> const& GetAttributeTypes() const
      {
        return m_attributeTypes;
      }
    };

    class TestSpan final : public Azure::Core::Tracing::_internal::Span {
      std::string m_spanName;
      Azure::Core::Tracing::_internal::SpanKind m_kind;
      std::map<std::string, std::string> m_attributes;
      std::map<std::string, std::string> m_attributeTypes;
      std::vector<std::string> m_events;
      std::vector<Azure::Core::Tracing::_internal::SpanStatus> m_statuses;

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
          : Azure::Core::Tracing::_internal::Span(), m_spanName(spanName), m_kind(options.Kind)
      {
        if (options.Attributes)
        {
          Merge(*static_cast<TestAttributeSet const*>(options.Attributes.get()));
        }
      }

      void AddAttributes(
          Azure::Core::Tracing::_internal::AttributeSet const& attributeToAdd) override
      {
        Merge(static_cast<TestAttributeSet const&>(attributeToAdd));
      }
      void AddAttribute(std::string const& attributeName, std::string const& attributeValue)
          override
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
      std::map<std::string, std::string> const& GetAttributes() const { return m_attributes; }
      std::map<std::string, std::string> const& GetAttributeTypes() const
      {
        return m_attributeTypes;
      }
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

    EventDataBatch CreateTestBatch()
    {
      EventDataBatchOptions batchOptions;
      batchOptions.MaxBytes = static_cast<std::uint64_t>((std::numeric_limits<uint16_t>::max)());
      EventDataBatch batch{_detail::EventDataBatchFactory::CreateEventDataBatch(batchOptions)};
      EXPECT_TRUE(batch.TryAdd(Models::EventData{"First message."}));
      EXPECT_TRUE(batch.TryAdd(Models::EventData{"Second message."}));
      return batch;
    }

    EventDataBatch CreateEmptyTestBatch()
    {
      EventDataBatchOptions batchOptions;
      batchOptions.MaxBytes = static_cast<std::uint64_t>((std::numeric_limits<uint16_t>::max)());
      return _detail::EventDataBatchFactory::CreateEventDataBatch(batchOptions);
    }

    std::map<std::string, std::string> ExpectedSendAttributes()
    {
      return std::map<std::string, std::string>{
          {"az.namespace", "Microsoft.EventHub"},
          {"messaging.system", "eventhubs"},
          {"messaging.destination.name", TestEventHubName},
          {"messaging.operation", "publish"},
          {"messaging.batch.message_count", "2"},
          {"net.peer.name", TestFullyQualifiedNamespace},
      };
    }

    std::shared_ptr<TestSpan> SingleSpan(std::shared_ptr<TestTracingProvider> const& provider)
    {
      EXPECT_EQ(1u, provider->GetTracers().size());
      auto const& tracer = provider->GetTracers().front();
      EXPECT_EQ(1u, tracer->GetSpans().size());
      if (tracer->GetSpans().size() != 1u)
      {
        return nullptr;
      }
      return tracer->GetSpans().front();
    }
  } // namespace

  TEST(EventHubsTracingTest, ProducerOptionsTracingProviderDefaultsEmpty)
  {
    ProducerClientOptions options;
    EXPECT_TRUE((std::is_same<
                 decltype(options.TracingProvider),
                 std::shared_ptr<Azure::Core::Tracing::TracerProvider>>::value));
    EXPECT_EQ(nullptr, options.TracingProvider);

    options.TracingProvider = std::make_shared<TestTracingProvider>();
    EXPECT_NE(nullptr, options.TracingProvider);
  }

  TEST(EventHubsTracingTest, ConsumerOptionsTracingProviderDefaultsEmpty)
  {
    ConsumerClientOptions options;
    EXPECT_TRUE((std::is_same<
                 decltype(options.TracingProvider),
                 std::shared_ptr<Azure::Core::Tracing::TracerProvider>>::value));
    EXPECT_EQ(nullptr, options.TracingProvider);

    options.TracingProvider = std::make_shared<TestTracingProvider>();
    EXPECT_NE(nullptr, options.TracingProvider);
  }

  TEST(EventHubsTracingTest, ProducerBuildsTracerFromOptions)
  {
    auto provider = std::make_shared<TestTracingProvider>();
    ProducerClientOptions options;
    options.TracingProvider = provider;

    ProducerClient client{TestConnectionString, TestEventHubName, options};

    ASSERT_EQ(1u, provider->GetTracers().size());
    auto const& tracer = provider->GetTracers().front();
    EXPECT_EQ("azure-messaging-eventhubs-cpp", tracer->GetName());
    EXPECT_EQ(_detail::PackageVersion::ToString(), tracer->GetVersion());
  }

  TEST(EventHubsTracingTest, ConsumerBuildsTracerFromOptions)
  {
    auto provider = std::make_shared<TestTracingProvider>();
    ConsumerClientOptions options;
    options.TracingProvider = provider;

    ConsumerClient client{TestConnectionString, TestEventHubName, DefaultConsumerGroup, options};

    ASSERT_EQ(1u, provider->GetTracers().size());
    auto const& tracer = provider->GetTracers().front();
    EXPECT_EQ("azure-messaging-eventhubs-cpp", tracer->GetName());
    EXPECT_EQ(_detail::PackageVersion::ToString(), tracer->GetVersion());
  }

  TEST(EventHubsTracingTest, SendSpanOnCancelledContext)
  {
    auto provider = std::make_shared<TestTracingProvider>();
    ProducerClientOptions options;
    options.TracingProvider = provider;

    ProducerClient client{TestConnectionString, TestEventHubName, options};
    EventDataBatch batch{CreateTestBatch()};

    Azure::Core::Context cancelledContext;
    cancelledContext.Cancel();
    EXPECT_THROW(client.Send(batch, cancelledContext), Azure::Core::OperationCancelledException);

    auto span = SingleSpan(provider);
    ASSERT_NE(nullptr, span);
    EXPECT_EQ("ProducerClient.Send", span->GetName());
    EXPECT_EQ(Azure::Core::Tracing::_internal::SpanKind::Producer, span->GetKind());
    EXPECT_EQ(ExpectedSendAttributes(), span->GetAttributes());
    EXPECT_EQ(span->GetAttributes().end(), span->GetAttributes().find("server.address"));

    ASSERT_EQ(1u, span->GetEvents().size());
    EXPECT_NE(std::string::npos, span->GetEvents()[0].find("cancelled"));

    ASSERT_FALSE(span->GetStatuses().empty());
    EXPECT_EQ(Azure::Core::Tracing::_internal::SpanStatus::Error, span->GetStatuses().back());
  }

  TEST(EventHubsTracingTest, SendSpanRecordsNonCancelException)
  {
    auto provider = std::make_shared<TestTracingProvider>();
    ProducerClientOptions options;
    options.TracingProvider = provider;

    ProducerClient client{TestConnectionString, TestEventHubName, options};
    EventDataBatch batch{CreateTestBatch()};

    EXPECT_THROW(client.Send(batch, Azure::Core::Context{}), std::runtime_error);

    auto span = SingleSpan(provider);
    ASSERT_NE(nullptr, span);
    EXPECT_EQ("ProducerClient.Send", span->GetName());
    EXPECT_EQ(Azure::Core::Tracing::_internal::SpanKind::Producer, span->GetKind());
    EXPECT_EQ(ExpectedSendAttributes(), span->GetAttributes());

    ASSERT_EQ(1u, span->GetEvents().size());
    EXPECT_FALSE(span->GetEvents()[0].empty());

    ASSERT_FALSE(span->GetStatuses().empty());
    EXPECT_EQ(Azure::Core::Tracing::_internal::SpanStatus::Error, span->GetStatuses().back());
  }

  // An empty batch fails in the conversion to an AMQP message. That failure must land in the
  // send span like every other send failure.
  TEST(EventHubsTracingTest, SendSpanRecordsEmptyBatchFailure)
  {
    auto provider = std::make_shared<TestTracingProvider>();
    ProducerClientOptions options;
    options.TracingProvider = provider;

    ProducerClient client{TestConnectionString, TestEventHubName, options};
    EventDataBatch batch{CreateEmptyTestBatch()};

    EXPECT_THROW(client.Send(batch, Azure::Core::Context{}), std::runtime_error);

    auto span = SingleSpan(provider);
    ASSERT_NE(nullptr, span);
    EXPECT_EQ("ProducerClient.Send", span->GetName());

    ASSERT_EQ(1u, span->GetAttributes().count("messaging.batch.message_count"));
    EXPECT_EQ("0", span->GetAttributes().at("messaging.batch.message_count"));

    ASSERT_EQ(1u, span->GetEvents().size());
    EXPECT_FALSE(span->GetEvents()[0].empty());

    ASSERT_FALSE(span->GetStatuses().empty());
    EXPECT_EQ(Azure::Core::Tracing::_internal::SpanStatus::Error, span->GetStatuses().back());
  }

  TEST(EventHubsTracingTest, ReceiveSpanShapeFromSharedHelper)
  {
    auto provider = std::make_shared<TestTracingProvider>();
    auto factory = _detail::CreateTracingContextFactory(provider);

    auto tracingContext = _detail::StartSpan(
        factory,
        "PartitionClient.ReceiveEvents",
        Azure::Core::Tracing::_internal::SpanKind::Client,
        "receive",
        TestEventHubName,
        TestFullyQualifiedNamespace,
        Azure::Nullable<size_t>{},
        Azure::Core::Context{});

    auto span = SingleSpan(provider);
    ASSERT_NE(nullptr, span);
    EXPECT_EQ("PartitionClient.ReceiveEvents", span->GetName());
    EXPECT_EQ(Azure::Core::Tracing::_internal::SpanKind::Client, span->GetKind());

    std::map<std::string, std::string> const expectedAttributes{
        {"az.namespace", "Microsoft.EventHub"},
        {"messaging.system", "eventhubs"},
        {"messaging.destination.name", TestEventHubName},
        {"messaging.operation", "receive"},
        {"net.peer.name", TestFullyQualifiedNamespace},
    };
    EXPECT_EQ(expectedAttributes, span->GetAttributes());
    EXPECT_EQ(span->GetAttributes().end(), span->GetAttributes().find("server.address"));
    EXPECT_EQ(
        span->GetAttributes().end(), span->GetAttributes().find("messaging.batch.message_count"));

    _detail::SetMessageCount(factory, tracingContext.Span, 3);
    ASSERT_EQ(1u, span->GetAttributes().count("messaging.batch.message_count"));
    EXPECT_EQ("3", span->GetAttributes().at("messaging.batch.message_count"));
  }

  // The OpenTelemetry semantic conventions define messaging.batch.message_count as an int, so
  // the count must reach the span through a numeric overload.
  TEST(EventHubsTracingTest, ReceiveSpanMessageCountIsUnsignedInteger)
  {
    auto provider = std::make_shared<TestTracingProvider>();
    auto factory = _detail::CreateTracingContextFactory(provider);

    auto tracingContext = _detail::StartSpan(
        factory,
        "PartitionClient.ReceiveEvents",
        Azure::Core::Tracing::_internal::SpanKind::Client,
        "receive",
        TestEventHubName,
        TestFullyQualifiedNamespace,
        Azure::Nullable<size_t>{},
        Azure::Core::Context{});

    auto span = SingleSpan(provider);
    ASSERT_NE(nullptr, span);

    _detail::SetMessageCount(factory, tracingContext.Span, 3);

    ASSERT_EQ(1u, span->GetAttributes().count("messaging.batch.message_count"));
    EXPECT_EQ("3", span->GetAttributes().at("messaging.batch.message_count"));

    ASSERT_EQ(1u, span->GetAttributeTypes().count("messaging.batch.message_count"));
    EXPECT_EQ(AttributeTypeUInt64, span->GetAttributeTypes().at("messaging.batch.message_count"));
  }

  TEST(EventHubsTracingTest, SendSpanMessageCountIsUnsignedInteger)
  {
    auto provider = std::make_shared<TestTracingProvider>();
    ProducerClientOptions options;
    options.TracingProvider = provider;

    ProducerClient client{TestConnectionString, TestEventHubName, options};
    EventDataBatch batch{CreateTestBatch()};

    Azure::Core::Context cancelledContext;
    cancelledContext.Cancel();
    EXPECT_THROW(client.Send(batch, cancelledContext), Azure::Core::OperationCancelledException);

    auto span = SingleSpan(provider);
    ASSERT_NE(nullptr, span);

    ASSERT_EQ(1u, span->GetAttributes().count("messaging.batch.message_count"));
    EXPECT_EQ("2", span->GetAttributes().at("messaging.batch.message_count"));

    ASSERT_EQ(1u, span->GetAttributeTypes().count("messaging.batch.message_count"));
    EXPECT_EQ(AttributeTypeUInt64, span->GetAttributeTypes().at("messaging.batch.message_count"));
  }

  // Characterization test. A factory with no tracer returns a null attribute set, and that
  // pointer is not null safe. The message count path must stay quiet in that case.
  TEST(EventHubsTracingTest, SetMessageCountWithoutTracerIsSafe)
  {
    auto factory = _detail::CreateTracingContextFactory(nullptr);
    ASSERT_FALSE(factory.HasTracer());
    EXPECT_EQ(nullptr, factory.CreateAttributeSet());

    auto tracingContext = _detail::StartSpan(
        factory,
        "PartitionClient.ReceiveEvents",
        Azure::Core::Tracing::_internal::SpanKind::Client,
        "receive",
        TestEventHubName,
        TestFullyQualifiedNamespace,
        Azure::Nullable<size_t>{},
        Azure::Core::Context{});

    _detail::SetMessageCount(factory, tracingContext.Span, 3);
    SUCCEED();
  }

  // Characterization test. The producer must behave the same way when the caller supplies no
  // tracing provider.
  TEST(EventHubsTracingTest, SendWithoutProviderIsUnchanged)
  {
    ProducerClientOptions options;
    ProducerClient client{TestConnectionString, TestEventHubName, options};
    EXPECT_EQ(TestEventHubName, client.GetEventHubName());

    EventDataBatch batch{CreateTestBatch()};

    Azure::Core::Context cancelledContext;
    cancelledContext.Cancel();
    EXPECT_THROW(client.Send(batch, cancelledContext), Azure::Core::OperationCancelledException);

    EXPECT_THROW(client.Send(batch, Azure::Core::Context{}), std::runtime_error);
  }

}}}} // namespace Azure::Messaging::EventHubs::Test
