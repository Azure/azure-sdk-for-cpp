// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "eventhubs_test_base.hpp"

#include <azure/core/context.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  class RoundTripTests : public EventHubsTestBase {
  };

  // Round trip a message with a string body using a sequence number filter.
  TEST_F(RoundTripTests, SendAndReceiveStringSequenceNumber_LIVEONLY_)
  {
    std::string const connectionString = GetEnv("EVENTHUB_CONNECTION_STRING");
    std::string const eventHubName = GetEnv("EVENTHUB_NAME");
    std::string const consumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");

    int64_t startSequenceNumber = 0;

    {
      Azure::Messaging::EventHubs::ProducerClient producer{connectionString, eventHubName};
      auto partitionProperties = producer.GetPartitionProperties("1");
      startSequenceNumber = partitionProperties.LastEnqueuedSequenceNumber;

      Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
      batchOptions.PartitionId = "1";
      Azure::Messaging::EventHubs::EventDataBatch eventBatch{producer.CreateBatch(batchOptions)};
      EXPECT_TRUE(
          eventBatch.TryAdd(Azure::Messaging::EventHubs::Models::EventData("Hello world!")));
      EXPECT_NO_THROW(producer.Send(eventBatch));
    }

    {
      Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
      partitionOptions.StartPosition.SequenceNumber = startSequenceNumber;

      Azure::Messaging::EventHubs::ConsumerClient consumer(
          connectionString, eventHubName, consumerGroup);
      auto receiver = consumer.CreatePartitionClient("1", partitionOptions);

      auto receivedEvents = receiver.ReceiveEvents(1);
      ASSERT_EQ(1ul, receivedEvents.size());
      std::vector<uint8_t> expected{'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '!'};

      EXPECT_EQ(expected, receivedEvents[0]->Body);
    }
  }

  // Round trip a message with a binary body using an offset filter.
  TEST_F(RoundTripTests, SendAndReceiveBinaryDataOffset_LIVEONLY_)
  {
    std::string const connectionString = GetEnv("EVENTHUB_CONNECTION_STRING");
    std::string const eventHubName = GetEnv("EVENTHUB_NAME");
    std::string const consumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");

    int64_t startOffset = 0;
    {
      Azure::Messaging::EventHubs::ProducerClient producer(connectionString, eventHubName);
      auto partitionProperties = producer.GetPartitionProperties("1");
      startOffset = partitionProperties.LastEnqueuedOffset;

      Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
      batchOptions.PartitionId = "1";
      Azure::Messaging::EventHubs::EventDataBatch eventBatch{producer.CreateBatch(batchOptions)};
      EXPECT_TRUE(
          eventBatch.TryAdd(Azure::Messaging::EventHubs::Models::EventData({1, 2, 3, 4, 5})));
      EXPECT_NO_THROW(producer.Send(eventBatch));
    }

    {
      Azure::Messaging::EventHubs::ConsumerClient consumer(
          connectionString, eventHubName, consumerGroup);

      Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
      partitionOptions.StartPosition.Offset = startOffset;

      auto receiver = consumer.CreatePartitionClient("1", partitionOptions);

      auto receivedEvents = receiver.ReceiveEvents(1);
      ASSERT_EQ(1ul, receivedEvents.size());
      for (auto const& event : receivedEvents)
      {
        GTEST_LOG_(INFO) << "Event: " << event;
        EXPECT_TRUE(event->EnqueuedTime);
        EXPECT_TRUE(event->Offset);
        EXPECT_TRUE(event->SequenceNumber);
      }

      std::vector<uint8_t> expected{1, 2, 3, 4, 5};

      EXPECT_EQ(expected, receivedEvents[0]->Body);
    }
  }

  // Round trip a message with a binary body using a queued time filter.
  TEST_F(RoundTripTests, SendAndReceiveTimestamp_LIVEONLY_)
  {
    std::string const connectionString = GetEnv("EVENTHUB_CONNECTION_STRING");
    std::string const eventHubName = GetEnv("EVENTHUB_NAME");
    std::string const consumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");

    Azure::DateTime startTime;
    {
      Azure::Messaging::EventHubs::ProducerClient producer(connectionString, eventHubName);
      auto partitionProperties = producer.GetPartitionProperties("1");
      GTEST_LOG_(INFO) << "Partition Properties: " << partitionProperties;
      startTime = partitionProperties.LastEnqueuedTimeUtc + std::chrono::seconds(1);

      GTEST_LOG_(INFO) << "Sleep for a second to reset enqueued time";
      std::this_thread::sleep_for(std::chrono::seconds(2));

      Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
      batchOptions.PartitionId = "1";
      Azure::Messaging::EventHubs::EventDataBatch eventBatch{producer.CreateBatch(batchOptions)};
      Azure::Messaging::EventHubs::Models::EventData eventData;
      eventData.Body = {1, 2, 3, 4, 5, 6, 7};
      eventData.ContentType = "application/binary";
      eventData.MessageId = "Test Message Id";
      EXPECT_TRUE(eventBatch.TryAdd(eventData));
      EXPECT_NO_THROW(producer.Send(eventBatch));
    }

    {
      Azure::Messaging::EventHubs::ConsumerClient consumer(
          connectionString, eventHubName, consumerGroup);

      Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
      partitionOptions.StartPosition.EnqueuedTime = startTime;
      partitionOptions.StartPosition.Inclusive = false;

      auto receiver = consumer.CreatePartitionClient("1", partitionOptions);

      auto receivedEvents = receiver.ReceiveEvents(1);
      ASSERT_EQ(1ul, receivedEvents.size());
      for (auto const& event : receivedEvents)
      {
        GTEST_LOG_(INFO) << "Event: " << event;
        EXPECT_TRUE(event->EnqueuedTime);
        EXPECT_TRUE(event->Offset);
        EXPECT_TRUE(event->SequenceNumber);
      }
      std::vector<uint8_t> expected{1, 2, 3, 4, 5, 6, 7};

      EXPECT_EQ(expected, receivedEvents[0]->Body);
      ASSERT_TRUE(receivedEvents[0]->ContentType);
      EXPECT_EQ("application/binary", receivedEvents[0]->ContentType.Value());
      ASSERT_TRUE(receivedEvents[0]->MessageId);
      EXPECT_EQ("Test Message Id", static_cast<std::string>(receivedEvents[0]->MessageId.Value()));
    }
  }

}}}} // namespace Azure::Messaging::EventHubs::Test
