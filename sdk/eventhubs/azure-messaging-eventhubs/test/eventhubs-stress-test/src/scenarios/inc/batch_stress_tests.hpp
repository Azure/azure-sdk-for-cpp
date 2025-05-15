// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "eventhubs_stress_scenarios.hpp"

#include <azure/core/internal/environment.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/messaging/eventhubs/consumer_client.hpp>
#include <azure/messaging/eventhubs/producer_client.hpp>

#if defined(_MSC_VER)
// The OpenTelemetry headers generate a couple of warnings on MSVC in the OTel 1.2 package, suppress
// the warnings across the includes.
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4244)
#pragma warning(disable : 4996)
#pragma warning(disable : 6323) // Disable "Use of arithmetic operator on Boolean type" warning.
#endif

#include <opentelemetry/trace/tracer.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

class BatchStressTest : public EventHubsStressScenario {
public:
  BatchStressTest();

private:
  std::string m_eventHubName;
  std::string m_eventHubNamespace;
  std::string m_checkpointStoreConnectionString;
  std::string m_partitionId{"0"};

  std::string m_tenantId;
  std::string m_clientId;
  std::string m_secret;

  uint32_t m_numberToSend{10000000};
  uint32_t m_batchSize{1000};
  std::chrono::system_clock::duration m_batchDuration{std::chrono::seconds(60)};
  uint32_t m_prefetchCount{0};
  std::uint32_t m_rounds{100};
  std::uint32_t m_paddingBytes{1024};
  std::uint32_t m_maxTimeouts{10};
  bool m_verbose{false};
  std::function<void(Azure::Core::Context)> m_sleepAfterFunction;

  std::string m_scenarioName{"BatchStressTest"};

  void SendEventsToPartition(
      std::unique_ptr<Azure::Messaging::EventHubs::ProducerClient> const& producerClient,
      Azure::Core::Context const& context);

  void AddEndProperty(
      Azure::Messaging::EventHubs::Models::EventData& event,
      uint64_t expectedCount);
  std::pair<
      Azure::Messaging::EventHubs::Models::StartPosition,
      Azure::Messaging::EventHubs::Models::EventHubPartitionProperties>
  SendMessages();
  void ReceiveMessages(Azure::Messaging::EventHubs::Models::StartPosition const& startPosition);
  void ConsumeForBatchTester(
      uint32_t round,
      Azure::Messaging::EventHubs::ConsumerClient& client,
      Azure::Messaging::EventHubs::Models::StartPosition const& startPosition,
      Azure::Core::Context const& context) const;

  // Inherited via EventHubsStressScenario
  const std::string& GetStressScenarioName() override;
  const std::vector<EventHubsScenarioOptions>& GetScenarioOptions() override;
  void Initialize(argagg::parser_results const& parserResults) override;

  void Run() override;
  void Cleanup() override;
};
