// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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

  Azure::Messaging::EventHubs::ProducerClient producerClient(
      eventhubHost, eventhubName, credential);

  auto eventhubProperties{producerClient.GetEventHubProperties()};

  std::cout << "Created event hub, properties: " << eventhubProperties << std::endl;
}
