// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Minimal sample showing how to create an Event Hubs consumer from a connection string. It then
// consumes events from the first partition of the Event Hub instance.

// This sample expects that the following environment variables exist:
// * EVENTHUB_CONNECTION_STRING - the connection string to an Event Hubs namespace.
// * EVENTHUB_NAME - the name of the Event Hub instance.
//
// Both of these are available from the Azure portal.
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

  auto eventhubProperties{consumerClient.GetEventHubProperties()};
  std::cout << "Event hub properties: " << eventhubProperties << std::endl;

  // Create a PartitionClient to read events from one partition, in this case the first one.
  //
  // This partition client reads events from the start of the partition. The default is to read
  // new events only.
  Azure::Messaging::EventHubs::PartitionClientOptions partitionClientOptions;
  partitionClientOptions.StartPosition.Earliest = true;
  partitionClientOptions.StartPosition.Inclusive = true;

  Azure::Messaging::EventHubs::PartitionClient partitionClient{consumerClient.CreatePartitionClient(
      eventhubProperties.PartitionIds[0], partitionClientOptions)};

  std::vector<std::shared_ptr<const Azure::Messaging::EventHubs::Models::ReceivedEventData>> events
      = partitionClient.ReceiveEvents(2);

  for (const auto& event : events)
  {
    std::cout << "Event: " << *event << std::endl;
  }
}
