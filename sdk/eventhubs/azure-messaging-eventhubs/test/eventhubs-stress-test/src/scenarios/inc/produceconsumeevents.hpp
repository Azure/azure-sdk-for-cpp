// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/core/internal/environment.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/messaging/eventhubs/consumer_client.hpp>
#include <azure/messaging/eventhubs/producer_client.hpp>
#include "eventhubs_stress_scenarios.hpp"

#include <opentelemetry/trace/tracer.h>

class ProduceConsumeEvents: public EventHubsStressScenario {
public:
  ProduceConsumeEvents();

private:
  std::string m_eventHubName;
  std::string m_eventHubConnectionString;
  std::string m_checkpointStoreConnectionString;
  std::string m_partitionId{"0"};

  std::string m_tenantId;
  std::string m_clientId;
  std::string m_secret;

  uint32_t m_numberToSend;
  uint32_t m_batchSize;
  uint32_t m_prefetchCount;
  size_t m_messageBodySize;

  int m_rounds{10};
  std::string m_scenarioName{"produceconsumeevents"};

  std::unique_ptr<Azure::Messaging::EventHubs::ProducerClient> m_client;
  std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;

  Azure::Messaging::EventHubs::Models::StartPosition m_receiveStartPosition;

  void SendEventsToPartition(Azure::Core::Context const& context);

  void AddEndProperty(
      Azure::Messaging::EventHubs::Models::EventData& event,
      uint64_t expectedCount);
  void SendMessages();
  void ReceiveMessages();
  void ConsumeForBatchTester(
      uint32_t round,
      Azure::Messaging::EventHubs::ConsumerClient& client,
      Azure::Messaging::EventHubs::Models::StartPosition const& startPosition,
      Azure::Core::Context const& context);

  // Inherited via EventHubsStressScenario
  const std::string& GetStressScenarioName() override;
  void Initialize() override;

  void Warmup(int repetitions) override;
  void Run(int repetitions) override;
  void Cleanup() override;
};
