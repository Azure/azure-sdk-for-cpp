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

  Azure::Messaging::EventHubs::ProducerClient producerClient(
      eventhubConnectionString, eventhubName);

  Azure::Messaging::EventHubs::Models::EventHubProperties eventhubProperties
      = producerClient.GetEventHubProperties();

  // By default, the producer will round-robin amongst all available partitions. You can use the
  // same producer instance to send to a specific partition.
  // To do so, specify the partition ID in the options when creating the batch.
  //
  // The event consumer sample reads from the 0th partition ID in the eventhub properties, so
  // configure this batch processor to send to that partition.
  Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
  batchOptions.PartitionId = eventhubProperties.PartitionIds[0];
  Azure::Messaging::EventHubs::EventDataBatch batch(producerClient.CreateBatch(batchOptions));

  // Send an event with a simple binary body.
  {
    Azure::Messaging::EventHubs::Models::EventData event;
    event.Body = {1, 3, 5, 7};
    event.MessageId = "test-message-id";
    if (!batch.TryAdd(event))
    {
      std::cerr << "Failed to add the event to the batch" << std::endl;
      return 1;
    }
  }

  {
    Azure::Messaging::EventHubs::Models::EventData event;
    event.Body = {2, 4, 6, 8, 10};
    if (!batch.TryAdd(event))
    {
      std::cerr << "Failed to add the event to the batch" << std::endl;
      return 1;
    }
  }

  // Send an event with a body initialized at EventData constructor time.
  {
    Azure::Messaging::EventHubs::Models::EventData event{1, 1, 2, 3, 5, 8};
    event.MessageId = "test-message-id-fibonacci";
    if (!batch.TryAdd(event))
    {
      std::cerr << "Failed to add the event to the batch" << std::endl;
      return 1;
    }
  }

  // Send an event with a UTF-8 encoded string body.
  {
    Azure::Messaging::EventHubs::Models::EventData event{"Hello Eventhubs!"};
    event.MessageId = "test-message-id-helloworld";
    if (!batch.TryAdd(event))
    {
      std::cerr << "Failed to add the event to the batch" << std::endl;
      return 1;
    }
  }

  producerClient.Send(batch);
}
