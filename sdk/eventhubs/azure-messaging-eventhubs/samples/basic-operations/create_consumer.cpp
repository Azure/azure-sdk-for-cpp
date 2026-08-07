// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Creates an Event Hubs consumer from a connection string and gets Event Hub properties.
//
// Required environment variable:
// * EVENTHUB_CONNECTION_STRING - an Event Hubs namespace or Event Hub connection string.
//
// Optional environment variable:
// * EVENTHUB_NAME - required when the connection string does not contain EntityPath.

#include <azure/messaging/eventhubs.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

int main()
{
  char const* const connectionStringValue{std::getenv("EVENTHUB_CONNECTION_STRING")};
  if (connectionStringValue == nullptr)
  {
    std::cerr << "Missing environment variable EVENTHUB_CONNECTION_STRING" << std::endl;
    return 1;
  }

  char const* const eventHubNameValue{std::getenv("EVENTHUB_NAME")};
  std::string const eventHubName{eventHubNameValue == nullptr ? "" : eventHubNameValue};

  Azure::Messaging::EventHubs::ConsumerClient consumerClient(connectionStringValue, eventHubName);

  auto eventHubProperties = consumerClient.GetEventHubProperties();
  std::cout << "Event Hub properties: " << eventHubProperties << std::endl;
}
