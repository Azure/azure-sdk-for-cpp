// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Minimal sample showing how to create an Event Hubs consumer using a connection string retrieved
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
  char* const eventhubConnectionString{std::getenv("EVENTHUB_CONNECTION_STRING")};
  char* const eventhubName{std::getenv("EVENTHUB_NAME")};
  if (eventhubConnectionString == nullptr)
  {
    std::cerr << "Missing environment variable EVENTHUB_CONNECTION_STRING" << std::endl;
    return 1;
  }
  if (eventhubName == nullptr)
  {
    std::cerr << "Missing environment variable EVENTHUB_NAME" << std::endl;
    return 1;
  }

  /* Create a sample EventHubs application using a PartitionClient to read all the messages from an
   * EventHubs instance. */
  Azure::Messaging::EventHubs::ConsumerClient consumerClient(
      eventhubConnectionString, eventhubName);

  // Retrieve properties about the EventHubs instance just created.
  auto eventhubProperties{consumerClient.GetEventHubProperties()};
  std::cout << "Created event hub, properties: " << eventhubProperties << std::endl;

  // Retrieve properties about the EventHubs instance just created.
  auto partitionProperties{
      consumerClient.GetPartitionProperties(eventhubProperties.PartitionIds[0])};

  // Create a PartitionClient that we can use to read events from a specific partition.
  //
  // This partition client is configured to read events from the start of the partition, since the
  // default is to read new events only.
  Azure::Messaging::EventHubs::PartitionClientOptions partitionClientOptions;
  partitionClientOptions.StartPosition.Earliest = true;
  partitionClientOptions.StartPosition.Inclusive = true;

  std::cout << "Creating partition client. Start position: "
            << partitionClientOptions.StartPosition;

  std::cout << "earliest: HasValue: " << std::boolalpha
            << partitionClientOptions.StartPosition.Earliest.HasValue();
  if (partitionClientOptions.StartPosition.Earliest.HasValue())
  {
    std::cout << "earliest: Value: " << std::boolalpha
              << partitionClientOptions.StartPosition.Earliest.Value() << std::endl;
  }

  Azure::Messaging::EventHubs::PartitionClient partitionClient{consumerClient.CreatePartitionClient(
      eventhubProperties.PartitionIds[0], partitionClientOptions)};

  std::vector<std::shared_ptr<const Azure::Messaging::EventHubs::Models::ReceivedEventData>> events
      = partitionClient.ReceiveEvents(4);

  // Dump the contents of each event received.
  for (const auto& event : events)
  {
    std::cout << "Event: " << *event << std::endl;
  }
}
