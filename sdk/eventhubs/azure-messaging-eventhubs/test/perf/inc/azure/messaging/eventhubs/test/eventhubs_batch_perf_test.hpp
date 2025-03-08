// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

/**
 * @file
 * @brief Test batch sends
 *
 */

#pragma once

#include <azure/core/internal/environment.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs/consumer_client.hpp>
#include <azure/messaging/eventhubs/models/partition_client_models.hpp>
#include <azure/messaging/eventhubs/producer_client.hpp>
#include <azure/perf.hpp>

#include <memory>
#include <string>
#include <vector>

namespace Azure { namespace Messaging { namespace EventHubs { namespace PerfTest { namespace Batch {

  /**
   * @brief A test to measure getting a key performance.
   *
   */
  class BatchTest : public Azure::Perf::PerfTest {
  private:
    std::string m_eventHubName;
    std::string m_eventHubConnectionString;
    std::string m_partitionId;
    std::string m_checkpointStoreConnectionString;
    uint32_t m_numberToSend;
    uint32_t m_batchSize;
    uint32_t m_prefetchCount;
    uint64_t m_rounds;
    uint32_t m_paddingBytes{};
    uint32_t m_maxDeadlineExceeded{};

    std::shared_ptr<Azure::Core::Credentials::TokenCredential const> m_credential;
    std::unique_ptr<Azure::Messaging::EventHubs::ProducerClient> m_client;

