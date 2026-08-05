// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "eventhubs_test_base.hpp"

#include <azure/core/internal/environment.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <chrono>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  namespace {
    // The keys below are fake base64 text. cspell tokenizes base64 into fragments that are not
    // words, so spell checking is disabled across this block.
    // cspell: disable
    constexpr const char* ConnectionStringNoEntityPath
        = "Endpoint=sb://fake.servicebus.windows.net/;SharedAccessKeyName=FakeKey;"
          "SharedAccessKey=ZmFrZWtleWZha2VrZXlmYWtla2V5ZmFrZWtleQ==";
    constexpr const char* ConnectionStringWithEntityPath
        = "Endpoint=sb://fake.servicebus.windows.net/;SharedAccessKeyName=FakeKey;"
          "SharedAccessKey=ZmFrZWtleWZha2VrZXlmYWtla2V5ZmFrZWtleQ==;EntityPath=eventhub1";
    constexpr const char* EmulatorConnectionString
        = "Endpoint=sb://localhost;SharedAccessKeyName=RootManageSharedAccessKey;"
          "SharedAccessKey=U0FTX0tFWV9WQUxVRQ==;UseDevelopmentEmulator=true;";
    constexpr const char* EmulatorTlsPortConnectionString
        = "Endpoint=sb://localhost:5671;SharedAccessKeyName=RootManageSharedAccessKey;"
          "SharedAccessKey=U0FTX0tFWV9WQUxVRQ==;UseDevelopmentEmulator=true;";
    // cspell: enable
  } // namespace

