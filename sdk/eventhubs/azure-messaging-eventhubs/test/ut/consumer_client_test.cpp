// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "gtest/gtest.h"

#include <azure/core/context.hpp>
#include <azure/core/internal/environment.hpp>
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

TEST(ConsumerClientTest, ConnectionStringNoEntityPath)
{
  std::string const connStringNoEntityPath
      = Azure::Core::_internal::Environment::GetVariable("EVENTHUB_CONNECTION_STRING");

  auto client = Azure::Messaging::EventHubs::ConsumerClient(connStringNoEntityPath, "eventhub", "$Default");
  EXPECT_EQ("eventhub", client.GetEventHubName());
}