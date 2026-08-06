// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Minimal sample showing how to create an Event Hubs consumer from a connection string. It then
// gets the properties of the Event Hub instance.

// This sample expects that the following environment variables exist:
// * EVENTHUB_CONNECTION_STRING - the connection string to an Event Hubs namespace.
// * EVENTHUB_NAME - the name of the Event Hub instance.
//
// Both of these are available from the Azure portal.
//
// When the connection string carries an EntityPath key, the EVENTHUB_NAME value must be empty or
// must match that entity path. A different value throws std::invalid_argument.
//

#include <azure/messaging/eventhubs.hpp>

#include <iostream>

int main()
{
  char const* const connectionString{std::getenv("EVENTHUB_CONNECTION_STRING")};
  char const* const eventhubName{std::getenv("EVENTHUB_NAME")};
  if (connectionString == nullptr)
  {
    std::cerr << "Missing environment variable EVENTHUB_CONNECTION_STRING" << std::endl;
    return 1;
  }
  if (eventhubName == nullptr)
  {
    std::cerr << "Missing environment variable EVENTHUB_NAME" << std::endl;
    return 1;
  }

  Azure::Messaging::EventHubs::ConsumerClient consumerClient(connectionString, eventhubName);

  Azure::Messaging::EventHubs::Models::EventHubProperties eventhubProperties
      = consumerClient.GetEventHubProperties();

  std::cout << "Created consumer for event hub, properties: " << eventhubProperties << std::endl;
}