  public:
    /**
     * @brief Get the Ids and secret
     *
     */
    void Setup() override
    {
      m_eventHubName = m_options.GetOptionOrDefault<std::string>(
          "EventHubName", Azure::Core::_internal::Environment::GetVariable("EVENTHUB_NAME"));
      m_eventHubConnectionString = m_options.GetOptionOrDefault<std::string>(
          "EventHubConnectionString",
          Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING"));
      m_checkpointStoreConnectionString = m_options.GetOptionOrDefault<std::string>(
          "CheckpointStoreConnectionString",
          Azure::Core::_internal::Environment::GetVariable("CHECKPOINT_STORE_CONNECTION_STRING"));

      m_numberToSend = m_options.GetOptionOrDefault<uint32_t>("NumberToSend", 1000);
      m_batchSize = m_options.GetOptionOrDefault<uint32_t>("BatchSize", 1000);
      m_prefetchCount = m_options.GetOptionOrDefault<uint32_t>("PrefetchCount", 1000);
      m_rounds = m_options.GetOptionOrDefault<uint64_t>("Rounds", 100);
      m_paddingBytes = m_options.GetOptionOrDefault<uint32_t>("PaddingBytes", 1024);
      m_partitionId = m_options.GetOptionOrDefault<std::string>("PartitionId", "0");
      m_maxDeadlineExceeded = m_options.GetOptionOrDefault<uint32_t>("MaxTimeouts", 10);

      if (m_eventHubConnectionString.empty())
      {
        m_credential = GetTestCredential();

        m_client = std::make_unique<Azure::Messaging::EventHubs::ProducerClient>(
            m_eventHubConnectionString, m_eventHubName, m_credential);
      }
      else
      {
        m_client = std::make_unique<Azure::Messaging::EventHubs::ProducerClient>(
            m_eventHubConnectionString, m_eventHubName);
      }
    }

    /**
     * @brief Construct a new Event Hubs performance test.
     *
     * @param options The test options.
     */
    BatchTest(Azure::Perf::TestOptions options) : PerfTest(options) {}

    /**
     * @brief Define the test
     *
     */
    void Run(Azure::Core::Context const& context) override
    {
      try
      {
        std::cout << "Starting test with: batch size: " << m_batchSize
                  << " Prefetch: " << m_prefetchCount << std::endl;

        // Warm up the connection to the remote instance.

        auto properties = m_client->GetEventHubProperties(context);

        std::cout << "Sending messages to partition " << m_partitionId << std::endl;
        std::tuple<Models::StartPosition, Models::EventHubPartitionProperties> sendResult
            = SendEventsToPartition(context);

        ConsumerClientOptions clientOptions;
        clientOptions.ApplicationID = "StressConsumerClient";

        ConsumerClient consumerClient(
            m_eventHubConnectionString, m_eventHubName, DefaultConsumerGroup, clientOptions);

        auto consumerProperties = consumerClient.GetEventHubProperties(context);

        std::cout << "Starting receive tests for partition " << m_partitionId << std::endl;
        std::cout << "  Start position: " << std::get<0>(sendResult) << std::endl;
        std::cout << "  Partition properties: " << std::get<1>(sendResult) << std::endl;

        for (auto i = 0ul; i < m_rounds; i += 1)
        {
          ConsumeForBatchTester(i, consumerClient, std::get<0>(sendResult), context);
        }
      }
      catch (std::exception const& ex)
      {
        std::cerr << "Exception thrown processing batch: " << ex.what() << std::endl;
      }
    }

    std::tuple<Models::StartPosition, Models::EventHubPartitionProperties> SendEventsToPartition(
        Core::Context const& context)
    {
      auto beforeSendProps = m_client->GetPartitionProperties(m_partitionId, context);
      std::vector<uint8_t> bodyData(m_paddingBytes, 'a');

      Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
      batchOptions.PartitionId = m_partitionId;
      Azure::Messaging::EventHubs::EventDataBatch batch{m_client->CreateBatch(batchOptions)};
      for (uint32_t j = 0; j < m_numberToSend; ++j)
      {

        Azure::Messaging::EventHubs::Models::EventData event;
        event.Body = bodyData;
        event.Properties["Number"] = j;
        event.Properties["PartitionId"]
            = static_cast<Azure::Core::Amqp::Models::AmqpValue>(m_partitionId);
        AddEndProperty(event, m_numberToSend);
        if (!batch.TryAdd(event))
        {
          throw std::runtime_error("Could not add message to batch.");
        }
      }
      m_client->Send(batch, context);

      auto afterSendProps = m_client->GetPartitionProperties(m_partitionId, context);

      Models::StartPosition startPosition;
      startPosition.Inclusive = false;
      startPosition.SequenceNumber = beforeSendProps.LastEnqueuedSequenceNumber;
      return std::make_tuple(startPosition, afterSendProps);
    }

    void ConsumeForBatchTester(
        uint32_t round,
        ConsumerClient& client,
        Models::StartPosition const& startPosition,
        Core::Context const& context)
    {

      PartitionClientOptions partitionOptions;
      partitionOptions.StartPosition = startPosition;
      partitionOptions.Prefetch = m_prefetchCount;
      PartitionClient partitionClient{
          client.CreatePartitionClient(m_partitionId, partitionOptions)};
      std::cout << "[r: " << round << "/" << m_rounds << "p: " << m_partitionId
                << "] Starting to receive messages from partition" << std::endl;

      size_t total = 0;
      //      uint32_t numCancels = 0;
      //      constexpr const uint32_t cancelLimit = 5;

      auto events = partitionClient.ReceiveEvents(m_batchSize, context);
      total += events.size();
      std::cout << "[r: " << round << "/" << m_rounds << "p: " << m_partitionId
                << "] Received: " << total << " messages" << std::endl;
    }

    void AddEndProperty(
        Azure::Messaging::EventHubs::Models::EventData& event,
        uint64_t expectedCount)
    {
      event.Properties["End"] = expectedCount;
    }

    /**
     * @brief Define the test options for the test.
     *
     * @return The list of test options.
     */
    std::vector<Azure::Perf::TestOption> GetTestOptions() override
    {
      return {
          {"EventHubName", {"--eventHubName"}, "The EventHub name.", 1, false},
          {"EventHubConnectionString",
           {"--eventHubConnectionString"},
           "The EventHub connection string.",
           1,
           false,
           true},
          {"CheckpointStoreConnectionString",
           {"--checkpointStoreConnectionString"},
           "The checkpoint store connection string.",
           1,
           false},
          {"NumberToSend", {"--numberToSend"}, "The number of events to send.", 1, false},
          {"BatchSize",
           {"--batchSize"},
           "Size to request each time we call ReceiveEvents(). Higher batch sizes will require "
           "higher amounts of memory for this test.",
           1,
           false},
          {"Timeout", {"--timeout"}, "Time to wait for each batch (ie. 1m, 30s, etc...)", 1, false},
          {"PrefetchCount",
           {"--prefetchCount"},
           "The number of events to set for the prefetch. Negative numbers disable prefetch "
           "altogether. 0 uses the default for the package.",
           1,
           false},
          {"Rounds",
           {"--rounds"},
           "The number of rounds to run with these parameters. -1 means MAX_UINT64.",
           1,
           false},
          {"PaddingBytes",
           {"--paddingBytes"},
           "The number of bytes to send in each message body.",
           1,
           false},
          {"partitionId",
           {"--partitionId"},
           "The partition Id to send and receive events to.",
           1,
           false},
          {"MaxTimeouts", {"--maxTimeouts"}, "The max number of timeouts.", 1, false},
          {"TenantId", {"--tenantId"}, "The tenant Id for the authentication.", 1, false},
          {"ClientId", {"--clientId"}, "The client Id for the authentication.", 1, false},
          {"Secret", {"--secret"}, "The secret for authentication.", 1, false, true}};
    }

    /**
     * @brief Get the static Test Metadata for the test.
     *
     * @return Azure::Perf::TestMetadata describing the test.
     */
    static Azure::Perf::TestMetadata GetTestMetadata()
    {
      return {"Batch", "Batch Processing", [](Azure::Perf::TestOptions options) {
                return std::make_unique<Azure::Messaging::EventHubs::PerfTest::Batch::BatchTest>(
                    options);
              }};
    }
  };

}}}}} // namespace Azure::Messaging::EventHubs::PerfTest::Batch
