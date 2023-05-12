// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include <azure/core/context.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>
#include "test_checkpoint_store.hpp"
namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  namespace {
    const std::string testEventHubFQDN = "fqdn";
    const std::string testConsumerGroup = "consumer-group";
    const std::string testEventHubName = "event-hub";
    Azure::Messaging::EventHubs::Ownership TestOwnership(
        std::string partitionID,
        std::string ownerID)
    {
      return Ownership{testConsumerGroup, testEventHubName, testEventHubFQDN, partitionID, ownerID};
    }

    Azure::Messaging::EventHubs::ConsumerClientDetails TestConsumerDetails(std::string clientID) {
      return ConsumerClientDetails{testEventHubFQDN, testConsumerGroup, testEventHubName, clientID};
    }
  }
  TEST(ProcessorLoadBalancerTest, Greedy_EnoughUnownedPartitions)
  {
    Azure::Messaging::EventHubs::Test::TestCheckpointStore checkpointStore;
        
       
    checkpointStore.ClaimOwnership(std::vector<Azure::Messaging::EventHubs::Ownership>{
        TestOwnership("0", "some-client"), TestOwnership("3", "some-client")
    });

    Azure::Messaging::EventHubs::ProcessorLoadBalancer loadBalancer(
        std::make_unique<TestCheckpointStore>( checkpointStore),
        TestConsumerDetails("new-client"),
        Azure::Messaging::EventHubs::ProcessorStrategy::ProcessorStrategyGreedy,
        std::chrono::minutes(2));
    auto ownerships = loadBalancer.LoadBalance(std::vector<std::string>{"0", "1", "2", "3"});

    EXPECT_EQ(ownerships.size(), 2);
    EXPECT_EQ(ownerships[0].PartitionID, "2");
    EXPECT_EQ(ownerships[1].PartitionID, "1");

    auto finalOwneships
        = loadBalancer.m_checkpointStore->ListOwnership(testEventHubFQDN, testEventHubName, testConsumerGroup);
    EXPECT_EQ(finalOwneships.size(), 4);
  }
}}}} // namespace Azure::Messaging::EventHubs::Test