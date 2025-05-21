// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Minimal sample demonstrating how to create an Event Hubs producer using an AAD token credential
// obtained from using the Azure Identity library.

// This sample expects that the following environment variables exist:
// * EVENTHUBS_HOST - contains the fully qualified domain name for the eventhub service instance.
// * EVENTHUB_NAME - the name of the Event Hub instance.
//
// Both of these should be available from the Azure portal.
//
#include <azure/identity.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <iostream>

int main()
{
  char const* const eventhubsHost{std::getenv("EVENTHUBS_HOST")};
  char const* const eventhubName{std::getenv("EVENTHUB_NAME")};
  if (eventhubsHost == nullptr)
  {
    std::cerr << "Missing environment variable EVENTHUBS_HOST" << std::endl;
    return 1;
  }
  if (eventhubName == nullptr)
  {
    std::cerr << "Missing environment variable EVENTHUB_NAME" << std::endl;
    return 1;
  }

  auto credential = std::make_shared<Azure::Identity::DefaultAzureCredential>();

  Azure::Messaging::EventHubs::ProducerClient producerClient(
      eventhubsHost, eventhubName, credential);

  auto eventhubProperties{producerClient.GetEventHubProperties()};

  std::cout << "Created event hub, properties: " << eventhubProperties << std::endl;
}
