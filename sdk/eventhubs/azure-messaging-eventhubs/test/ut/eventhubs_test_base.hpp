// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "gtest/gtest.h"

#include <azure/core/test/test_base.hpp>
#include <azure/messaging/eventhubs/consumer_client.hpp>
#include <azure/messaging/eventhubs/producer_client.hpp>

#include <memory>

enum class AuthType
{
  Key,
  //  ConnectionString,
  Emulator,
};

class EventHubsTestBaseParameterized : public Azure::Core::Test::TestBase,
                                       public ::testing::WithParamInterface<AuthType> {
public:
  EventHubsTestBaseParameterized() { TestBase::SetUpTestSuiteLocal(AZURE_TEST_ASSETS_DIR); }
  // Create
  virtual void SetUp() override
  {
    Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
  }
  virtual void TearDown() override
  {
    // Make sure you call the base classes TearDown method to ensure recordings are made.
    TestBase::TearDown();
  }

protected:
  std::unique_ptr<Azure::Messaging::EventHubs::ConsumerClient> CreateConsumerClient(
      std::string eventHubName = {},
      Azure::Messaging::EventHubs::ConsumerClientOptions options = {})
  {
    if (eventHubName.empty())
    {
      eventHubName = GetEnv("EVENTHUB_NAME");
    }
    std::string eventConsumerGroup = GetEnv("EVENTHUB_CONSUMER_GROUP");
    switch (GetParam())
    {
      case AuthType::Key: {
        std::string eventHubNamespace = GetEnv("EVENTHUBS_HOST");
        return std::make_unique<Azure::Messaging::EventHubs::ConsumerClient>(
            eventHubNamespace, eventHubName, GetTestCredential(), eventConsumerGroup, options);
      }
        // case AuthType::Emulator: {
        //   return std::make_unique<Azure::Messaging::EventHubs::ConsumerClient>(
        //       "Endpoint=sb://localhost:5672/"
        //       ";SharedAccessKeyName=RootManageSharedAccessKey;SharedAccessKey=abcdefabcdef;"
        //       "UseDevelopmentEmulator=true",
        //       "eh1",
        //       "$default",
        //       options);
        // }
    }
    return nullptr;
  }

  std::unique_ptr<Azure::Messaging::EventHubs::ProducerClient> CreateProducerClient(
      std::string eventHubName = {},
      Azure::Messaging::EventHubs::ProducerClientOptions const& options = {})
  {
    if (eventHubName.empty())
    {
      eventHubName = GetEnv("EVENTHUB_NAME");
    }

    std::unique_ptr<Azure::Messaging::EventHubs::ProducerClient> producer;
    switch (GetParam())
    {
      case AuthType::Key: {
        std::string eventHubNamespace = GetEnv("EVENTHUBS_HOST");
        producer = std::make_unique<Azure::Messaging::EventHubs::ProducerClient>(
            eventHubNamespace, eventHubName, GetTestCredential(), options);
        break;
      }
        // case AuthType::Emulator: {
        //   producer = std::make_unique<Azure::Messaging::EventHubs::ProducerClient>(
        //       "Endpoint=sb://localhost:5672/"
        //       ";SharedAccessKeyName=RootManageSharedAccessKey;SharedAccessKey=abcdefabcdef;"
        //       "UseDevelopmentEmulator=true",
        //       "eh1",
        //       options);
        // }
    }
    return producer;
  }
};

class EventHubsTestBase : public Azure::Core::Test::TestBase {
public:
  EventHubsTestBase() { TestBase::SetUpTestSuiteLocal(AZURE_TEST_ASSETS_DIR); }
  // Create
  virtual void SetUp() override
  {
    Azure::Core::Test::TestBase::SetUpTestBase(AZURE_TEST_RECORDING_DIR);
  }
  virtual void TearDown() override
  {
    // Make sure you call the base classes TearDown method to ensure recordings are made.
    TestBase::TearDown();
  }
};
