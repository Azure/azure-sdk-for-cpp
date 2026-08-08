// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Sends two events to the first partition using connection-string authentication.
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

  Azure::Messaging::EventHubs::ProducerClient producerClient(connectionStringValue, eventHubName);
  auto eventHubProperties = producerClient.GetEventHubProperties();

  Azure::Messaging::EventHubs::EventDataBatchOptions batchOptions;
  batchOptions.PartitionId = eventHubProperties.PartitionIds[0];
  Azure::Messaging::EventHubs::EventDataBatch batch{producerClient.CreateBatch(batchOptions)};

  Azure::Messaging::EventHubs::Models::EventData firstEvent{"First connection-string event"};
  firstEvent.MessageId = "connection-string-message-1";
  if (!batch.TryAdd(firstEvent))
  {
    std::cerr << "Failed to add the first event to the batch" << std::endl;
    return 1;
  }

  Azure::Messaging::EventHubs::Models::EventData secondEvent{"Second connection-string event"};
  secondEvent.MessageId = "connection-string-message-2";
  if (!batch.TryAdd(secondEvent))
  {
    std::cerr << "Failed to add the second event to the batch" << std::endl;
    return 1;
  }

  producerClient.Send(batch);
  std::cout << "Sent two events to partition " << batchOptions.PartitionId << std::endl;
}
