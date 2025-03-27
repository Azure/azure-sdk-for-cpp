// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "eventhubs_test_base.hpp"
#include "test_checkpoint_store.hpp"

#include <azure/core/context.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  class CheckpointStoreTest : public EventHubsTestBase {
    virtual void SetUp() override { EventHubsTestBase::SetUp(); }

  protected:
    std::string GetRandomName()
    {
      std::string name = "checkpoint";
      if (m_testContext.IsLiveMode())
      {
        name.append(Azure::Core::Uuid::CreateUuid().ToString());
      }
      else
      {
        name.append("-recording");
      }
      return name;
    }
  };

  TEST_F(CheckpointStoreTest, TestCheckpoints)
  {
    std::string const testName = GetRandomName();
    std::string consumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");

    std::shared_ptr<CheckpointStore> checkpointStore{
        std::make_shared<Azure::Messaging::EventHubs::Test::TestCheckpointStore>()};

    auto checkpoints = checkpointStore->ListCheckpoints(
        "fully-qualified-namespace", "event-hub-name", "consumer-group");

    EXPECT_EQ(0ul, checkpoints.size());

    checkpointStore->UpdateCheckpoint(Azure::Messaging::EventHubs::Models::Checkpoint{
        consumerGroup,
        "event-hub-name",
        "ns.servicebus.windows.net",
        "partition-id",
        std::string("101"),
        202,
    });

    {
      // There still should be no checkpoints in the partition we first queried.
      checkpoints = checkpointStore->ListCheckpoints(
          "fully-qualified-namespace", "event-hub-name", "consumer-group");
      EXPECT_EQ(0ul, checkpoints.size());
    }

    checkpoints = checkpointStore->ListCheckpoints(
        "ns.servicebus.windows.net", "event-hub-name", consumerGroup);
    EXPECT_EQ(checkpoints.size(), 1ul);
    EXPECT_EQ(consumerGroup, checkpoints[0].ConsumerGroup);
    EXPECT_EQ("event-hub-name", checkpoints[0].EventHubName);
    EXPECT_EQ("ns.servicebus.windows.net", checkpoints[0].FullyQualifiedNamespaceName);
    EXPECT_EQ("partition-id", checkpoints[0].PartitionId);
    EXPECT_EQ(202, checkpoints[0].SequenceNumber.Value());
    EXPECT_EQ("101", checkpoints[0].Offset.Value());

    checkpointStore->UpdateCheckpoint(Azure::Messaging::EventHubs::Models::Checkpoint{
        consumerGroup,
        "event-hub-name",
        "ns.servicebus.windows.net",
        "partition-id",
        std::string("102"),
        203,
    });

    checkpoints = checkpointStore->ListCheckpoints(
        "ns.servicebus.windows.net", "event-hub-name", consumerGroup);
    EXPECT_EQ(checkpoints.size(), 1ul);
    EXPECT_EQ(consumerGroup, checkpoints[0].ConsumerGroup);
    EXPECT_EQ("event-hub-name", checkpoints[0].EventHubName);
    EXPECT_EQ("ns.servicebus.windows.net", checkpoints[0].FullyQualifiedNamespaceName);
    EXPECT_EQ("partition-id", checkpoints[0].PartitionId);
    EXPECT_EQ(203, checkpoints[0].SequenceNumber.Value());
    EXPECT_EQ("102", checkpoints[0].Offset.Value());
  }

  TEST_F(CheckpointStoreTest, TestOwnerships)
  {
    std::string const testName = GetRandomName();

    std::unique_ptr<CheckpointStore> checkpointStore = std::make_unique<TestCheckpointStore>();

    auto ownerships = checkpointStore->ListOwnership(
        "fully-qualified-namespace", "event-hub-name", "consumer-group");
    EXPECT_EQ(0ul, ownerships.size());

    ownerships = checkpointStore->ClaimOwnership(
        std::vector<Azure::Messaging::EventHubs::Models::Ownership>{});
    EXPECT_EQ(0ul, ownerships.size());

    ownerships = checkpointStore->ClaimOwnership(
        std::vector<Azure::Messaging::EventHubs::Models::Ownership>{
            Azure::Messaging::EventHubs::Models::Ownership{
                "$Default",
                "event-hub-name",
                "ns.servicebus.windows.net",
                "partition-id",
                "owner-id"}});

    // Fail the test immediately if there isn't an entry in the ownerships vector.
    ASSERT_EQ(1ul, ownerships.size());
    EXPECT_EQ("$Default", ownerships[0].ConsumerGroup);
    EXPECT_EQ("event-hub-name", ownerships[0].EventHubName);
    EXPECT_EQ("ns.servicebus.windows.net", ownerships[0].FullyQualifiedNamespace);
    EXPECT_EQ("partition-id", ownerships[0].PartitionId);
    EXPECT_EQ("owner-id", ownerships[0].OwnerId);
    EXPECT_TRUE(ownerships[0].ETag.HasValue());
    EXPECT_TRUE(ownerships[0].LastModifiedTime.HasValue());
    Azure::ETag validEtag = ownerships[0].ETag.Value();
    //    Azure::DateTime lastDatetime = ownerships[0].LastModifiedTime.Value();
    //
    // This ownership should NOT take precedence over the previous ownership, so the set of
    // ownerships returned should be empty.
    ownerships = checkpointStore->ClaimOwnership(
        std::vector<Azure::Messaging::EventHubs::Models::Ownership>{
            Azure::Messaging::EventHubs::Models::Ownership{
                "$Default",
                "event-hub-name",
                "ns.servicebus.windows.net",
                "partition-id",
                "owner-id",
                Azure::ETag("randomETAG")}});
    EXPECT_EQ(0ul, ownerships.size());

    ownerships = checkpointStore->ClaimOwnership(
        std::vector<Azure::Messaging::EventHubs::Models::Ownership>{
            Azure::Messaging::EventHubs::Models::Ownership{
                "$Default",
                "event-hub-name",
                "ns.servicebus.windows.net",
                "partition-id",
                "owner-id",
                validEtag}});

    EXPECT_EQ(1ul, ownerships.size());
    EXPECT_NE(validEtag, ownerships[0].ETag.Value());
    EXPECT_EQ("$Default", ownerships[0].ConsumerGroup);
    EXPECT_EQ("event-hub-name", ownerships[0].EventHubName);
    EXPECT_EQ("ns.servicebus.windows.net", ownerships[0].FullyQualifiedNamespace);
    EXPECT_EQ("partition-id", ownerships[0].PartitionId);
    EXPECT_EQ("owner-id", ownerships[0].OwnerId);
  }
}}}} // namespace Azure::Messaging::EventHubs::Test
