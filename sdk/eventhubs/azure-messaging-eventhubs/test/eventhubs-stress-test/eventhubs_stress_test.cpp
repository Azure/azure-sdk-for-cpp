// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @brief Validates the Azure Core transport adapters with fault responses from server.
 *
 * @note This test requires the Http-fault-injector
 * (https://github.com/Azure/azure-sdk-tools/tree/main/tools/http-fault-injector) running. Follow
 * the instructions to install and run the server before running this test.
 *
 */

#define REQUESTS 100
#define WARMUP 100
#define ROUNDS 100

#include <azure/core.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/identity/client_secret_credential.hpp>
#include <azure/messaging/eventhubs/consumer_client.hpp>
#include <azure/messaging/eventhubs/producer_client.hpp>

#include <iostream>

using namespace Azure::Messaging::EventHubs;

class EventHubsStress {
public:
  EventHubsStress()
  {
    m_eventHubName = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_NAME");
    m_eventHubConnectionString
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING");
    m_checkpointStoreConnectionString
        = Azure::Core::_internal::Environment::GetVariable("CHECKPOINT_STORE_CONNECTION_STRING");

    m_numberToSend = 100;
    m_batchSize = 100;
    m_prefetchCount = 10;
    m_messageBodySize = 1024;

    m_tenantId = Azure::Core::_internal::Environment::GetVariable("AZURE_TENANT_ID");
    m_clientId = Azure::Core::_internal::Environment::GetVariable("AZURE_CLIENT_ID");
    m_secret = Azure::Core::_internal::Environment::GetVariable("AZURE_CLIENT_SECRET");

    ProducerClientOptions clientOptions;
    clientOptions.VerboseLogging = m_verboseClient;

    if (m_eventHubConnectionString.empty())
    {
      m_credential = std::make_shared<Azure::Identity::ClientSecretCredential>(
          m_tenantId, m_clientId, m_secret);

      m_client = std::make_unique<Azure::Messaging::EventHubs::ProducerClient>(
          m_eventHubConnectionString, m_eventHubName, m_credential, clientOptions);
    }
    else
    {
      m_client = std::make_unique<Azure::Messaging::EventHubs::ProducerClient>(
          m_eventHubConnectionString, m_eventHubName, clientOptions);
    }
  }

  void Warmup(int repetitions)
  {
    for (int i = 0; i < repetitions; i++)
    {
      std::cout << "Warmup " << i << std::endl;
      SendMessages();
      ReceiveMessages();
    }
  }
  void Run(int repetitions)
  {
    for (int i = 0; i < repetitions; i++)
    {
      std::cout << "Run " << i << std::endl;
      SendMessages();
      ReceiveMessages();
    }
  }
  void Cleanup() {}

private:
  std::string m_eventHubName;
  std::string m_eventHubConnectionString;
  std::string m_checkpointStoreConnectionString;
  std::string m_partitionId{"0"};
  bool m_verboseClient{true};

  std::string m_tenantId;
  std::string m_clientId;
  std::string m_secret;

  uint32_t m_numberToSend;
  uint32_t m_batchSize;
  uint32_t m_prefetchCount;
  size_t m_messageBodySize;

  int m_rounds{10};

  std::unique_ptr<Azure::Messaging::EventHubs::ProducerClient> m_client;
  std::shared_ptr<Azure::Core::Credentials::TokenCredential> m_credential;

  Models::StartPosition m_receiveStartPosition;

  void SendEventsToPartition(Azure::Core::Context const& context)
  {
    auto beforeSendProps = m_client->GetPartitionProperties(m_partitionId, context);
    std::vector<uint8_t> bodyData(m_messageBodySize, 'a');

    Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
    batchOptions.PartitionID = m_partitionId;
    Azure::Messaging::EventHubs::EventDataBatch batch(batchOptions);
    for (uint32_t j = 0; j < m_numberToSend; ++j)
    {

      Azure::Messaging::EventHubs::Models::EventData event;
      event.Body.Data = bodyData;
      event.Properties["Number"] = j;
      event.Properties["PartitionId"]
          = static_cast<Azure::Core::Amqp::Models::AmqpValue>(m_partitionId);
      AddEndProperty(event, m_numberToSend);
      batch.AddMessage(event);
    }
    m_client->SendEventDataBatch(batch, context);

    auto afterSendProps = m_client->GetPartitionProperties(m_partitionId, context);

    m_receiveStartPosition.Inclusive = false;
    m_receiveStartPosition.SequenceNumber = beforeSendProps.LastEnqueuedSequenceNumber;
  }

  void AddEndProperty(Azure::Messaging::EventHubs::Models::EventData& event, uint64_t expectedCount)
  {
    event.Properties["End"] = expectedCount;
  }
  void SendMessages()
  {
    try
    {
      Azure::Core::Context context;
      SendEventsToPartition(context);
    }
    catch (std::exception const& ex)
    {
      std::cerr << "Exception " << ex.what();
      throw;
    }
  }
  void ReceiveMessages()
  {

    try
    {
      Azure::Core::Context context;
      ConsumerClientOptions clientOptions;
      clientOptions.ApplicationID = "StressConsumerClient";
      clientOptions.VerboseLogging = m_verboseClient;

      ConsumerClient consumerClient(
          m_eventHubConnectionString, m_eventHubName, DefaultConsumerGroup, clientOptions);

      auto consumerProperties = consumerClient.GetEventHubProperties(context);

      std::cout << "Starting receive tests for partition " << m_partitionId << std::endl;
      std::cout << "  Start position: " << m_receiveStartPosition << std::endl;

      for (auto round = 0; round < m_rounds; round += 1)
      {
        ConsumeForBatchTester(round, consumerClient, m_receiveStartPosition, context);
      }
    }
    catch (std::exception const& ex)
    {
      std::cerr << "Exception " << ex.what();
      throw;
    }
  }
  void ConsumeForBatchTester(
      uint32_t round,
      ConsumerClient& client,
      Models::StartPosition const& startPosition,
      Azure::Core::Context const& context)
  {

    PartitionClientOptions partitionOptions;
    partitionOptions.StartPosition = startPosition;
    partitionOptions.Prefetch = m_prefetchCount;
    PartitionClient partitionClient{client.CreatePartitionClient(m_partitionId, partitionOptions)};
    std::cout << "[r: " << round << "/" << m_rounds << "p: " << m_partitionId
              << "] Starting to receive messages from partition" << std::endl;

    size_t total = 0;
    //      uint32_t numCancels = 0;
    //      constexpr const uint32_t cancelLimit = 5;

    auto events = partitionClient.ReceiveEvents(m_batchSize, context);
    total += events.size();
  }
};

int main(int argc, char**)
{
  try
  {

    EventHubsStress stressTest;
    // some param was passed to the program, doesn't matter what it is,
    // it is meant for the moment to just run a quick iteration to check for sanity of the test.
    // since prototype TODO: pass in warmup/rounds/requests as params.
    if (argc != 1)
    {
      std::cout << "--------------\tBUILD TEST\t--------------" << std::endl;
      stressTest.Warmup(1);
      stressTest.Run(5);
      stressTest.Cleanup();
      std::cout << "--------------\tEND BUILD TEST\t--------------" << std::endl;
      return 0;
    }

    std::cout << "--------------\tSTARTING TEST\t--------------" << std::endl;
    std::cout << "--------------\tPRE WARMUP\t--------------" << std::endl;

    stressTest.Warmup(WARMUP);

    std::cout << "--------------\tPOST WARMUP\t--------------" << std::endl;

    for (int i = 0; i < ROUNDS; i++)
    {
      std::cout << "--------------\tTEST ITERATION:" << i << "\t--------------" << std::endl;

      stressTest.Run(REQUESTS);

      std::cout << "--------------\tDONE ITERATION:" << i << "\t--------------" << std::endl;
    }

    stressTest.Cleanup();
  }
  catch (std::exception const& ex)
  {
    std::cerr << "Test failed due to exception thrown: " << ex.what() << std::endl;
  }
  return 0;
}
