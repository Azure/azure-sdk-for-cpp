// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Minimal sample demonstrating how to create an Event Hubs producer using an AAD token credential
// obtained from using the Azure Identity library.

// This sample expects that the following environment variables exist:
// * EVENTHUBS_HOST - contains the fully qualified domain name for the eventhub service instance.
// * EVENTHUB_NAME - the name of the Event Hub instance.
//
// The following environment variables are required to authenticate the request using
// Azure::Identity::EnvironmentCredential:
// * AZURE_CLIENT_ID - the application client ID used to authenticate the request.
// * AZURE_TENANT_ID - the tenant ID or domain used to authenticate the request.
// * AZURE_CLIENT_SECRET - the application client secret used to authenticate the request.
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

  Azure::Messaging::EventHubs::ProducerClient producerClient(
      eventhubHost, eventhubName, credential);

  auto eventhubProperties{producerClient.GetEventHubProperties()};

  std::cout << "Created event hub, properties: " << eventhubProperties << std::endl;
}
