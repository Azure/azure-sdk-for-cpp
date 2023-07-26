// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// This sample demonstrates how to create an Event Hubs consumer using an AAD token credential
// obtained from using the Azure Identity library.

#include <azure/identity/client_secret_credential.hpp>
#include <azure/identity/environment_credential.hpp>
#include <azure/messaging/eventhubs.hpp>

#include <chrono>
#include <iostream>
#include <limits>
#include <string>

int main()
{
  std::string eventhubHost{std::getenv("EVENTHUB_HOST")};
  std::string eventhubName{std::getenv("EVENTHUB_NAME")};

  std::shared_ptr<Azure::Core::Credentials::TokenCredential> credential
      = std::make_shared<Azure::Identity::EnvironmentCredential>();

  Azure::Messaging::EventHubs::ConsumerClient consumerClient(
      eventhubHost, eventhubName, credential);

  auto eventhubProperties{consumerClient.GetEventHubProperties()};

  std::cout << "Created event hub, properties: " << eventhubProperties << std::endl;
}
