// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <azure/messaging/eventhubs.hpp>

#include <chrono>
#include <iostream>
#include <limits>
#include <string>

int main()
{
  std::string eventhubConnectionString{std::getenv("EVENTHUB_CONNECTION_STRING")};
  std::string eventhubName{std::getenv("EVENTHUB_NAME")};

  Azure::Messaging::EventHubs::ConsumerClient consumerClient(
      eventhubConnectionString, eventhubName);

  auto eventhubProperties{consumerClient.GetEventHubProperties()};

  std::cout << "Created event hub, properties: " << eventhubProperties << std::endl;
}
