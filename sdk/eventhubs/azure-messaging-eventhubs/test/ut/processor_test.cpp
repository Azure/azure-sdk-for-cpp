// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "eventhubs_test_base.hpp"

#include <azure/core/context.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  class ProcessorTest : public EventHubsTestBase {
  };
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
    Azure::Messaging::EventHubs::BlobCheckpointStore checkpointStore(
        GetEnv("CHECKPOINTSTORE_STORAGE_CONNECTION_STRING"), testName);

    std::string const connStringNoEntityPath
        = GetEnv("EVENTHUB_CONNECTION_STRING") + ";EntityPath=eventhub";
    Azure::Messaging::EventHubs::Models::ConsumerClientOptions options;
    options.ApplicationID = "unit-test";

    options.ReceiverOptions.Name = "unit-test";
    options.ReceiverOptions.SettleMode = Azure::Core::Amqp::_internal::ReceiverSettleMode::First;
    options.ReceiverOptions.MessageTarget = "ingress";
    options.ReceiverOptions.EnableTrace = true;
    options.ReceiverOptions.MaxMessageSize = std::numeric_limits<uint16_t>::max();

    auto client = Azure::Messaging::EventHubs::ConsumerClient(
        connStringNoEntityPath, "eventhub", "$Default", options);
    ProcessorOptions processorOptions;
    processorOptions.LoadBalancingStrategy = ProcessorStrategy::ProcessorStrategyBalanced;
    processorOptions.UpdateInterval = std::chrono::seconds(2);

    Processor processor(
        std::make_shared<ConsumerClient>(client),
        std::make_shared<BlobCheckpointStore>(checkpointStore),
        processorOptions);

    processor.Run();
    std::this_thread::sleep_for(std::chrono::seconds(10));
  }

}}}} // namespace Azure::Messaging::EventHubs::Test
