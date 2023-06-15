// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include <azure/core/context.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  namespace {

    std::string GetRandomName()
    {
      std::string name = "checkpoint";
      name.append(Azure::Core::Uuid::CreateUuid().ToString());
      return name;
    }

  } // namespace
  TEST(ProcessorTest, LoadBalancing)
  {
    std::string const testName = GetRandomName();
    Azure::Messaging::EventHubs::BlobCheckpointStore checkpointStore(
        Azure::Core::_internal::Environment::GetVariable(
            "CHECKPOINTSTORE_STORAGE_CONNECTION_STRING"),
        testName);

    std::string const connStringNoEntityPath
        = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING")
        + ";EntityPath=eventhub";
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
    processorOptions.UpdateInterval = std::chrono::hours(1);

    Processor processor(
        std::make_shared<ConsumerClient>(client),
        std::make_shared<BlobCheckpointStore>(checkpointStore),
        processorOptions);

    processor.Run();
  }

}}}} // namespace Azure::Messaging::EventHubs::Test
