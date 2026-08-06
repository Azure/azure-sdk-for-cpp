// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Minimal sample showing how to create an Event Hubs producer from a connection string. It then
// creates 2 events in a single batch and sends those events to the first partition.

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

  Azure::Messaging::EventHubs::ProducerClient producerClient(connectionString, eventhubName);

  Azure::Messaging::EventHubs::Models::EventHubProperties eventhubProperties
      = producerClient.GetEventHubProperties();

  // By default, the producer sends to all available partitions in turn. Name a partition in the
  // batch options to send to one partition. The consume-events sample reads from the first
  // partition, so send to that one.
  Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
  batchOptions.PartitionId = eventhubProperties.PartitionIds[0];
  Azure::Messaging::EventHubs::EventDataBatch batch{producerClient.CreateBatch(batchOptions)};

  // Send an event with a simple binary body.
  {
    Azure::Messaging::EventHubs::Models::EventData event;
    event.Body = {1, 3, 5, 7};
    event.MessageId = "connection-string-message-id-1";
    if (!batch.TryAdd(event))
    {
      std::cerr << "Failed to add the event to the batch" << std::endl;
      return 1;
    }
  }

  // Send an event with a string body.
  {
    Azure::Messaging::EventHubs::Models::EventData event{"Hello Eventhubs via connection string!"};
    event.MessageId = "connection-string-message-id-2";
    if (!batch.TryAdd(event))
    {
      std::cerr << "Failed to add the event to the batch" << std::endl;
      return 1;
    }
  }

  producerClient.Send(batch);

  std::cout << "Sent the batch to partition " << batchOptions.PartitionId << std::endl;
}
