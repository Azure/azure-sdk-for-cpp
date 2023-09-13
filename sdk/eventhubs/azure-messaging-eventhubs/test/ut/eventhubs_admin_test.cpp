// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// cspell: words hehe

#include "eventhubs_admin.hpp"
#include "eventhubs_test_base.hpp"

#include <azure/core/context.hpp>
#include <azure/core/test/test_base.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <gtest/gtest.h>

namespace Azure { namespace Messaging { namespace EventHubs { namespace Test {
  class AdminTest : public EventHubsTestBase {};
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
    EventHubsManagement administrationClient;
    auto response = administrationClient.ListNamespaces();
    EXPECT_TRUE(response.size() > 0);
  }

  TEST_F(AdminTest, DoesNamespaceExistTest_LIVEONLY_)
  {
    EventHubsManagement administrationClient;
    auto response = administrationClient.DoesNamespaceExist(GetRandomName());
    EXPECT_FALSE(response);
  }

  TEST_F(AdminTest, CreateDeleteNamespaceTest_LIVEONLY_)
  {
    EventHubsManagement administrationClient;
    std::string namespaceName = GetRandomName("eventhub");
    administrationClient.CreateNamespace(namespaceName);
    administrationClient.DeleteNamespace(namespaceName);
  }

  TEST_F(AdminTest, EnumerateEventHubs_LIVEONLY_)
  {
    EventHubsManagement administrationClient;
    std::string namespaceName = GetRandomName("eventhub");
    auto eventhubsNamespace = administrationClient.GetNamespace(
        "eh-t7d71a09db13a76aa" /* Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_NAMESPACE")*/);

    auto eventhubs = eventhubsNamespace.ListEventHubs();
    EXPECT_TRUE(eventhubs.size() > 0);
  }

  TEST_F(AdminTest, CreateEventHub_LIVEONLY_)
  {
    EventHubsManagement administrationClient;
    std::string eventHubName = GetRandomName("eventhub");
    auto eventhubsNamespace = administrationClient.GetNamespace(
        Azure::Core::_internal::Environment::GetVariable("EVENTHUBS_NAMESPACE"));

    auto eventhubs = eventhubsNamespace.CreateEventHub(eventHubName);
    EXPECT_TRUE(eventhubs.Name() == eventHubName);

    // Now delete the eventhub we just created.
    eventhubsNamespace.DeleteEventHub(eventHubName);
  }

}}}} // namespace Azure::Messaging::EventHubs::Test
