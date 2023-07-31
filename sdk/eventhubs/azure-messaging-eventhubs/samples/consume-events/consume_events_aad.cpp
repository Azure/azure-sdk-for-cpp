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

#include <azure/identity/environment_credential.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <iostream>

int main()
{
  std::string eventhubHost{std::getenv("EVENTHUBS_HOST")};
  std::string eventhubName{std::getenv("EVENTHUB_NAME")};

  std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential
      = std::make_shared<Azure::Identity::EnvironmentCredential>();

  Azure::Messaging::EventHubs::ConsumerClient consumerClient(
      eventhubHost, eventhubName, credential);

  // Retrieve properties about the EventHubs instance just created.
  auto eventhubProperties{consumerClient.GetEventHubProperties()};
  std::cout << "Created event hub, properties: " << eventhubProperties << std::endl;

  // Retrieve properties about the EventHubs instance just created.
  auto partitionProperties{
      consumerClient.GetPartitionProperties(eventhubProperties.PartitionIds[0])};

  Azure::Messaging::EventHubs::PartitionClient partitionClient{
      consumerClient.CreatePartitionClient(eventhubProperties.PartitionIds[0])};

  std::vector<Azure::Messaging::EventHubs::Models::ReceivedEventData> events
      = partitionClient.ReceiveEvents(4);

  // Dump the contents of each event received.
  for (const auto& event : events)
  {
    std::cout << "Event: " << event << std::endl;
  }
}
