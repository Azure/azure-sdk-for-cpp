// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Minimal sample showing how to create an Event Hubs event consumer using AAD credentials and then
// consume events from an EventHub partition.

// This sample expects that the following environment variables exist:
// * EVENTHUBS_HOST - contains the host name of to a specific Event Hubs instance.
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

  Azure::Messaging::EventHubs::ConsumerClient consumerClient(
      eventhubsHost, eventhubName, credential);

  // Retrieve properties about the EventHubs instance just created.
  auto eventhubProperties{consumerClient.GetEventHubProperties()};
  std::cout << "Created event hub, properties: " << eventhubProperties << std::endl;

  // Retrieve properties about the EventHubs instance just created.
  auto partitionProperties{
      consumerClient.GetPartitionProperties(eventhubProperties.PartitionIds[0])};

  // Create a PartitionClient that we can use to read events from a specific partition (in this
  // case, the first partition).
  //
  // This partition client is configured to read events from the start of the partition, since the
  // default is to read new events only.
  Azure::Messaging::EventHubs::PartitionClientOptions partitionClientOptions;
  partitionClientOptions.StartPosition.Earliest = true;
  partitionClientOptions.StartPosition.Inclusive = true;

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
