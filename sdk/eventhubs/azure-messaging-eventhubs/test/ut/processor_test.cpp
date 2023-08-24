// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "eventhubs_test_base.hpp"

#include <azure/core/context.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  class ProcessorTest : public EventHubsTestBase {};
  namespace {

    std::string GetRandomName()
    {
      std::string name = "checkpoint";
      name.append(Azure::Core::Uuid::CreateUuid().ToString());
      return name;
    }

  } // namespace
  TEST_F(ProcessorTest, LoadBalancing_LIVEONLY_)
  {
    std::string const testName = GetRandomName();
    auto containerClient{Azure::Storage::Blobs::BlobContainerClient::CreateFromConnectionString(
        Azure::Core::_internal::Environment::GetVariable(
            "CHECKPOINTSTORE_STORAGE_CONNECTION_STRING"),
        testName)};
    Azure::Messaging::EventHubs::BlobCheckpointStore checkpointStore(containerClient);

    std::string eventHubName{GetEnv("EVENTHUB_NAME")};
    std::string consumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");

    std::string const connStringNoEntityPath
        = GetEnv("EVENTHUB_CONNECTION_STRING") + ";EntityPath=" + eventHubName;
    Azure::Messaging::EventHubs::ConsumerClientOptions options;
    options.ApplicationID = testing::UnitTest::GetInstance()->current_test_info()->name();
    options.Name = testing::UnitTest::GetInstance()->current_test_info()->name();

    auto client = Azure::Messaging::EventHubs::ConsumerClient(
        connStringNoEntityPath, eventHubName, consumerGroup, options);
    ProcessorOptions processorOptions;
    processorOptions.LoadBalancingStrategy = Models::ProcessorStrategy::ProcessorStrategyBalanced;
    processorOptions.UpdateInterval = std::chrono::seconds(2);

    Processor processor(
        std::make_shared<ConsumerClient>(client),
        std::make_shared<BlobCheckpointStore>(checkpointStore),
        processorOptions);

    processor.Run();
    std::this_thread::sleep_for(std::chrono::seconds(10));

    processor.Close();
  }

}}}} // namespace Azure::Messaging::EventHubs::Test
