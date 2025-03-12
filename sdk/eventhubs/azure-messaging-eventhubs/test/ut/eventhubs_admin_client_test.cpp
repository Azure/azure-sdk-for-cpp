// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words hehe

#include "eventhubs_admin_client.hpp"
#include "eventhubs_test_base.hpp"

#include <azure/core/context.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  class AdminTest : public EventHubsTestBase {
  };
  namespace {

    std::string GetRandomName(std::string const& baseName = "checkpoint")
    {
      std::string name = baseName;
      name.append(Azure::Core::Uuid::CreateUuid().ToString());
      return name;
    }
  } // namespace

  TEST_F(AdminTest, ListNamespaceTest_LIVEONLY_)
  {
    EventHubsManagement administrationClient{GetTestCredential()};
    auto response = administrationClient.ListNamespaces();
    EXPECT_TRUE(response.size() > 0);
  }

  TEST_F(AdminTest, DoesNamespaceExistTest_LIVEONLY_)
  {

    EventHubsManagement administrationClient{GetTestCredential()};
    auto response = administrationClient.DoesNamespaceExist(GetRandomName());
    EXPECT_FALSE(response);

    response = administrationClient.DoesNamespaceExist(
        Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_NAMESPACE"));
    EXPECT_TRUE(response);
  }

  TEST_F(AdminTest, CreateDeleteNamespaceTest_LIVEONLY_)
  {
    EventHubsManagement administrationClient{GetTestCredential()};
    Azure::Core::Context context{Azure::DateTime::clock::now() + std::chrono::minutes(15)};
    std::string namespaceName = GetRandomName("ehCreate");
    auto createOperation = administrationClient.CreateNamespace(namespaceName);
    createOperation.PollUntilDone(std::chrono::milliseconds(500));
    try
    {
      auto deleteOperation = administrationClient.DeleteNamespace(namespaceName, false, context);
      deleteOperation.PollUntilDone(std::chrono::milliseconds(500), context);
    }
    catch (Azure::Core::RequestFailedException& e)
    {
      GTEST_LOG_(INFO) << "CreateDeleteNamespaceTest_LIVEONLY_ failed with code: "
                       << static_cast<std::underlying_type<decltype(e.StatusCode)>::type>(
                              e.StatusCode);
      GTEST_LOG_(INFO) << "Message: " << e.Message;
      auto body = e.RawResponse->GetBody();
      auto bodyAsString = std::string(body.begin(), body.end());

      GTEST_LOG_(ERROR) << "Response: " << bodyAsString;
      GTEST_FAIL();
    }
  }

  TEST_F(AdminTest, EnumerateEventHubs_LIVEONLY_)
  {
    EventHubsManagement administrationClient{GetTestCredential()};
    std::string namespaceName = GetRandomName("eventhub");
    auto eventhubsNamespace = administrationClient.GetNamespace(
        Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_NAMESPACE"));

    auto eventhubs = eventhubsNamespace.ListEventHubs();
    EXPECT_TRUE(eventhubs.size() > 0);
  }

  TEST_F(AdminTest, CreateEventHub_LIVEONLY_)
  {
    EventHubsManagement administrationClient{GetTestCredential()};
    std::string eventHubName = GetRandomName("eventhub");
    auto eventhubsNamespace = administrationClient.GetNamespace(
        Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_NAMESPACE"));

    auto eventhubs = eventhubsNamespace.CreateEventHub(eventHubName);
    EXPECT_TRUE(eventhubs.Name() == eventHubName);

    // Now delete the eventhub we just created.
    eventhubsNamespace.DeleteEventHub(eventHubName);
  }

  TEST_F(AdminTest, CreateConsumerGroup_LIVEONLY_)
  {
    EventHubsManagement administrationClient{GetTestCredential()};
    std::string eventHubName = GetRandomName("eventhub");
    auto eventhubsNamespace = administrationClient.GetNamespace(
        Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_NAMESPACE"));

    auto eventhubs = eventhubsNamespace.CreateEventHub(eventHubName);
    EXPECT_TRUE(eventhubs.Name() == eventHubName);

    std::string cgName = GetRandomName("ConsumerGroup");

    EXPECT_TRUE(eventhubs.CreateConsumerGroup(cgName));

    EXPECT_TRUE(eventhubs.DeleteConsumerGroup(cgName));

    // Now delete the eventhub we just created.
    eventhubsNamespace.DeleteEventHub(eventHubName);
  }

}}}} // namespace Azure::Messaging::EventHubs::Test
