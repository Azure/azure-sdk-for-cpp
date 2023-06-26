// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"
#include "test_checkpoint_store.hpp"

#include <azure/core/context.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>
namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  namespace {
    const std::string testEventHubFQDN = "fqdn";
    const std::string testConsumerGroup = "consumer-group";
    const std::string testEventHubName = "event-hub";
    Azure::Messaging::EventHubs::Models::Ownership TestOwnership(
        std::string partitionID,
        std::string ownerID)
    {
      return Ownership{
          testConsumerGroup,
          testEventHubName,
          testEventHubFQDN,
          partitionID,
          ownerID,
          Azure::ETag(Azure::Core::Uuid::CreateUuid().ToString()),
          Azure::DateTime(std::chrono::system_clock::now())};
    }

    Azure::Messaging::EventHubs::Models::ConsumerClientDetails TestConsumerDetails(
        std::string clientID)
    {
      return ConsumerClientDetails{testEventHubFQDN, testConsumerGroup, testEventHubName, clientID};
    }
    std::map<std::string, std::vector<std::string>> GroupByOwner(std::vector<Ownership> ownerships)
    {
      std::map<std::string, std::vector<std::string>> byOwnerID{};
      for (auto ownership : ownerships)
      {
        byOwnerID[ownership.OwnerID].push_back(ownership.PartitionID);
      }

      return byOwnerID;
    }
    void RequireBalanced(std::vector<Ownership> ownerships, int totalPartitions, int numConsumers)
    {
      int min = totalPartitions / numConsumers;
      int max = min;

      if (totalPartitions % numConsumers > 0)
      {
        max++;
      }

      EXPECT_EQ(totalPartitions, ownerships.size());

      std::map<std::string, std::vector<std::string>> byOwnerID = GroupByOwner(ownerships);

      EXPECT_EQ(byOwnerID.size(), numConsumers);

      for (auto entry : byOwnerID)
      {
        EXPECT_TRUE(entry.second.size() == min || entry.second.size() == max);
      }
    }

    std::vector<std::string> FindCommon(
        std::map<std::string, std::vector<std::string>> ownershipMap)
    {
      std::vector<std::string> commons{};
      for (auto checking : ownershipMap)
      {
        for (auto against : ownershipMap)
        {
          if (checking.first == against.first)
          {
            continue;
          }

          for (auto value : checking.second)
          {
            for (auto againstValue : against.second)
            {
              if (value == againstValue)
              {
                commons.push_back(value);
                break;
              }
            }
          }
        }
      }
      std::sort(commons.begin(), commons.end());
      commons.erase(std::unique(commons.begin(), commons.end()), commons.end());
      return commons;
    }
  } // namespace
  TEST(ProcessorLoadBalancerTest, Greedy_EnoughUnownedPartitions)
  {
    Azure::Messaging::EventHubs::Test::TestCheckpointStore checkpointStore;

    checkpointStore.ClaimOwnership(std::vector<Azure::Messaging::EventHubs::Models::Ownership>{
        TestOwnership("0", "some-client"), TestOwnership("3", "some-client")});

    Azure::Messaging::EventHubs::ProcessorLoadBalancer loadBalancer(
        std::make_shared<TestCheckpointStore>(checkpointStore),
        TestConsumerDetails("new-client"),
        Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyGreedy,
        std::chrono::minutes(2));
    auto ownerships = loadBalancer.LoadBalance(std::vector<std::string>{"0", "1", "2", "3"});

    EXPECT_EQ(ownerships.size(), 2);
    EXPECT_TRUE(ownerships[0].PartitionID == "1" || ownerships[0].PartitionID == "2");
    EXPECT_TRUE(ownerships[1].PartitionID == "1" || ownerships[1].PartitionID == "2");

    auto finalOwneships = loadBalancer.m_checkpointStore->ListOwnership(
        testEventHubFQDN, testEventHubName, testConsumerGroup);
    EXPECT_EQ(finalOwneships.size(), 4);
  }

  TEST(ProcessorLoadBalancerTest, Balanced_UnownedPartitions)
  {
    Azure::Messaging::EventHubs::Test::TestCheckpointStore checkpointStore;

    checkpointStore.ClaimOwnership(std::vector<Azure::Messaging::EventHubs::Models::Ownership>{
        TestOwnership("0", "some-client"), TestOwnership("3", "some-client")});

    Azure::Messaging::EventHubs::ProcessorLoadBalancer loadBalancer(
        std::make_shared<TestCheckpointStore>(checkpointStore),
        TestConsumerDetails("new-client"),
        Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyBalanced,
        std::chrono::minutes(2));

    auto ownerships = loadBalancer.LoadBalance(std::vector<std::string>{"0", "1", "2", "3"});

    EXPECT_EQ(ownerships.size(), 1);

    ownerships = loadBalancer.LoadBalance(std::vector<std::string>{"0", "1", "2", "3"});

    EXPECT_EQ(ownerships.size(), 2);

    RequireBalanced(
        loadBalancer.m_checkpointStore->ListOwnership(
            testEventHubFQDN, testEventHubName, testConsumerGroup),
        4,
        2);
  }

  TEST(ProcessorLoadBalancerTest, Greedy_ForcedToSteal)
  {
    Azure::Messaging::EventHubs::Test::TestCheckpointStore checkpointStore;

    checkpointStore.ClaimOwnership(std::vector<Azure::Messaging::EventHubs::Models::Ownership>{
        TestOwnership("0", "some-client"),
        TestOwnership("1", "some-client"),
        TestOwnership("2", "some-client"),
        TestOwnership("3", "some-client"),
    });

    Azure::Messaging::EventHubs::ProcessorLoadBalancer loadBalancer(
        std::make_shared<TestCheckpointStore>(checkpointStore),
        TestConsumerDetails("new-client"),
        Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyGreedy,
        std::chrono::minutes(2));

    auto ownerships = loadBalancer.LoadBalance(std::vector<std::string>{"0", "1", "2", "3"});

    EXPECT_FALSE(ownerships.size() == 0);
    auto finalOwneships = loadBalancer.m_checkpointStore->ListOwnership(
        testEventHubFQDN, testEventHubName, testConsumerGroup);
    auto ownersMap = GroupByOwner(finalOwneships);
    auto commons = FindCommon(ownersMap);
    EXPECT_EQ(commons.size(), 0);
  }

  TEST(ProcessorLoadBalancerTest, AnyStrategy_GetExpiredPartition)
  {
    for (auto strategy :
         {Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyBalanced,
          Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyGreedy})
    {
      Azure::Messaging::EventHubs::Test::TestCheckpointStore checkpointStore;

      const std::string clientA = "clientA";
      const std::string clientB = "clientB";

      Ownership midOwner = TestOwnership("2", "clientC");

      checkpointStore.ClaimOwnership(
          {TestOwnership("0", clientA),
           TestOwnership("1", clientA),
           midOwner,
           TestOwnership("3", clientB),
           TestOwnership("4", clientB)});

      checkpointStore.ExpireOwnership(midOwner);

      Azure::Messaging::EventHubs::ProcessorLoadBalancer loadBalancer(
          std::make_shared<TestCheckpointStore>(checkpointStore),
          TestConsumerDetails(clientB),
          strategy,
          std::chrono::minutes(2));

      auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3", "4"});

      EXPECT_TRUE(ownerships.size() > 0);
      RequireBalanced(
          loadBalancer.m_checkpointStore->ListOwnership(
              testEventHubFQDN, testEventHubName, testConsumerGroup),
          5,
          2);
    }
  }

  TEST(ProcessorLoadBalancerTest, AnyStrategy_FullyBalancedOdd)
  {
    for (auto strategy :
         {Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyBalanced,
          Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyGreedy})
    {
      Azure::Messaging::EventHubs::Test::TestCheckpointStore checkpointStore;

      const std::string clientA = "clientA";
      const std::string clientB = "clientB";

      checkpointStore.ClaimOwnership(
          {TestOwnership("0", clientA),
           TestOwnership("1", clientA),
           TestOwnership("2", clientA),
           TestOwnership("3", clientB),
           TestOwnership("4", clientB)});

      {

        Azure::Messaging::EventHubs::ProcessorLoadBalancer loadBalancer(
            std::make_shared<TestCheckpointStore>(checkpointStore),
            TestConsumerDetails(clientB),
            strategy,
            std::chrono::minutes(2));

        auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3", "4"});

        EXPECT_EQ(GroupByOwner(ownerships)[clientB][0], "3");
        EXPECT_EQ(GroupByOwner(ownerships)[clientB][1], "4");
        EXPECT_EQ(GroupByOwner(ownerships)[clientB].size(), 2);

        RequireBalanced(
            loadBalancer.m_checkpointStore->ListOwnership(
                testEventHubFQDN, testEventHubName, testConsumerGroup),
            5,
            2);
      }

      {

        Azure::Messaging::EventHubs::ProcessorLoadBalancer loadBalancer(
            std::make_shared<TestCheckpointStore>(checkpointStore),
            TestConsumerDetails(clientA),
            strategy,
            std::chrono::minutes(2));

        auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3", "4"});

        EXPECT_EQ(GroupByOwner(ownerships)[clientA][0], "0");
        EXPECT_EQ(GroupByOwner(ownerships)[clientA][1], "1");
        EXPECT_EQ(GroupByOwner(ownerships)[clientA][2], "2");
        EXPECT_EQ(GroupByOwner(ownerships)[clientA].size(), 3);

        RequireBalanced(
            loadBalancer.m_checkpointStore->ListOwnership(
                testEventHubFQDN, testEventHubName, testConsumerGroup),
            5,
            2);
      }
    }
  }

  TEST(ProcessorLoadBalancerTest, AnyStrategy_FullyBalancedEven)
  {
    for (auto strategy :
         {Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyBalanced,
          Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyGreedy})
    {
      Azure::Messaging::EventHubs::Test::TestCheckpointStore checkpointStore;

      const std::string clientA = "clientA";
      const std::string clientB = "clientB";

      checkpointStore.ClaimOwnership(
          {TestOwnership("0", clientA),
           TestOwnership("1", clientA),
           TestOwnership("2", clientB),
           TestOwnership("3", clientB)});

      {

        Azure::Messaging::EventHubs::ProcessorLoadBalancer loadBalancer(
            std::make_shared<TestCheckpointStore>(checkpointStore),
            TestConsumerDetails(clientB),
            strategy,
            std::chrono::minutes(2));

        auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3"});

        EXPECT_EQ(GroupByOwner(ownerships)[clientB][0], "2");
        EXPECT_EQ(GroupByOwner(ownerships)[clientB][1], "3");
        EXPECT_EQ(GroupByOwner(ownerships)[clientB].size(), 2);

        RequireBalanced(
            loadBalancer.m_checkpointStore->ListOwnership(
                testEventHubFQDN, testEventHubName, testConsumerGroup),
            4,
            2);
      }

      {

        Azure::Messaging::EventHubs::ProcessorLoadBalancer loadBalancer(
            std::make_shared<TestCheckpointStore>(checkpointStore),
            TestConsumerDetails(clientA),
            strategy,
            std::chrono::minutes(2));

        auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3"});

        EXPECT_EQ(GroupByOwner(ownerships)[clientA][0], "0");
        EXPECT_EQ(GroupByOwner(ownerships)[clientA][1], "1");
        EXPECT_EQ(GroupByOwner(ownerships)[clientA].size(), 2);

        RequireBalanced(
            loadBalancer.m_checkpointStore->ListOwnership(
                testEventHubFQDN, testEventHubName, testConsumerGroup),
            4,
            2);
      }
    }
  }

  TEST(ProcessorLoadBalancerTest, AnyStrategy_GrabExtraPartitionBecauseAboveMax)
  {
    for (auto strategy :
         {Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyBalanced,
          Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyGreedy})
    {
      Azure::Messaging::EventHubs::Test::TestCheckpointStore checkpointStore;

      const std::string clientA = "clientA";
      const std::string clientB = "clientB";

      checkpointStore.ClaimOwnership(
          {TestOwnership("0", clientA),
           TestOwnership("1", clientA),
           TestOwnership("2", ""),
           TestOwnership("3", clientB),
           TestOwnership("4", clientB)});

      Azure::Messaging::EventHubs::ProcessorLoadBalancer loadBalancer(
          std::make_shared<TestCheckpointStore>(checkpointStore),
          TestConsumerDetails(clientB),
          strategy,
          std::chrono::minutes(2));

      auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3", "4"});
      auto clientOwned = GroupByOwner(ownerships)[clientB];
      std::sort(clientOwned.begin(), clientOwned.end());
      EXPECT_EQ(clientOwned[0], "2");
      EXPECT_EQ(clientOwned[1], "3");
      EXPECT_EQ(clientOwned[2], "4");
      EXPECT_EQ(clientOwned.size(), 3);

      RequireBalanced(
          loadBalancer.m_checkpointStore->ListOwnership(
              testEventHubFQDN, testEventHubName, testConsumerGroup),
          5,
          2);
    }
  }

  TEST(ProcessorLoadBalancerTest, AnyStrategy_StealsToBalance)
  {
    for (auto strategy :
         {Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyBalanced,
          Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyGreedy})
    {
      Azure::Messaging::EventHubs::Test::TestCheckpointStore checkpointStore;

      const std::string clientA = "clientA";
      const std::string clientB = "clientB";

      checkpointStore.ClaimOwnership(
          {TestOwnership("0", clientA),
           TestOwnership("1", clientA),
           TestOwnership("2", clientA),
           TestOwnership("3", clientB)});

      {

        Azure::Messaging::EventHubs::ProcessorLoadBalancer loadBalancer(
            std::make_shared<TestCheckpointStore>(checkpointStore),
            TestConsumerDetails(clientA),
            strategy,
            std::chrono::minutes(2));

        auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3"});

        auto clientOwned = GroupByOwner(ownerships)[clientA];
        std::sort(clientOwned.begin(), clientOwned.end());
        EXPECT_EQ(clientOwned[0], "0");
        EXPECT_EQ(clientOwned[1], "1");
        EXPECT_EQ(clientOwned[2], "2");
        EXPECT_EQ(clientOwned.size(), 3);
      }

      {

        Azure::Messaging::EventHubs::ProcessorLoadBalancer loadBalancer(
            std::make_shared<TestCheckpointStore>(checkpointStore),
            TestConsumerDetails(clientB),
            strategy,
            std::chrono::minutes(2));

        auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3"});

        auto clientOwned = GroupByOwner(ownerships)[clientB];
        EXPECT_EQ(clientOwned.size(), 2);

        RequireBalanced(
            loadBalancer.m_checkpointStore->ListOwnership(
                testEventHubFQDN, testEventHubName, testConsumerGroup),
            4,
            2);
      }
    }
  }
}}}} // namespace Azure::Messaging::EventHubs::Test
