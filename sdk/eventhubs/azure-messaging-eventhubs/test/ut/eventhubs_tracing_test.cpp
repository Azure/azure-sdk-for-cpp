// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/private/eventhubs_diagnostics.hpp"
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

    auto span = FindSpan(provider, "ProducerClient.Send");
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

    auto span = FindSpan(provider, "ProducerClient.Send");
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

    auto span = FindSpan(provider, "ProducerClient.Send");
    ASSERT_NE(nullptr, span);
    EXPECT_EQ("ProducerClient.Send", span->GetName());

    ASSERT_EQ(1u, span->GetAttributes().count("messaging.batch.message_count"));
    EXPECT_EQ("0", span->GetAttributes().at("messaging.batch.message_count"));

    ASSERT_EQ(1u, span->GetEvents().size());
    EXPECT_FALSE(span->GetEvents()[0].empty());

    ASSERT_FALSE(span->GetStatuses().empty());
    EXPECT_EQ(Azure::Core::Tracing::_internal::SpanStatus::Error, span->GetStatuses().back());
  }

  // The convenience overloads build the batch first, and that opens the AMQP link. A bad host
  // fails there, before any batch exists, so the span must start before the batch.
  TEST(EventHubsTracingTest, SendEventSpanRecordsCreateBatchFailure)
  {
    auto provider = std::make_shared<TestTracingProvider>();
    ProducerClientOptions options;
    options.TracingProvider = provider;

    ProducerClient client{TestConnectionString, TestEventHubName, options};

    EXPECT_THROW(
        client.Send(Models::EventData{"Single message."}, Azure::Core::Context{}),
        std::runtime_error);

    auto span = FindSpan(provider, "ProducerClient.Send");
    ASSERT_NE(nullptr, span);
    EXPECT_EQ("ProducerClient.Send", span->GetName());
    EXPECT_EQ(Azure::Core::Tracing::_internal::SpanKind::Producer, span->GetKind());

    EXPECT_EQ(
        span->GetAttributes().end(), span->GetAttributes().find("messaging.batch.message_count"));

    ASSERT_EQ(1u, span->GetEvents().size());
    EXPECT_FALSE(span->GetEvents()[0].empty());

    ASSERT_FALSE(span->GetStatuses().empty());
    EXPECT_EQ(Azure::Core::Tracing::_internal::SpanStatus::Error, span->GetStatuses().back());
  }

  TEST(EventHubsTracingTest, SendEventVectorSpanRecordsCreateBatchFailure)
  {
    auto provider = std::make_shared<TestTracingProvider>();
    ProducerClientOptions options;
    options.TracingProvider = provider;

    ProducerClient client{TestConnectionString, TestEventHubName, options};

    std::vector<Models::EventData> const eventData{
        Models::EventData{"First message."},
        Models::EventData{"Second message."},
        Models::EventData{"Third message."},
    };

    EXPECT_THROW(client.Send(eventData, Azure::Core::Context{}), std::runtime_error);

    auto span = FindSpan(provider, "ProducerClient.Send");
    ASSERT_NE(nullptr, span);
    EXPECT_EQ("ProducerClient.Send", span->GetName());
    EXPECT_EQ(Azure::Core::Tracing::_internal::SpanKind::Producer, span->GetKind());

    ASSERT_EQ(1u, span->GetAttributes().count("messaging.batch.message_count"));
    EXPECT_EQ("3", span->GetAttributes().at("messaging.batch.message_count"));

    ASSERT_EQ(1u, span->GetAttributeTypes().count("messaging.batch.message_count"));
    EXPECT_EQ(AttributeTypeUInt64, span->GetAttributeTypes().at("messaging.batch.message_count"));

    ASSERT_EQ(1u, span->GetEvents().size());
    EXPECT_FALSE(span->GetEvents()[0].empty());

    ASSERT_FALSE(span->GetStatuses().empty());
    EXPECT_EQ(Azure::Core::Tracing::_internal::SpanStatus::Error, span->GetStatuses().back());
  }

  // Characterization test. The convenience overloads must behave the same way when the caller
  // supplies no tracing provider.
  TEST(EventHubsTracingTest, SendEventWithoutProviderIsUnchanged)
  {
    ProducerClientOptions options;
    ProducerClient client{TestConnectionString, TestEventHubName, options};

    EXPECT_THROW(
        client.Send(Models::EventData{"Single message."}, Azure::Core::Context{}),
        std::runtime_error);

    std::vector<Models::EventData> const eventData{
        Models::EventData{"First message."},
        Models::EventData{"Second message."},
        Models::EventData{"Third message."},
    };
    EXPECT_THROW(client.Send(eventData, Azure::Core::Context{}), std::runtime_error);
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
        Azure::Core::Context{},
        _detail::MessagingEntityKind::Source);

    auto span = FindSpan(provider, "PartitionClient.ReceiveEvents");
    ASSERT_NE(nullptr, span);
    EXPECT_EQ("PartitionClient.ReceiveEvents", span->GetName());
    EXPECT_EQ(Azure::Core::Tracing::_internal::SpanKind::Client, span->GetKind());

    std::map<std::string, std::string> const expectedAttributes{
        {"az.namespace", "Microsoft.EventHub"},
        {"messaging.system", "eventhubs"},
        {"messaging.source.name", TestEventHubName},
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

    auto span = FindSpan(provider, "PartitionClient.ReceiveEvents");
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

    auto span = FindSpan(provider, "ProducerClient.Send");
    ASSERT_NE(nullptr, span);

    ASSERT_EQ(1u, span->GetAttributes().count("messaging.batch.message_count"));
    EXPECT_EQ("2", span->GetAttributes().at("messaging.batch.message_count"));

    ASSERT_EQ(1u, span->GetAttributeTypes().count("messaging.batch.message_count"));
    EXPECT_EQ(AttributeTypeUInt64, span->GetAttributeTypes().at("messaging.batch.message_count"));
  }

  TEST(EventHubsTracingTest, AmqpSpanIdentifiesComponentAndAttempt)
  {
    auto provider = std::make_shared<TestTracingProvider>();
    auto factory = _detail::CreateTracingContextFactory(provider);
    auto operationContext = _detail::StartSpan(
        factory,
        "ProducerClient.Send",
        Azure::Core::Tracing::_internal::SpanKind::Producer,
        "publish",
        TestEventHubName,
        TestFullyQualifiedNamespace,
        size_t{2},
        Azure::Core::Context{});

    _detail::AmqpDiagnosticsContext diagnosticsContext{
        "producer:test-client", "2", "link", "sender-link", 3};
    auto amqpContext = _detail::StartAmqpSpan(
        factory,
        "ProducerClient.AmqpSend",
        "publish",
        TestEventHubName,
        TestFullyQualifiedNamespace,
        size_t{2},
        diagnosticsContext,
        std::uint64_t{4},
        operationContext.Context);

    auto span = FindSpan(provider, "ProducerClient.AmqpSend");
    ASSERT_NE(nullptr, span);
    EXPECT_TRUE(span->HasParent());
    EXPECT_EQ(Azure::Core::Tracing::_internal::SpanKind::Client, span->GetKind());
    EXPECT_EQ("producer:test-client", span->GetAttributes().at("az.eventhubs.client.id"));
    EXPECT_EQ("2", span->GetAttributes().at("az.eventhubs.partition.id"));
    EXPECT_EQ("link", span->GetAttributes().at("az.eventhubs.amqp.component.type"));
    EXPECT_EQ("sender-link", span->GetAttributes().at("az.eventhubs.amqp.component.name"));
    EXPECT_EQ(
        "producer:test-client/partition/2/generation/3/link",
        span->GetAttributes().at("az.eventhubs.amqp.component.id"));
    EXPECT_EQ("3", span->GetAttributes().at("az.eventhubs.amqp.component.generation"));
    EXPECT_EQ("4", span->GetAttributes().at("az.eventhubs.retry.attempt"));
    EXPECT_EQ(
        AttributeTypeUInt64,
        span->GetAttributeTypes().at("az.eventhubs.amqp.component.generation"));
    EXPECT_EQ(AttributeTypeUInt64, span->GetAttributeTypes().at("az.eventhubs.retry.attempt"));
  }

  TEST(EventHubsTracingTest, LifecycleLogIdentifiesGatewayComponent)
  {
    _detail::AmqpDiagnosticsContext diagnosticsContext{
        "producer:test-client", "", "connection", "connection-1", 7};

    EXPECT_EQ(
        "Event Hubs AMQP lifecycle: event='create_failed' client.id='producer:test-client' "
        "partition.id='<gateway>' component.type='connection' component.name='connection-1' "
        "component.id='producer:test-client/partition/<gateway>/generation/7/connection' "
        "component.generation=7 detail='socket closed'",
        _detail::FormatAmqpLifecycleEvent(diagnosticsContext, "create_failed", "socket closed"));
  }

  TEST(EventHubsTracingTest, LifecycleLogEscapesQuotedAndMultilineValues)
  {
    _detail::AmqpDiagnosticsContext diagnosticsContext{
        "producer:client's", "2", "link", "sender\\link", 1};

    EXPECT_NE(
        std::string::npos,
        _detail::FormatAmqpLifecycleEvent(
            diagnosticsContext, "failed", "can't send\nconnection closed")
            .find("client.id='producer:client\\'s'"));
    EXPECT_NE(
        std::string::npos,
        _detail::FormatAmqpLifecycleEvent(
            diagnosticsContext, "failed", "can't send\nconnection closed")
            .find("detail='can\\'t send\\nconnection closed'"));
  }

  TEST(EventHubsTracingTest, CloseAmqpComponentLogsClosedAfterSuccessfulClose)
  {
    _detail::AmqpDiagnosticsContext diagnosticsContext{
        "producer:test-client", "2", "link", "sender-link", 7};
    std::vector<std::string> events;
    auto log = [&events](
                   Azure::Core::Diagnostics::Logger::Level,
                   _detail::AmqpDiagnosticsContext const&,
                   std::string const& eventName,
                   std::string const&) { events.push_back(eventName); };

    _detail::CloseAmqpComponent(
        diagnosticsContext, false, {}, [&events]() { events.push_back("close"); }, log);

    EXPECT_EQ((std::vector<std::string>{"closing", "close", "closed"}), events);
  }

  TEST(EventHubsTracingTest, CloseAmqpComponentLogsFailureWithoutClosed)
  {
    _detail::AmqpDiagnosticsContext diagnosticsContext{
        "producer:test-client", "2", "link", "sender-link", 7};
    std::vector<std::string> events;
    auto log = [&events](
                   Azure::Core::Diagnostics::Logger::Level,
                   _detail::AmqpDiagnosticsContext const&,
                   std::string const& eventName,
                   std::string const&) { events.push_back(eventName); };

    _detail::CloseAmqpComponent(
        diagnosticsContext,
        false,
        {},
        [&events]() {
          events.push_back("close");
          throw std::runtime_error("socket closed");
        },
        log);

    EXPECT_EQ((std::vector<std::string>{"closing", "close", "close_failed"}), events);
  }

  TEST(EventHubsTracingTest, ClientIdentifiersAreUniqueWithTheSameConfiguredName)
  {
    auto const first = _detail::CreateClientIdentifier("producer", "orders");
    auto const second = _detail::CreateClientIdentifier("producer", "orders");

    EXPECT_NE(first, second);
    EXPECT_EQ(0u, first.find("producer:orders:"));
    EXPECT_EQ(0u, second.find("producer:orders:"));
  }

  TEST(EventHubsTracingTest, FailureComponentTypeUsesAmqpErrorScope)
  {
    EXPECT_EQ("connection", _detail::GetAmqpFailureComponentType("amqp:connection:forced"));
    EXPECT_EQ("session", _detail::GetAmqpFailureComponentType("amqp:session:window-violation"));
    EXPECT_EQ("link", _detail::GetAmqpFailureComponentType("amqp:link:detach-forced"));
    EXPECT_EQ("link", _detail::GetAmqpFailureComponentType("amqp:unauthorized-access"));
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
