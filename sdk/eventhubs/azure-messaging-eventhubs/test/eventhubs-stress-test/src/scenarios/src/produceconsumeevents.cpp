// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "produceconsumeevents.hpp"

#include <opentelemetry/logs/provider.h>
#include <opentelemetry/sdk/logs/logger.h>
#include <opentelemetry/sdk/trace/tracer.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/semantic_conventions.h>

using namespace Azure::Messaging::EventHubs;

namespace trace_sdk = opentelemetry::sdk::trace;
namespace trace = opentelemetry::trace;
namespace logs_sdk = opentelemetry::sdk::logs;
namespace logs = opentelemetry::logs;

namespace {

auto GetLogger()
{
  auto logger{opentelemetry::logs::Provider::GetLoggerProvider()->GetLogger(EventHubsLoggerName)};
  return logger;
}

auto GetTracer()
{
  return opentelemetry::trace::Provider::GetTracerProvider()->GetTracer(EventHubsLoggerName);
}

auto CreateStressSpan(std::string const& name)
{
  auto tracer = GetTracer();
  trace::StartSpanOptions options;
  options.parent = tracer->GetCurrentSpan()->GetContext();
  options.kind = trace::SpanKind::kClient;
  auto newSpan = tracer->StartSpan(name, options);

  return newSpan;
}

} // namespace
ProduceConsumeEvents::ProduceConsumeEvents()
    : m_numberToSend{100}, m_batchSize{100}, m_prefetchCount{10}, m_messageBodySize{1024},
      m_rounds{10}
{
}

void ProduceConsumeEvents::Initialize()
{
  m_eventHubName = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_NAME");

  if (m_eventHubName.empty())
  {
    GetLogger()->Fatal("Could not find required environment variable EVENTHUB_NAME");
    std::cerr << "Missing required environment variable EVENTHUB_NAME" << std::endl;
  }

  m_eventHubConnectionString
      = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING");
  if (m_eventHubConnectionString.empty())
  {
    std::cerr << "Missing required environment variable EVENTHUB_CONNECTION_STRING" << std::endl;
    GetLogger()->Fatal("Could not find required environment variable EVENTHUB_NAME");
  }
  m_checkpointStoreConnectionString
      = Azure::Core::_internal::Environment::GetVariable("CHECKPOINT_STORE_CONNECTION_STRING");

  m_tenantId = Azure::Core::_internal::Environment::GetVariable("AZURE_TENANT_ID");
  m_clientId = Azure::Core::_internal::Environment::GetVariable("AZURE_CLIENT_ID");
  m_secret = Azure::Core::_internal::Environment::GetVariable("AZURE_CLIENT_SECRET");

  if (m_eventHubConnectionString.empty())
  {
    m_credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
        m_tenantId, m_clientId, m_secret);

    m_client = std::make_unique<Azure::Messaging::EventHubs::ProducerClient>(
        m_eventHubConnectionString, m_eventHubName, m_credential);
  }
  else
  {
    m_client = std::make_unique<Azure::Messaging::EventHubs::ProducerClient>(
        m_eventHubConnectionString, m_eventHubName);
  }
}

void ProduceConsumeEvents::Warmup(int repetitions)
{
  for (int i = 0; i < repetitions; i++)
  {
    auto span{CreateStressSpan("ProduceConsumeEvents::Warmup")};
    auto scope{trace::Tracer::WithActiveSpan(span)};
    span->AddEvent("Iteration Start", {{"iteration", i}});
    std::cout << "Warmup " << i << std::endl;
    SendMessages();
    ReceiveMessages();
    span->AddEvent("Iteration End", {{"iteration", i}});
  }
}
void ProduceConsumeEvents::Run(int repetitions)
{
  for (int i = 0; i < repetitions; i++)
  {
    std::cout << "Run " << i << std::endl;
    SendMessages();
    ReceiveMessages();
  }
}
void ProduceConsumeEvents::Cleanup() {}

