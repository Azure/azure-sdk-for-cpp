// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <iostream>

#include <azure/identity/default_azure_credential.hpp>

#include <azure/service/client.hpp>

int main()
{
  try
  {
    // Step 1: Initialize Default Azure Credential.
    // Default Azure Credential is good for samples and initial development stages only.
    // It is not recommended used it in a production environment.
    // To diagnose, see https://aka.ms/azsdk/cpp/identity/troubleshooting

    auto defaultAzureCredential = std::make_shared<Azure::Identity::DefaultAzureCredential>();

    // Step 2: Pass the credential to an Azure Service Client.
    Azure::Service::Client azureServiceClient("serviceUrl", defaultAzureCredential);

    // Step 3: Start using the Azure Service Client.
    azureServiceClient.DoSomething(Azure::Core::Context::ApplicationContext);

    std::cout << "Success!" << std::endl;
  }
  catch (const Azure::Core::Credentials::AuthenticationException& exception)
  {
    // Step 4: Handle authentication errors, if needed
    // (invalid credential parameters, insufficient permissions).
    std::cout << "Authentication error: " << exception.what() << std::endl;
    return 1;
  }

  return 0;
}
