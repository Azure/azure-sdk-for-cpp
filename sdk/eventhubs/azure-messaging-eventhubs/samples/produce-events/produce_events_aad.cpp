// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Minimal sample showing how to create an Event Hubs producer using a connection string retrieved
// from the Azure portal.

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
  std::string eventhubHost{std::getenv("EVENTHUBS_HOST")};
  std::string eventhubName{std::getenv("EVENTHUB_NAME")};

  std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential
      = std::make_shared<Azure::Identity::EnvironmentCredential>();

  Azure::Messaging::EventHubs::ProducerClient producerClient(
      eventhubHost, eventhubName, credential);

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
  Azure::Messaging::EventHubs::EventDataBatch batch(batchOptions);

  // Send an event with a simple binary body.
  {
    Azure::Messaging::EventHubs::Models::EventData event;
    event.Body = {1, 3, 5, 7};
    event.MessageId = "test-message-id";
    batch.AddMessage(event);
  }
  {
    Azure::Messaging::EventHubs::Models::EventData event;
    event.Body = {2, 4, 6, 8, 10};
    event.MessageId = "test-message-id-2";
    batch.AddMessage(event);
  }
  {
    Azure::Messaging::EventHubs::Models::EventData event{1, 1, 2, 3, 5, 8};
    event.MessageId = "test-message-id5";
    batch.AddMessage(event);
  }
  {
    Azure::Messaging::EventHubs::Models::EventData event{"Hello Eventhubs via AAD!"};
    event.MessageId = "test-message-id4";
    batch.AddMessage(event);
  }

  if (!producerClient.SendEventDataBatch(batch))
  {
    std::cerr << "Failed to send message to the Event Hub instance." << std::endl;
  }
  else
  {
    std::cout << "Sent message to the Event Hub instance." << std::endl;
  }
}