#if ENABLE_UAMQP
  TEST(ConnectionStringClientTest, ProducerUsesExplicitEventHubWithoutEntityPath)
  {
    ProducerClient client(ConnectionStringNoEntityPath, "eventhub1");
    EXPECT_EQ("eventhub1", client.GetEventHubName());
  }

  TEST(ConnectionStringClientTest, ProducerUsesEntityPathWhenEventHubIsEmpty)
  {
    ProducerClient client(ConnectionStringWithEntityPath, "");
    EXPECT_EQ("eventhub1", client.GetEventHubName());
  }

  TEST(ConnectionStringClientTest, ProducerAcceptsMatchingEntityPath)
  {
    ProducerClient client(ConnectionStringWithEntityPath, "eventhub1");
    EXPECT_EQ("eventhub1", client.GetEventHubName());
  }

  TEST(ConnectionStringClientTest, ProducerRejectsMismatchedEntityPath)
  {
    EXPECT_THROW(
        { ProducerClient client(ConnectionStringWithEntityPath, "eventhub2"); },
        std::invalid_argument);
  }

  TEST(ConnectionStringClientTest, ProducerRejectsMissingEventHub)
  {
    EXPECT_THROW(
        { ProducerClient client(ConnectionStringNoEntityPath, ""); }, std::invalid_argument);
  }

  TEST(ConnectionStringClientTest, ConsumerUsesExplicitEventHubWithoutEntityPath)
  {
    ConsumerClient client(ConnectionStringNoEntityPath, "eventhub1", "consumer-group");
    EXPECT_EQ("eventhub1", client.GetEventHubName());
    EXPECT_EQ("consumer-group", client.GetConsumerGroup());
    EXPECT_EQ("fake.servicebus.windows.net", client.GetDetails().FullyQualifiedNamespace);
  }

  TEST(ConnectionStringClientTest, ConsumerUsesEntityPathAndDefaultConsumerGroup)
  {
    ConsumerClient client(ConnectionStringWithEntityPath);
    EXPECT_EQ("eventhub1", client.GetEventHubName());
    EXPECT_EQ(DefaultConsumerGroup, client.GetConsumerGroup());
  }

  TEST(ConnectionStringClientTest, ConsumerAcceptsMatchingEntityPath)
  {
    ConsumerClient client(ConnectionStringWithEntityPath, "eventhub1");
    EXPECT_EQ("eventhub1", client.GetEventHubName());
  }

  TEST(ConnectionStringClientTest, ConsumerRejectsMismatchedEntityPath)
  {
    EXPECT_THROW(
        { ConsumerClient client(ConnectionStringWithEntityPath, "eventhub2"); },
        std::invalid_argument);
  }

  TEST(ConnectionStringClientTest, ConsumerRejectsMissingEventHub)
  {
    EXPECT_THROW({ ConsumerClient client(ConnectionStringNoEntityPath); }, std::invalid_argument);
  }

  TEST(ConnectionStringClientTest, EmulatorConnectionStringConstructsClients)
  {
    ProducerClient producer(EmulatorConnectionString, "eh1");
    ConsumerClient consumer(EmulatorConnectionString, "eh1", "cg1");

    EXPECT_EQ("eh1", producer.GetEventHubName());
    EXPECT_EQ("eh1", consumer.GetEventHubName());
    EXPECT_EQ("localhost", consumer.GetDetails().FullyQualifiedNamespace);
  }

  TEST(ConnectionStringClientTest, MissingEndpointThrows)
  {
    EXPECT_THROW(
        { ProducerClient client("SharedAccessKeyName=FakeKey;SharedAccessKey=FakeKey", "eh1"); },
        std::runtime_error);
  }

  TEST(ConnectionStringClientTest, EmulatorRejectsTlsPort)
  {
    EXPECT_THROW(
        { ProducerClient client(EmulatorTlsPortConnectionString, "eh1"); }, std::invalid_argument);
    EXPECT_THROW(
        { ConsumerClient client(EmulatorTlsPortConnectionString, "eh1"); }, std::invalid_argument);
  }

  // Standard live-test resources disable local authentication. This test runs only when a
  // dedicated SAS-enabled resource supplies the required environment variables.
  class ConnectionStringLiveTest : public EventHubsTestBase {
  };

  TEST_F(ConnectionStringLiveTest, RoundTrip_LIVEONLY_)
  {
    std::string const connectionString
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING");
    std::string const eventHubName
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_NAME");
    if (connectionString.empty() || eventHubName.empty())
    {
      GTEST_SKIP() << "EVENTHUB_CONNECTION_STRING and EVENTHUB_NAME are required.";
    }

    std::string consumerGroup
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONSUMER_GROUP");
    if (consumerGroup.empty())
    {
      consumerGroup = DefaultConsumerGroup;
    }

    std::string partitionId
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_PARTITION_ID");
    if (partitionId.empty())
    {
      partitionId = "0";
    }

    std::vector<uint8_t> const expectedBody{
        'c', 'o', 'n', 'n', 'e', 'c', 't', 'i', 'o', 'n', '-', 's', 't', 'r', 'i', 'n', 'g'};

    Azure::Core::Context rootContext;
    auto context
        = rootContext.WithDeadline(Azure::DateTime::clock::now() + std::chrono::seconds(30));

    ConsumerClient consumer(connectionString, eventHubName, consumerGroup);
    auto partitionClient = consumer.CreatePartitionClient(partitionId, {}, context);

    ProducerClient producer(connectionString, eventHubName);
    EventDataBatchOptions batchOptions;
    batchOptions.PartitionId = partitionId;
    EventDataBatch batch{producer.CreateBatch(batchOptions, context)};
    ASSERT_TRUE(batch.TryAdd(Models::EventData(expectedBody)));
    producer.Send(batch, context);

    auto events = partitionClient.ReceiveEvents(1, context);

    ASSERT_EQ(std::size_t{1}, events.size());
    EXPECT_EQ(expectedBody, events[0]->Body);
  }
#elif ENABLE_RUST_AMQP
  TEST(ConnectionStringClientTest, ProducerRejectsRustBackend)
  {
    EXPECT_THROW(
        { ProducerClient client(ConnectionStringNoEntityPath, "eventhub1"); }, std::runtime_error);
  }

  TEST(ConnectionStringClientTest, ConsumerRejectsRustBackend)
  {
    EXPECT_THROW(
        { ConsumerClient client(ConnectionStringNoEntityPath, "eventhub1"); }, std::runtime_error);
  }
#endif

}}}} // namespace Azure::Messaging::EventHubs::Test
