// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "eventhubs_test_base.hpp"

#include <azure/core/context.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  class RoundTripTests : public EventHubsTestBaseParameterized {};

  // Round trip a message with a string body using a sequence number filter.
  TEST_P(RoundTripTests, SendAndReceiveStringSequenceNumber_LIVEONLY_)
  {
    int64_t startSequenceNumber = 0;

    {
      auto producer{CreateProducerClient()};
      auto partitionProperties = producer->GetPartitionProperties("1");
      startSequenceNumber = partitionProperties.LastEnqueuedSequenceNumber;

      Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
      batchOptions.PartitionId = "1";
      Azure::Messaging::EventHubs::EventDataBatch eventBatch{producer->CreateBatch(batchOptions)};
      EXPECT_TRUE(
          eventBatch.TryAdd(Azure::Messaging::EventHubs::Models::EventData("Hello world!")));
      EXPECT_NO_THROW(producer->Send(eventBatch));
    }

    {
      Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
      partitionOptions.StartPosition.SequenceNumber = startSequenceNumber;

      auto consumer(CreateConsumerClient());
      auto receiver = consumer->CreatePartitionClient("1", partitionOptions);

      auto receivedEvents = receiver.ReceiveEvents(1);
      ASSERT_EQ(1ul, receivedEvents.size());
      std::vector<uint8_t> expected{'H', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '!'};

      EXPECT_EQ(expected, receivedEvents[0]->Body);
    }
  }

  // Round trip a message with a binary body using an offset filter.
  TEST_P(RoundTripTests, SendAndReceiveBinaryDataOffset_LIVEONLY_)
  {
    int64_t startOffset = 0;
    {
      auto producer{CreateProducerClient()};
      auto partitionProperties = producer->GetPartitionProperties("1");
      startOffset = partitionProperties.LastEnqueuedOffset;

      Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
      batchOptions.PartitionId = "1";
      Azure::Messaging::EventHubs::EventDataBatch eventBatch{producer->CreateBatch(batchOptions)};
      EXPECT_TRUE(
          eventBatch.TryAdd(Azure::Messaging::EventHubs::Models::EventData({1, 2, 3, 4, 5})));
      EXPECT_NO_THROW(producer->Send(eventBatch));
    }

    {
      auto consumer{CreateConsumerClient()};

      Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
      partitionOptions.StartPosition.Offset = startOffset;

      auto receiver = consumer->CreatePartitionClient("1", partitionOptions);

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
  TEST_P(RoundTripTests, SendAndReceiveTimestamp_LIVEONLY_)
  {
    Azure::DateTime startTime;
    {
      auto producer{CreateProducerClient()};
      auto partitionProperties = producer->GetPartitionProperties("1");
      GTEST_LOG_(INFO) << "Partition Properties: " << partitionProperties;
      startTime = partitionProperties.LastEnqueuedTimeUtc + std::chrono::seconds(1);

      GTEST_LOG_(INFO) << "Sleep for a second to reset enqueued time";
      std::this_thread::sleep_for(std::chrono::seconds(2));

      Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
      batchOptions.PartitionId = "1";
      Azure::Messaging::EventHubs::EventDataBatch eventBatch{producer->CreateBatch(batchOptions)};
      Azure::Messaging::EventHubs::Models::EventData eventData;
      eventData.Body = {1, 2, 3, 4, 5, 6, 7};
      eventData.ContentType = "application/binary";
      eventData.MessageId = "Test Message Id";
      EXPECT_TRUE(eventBatch.TryAdd(eventData));
      EXPECT_NO_THROW(producer->Send(eventBatch));
    }

    {
      auto consumer{CreateConsumerClient()};

      Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
      partitionOptions.StartPosition.EnqueuedTime = startTime;
      partitionOptions.StartPosition.Inclusive = false;

      auto receiver = consumer->CreatePartitionClient("1", partitionOptions);

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
      ASSERT_FALSE(receivedEvents[0]->MessageId.IsNull());
      EXPECT_EQ("Test Message Id", static_cast<std::string>(receivedEvents[0]->MessageId));
    }
  }

  namespace {
    static std::string GetSuffix(const testing::TestParamInfo<AuthType>& info)
    {
      std::string stringValue = "";
      switch (info.param)
      {
        case AuthType::Key:
          stringValue = "Key_LIVEONLY_";
          break;
        case AuthType::Emulator:
          stringValue = "Emulator";
          break;
      }
      return stringValue;
    }
  } // namespace

  INSTANTIATE_TEST_SUITE_P(
      EventHubs,
      RoundTripTests,
      ::testing::Values(AuthType::Key /*, AuthType::Emulator*/),
      GetSuffix);

}}}} // namespace Azure::Messaging::EventHubs::Test
