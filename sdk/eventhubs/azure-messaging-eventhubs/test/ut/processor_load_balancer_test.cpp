// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "../src/private/processor_load_balancer.hpp"
#include "eventhubs_test_base.hpp"
#include "test_checkpoint_store.hpp"

#include <azure/core/context.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {

  class ProcessorLoadBalancerTest : public EventHubsTestBase {
  };

  namespace {
    const std::string testEventHubFQDN = "fqdn";
    const std::string testConsumerGroup = "consumer-group";
    const std::string testEventHubName = "event-hub";
    Azure::Messaging::EventHubs::Models::Ownership TestOwnership(
        std::string partitionID,
        std::string ownerID)
    {
      return Models::Ownership{
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
      return Models::ConsumerClientDetails{
          testEventHubFQDN, testConsumerGroup, testEventHubName, clientID};
    }
    std::map<std::string, std::vector<std::string>> GroupByOwner(
        std::vector<Models::Ownership> ownerships)
    {
      std::map<std::string, std::vector<std::string>> byOwnerID{};
      for (auto const& ownership : ownerships)
      {
        byOwnerID[ownership.OwnerId].push_back(ownership.PartitionId);
      }

      return byOwnerID;
    }
    void RequireBalanced(
        std::vector<Models::Ownership> ownerships,
        size_t totalPartitions,
        size_t numConsumers)
    {
      size_t min = totalPartitions / numConsumers;
      size_t max = min;

      if (totalPartitions % numConsumers > 0)
      {
        max++;
      }

      EXPECT_EQ(totalPartitions, ownerships.size());

      std::map<std::string, std::vector<std::string>> byOwnerID = GroupByOwner(ownerships);

      EXPECT_EQ(byOwnerID.size(), numConsumers);

      for (auto const& entry : byOwnerID)
      {
        EXPECT_TRUE(entry.second.size() == min || entry.second.size() == max);
      }
    }

    std::vector<std::string> FindCommon(
        std::map<std::string, std::vector<std::string>> ownershipMap)
    {
      std::vector<std::string> commons{};
      for (auto const& checking : ownershipMap)
      {
        for (auto const& against : ownershipMap)
        {
          if (checking.first == against.first)
          {
            continue;
          }

          for (auto const& value : checking.second)
          {
            for (auto const& againstValue : against.second)
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
  TEST_F(ProcessorLoadBalancerTest, Greedy_EnoughUnownedPartitions)
  {
    std::shared_ptr<CheckpointStore> checkpointStore{
        std::make_shared<Azure::Messaging::EventHubs::Test::TestCheckpointStore>()};

    checkpointStore->ClaimOwnership(std::vector<Azure::Messaging::EventHubs::Models::Ownership>{
        TestOwnership("0", "some-client"), TestOwnership("3", "some-client")});

    Azure::Messaging::EventHubs::_detail::ProcessorLoadBalancer loadBalancer(
        checkpointStore,
        TestConsumerDetails("new-client"),
        Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyGreedy,
        std::chrono::minutes(2));
    auto const& ownerships = loadBalancer.LoadBalance(std::vector<std::string>{"0", "1", "2", "3"});

    EXPECT_EQ(ownerships.size(), 2ul);
    EXPECT_TRUE(ownerships[0].PartitionId == "1" || ownerships[0].PartitionId == "2");
    EXPECT_TRUE(ownerships[1].PartitionId == "1" || ownerships[1].PartitionId == "2");

    auto finalOwnerships = loadBalancer.m_checkpointStore->ListOwnership(
        testEventHubFQDN, testEventHubName, testConsumerGroup);
    EXPECT_EQ(finalOwnerships.size(), 4ul);
  }

  TEST_F(ProcessorLoadBalancerTest, Balanced_UnownedPartitions)
  {
    std::shared_ptr<CheckpointStore> checkpointStore{
        std::make_shared<Azure::Messaging::EventHubs::Test::TestCheckpointStore>()};

    checkpointStore->ClaimOwnership(std::vector<Azure::Messaging::EventHubs::Models::Ownership>{
        TestOwnership("0", "some-client"), TestOwnership("3", "some-client")});

    Azure::Messaging::EventHubs::_detail::ProcessorLoadBalancer loadBalancer(
        checkpointStore,
        TestConsumerDetails("new-client"),
        Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyBalanced,
        std::chrono::minutes(2));

    auto ownerships = loadBalancer.LoadBalance(std::vector<std::string>{"0", "1", "2", "3"});

    EXPECT_EQ(ownerships.size(), 1ul);

    ownerships = loadBalancer.LoadBalance(std::vector<std::string>{"0", "1", "2", "3"});

    EXPECT_EQ(ownerships.size(), 2ul);

    RequireBalanced(
        loadBalancer.m_checkpointStore->ListOwnership(
            testEventHubFQDN, testEventHubName, testConsumerGroup),
        4,
        2);
  }

  TEST_F(ProcessorLoadBalancerTest, Greedy_ForcedToSteal)
  {
    std::shared_ptr<CheckpointStore> checkpointStore{
        std::make_shared<Azure::Messaging::EventHubs::Test::TestCheckpointStore>()};

    checkpointStore->ClaimOwnership(std::vector<Azure::Messaging::EventHubs::Models::Ownership>{
        TestOwnership("0", "some-client"),
        TestOwnership("1", "some-client"),
        TestOwnership("2", "some-client"),
        TestOwnership("3", "some-client"),
    });

    Azure::Messaging::EventHubs::_detail::ProcessorLoadBalancer loadBalancer(
        checkpointStore,
        TestConsumerDetails("new-client"),
        Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyGreedy,
        std::chrono::minutes(2));

    auto ownerships = loadBalancer.LoadBalance(std::vector<std::string>{"0", "1", "2", "3"});

    EXPECT_FALSE(ownerships.size() == 0);
    auto finalOwnerships = loadBalancer.m_checkpointStore->ListOwnership(
        testEventHubFQDN, testEventHubName, testConsumerGroup);
    auto ownersMap = GroupByOwner(finalOwnerships);
    auto commons = FindCommon(ownersMap);
    EXPECT_EQ(commons.size(), 0ul);
  }

  TEST_F(ProcessorLoadBalancerTest, AnyStrategy_GetExpiredPartition)
  {
    for (auto strategy :
         {Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyBalanced,
          Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyGreedy})
    {
      std::shared_ptr<TestCheckpointStore> checkpointStore{
          std::make_shared<Azure::Messaging::EventHubs::Test::TestCheckpointStore>()};

      const std::string clientA = "clientA";
      const std::string clientB = "clientB";

      Models::Ownership midOwner = TestOwnership("2", "clientC");

      checkpointStore->ClaimOwnership(
          {TestOwnership("0", clientA),
           TestOwnership("1", clientA),
           midOwner,
           TestOwnership("3", clientB),
           TestOwnership("4", clientB)});

      // NOTE: TEST HOOK FOR ExpireOwnership.
      checkpointStore->ExpireOwnership(midOwner);

      Azure::Messaging::EventHubs::_detail::ProcessorLoadBalancer loadBalancer(
          checkpointStore, TestConsumerDetails(clientB), strategy, std::chrono::minutes(2));

      auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3", "4"});

      EXPECT_TRUE(ownerships.size() > 0);
      RequireBalanced(
          loadBalancer.m_checkpointStore->ListOwnership(
              testEventHubFQDN, testEventHubName, testConsumerGroup),
          5,
          2);
    }
  }

  TEST_F(ProcessorLoadBalancerTest, AnyStrategy_FullyBalancedOdd)
  {
    for (auto strategy :
         {Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyBalanced,
          Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyGreedy})
    {
      std::shared_ptr<CheckpointStore> checkpointStore{
          std::make_shared<Azure::Messaging::EventHubs::Test::TestCheckpointStore>()};

      const std::string clientA = "clientA";
      const std::string clientB = "clientB";

      checkpointStore->ClaimOwnership(
          {TestOwnership("0", clientA),
           TestOwnership("1", clientA),
           TestOwnership("2", clientA),
           TestOwnership("3", clientB),
           TestOwnership("4", clientB)});

      {

        Azure::Messaging::EventHubs::_detail::ProcessorLoadBalancer loadBalancer(
            checkpointStore, TestConsumerDetails(clientB), strategy, std::chrono::minutes(2));

        auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3", "4"});

        EXPECT_EQ(GroupByOwner(ownerships)[clientB][0], "3");
        EXPECT_EQ(GroupByOwner(ownerships)[clientB][1], "4");
        EXPECT_EQ(GroupByOwner(ownerships)[clientB].size(), 2ul);

        RequireBalanced(
            loadBalancer.m_checkpointStore->ListOwnership(
                testEventHubFQDN, testEventHubName, testConsumerGroup),
            5,
            2);
      }

      {

        Azure::Messaging::EventHubs::_detail::ProcessorLoadBalancer loadBalancer(
            checkpointStore, TestConsumerDetails(clientA), strategy, std::chrono::minutes(2));

        auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3", "4"});

        EXPECT_EQ(GroupByOwner(ownerships)[clientA][0], "0");
        EXPECT_EQ(GroupByOwner(ownerships)[clientA][1], "1");
        EXPECT_EQ(GroupByOwner(ownerships)[clientA][2], "2");
        EXPECT_EQ(GroupByOwner(ownerships)[clientA].size(), 3ul);

        RequireBalanced(
            loadBalancer.m_checkpointStore->ListOwnership(
                testEventHubFQDN, testEventHubName, testConsumerGroup),
            5,
            2);
      }
    }
  }

  TEST_F(ProcessorLoadBalancerTest, AnyStrategy_FullyBalancedEven)
  {
    for (auto strategy :
         {Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyBalanced,
          Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyGreedy})
    {
      std::shared_ptr<CheckpointStore> checkpointStore{
          std::make_shared<Azure::Messaging::EventHubs::Test::TestCheckpointStore>()};

      const std::string clientA = "clientA";
      const std::string clientB = "clientB";

      checkpointStore->ClaimOwnership(
          {TestOwnership("0", clientA),
           TestOwnership("1", clientA),
           TestOwnership("2", clientB),
           TestOwnership("3", clientB)});

      {

        Azure::Messaging::EventHubs::_detail::ProcessorLoadBalancer loadBalancer(
            checkpointStore, TestConsumerDetails(clientB), strategy, std::chrono::minutes(2));

        auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3"});

        EXPECT_EQ(GroupByOwner(ownerships)[clientB].size(), 2ul);
        EXPECT_EQ(GroupByOwner(ownerships)[clientB][0], "2");
        EXPECT_EQ(GroupByOwner(ownerships)[clientB][1], "3");

        RequireBalanced(
            loadBalancer.m_checkpointStore->ListOwnership(
                testEventHubFQDN, testEventHubName, testConsumerGroup),
            4,
            2);
      }

      {

        Azure::Messaging::EventHubs::_detail::ProcessorLoadBalancer loadBalancer(
            checkpointStore, TestConsumerDetails(clientA), strategy, std::chrono::minutes(2));

        auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3"});

        EXPECT_EQ(GroupByOwner(ownerships)[clientA].size(), 2ul);
        EXPECT_EQ(GroupByOwner(ownerships)[clientA][0], "0");
        EXPECT_EQ(GroupByOwner(ownerships)[clientA][1], "1");

        RequireBalanced(
            loadBalancer.m_checkpointStore->ListOwnership(
                testEventHubFQDN, testEventHubName, testConsumerGroup),
            4,
            2);
      }
    }
  }

  TEST_F(ProcessorLoadBalancerTest, AnyStrategy_GrabExtraPartitionBecauseAboveMax)
  {
    for (auto strategy :
         {Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyBalanced,
          Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyGreedy})
    {
      std::shared_ptr<CheckpointStore> checkpointStore{
          std::make_shared<Azure::Messaging::EventHubs::Test::TestCheckpointStore>()};

      const std::string clientA = "clientA";
      const std::string clientB = "clientB";

      checkpointStore->ClaimOwnership(
          {TestOwnership("0", clientA),
           TestOwnership("1", clientA),
           TestOwnership("3", clientB),
           TestOwnership("4", clientB)});

      Azure::Messaging::EventHubs::_detail::ProcessorLoadBalancer loadBalancer(
          checkpointStore, TestConsumerDetails(clientB), strategy, std::chrono::minutes(2));

      auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3", "4"});
      auto clientOwned = GroupByOwner(ownerships)[clientB];
      std::sort(clientOwned.begin(), clientOwned.end());
      ASSERT_EQ(clientOwned.size(), 3ul);
      EXPECT_EQ(clientOwned[0], "2");
      EXPECT_EQ(clientOwned[1], "3");
      EXPECT_EQ(clientOwned[2], "4");

      RequireBalanced(
          loadBalancer.m_checkpointStore->ListOwnership(
              testEventHubFQDN, testEventHubName, testConsumerGroup),
          5,
          2);
    }
  }

  TEST_F(ProcessorLoadBalancerTest, AnyStrategy_StealsToBalance)
  {
    for (auto strategy :
         {Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyBalanced,
          Azure::Messaging::EventHubs::Models::ProcessorStrategy::ProcessorStrategyGreedy})
    {
      std::shared_ptr<CheckpointStore> checkpointStore{
          std::make_shared<Azure::Messaging::EventHubs::Test::TestCheckpointStore>()};

      const std::string clientA = "clientA";
      const std::string clientB = "clientB";

      checkpointStore->ClaimOwnership(
          {TestOwnership("0", clientA),
           TestOwnership("1", clientA),
           TestOwnership("2", clientA),
           TestOwnership("3", clientB)});

      {

        Azure::Messaging::EventHubs::_detail::ProcessorLoadBalancer loadBalancer(
            checkpointStore, TestConsumerDetails(clientA), strategy, std::chrono::minutes(2));

        auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3"});

        auto clientOwned = GroupByOwner(ownerships)[clientA];
        std::sort(clientOwned.begin(), clientOwned.end());
        ASSERT_EQ(clientOwned.size(), 3ul);
        EXPECT_EQ(clientOwned[0], "0");
        EXPECT_EQ(clientOwned[1], "1");
        EXPECT_EQ(clientOwned[2], "2");
      }

      {

        Azure::Messaging::EventHubs::_detail::ProcessorLoadBalancer loadBalancer(
            checkpointStore, TestConsumerDetails(clientB), strategy, std::chrono::minutes(2));

        auto ownerships = loadBalancer.LoadBalance({"0", "1", "2", "3"});

        auto clientOwned = GroupByOwner(ownerships)[clientB];
        EXPECT_EQ(clientOwned.size(), 2ul);

        RequireBalanced(
            loadBalancer.m_checkpointStore->ListOwnership(
                testEventHubFQDN, testEventHubName, testConsumerGroup),
            4,
            2);
      }
    }
  }
}}}} // namespace Azure::Messaging::EventHubs::Test