void ProduceConsumeEvents::SendEventsToPartition(Azure::Core::Context const& context)
{
  auto span{CreateStressSpan("SendEventsToPartition")};
  auto scope{trace::Tracer::WithActiveSpan(span)};

  Azure::Messaging::EventHubs::Models::EventHubPartitionProperties beforeSendProps;
  {
    auto getPropertiesSpan{CreateStressSpan("SendEventsToPartition::GetPartitionProperties begin")};
    auto getPropertiesScope{trace::Tracer::WithActiveSpan(getPropertiesSpan)};
    beforeSendProps = m_client->GetPartitionProperties(m_partitionId, context);
  }
  std::vector<uint8_t> bodyData(m_messageBodySize, 'a');

  Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
  batchOptions.PartitionId = m_partitionId;
  Azure::Messaging::EventHubs::EventDataBatch batch(m_client->CreateBatch(batchOptions));
  for (uint32_t j = 0; j < m_numberToSend; ++j)
  {

    Azure::Messaging::EventHubs::Models::EventData event;
    event.Body = bodyData;
    event.Properties["Number"] = j;
    event.Properties["PartitionId"]
        = static_cast<Azure::Core::Amqp::Models::AmqpValue>(m_partitionId);
    AddEndProperty(event, m_numberToSend);
    batch.TryAddMessage(event);
  }

  {
    auto sendBatchSpan{CreateStressSpan("SendBatch")};
    auto sendBatchScope{trace::Tracer::WithActiveSpan(sendBatchSpan)};
    {
      span->AddEvent("Send events", {{"event count", m_numberToSend}});
      m_client->Send(batch, context);
    }
  }
  {
    auto getPartitionPropertiesSpan{CreateStressSpan("GetPartitionProperties")};
    auto getPartitionPropertiesScope{trace::Tracer::WithActiveSpan(getPartitionPropertiesSpan)};
    auto afterSendProps = m_client->GetPartitionProperties(m_partitionId, context);

    m_receiveStartPosition.Inclusive = false;
    m_receiveStartPosition.SequenceNumber = beforeSendProps.LastEnqueuedSequenceNumber;
    span->AddEvent(
        "After Properties", {{"sequenceNumber", beforeSendProps.LastEnqueuedSequenceNumber}});
  }
}

void ProduceConsumeEvents::AddEndProperty(
    Azure::Messaging::EventHubs::Models::EventData& event,
    uint64_t expectedCount)
{
  event.Properties["End"] = expectedCount;
}
void ProduceConsumeEvents::SendMessages()
{
  try
  {
    Azure::Core::Context context;
    SendEventsToPartition(context);
  }
  catch (std::exception const& ex)
  {
    GetTracer()->GetCurrentSpan()->AddEvent(
        "Exception received", {{trace::SemanticConventions::kExceptionMessage, ex.what()}});
    std::cerr << "Exception " << ex.what();
    throw;
  }
}
void ProduceConsumeEvents::ReceiveMessages()
{
  auto span{CreateStressSpan("ReceiveMessages")};
  auto scope{trace::Tracer::WithActiveSpan(span)};

  try
  {
    Azure::Core::Context context;
    ConsumerClientOptions clientOptions;
    clientOptions.ApplicationID = "StressConsumerClient";

    ConsumerClient consumerClient(
        m_eventHubConnectionString, m_eventHubName, DefaultConsumerGroup, clientOptions);

    {
      auto getPartitionPropertiesSpan{CreateStressSpan("SendEventsToPartition::GetPartitionProperties begin")};
      auto getPartitionPropertiesScope{trace::Tracer::WithActiveSpan(span)};

      auto consumerProperties = consumerClient.GetEventHubProperties(context);
    }

    std::cout << "Starting receive tests for partition " << m_partitionId << std::endl;
    std::cout << "  Start position: " << m_receiveStartPosition << std::endl;

    for (auto round = 0; round < m_rounds; round += 1)
    {
      ConsumeForBatchTester(round, consumerClient, m_receiveStartPosition, context);
    }
  }
  catch (std::exception const& ex)
  {
    GetTracer()->GetCurrentSpan()->AddEvent(
        "Exception received", {{trace::SemanticConventions::kExceptionMessage, ex.what()}});
    std::cerr << "Exception " << ex.what();
    throw;
  }
}
void ProduceConsumeEvents::ConsumeForBatchTester(
    uint32_t round,
    ConsumerClient& client,
    Models::StartPosition const& startPosition,
    Azure::Core::Context const& context)
{
  std::unique_ptr<PartitionClient> partitionClient;
  {
    auto span{CreateStressSpan("ConsumeForBatchTester::CreatePartitionClient")};
    auto scope{trace::Tracer::WithActiveSpan(span)};
    PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition = startPosition;
    partitionOptions.Prefetch = m_prefetchCount;
    partitionClient = std::make_unique<PartitionClient>(client.CreatePartitionClient(m_partitionId, partitionOptions));
    std::cout << "[r: " << round << "/" << m_rounds << "p: " << m_partitionId
              << "] Starting to receive messages from partition" << std::endl;
  }
  size_t total = 0;
  //      uint32_t numCancels = 0;
  //      constexpr const uint32_t cancelLimit = 5;

  {
    auto span{CreateStressSpan("ConsumeForBatchTester::ReceiveEvents")};
    auto scope{trace::Tracer::WithActiveSpan(span)};
    auto events = partitionClient->ReceiveEvents(m_batchSize, context);
    total += events.size();
  }
  std::cout << "Total: " << total << std::endl;
}

const std::string& ProduceConsumeEvents::GetStressScenarioName() { return m_scenarioName; }
