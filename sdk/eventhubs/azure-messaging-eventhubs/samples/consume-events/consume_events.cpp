// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Receives events from the first partition using connection-string authentication.
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

  Azure::Messaging::EventHubs::ConsumerClient consumerClient(connectionStringValue, eventHubName);
  auto eventHubProperties = consumerClient.GetEventHubProperties();

  Azure::Messaging::EventHubs::PartitionClientOptions partitionOptions;
  partitionOptions.StartPosition.Earliest = true;
  partitionOptions.StartPosition.Inclusive = true;

  auto partitionClient
      = consumerClient.CreatePartitionClient(eventHubProperties.PartitionIds[0], partitionOptions);
  auto events = partitionClient.ReceiveEvents(2);

  for (auto const& event : events)
  {
    std::cout << "Received event: " << *event << std::endl;
  }
}
