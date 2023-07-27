// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Minimal sample showing how to create an Event Hubs producer using a connection string retrieved
// from the Azure portal.

// This sample expects that the following environment variables exist:
// * EVENTHUB_CONNECTION_STRING - contains the connection string to a specific Event Hub instance.
// * EVENTHUB_NAME - the name of the Event Hub instance.
//
// Both of these should be available from the Azure portal.
//

#include <azure/messaging/eventhubs.hpp>

#include <iostream>

int main()
{
  std::string eventhubConnectionString{std::getenv("EVENTHUB_CONNECTION_STRING")};
  std::string eventhubName{std::getenv("EVENTHUB_NAME")};

  Azure::Messaging::EventHubs::ProducerClient producerClient(
      eventhubConnectionString, eventhubName);

  // Send an event with a simple binary body.
  Azure::Messaging::EventHubs::Models::EventData event;
  event.Body.Data = {1, 3, 5, 7};
  event.MessageId = "test-message-id";
  Azure::Messaging::EventHubs::EventDataBatch batch;
  batch.AddMessage(event);

  if (!producerClient.SendEventDataBatch(batch))
  {
    std::cerr << "Failed to send message to the Event Hub instance." << std::endl;
  }
  else
  {
    std::cout << "Sent message to the Event Hub instance." << std::endl;
  }
}
