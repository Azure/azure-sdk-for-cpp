// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/private/eventhubs_tracing.hpp"
#include "../src/private/eventhubs_utilities.hpp"
#include "../src/private/package_version.hpp"
#include "eventhubs_tracing_test_doubles.hpp"

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
